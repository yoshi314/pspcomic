/** \file
    GUI-related functions

    Copyright (C) 2007 - 2008 Jeffrey P.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**/

#include "pspcomic.h"

///The currently loaded font
static TTF_Font *font = NULL;
///The name of the currently loaded theme
static char *theme_loaded = NULL;
///The currently loaded background image
static SDL_Surface *bg = NULL;
///The currently loaded theme
static theme_info *loaded_theme = NULL;
char selection_on = 0;
///The current X position of the text cursor
static Uint16 textX = 0;
///The current Y position of the text cursor
static Uint16 textY = 0;
///The number of lines of text that fit on the screen for the current font
static Uint16 lines_per_screen = 0; 
///The explicitly set color that overrides other text rendering
static Uint32 explicit_color;
///Whether or not an explicit color is set
static char is_explicit_color = 0;

/**
 * Load a font to the current font
 *
 * \param font_file The file to load, relative to themes_dir
 * \return 1 on success, 0 on failure
 *
 * \warning This function does not unload the old font before loading a new one
 */
static char load_font(const char *font_file) {
	if(strcmp(font_file,"builtin") == 0) {
		SDL_RWops *font_rwops = SDL_RWFromConstMem(default_font,size_default_font);
		if(!font_rwops) return 0;
		font = TTF_OpenFontRW(font_rwops,1,loaded_theme->font_attributes.font_height);
	} else {
		char *buffer = malloc(sizeof(char)*(strlen(root_dir)+strlen(themes_dir)+strlen(font_file)+2));
		if(!buffer) return 0;
		sprintf(buffer,"%s%s%s",root_dir,themes_dir,font_file);
		font = TTF_OpenFont(buffer,loaded_theme->font_attributes.font_height);
		free(buffer);
	}
	return 1;
}

#if defined(PSP) && !defined(DOXYGEN)
//Must return 0!
int resume_cb(int unknown, int flags, void *arg) {
	if(!theme_loaded) return 0;
	if(flags & MS_CB_EVENT_INSERTED) {
		if(strcmp(loaded_theme->font_file,"builtin") == 0) return 0;
		if(font) {
			TTF_CloseFont(font);
			font = NULL;
		}
		load_font(loaded_theme->font_file);
	} else {
		TTF_CloseFont(font);
		font = NULL;
		load_font("builtin");
		error_message(get_message(mesg_insert_ms));
		while(!MScmIsMediumInserted()) sceKernelDelayThread(3000000);
		SDL_Event event;
		event.type = SDL_USEREVENT;
		event.user.code = redraw_ev;
		SDL_PushEvent(&event);
	}
	return 0;
}
#endif

char init_theme_system() {
	TTF_Init();
	#ifdef PSP
	SceUID callback = sceKernelCreateCallback("PSPComic frcb",
		resume_cb, NULL);
	if(callback < 0) return 0;
	int ret = MScmRegisterMSInsertEjectCallback(callback);
	return ret >= 0;
	#else
	return 1;
	#endif
}

char load_theme(const char *theme) {
	if(strchr(theme,'/')) return 0;
	if(theme_loaded) unload_theme();

	device scr_acc = access_device(access_screen);
	SDL_Surface *screen = scr_acc.screen;
	char *buffer;

	loaded_theme = calloc(1,sizeof(theme_info));
	if(!loaded_theme) return 0;
	if(get_theme_info(theme,loaded_theme) != 0) {
		free(loaded_theme);
		loaded_theme = NULL;
		return 0;
	}
	if(ranges_compatible(*loaded_theme->ranges,get_loaded_language_ranges()) == 0) {
		free_theme_info(loaded_theme);
		free(loaded_theme);
		loaded_theme = NULL;
		return -1;
	}

	if(loaded_theme->bg_file) {
		buffer = malloc(sizeof(char)*(strlen(root_dir)+strlen(themes_dir)+strlen(loaded_theme->bg_file)+2));
		if(!buffer) {
			free_theme_info(loaded_theme);
			free(loaded_theme);
			loaded_theme = NULL;
			return 0;
		}
		sprintf(buffer,"%s%s%s",root_dir,themes_dir,loaded_theme->bg_file);
		SDL_Surface *surf_buf = IMG_Load(buffer);
		if(surf_buf) {
			bg = rotozoom(surf_buf,NULL,screen->w,screen->h,0);
			if(bg != surf_buf) SDL_FreeSurface(surf_buf);
		}
		free(buffer);
	}

	if(load_font(loaded_theme->font_file) == 0) {		
		free_theme_info(loaded_theme);
		free(loaded_theme);
		loaded_theme = NULL;
		if(bg) {
			SDL_FreeSurface(bg);
			bg = NULL;
		}
	}

	if(!font) {
		error_message(get_message(mesg_sdl_ttf_error),TTF_GetError());
		fflush(stderr);
		if(bg) {
			SDL_FreeSurface(bg);
			bg = NULL;
		}
		free(loaded_theme);
		loaded_theme = NULL;
		return 0;
	}
	if(!loaded_theme->font_attributes.line_spacing) loaded_theme->font_attributes.line_spacing = TTF_FontLineSkip(font);
	lines_per_screen = scr_acc.screen->h/loaded_theme->font_attributes.line_spacing;
	theme_loaded = make_string(theme);

	return 1;
}
void unload_theme() {
	if(loaded_theme) {
		free_theme_info(loaded_theme);
		free(loaded_theme);
		loaded_theme = NULL;
	}
	if(theme_loaded) {
		free(theme_loaded);
		theme_loaded = NULL;
	}
	if(bg) {
		free(bg);
		bg = NULL;
	}
	if(font) {
		TTF_CloseFont(font);
		font = NULL;
	}
}

#ifndef DOXYGEN
//this causes warnings when more than 2 parameters are given
__attribute__((format(printf,1,2)))
#endif
void error_message(const char *message, ...) {
	va_list args;
	char buffer[1024];
	va_start(args,message);

	if(theme_loaded) {
		int result = vsnprintf(buffer,1024,message,args);
		if(result == 1024) {
			buffer[1022] = buffer[1021] = buffer[1020] = '.';
			buffer[1023] = 0;
		} else if(result == -1) return;
		clear_screen();
		show_image(get_background(),0,0,0);
		reset_text_pos();
		set_explicit_color(get_color(color_error));
		say_centered(buffer);
		unset_explicit_color();
		flip_screen();
		SDL_Delay(2500);
	} else {
		FILE *errf = fopen("error.txt","a");
		vfprintf(errf,message,args);
		fprintf(errf,"\n");
		fclose(errf);
	}
	va_end(args);
}
SDL_Surface* get_background() {
	if(bg) return bg;
	return NULL;
}

/**
 * Generate the image for some specified text
 * 
 * \param string The text to use
 * \param flags An ORed combination of ::generate_text_flag enumerators
 * \param color The color, in 0xRRGGBB, for the text to be generated
 * \return The image, or NULL on error
 */
static SDL_Surface* generate_text(const char *string, int flags, unsigned color) {
	if(!font) return NULL;
	SDL_Color col;
	col.r = (color&0xFF0000)>>16;
	col.g = (color&0xFF00)>>8;
	col.b = color&0xFF;
	return (flags & text_antialias ?
		TTF_RenderUTF8_Blended(font,string,col) :
		TTF_RenderUTF8_Solid(font,string,col));
}
char get_text_dimensions(const char *string, int *w, int *h) {
	if(!font) return -1;
	return TTF_SizeUTF8(font,string,w,h);
}

#ifndef DOXYGEN
__attribute__((format(printf,1,2)))
#endif
void say(const char *message, ...) {
	if(!loaded_theme) return;
	va_list args;
	char buffer[4096];
	va_start(args,message);
	int result = vsnprintf(buffer,4096,message,args) == 4096;
	if(result == 4096) {
		buffer[4094] = buffer[4093] = buffer[4092] = '.';
		buffer[4095] = 0;
	} else if(result == -1) return;
	va_end(args);
	char *token = strtok(buffer,"\n");
	while(token != NULL) {
		int text_width;
		TTF_SizeUTF8(font,token,&text_width,NULL);
		place_text(token,textX,textY,
			is_explicit_color?explicit_color:(selection_on?get_color(color_selected):get_color(color_normal)));
		token = strtok(NULL,"\n");
		if(token) {
			textY += loaded_theme->font_attributes.line_spacing;
			textX = 0;
		} else {
			textX += text_width;
		}
	}
	
}

void next_line() {
	if(!loaded_theme) return;
	textX = 0;
	textY += loaded_theme->font_attributes.line_spacing;
}
void reset_text_pos() {
	textX = textY = 0;
}
Uint16 getX() { return textX; }
Uint16 getY() { return textY; }
void setXY(Uint16 x, Uint16 y) {
	device scr_acc = access_device(access_screen);
	textX = (scr_acc.screen->w > x?x:scr_acc.screen->w);
	textY = (scr_acc.screen->h > y?y:scr_acc.screen->h);
}
char get_font_attr(font_attr* attrs) {
	if(loaded_theme && attrs) {
		*attrs = loaded_theme->font_attributes;
		return 1;
	}
	return 0;
}
char_ranges get_font_ranges() {
	if(loaded_theme && loaded_theme->ranges) {
		return *loaded_theme->ranges;
	}
	return default_ranges;
}

Uint16 options_per_page() {
	return lines_per_screen - 2;
}

char place_text(const char *string, unsigned short x, unsigned short y, unsigned color) {
	if(!loaded_theme) return 0;
	device scr_acc = access_device(access_screen);
	SDL_Surface *text = generate_text(string,
		(loaded_theme->font_attributes.font_aa?text_antialias:0),color);
	if(!text) return 0;
	SDL_Rect screen_place = { x, y, text->w, text->h };
	SDL_BlitSurface(text,NULL,scr_acc.screen,&screen_place);
	SDL_FreeSurface(text);
	return 1;
}

/**
 * Wrap the current text (i.e. replace spaces with newlines)
 *
 * The wrapping is based on the current width of the screen and the current
 * position of the text cursor (for the first line only).
 *
 * \param message The text to wrap. This is edited in place.
 * \return The number of lines of text in the end. 0 on error.
 */
static short wrap_text(char *message) {
	if(!loaded_theme) return 0;
	device scr_acc = access_device(access_screen);
	char *current_token = message, *next_token = strchr(current_token,'\n');
	unsigned token_length;
	short lines = 0;
	Uint16 localX = textX;
	do {
		if(next_token) {
			++next_token;
			token_length = (unsigned)next_token - (unsigned)current_token;
		}
		else token_length = strlen(current_token);
		if(token_length == 0) break;
		char still_adding_newline = 1;
		unsigned check_length = (token_length>481?481:token_length),
			increment = check_length/2;
		int text_width;
		char to_check_against[482];
		char *last_space = NULL;
		while(still_adding_newline) {
			strncpy(to_check_against,current_token,check_length);
			to_check_against[check_length] = 0;
			TTF_SizeUTF8(font,to_check_against,&text_width,NULL);
			if(text_width > scr_acc.screen->w-localX) check_length -= increment;
			else {
				if(check_length >= token_length) break;
				if(to_check_against[check_length-1] == ' ')
					last_space = current_token + check_length-1;
				check_length += increment;
			}
			increment >>= 1;
			if(!increment) {
				if(last_space) *last_space = '\n';
				else {
					int check_position;
					for(check_position = check_length-1; check_position >= 0; --check_position) {
						if(to_check_against[check_position] == ' ') {
							current_token[check_position] = '\n';
							break;
						}
					}
				}
				still_adding_newline = 0;
			}
		}
		current_token = strchr(current_token,'\n');
		if(current_token) {
			++current_token;
			next_token = strchr(current_token,'\n');
		} else next_token = NULL;
		localX = 0;
		++lines;
	} while(current_token);
	return lines;
}

void say_centered(const char *message) {
	if(!loaded_theme) return;
	reset_text_pos();
	device scr_acc = access_device(access_screen);
	char *buffer = make_string(message);
	if(!buffer) return;
	short lines = wrap_text(buffer);
	char *token = strtok(buffer,"\n");
	unsigned short currentY = (scr_acc.screen->h - (loaded_theme->font_attributes.line_spacing*lines))/2;
	while(token != NULL) {
		int text_width;
		unsigned short placeX;
		TTF_SizeUTF8(font,token,&text_width,NULL);
		if(text_width > scr_acc.screen->w) placeX = 0;
		else placeX = (scr_acc.screen->w - text_width)/2;
		place_text(token,placeX,currentY,is_explicit_color?explicit_color:(selection_on?get_color(color_selected):get_color(color_normal)));
		currentY += loaded_theme->font_attributes.line_spacing;
		token = strtok(NULL,"\n");
	}
	free(buffer);
}

void status_message(const char *message) {
	if(!loaded_theme) return;
	device scr_acc = access_device(access_screen);
	SDL_Surface *message_rendered = generate_text(message,text_antialias,loaded_theme->font_attributes.font_color);
	if(message_rendered) {
		SDL_Rect blit_loc = {
			scr_acc.screen->w - message_rendered->w,
			scr_acc.screen->h - message_rendered->h,
			message_rendered->w,
			message_rendered->h
		};
		SDL_BlitSurface(message_rendered,NULL,scr_acc.screen,&blit_loc);
		SDL_FreeSurface(message_rendered);
	}
}
char* ellipsis(char *string, Uint16 offset) {
	if(!loaded_theme) return string;
	int w;
	device scr_acc = access_device(access_screen);
	get_text_dimensions(string,&w,NULL);
	while(w + offset >  scr_acc.screen->w) {
		size_t string_length = strlen(string);
		if(string_length < 5) break;
		string[string_length-2] = string[string_length-3] = string[string_length-4] = '.';
		string[string_length-1] = 0;
		get_text_dimensions(string,&w,NULL);
	}
	return string;
}

Uint32 get_color(int context) {
	if(!loaded_theme) return 0xFFFFFFFF;
	switch(context) {
	case color_normal: return loaded_theme->font_attributes.font_color;
	case color_selected: return loaded_theme->font_attributes.selected_color;
	case color_secondary: return loaded_theme->font_attributes.secondary_color;
	case color_tertiary: return loaded_theme->font_attributes.tertiary_color;
	case color_battery_low: return loaded_theme->font_attributes.battery_low_color;
	case color_battery_charging: return loaded_theme->font_attributes.battery_charging_color;
	case color_zb_1: return loaded_theme->zb_color_1;
	case color_zb_2: return loaded_theme->zb_color_2;
	case color_error: return loaded_theme->font_attributes.error_color;
	default: return loaded_theme->font_attributes.font_color;
	}
}
void set_explicit_color(Uint32 color) {
	explicit_color = color;
	is_explicit_color = 1;
}
void unset_explicit_color() {
	is_explicit_color = 0;
}
const char* get_loaded_theme_name() {
	return theme_loaded;
}

void free_theme_info(theme_info *theme) {
	if(theme->theme_name) free(theme->theme_name);
	if(theme->bg_file) free(theme->bg_file);
	if(theme->font_file) free(theme->font_file);
	if(theme->ranges) {
		if(theme->ranges->range_begins) free(theme->ranges->range_begins);
		if(theme->ranges->range_ends) free(theme->ranges->range_ends);
	}
	if(theme->images) {
		int i;
		for(i = 0; i < image_end; ++i) {
			if(theme->images[i]) SDL_FreeSurface(theme->images[i]);
		}
		free(theme->images);
	}
}

char show_popup(int which) {
	char *filename;
	switch(which) {
		case 0: filename = "about.png"; break; //About screen
		case 1: filename = "notheme.png"; break; //Error loading theme
		default: return 0;
	}
	SDL_Surface *img = IMG_Load(filename);
	if(!img) {
		error_message(get_message(mesg_sdl_image_error),IMG_GetError());
		return 0;
	}
	char temp = show_image(img,0,0,1);
	SDL_FreeSurface(img);
	if(!temp) return 0;
	int input;
	clock_t start = 0;
	int action = no_event;
	while((input = handle_input(1)) != error_ev) {
		if(!(input & pan_event) && (input != no_event)) {
			if(input & event_end) {
				if((clock()-start)/CLOCKS_PER_SEC < 2) break;
				else start = action = 0;
			}
			else if(action == no_event) {
				start = clock();
				action = input;
			}
		}
	}
	return 1;
}

void clear_screen() {
	device scr_acc = access_device(access_screen);
	SDL_FillRect(scr_acc.screen,NULL,(loaded_theme?SDL_MapRGB(scr_acc.screen->format,
		(loaded_theme->bg_fallback&0xFF0000)>>16,
		(loaded_theme->bg_fallback&0xFF00)>>8,
		(loaded_theme->bg_fallback&0xFF)):SDL_MapRGB(scr_acc.screen->format,127,127,127)));
}

SDL_Surface* get_image(int image) {
	//Bad implementation until more images are put in place
	if(!loaded_theme || !loaded_theme->images) return NULL;
	switch(image) {
		case image_usb: return loaded_theme->images[0];
	}
	return NULL;
}
