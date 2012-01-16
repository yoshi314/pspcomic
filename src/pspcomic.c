/** \file
    Main PSPComic functionality

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

#if defined(PSP) && (_PSP_FW_VERSION > 150)
PSP_HEAP_SIZE_MAX();
#endif
#if defined(PSP) && !defined(DOXYGEN)
PSP_MODULE_INFO("PSPComic",0,1,0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
#endif
#ifdef PSP
const float joy_sense = 0.0012f;
#else /* PSP */
const float joy_sense = 0.001f;
#endif /* PSP */

const Uint8 pan_delay = 0;
const Uint16 dead_zone = 6000;

const Uint16 menu_scroll_rate = 175;

const float zoom_amount = 0.1f;
const float zoom_max = 1.0f;
const float zoom_min = 0.25f;
const Uint8 pan_max = 99;
const Uint8 pan_min = 5;

///The start of the ASCII range. Despite not being const, do not modify this value
static unsigned ascii_start = 0;
///The end of the ASCII range. Despite not being const, do not modify this value
static unsigned ascii_end = 127;
const char_ranges default_ranges = { 1U, &ascii_start, &ascii_end };

///The initial clock frequency
///
///This will get set to the actual initial value when the application runs
static int initial_clock = 222;
///The current working directory
static char *cwd = NULL;

const char pspcomic_v[] = "1.0.1";

#if !defined(PSP) && !defined(_WIN32)
char *root_dir = NULL;
#endif

enum menu_number {
	///Invalid menu option
	menu_invalid = 0,
	///Open book menu
	menu_open_book,
	///In-book (main) menu
	menu_in_book,
	///Jump to page menu
	menu_jump_page,
	///Adjust clock frequency
	menu_adjust_clock,
	///Configuration menu
	menu_config,
	///Adjust pan rate menu
	menu_adjust_pan,
	///Adjust scroll skip amount
	menu_adjust_scroll_skip,
	///Bookmark menu
	menu_bookmark,
	///Set zoom box width
	menu_adjust_zb_w,
	///Set zoom box height
	menu_adjust_zb_h,
	///Autozoom mode
	menu_autozoom,
	///Set theme menu
	menu_theme,
	///Set language menu
	menu_language
};

///Unload some stuff before exiting
static void _atexit() {
	unload_language();
	unload_theme();
	set_clock(initial_clock);
	TTF_Quit();
	SDL_Quit();
}

inline void sdl_error() {
	char *e = SDL_GetError();
	if(strlen(e)) error_message(get_message(mesg_sdl_error), e);
}
void init_video(Uint16 w, Uint16 h) {
	static char initd = 0;
	if(initd) return;
	SDL_ShowCursor(SDL_DISABLE);
	#ifdef PSP
	SDL_Surface *screen = SDL_SetVideoMode(w,h,0,SDL_HWSURFACE|SDL_DOUBLEBUF);
	#else
	SDL_Surface *screen = SDL_SetVideoMode(w,h,0,SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_RESIZABLE);
	#endif
	if(!screen) {
		sdl_error();
	}
}
device access_device(int device_code) {
	static SDL_Surface *screen = NULL;
	static SDL_Joystick *joy = NULL;
	device ret;
	ret.type = -1;
	ret.screen = NULL;
	ret.joystick = NULL;
	switch(device_code) {
		case access_screen:
			if(!screen) screen = SDL_GetVideoSurface();
			ret.screen = screen;
			ret.type = access_screen;
		break;
		case access_joystick:
			if(!joy) {
				if(SDL_JoystickOpened(0)) return ret;
				joy = SDL_JoystickOpen(0);
			}
			ret.joystick = joy;
			ret.type = access_joystick;
		break;
	}
	return ret;
}
int* access_int_global(int global_code) {
	//The l is for local (I guess...)
	static int 
		l_resize_method = resize_nn,
		l_zoom_persist = 0,
		l_rotate_persist = 0,
		l_pan_rate = 20,
		l_scroll_skip = 5,
		l_precaching = 0,
		l_manga_mode = 0,
		l_zoom_box_w = 150,
		l_zoom_box_h = 200,
		l_bookmark_on_load = 0,
		l_autozoom_mode = full_width,
		l_singlehanded = 0,
		l_analog_disabled = 0,
		l_dynamic_cpu = 0;
	switch(global_code) {
		case access_resize: return &l_resize_method;
		case access_pan_rate: return &l_pan_rate;
		case access_zoom_persist: return &l_zoom_persist;
		case access_rotate_persist: return &l_rotate_persist;
		case access_scroll_skip: return &l_scroll_skip;
		case access_precaching: return &l_precaching;
		case access_manga_mode: return &l_manga_mode;
		case access_zoom_box_w: return &l_zoom_box_w;
		case access_zoom_box_h: return &l_zoom_box_h;
		case access_bookmark_on_load: return &l_bookmark_on_load;
		case access_autozoom_mode: return &l_autozoom_mode;
		case access_singlehanded: return &l_singlehanded;
		case access_analog_disabled: return &l_analog_disabled;
		case access_dynamic_cpu: return &l_dynamic_cpu;
	}
	return NULL;
}
int set_clock(int new_clock) {
	#ifdef PSP
	if((new_clock > 333) || (new_clock < 20)) return 0;
	return scePowerSetClockFrequency(new_clock, new_clock, new_clock/2);
	#else
	return 0;
	#endif
}
static int init_zoom_box(void *zbi) {
	//Could be done better with timers, but I didn't know about those when I
	//wrote this. Live and learn.
	zoom_box_info *zbid = (zoom_box_info*)zbi;
	SDL_Delay(2000);
	device acc = access_device(access_screen);
	if(*(zbid->action_status) != zoom_in) return 0;
	if((zbid->scaled_page->w < acc.screen->w) || (zbid->scaled_page->h < acc.screen->h))
		clear_screen();
	show_image(zbid->scaled_page,*zbid->offsetX,*zbid->offsetY,0);
	draw_zoom_box(zbid->page_img,zbid->scale_ratio,zbid->rotation,-1,-1,*zbid->offsetX,*zbid->offsetY,2);
	zbid->zoom_box_is_active = 1;
	return 1;
}
void flip_screen() {
	device scr_acc = access_device(access_screen);
	SDL_Flip(scr_acc.screen);
}

char show_image(SDL_Surface *surf, Uint16 offsetX, Uint16 offsetY, char flip) {
	if(!surf) return 0;
	SDL_Rect dest, source;
	device scr_acc = access_device(access_screen);
	SDL_Surface *screen = scr_acc.screen;
	dest.w = screen->w;
	dest.h = screen->h;
	if(surf->w < screen->w) dest.x = (screen->w-surf->w)/2;
	else dest.x = 0;
	if(surf->h < screen->h) dest.y = (screen->h-surf->h)/2;
	else dest.y = 0;
	source.w = (screen->w > surf->w)?screen->w:surf->w;
	source.h = (screen->h > surf->h)?screen->h:surf->h;
	source.x = offsetX;
	source.y = offsetY;
	scale_up_cpu();
	if(surf->flags & (SDL_SRCALPHA|SDL_SRCCOLORKEY)) {
		clear_screen();
		//SDL_SetAlpha(surf,0,SDL_ALPHA_OPAQUE);
	}
	if(SDL_BlitSurface(surf,&source,screen,&dest) > -1) {
		if(flip) flip_screen();
		scale_down_cpu();
		return 1;
	} else {
		scale_down_cpu();
		sdl_error();
		return 0;
	}
}
char show_image2(SDL_Surface *surf, SDL_Rect *src, SDL_Rect *dst, char rotation, char alpha, char flip) {
	device scr_acc = access_device(access_screen);
	SDL_Surface *screen = scr_acc.screen;
	SDL_Rect src_temp = {0,0,0,0}, dst_temp = {0,0,0,0};
	SDL_Surface *buffer1, *buffer2;
	if(!src) {
		src_temp.w = surf->w;
		src_temp.h = surf->h;
		src = &src_temp;
		buffer1 = surf;
	} else {
		buffer1 = SDL_CreateRGBSurface(
			SDL_SWSURFACE, src->w, src->h, 32,
			0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
		);
		if(!buffer1) return 0;
		SDL_BlitSurface(surf,src,buffer1,&dst_temp);
	}
	if(!dst) {
		dst_temp.w = screen->w;
		dst_temp.h = screen->h;
	} else dst_temp = *dst;
	dst = &dst_temp;
	buffer2 = (rotation & 1?
		rotozoom(buffer1,NULL,dst->h,dst->w,rotation):
		rotozoom(buffer1,NULL,dst->w,dst->h,rotation)
	);
	if((buffer1 != buffer2) && (buffer1 != surf)) SDL_FreeSurface(buffer1);
	if(!buffer2) return 0;
	if(!alpha) SDL_SetAlpha(buffer2,0,SDL_ALPHA_OPAQUE);
	SDL_Rect temp = {0,0,buffer2->w,buffer2->h};
	scale_up_cpu();
	SDL_BlitSurface(buffer2,&temp,screen,dst);
	scale_down_cpu();
	if(buffer2 != surf) SDL_FreeSurface(buffer2);
	return 1;
}
event show_book(char *dir, char *book_name) {
	reset_text_pos();
	clear_screen();
	show_image(get_background(),0,0,0);
	say_centered(get_message(mesg_loading));
	flip_screen();
	comic_book book = open_comic_book(dir,book_name);
	if(book.num_pages == 0) {
		error_message(get_message(mesg_open_book_error));
		return init_event(error_ev);
	}
	char *bookcatted = malloc(sizeof(char)*(strlen(dir)+strlen(book_name)+1));
	if(!bookcatted) return init_event(error_ev);
	strcpy(bookcatted, dir);
	strcat(bookcatted, book_name);
	set_last_book(bookcatted);
	free(bookcatted);
	event operation = init_event(no_event);
	float pscale_ratio = (*access_int_global(access_autozoom_mode) == full_view?1.0f:0.0f);
	Uint8 protation = 0;
	if(*access_int_global(access_bookmark_on_load))
		book = goto_page(book,get_bookmark(&book));
	SDL_Surface *page_img;
	precache_info pci = {NULL,0,0,&book};
	SDL_Thread *precacher;
	while(!(operation.command & (quit_command|close_book|aux_command|error_ev)) ||
			((operation.command == aux_command) && (operation.int_data != open_book))) {
		if(!*access_int_global(access_rotate_persist)) protation = 0;
		if(!*access_int_global(access_zoom_persist))
			pscale_ratio = (*access_int_global(access_autozoom_mode) == full_view?1.0f:0.0f);
		if((pci.page == book.current_page) && pci.new_page && (pci.loaded == 2)) {
			page_img = pci.new_page;
		} else {
			if(pci.loaded == 1) {
				SDL_WaitThread(precacher,NULL);
			}
			if((pci.loaded == 2) && (pci.page == book.current_page)) {
				page_img = pci.new_page;
			} else {
				if(pci.loaded == 2) {
					SDL_FreeSurface(pci.new_page);
					pci.new_page = NULL;
				}
				size_t size = 0;
				char *block = extract_file(&book,book.current_page,&size);
				if(block == NULL) {
					Uint16 page = book.current_page;
					close_comic_book(book);
					book = open_comic_book(dir,book_name);
					if(book.num_pages == 0) {
						error_message(get_message(mesg_open_book_error));
						return init_event(error_ev);
					}
					book.current_page = page;
					block = extract_file(&book,book.current_page,&size);
					if(block == NULL) {
						error_message(get_message(mesg_load_page_error));
						return init_event(error_ev);
					}
				}
				SDL_RWops *page = SDL_RWFromMem(block,size);
				if(page) {
					page_img = IMG_Load_RW(page,0);
					SDL_FreeRW(page);
				}
				free(block);
				free_event(operation);
			}
		}
		pci.loaded = 0;
		pci.page = book.current_page + 1;
		if(*access_int_global(access_precaching) && (pci.page < book.num_pages)) precacher = SDL_CreateThread(cache_page,&pci);
		if(!page_img) {
			error_message(get_message(mesg_sdl_image_error),IMG_GetError());
			operation.command = error_ev;
		} else {
			operation = show_page(page_img,&book,&protation,&pscale_ratio);
			SDL_FreeSurface(page_img);
		}
		if(operation.command == error_ev) {
			event op2 = init_event(no_event);
			while(op2.command == no_event) {
				op2.command = handle_input(1);
				if(op2.command == (open_menu|event_end)) {
					free_event(op2);
					op2 = show_menu(&book);
					error_message(get_message(mesg_sdl_image_error),IMG_GetError());
				}
				switch(op2.command) {
					case quit_command: case next_page|event_end: case prev_page|event_end:
					case jump_page: case close_book: case aux_command:
						if((op2.command != aux_command) || (op2.int_data == open_book)) break;
					case redraw_ev:
						error_message(get_message(mesg_sdl_image_error),IMG_GetError());
					default: op2.command = no_event;
				}
			}
			operation = op2;
		}
		switch(operation.command) {
			case next_page: case next_page|event_end: book = goto_next_page(book); break;
			case prev_page: case prev_page|event_end: book = goto_prev_page(book); break;
			case jump_page: book = goto_page(book, operation.int_data); break;
		}
	}
	if(pci.page < book.num_pages) {
		if(pci.loaded == 1) {
			SDL_WaitThread(precacher,NULL);
		}
		if(pci.loaded == 2) SDL_FreeSurface(pci.new_page);
	}
	close_comic_book(book);
	return operation;
}
int cache_page(void *pci) {
	#ifdef PSP
	SceKernelThreadInfo info;
	SceUID thid = sceKernelGetThreadId();
	info.size = sizeof(SceKernelThreadInfo);
	if(!sceKernelReferThreadStatus(thid,&info))
		sceKernelChangeThreadPriority(thid,info.currentPriority+8);
	#endif
	precache_info *pcis = (precache_info*)pci;
	pcis->loaded = 1;
	if(pcis->page >= pcis->book->num_pages) return -4;
	Uint16 precache_page = pcis->page;
	size_t size = 0;
	char *block = extract_file(pcis->book,pcis->page,&size);
	if(block == NULL) {
		pcis->loaded = -2;
		return -2;
		//Don't try any fancy stuff here.
		//If you suspend your PSP while it's trying to precache, just bail.
	}
	SDL_RWops *page = SDL_RWFromMem(block,size);
	if(page) {
		pcis->new_page = IMG_Load_RW(page,0);
		SDL_FreeRW(page);
	} else {
		pcis->loaded = -3;
		return -3;
	}
	free(block);
	if(!pcis->new_page) {
		pcis->loaded = -3;
		return -3;
	}
	if(pcis->page != precache_page) {
		SDL_FreeSurface(pcis->new_page);
		pcis->loaded = -1;
		return -1;
	}
	pcis->loaded = 2;
	return 1;
}
void draw_zoom_box(SDL_Surface *page_img, float scale_ratio, Uint8 rotation, Sint32 x, Sint32 y, Uint16 offsetX, Uint16 offsetY, Uint8 zoom_level) {
	device scr_acc = access_device(access_screen);
	SDL_Surface *screen = scr_acc.screen;
	SDL_Rect big_rect, little_rect, src_rect, dst_rect;
	big_rect.w = *access_int_global(access_zoom_box_w);
	big_rect.h = *access_int_global(access_zoom_box_h); 
	float newW, newH;
	newW = (rotation & 1?page_img->h*scale_ratio:page_img->w*scale_ratio);
	newH = (rotation & 1?page_img->w*scale_ratio:page_img->h*scale_ratio);
	Uint16 smallerW, smallerH;
	smallerW = (screen->w > newW?newW:screen->w);
	smallerH = (screen->h > newH?newH:screen->h);
	if(zoom_level > 4) zoom_level = 4;
	else if(zoom_level == 0) zoom_level = 1;
	little_rect.w = big_rect.w/zoom_level;
	if(little_rect.w > newW) {
		little_rect.w = newW;
		big_rect.w = newW*zoom_level;
	}
	little_rect.h = big_rect.h/zoom_level;
	if(little_rect.h > newH) {
		little_rect.h = newH;
		big_rect.h = newH*zoom_level;
	}
	if(x > (Uint16)newW) little_rect.x = 0;
	else if(x < 0) little_rect.x = (smallerW - little_rect.w)/2;
	else little_rect.x = x;
	if(y > (Uint16)newH) little_rect.y = 0;
	if(y < 0) little_rect.y = (smallerH - little_rect.h)/2;
	else little_rect.y = y;
	if(little_rect.x + little_rect.w/2 > smallerW/2) {
		big_rect.x = 10;
		big_rect.y = (screen->h - big_rect.h)/2;
	} else {
		big_rect.x = screen->w - big_rect.w - 10;
		big_rect.y = (screen->h - big_rect.h)/2;
	}
	dst_rect = big_rect;
	dst_rect.x++;
	dst_rect.y++;
	dst_rect.w -= 2;
	dst_rect.h -= 2;

	switch(rotation % 4) {
		case 0:
			src_rect.x = (Uint16)((little_rect.x + offsetX)/scale_ratio);
			src_rect.y = (Uint16)((little_rect.y + offsetY)/scale_ratio);
			src_rect.w = (Uint16)(little_rect.w/scale_ratio);
			src_rect.h = (Uint16)(little_rect.h/scale_ratio);
		break;
		case 1:
			src_rect.h = (Uint16)(little_rect.w/scale_ratio);
			src_rect.w = (Uint16)(little_rect.h/scale_ratio);
			src_rect.x = (Uint16)((little_rect.y + offsetY)/scale_ratio);
			src_rect.y = page_img->h - src_rect.h - (Uint16)((little_rect.x + offsetX)/scale_ratio);
		break;
		case 2:
			src_rect.w = (Uint16)(little_rect.w/scale_ratio);
			src_rect.h = (Uint16)(little_rect.h/scale_ratio);
			src_rect.x = page_img->w - src_rect.w - (Uint16)((little_rect.x + offsetX)/scale_ratio);
			src_rect.y = page_img->h - src_rect.h - (Uint16)((little_rect.y + offsetY)/scale_ratio);
		break;
		case 3:
			src_rect.h = (Uint16)(little_rect.w/scale_ratio);
			src_rect.w = (Uint16)(little_rect.h/scale_ratio);
			src_rect.x = page_img->w - src_rect.w - (Uint16)((little_rect.y + offsetY)/scale_ratio);
			src_rect.y = (Uint16)((little_rect.x + offsetX)/scale_ratio);
		break;
	}

	show_image2(page_img,&src_rect,&dst_rect,rotation,0,0);
	src_rect.x = src_rect.y = 0;
	src_rect.w = dst_rect.w;
	src_rect.h = dst_rect.h;
	{ //More stack space saving!
		float visiW, visiH;
		visiW = (rotation & 1?scale_ratio*page_img->h:scale_ratio*page_img->w);
		visiH = (rotation & 1?scale_ratio*page_img->w:scale_ratio*page_img->h);
		if(screen->w > visiW)
			little_rect.x += (screen->w - visiW)/2;
		if(screen->h > visiH)
			little_rect.y += (screen->h - visiH)/2;
	}
	Uint32 zb_color_1 = get_color(color_zb_1);
	Uint32 zb_color_2 = get_color(color_zb_2);
	blend_rect(screen, little_rect, get_red(zb_color_1), get_green(zb_color_1),
		get_blue(zb_color_1), get_alpha(zb_color_1));
	blend_rect(screen, big_rect, get_red(zb_color_2), get_green(zb_color_2),
		get_blue(zb_color_2), get_alpha(zb_color_2));
	flip_screen();
}

int handle_input(char wait) {
	static int down = 0;
	SDL_Event ev;
	device joy_acc = access_device(access_joystick);
	SDL_Joystick *joy = joy_acc.joystick;
	if(wait > 0?SDL_WaitEvent(&ev):SDL_PollEvent(&ev)) {
		switch(ev.type) {
			#if defined(PSP) || defined(_WIN32) //This is broken on Linux
			case SDL_JOYBUTTONDOWN: case SDL_JOYBUTTONUP: {
				int ret;
				switch(ev.jbutton.button) {
					case 8:	ret = pan_up; break;
					case 9:	ret = pan_right; break;
					case 6:	ret = pan_down; break;
					case 7:	ret = pan_left; break;
					case 4:	ret = (*access_int_global(access_singlehanded)?
							next_page:prev_page); break;
					case 5:	ret = (*access_int_global(access_singlehanded)?
							prev_page:next_page); break;
					case 3:	ret = rotate_ccw; break;
					case 1:	ret = rotate_cw; break;
					case 0:	ret = zoom_in; break;
					case 2:	ret = zoom_out; break;
					case 10: ret = zoom_fixed; break;
					case 11: ret = open_menu; break;
					default: ret = no_event; break;
				}
				if((!SDL_JoystickGetButton(joy,ev.jbutton.button)) && (ret != no_event)) {
					if(down & ret) down ^= ret;
					ret |= event_end;
					return ret;
				}
				if(down & ret) return no_event;
				down |= ret;
				return ret;
			} break;
			case SDL_JOYAXISMOTION: {
				if(*access_int_global(access_analog_disabled)) return no_event;
				return pan_event;
			} break;
			#endif
			case SDL_USEREVENT: {
				return ev.user.code;
			}
			#ifndef PSP
			case SDL_QUIT: return quit_command;
			case SDL_KEYDOWN: case SDL_KEYUP: {
				int ret;
				SDL_KeyboardEvent k = ev.key;
				switch(k.keysym.sym) {
					case SDLK_UP:		ret = pan_up;	 break;
					case SDLK_RIGHT:	ret = pan_right; break;
					case SDLK_DOWN:		ret = pan_down;	 break;
					case SDLK_LEFT:		ret = pan_left;	 break;
					case SDLK_PAGEUP:	ret = prev_page; break;
					case SDLK_PAGEDOWN:	ret = next_page; break;
					case SDLK_PERIOD:	ret = rotate_ccw ;break;
					case SDLK_SLASH:	ret = rotate_cw; break;
					case SDLK_QUOTE:	ret = zoom_in;	 break;
					case SDLK_SEMICOLON:	ret = zoom_out; break;
					case SDLK_SPACE:	ret = open_menu; break;
					case SDLK_l: ret = zoom_fixed; break;
					case SDLK_q: ret = quit_command; break;
					default: ret = no_event; break;
				}
				if((k.state == SDL_RELEASED) && (ret ^ no_event)) ret |= event_end;
				return ret;
			} break;
			#endif
		}
	} else {
		if(wait > 0) exit(0); //This is only triggered when using the Home menu to exit, so bail anyway
		return no_event;
	}
	return no_event;
}
event show_page(SDL_Surface *page_img, comic_book* book, Uint8 *rotation, float *scale_ratio) {
	reset_text_pos();
	if(!page_img) return init_event(error_ev);
	device acc = access_device(access_screen);
	SDL_Surface *screen = acc.screen;
	acc = access_device(access_joystick);
	SDL_Joystick *joy = acc.joystick;
	int quit = 0, poll_command, autozoom_mode;
	Uint32 pan_amount = *access_int_global(access_pan_rate);
	Uint16 scale_width;
	if(*scale_ratio) {
		if(*scale_ratio == 1) autozoom_mode = full_view;
		else autozoom_mode = custom_zoom;
		//scale_width = (*rotation & 1?(Uint16)page_img->h:(Uint16)page_img->w)**scale_ratio;
		scale_width = ((Uint16)page_img->w)**scale_ratio;
	} else {
		autozoom_mode = *access_int_global(access_autozoom_mode);
		switch(autozoom_mode) {
			case full_width: scale_width = screen->w; break;
			case twice_width: scale_width = screen->w*2; break;
			case autodetect_zoom:
				if(!(*rotation & 1) && (page_img->w > page_img->h))
					scale_width = screen->w*2;
				else if((*rotation & 1) && (page_img->w < page_img->h))
					scale_width = screen->w*2;
				else scale_width = screen->w;
			break;
		}
		if(*rotation & 1) scale_width = page_img->w*(scale_width/(float)page_img->h);
	}
	char joy_in_use = 0;
	SDL_Surface *new_surf;
	new_surf = rotozoom(page_img,NULL,scale_width,0,*rotation);
	if(!new_surf) return init_event(error_ev);
	clear_screen();
	Uint16 offsetX = 0, offsetY = 0;
	if(*access_int_global(access_manga_mode) && (new_surf->w > screen->w))
		offsetX = new_surf->w - screen->w;
	show_image(new_surf,offsetX,0,0);
	char page_num[32];
	snprintf(page_num,32,"%u/%u",book->current_page+1,book->num_pages);
	page_num[31] = '\0';
	int textW, textH;
	if(get_text_dimensions(page_num,&textW,&textH) == 0) {
		place_text(page_num, screen->w - textW, screen->h - textH,0x000000);
		place_text(page_num, screen->w - textW-1, screen->h - textH-1,0xFFFFFF);
	}
	flip_screen();
	reset_text_pos();
	char wait_for_input = 1;
	unsigned short delay = 0;
	signed char offsetX_shift = 0, offsetY_shift = 0;
	event ret = init_event(no_event);
	clock_t start = 0;
	int pending_action = no_event;
	SDL_Thread *izb_thread = NULL;
	zoom_box_info zbi = {0,NULL,NULL,&offsetX,&offsetY,*scale_ratio,*rotation,NULL};
	Uint8 zb_zoom_level = 2;
	Uint16 zb_offsetX = 0;
	Uint16 zb_offsetY = 0;
	while(!quit) {
		poll_command = handle_input(wait_for_input);
		switch(poll_command) {
			case next_page: case prev_page:
			case rotate_cw: case rotate_ccw:
			case zoom_in: case zoom_out: case zoom_fixed:
			case open_menu:
				if(!pending_action) {
					pending_action = poll_command;
					start = SDL_GetTicks();
				}
				if((pending_action == zoom_in) && !zbi.zoom_box_is_active) {
					zbi.action_status = &pending_action;
					zbi.page_img = page_img;
					zbi.scaled_page = new_surf;
					zbi.rotation = *rotation;
					zbi.scale_ratio = ((float)scale_width)/page_img->w;
					izb_thread = SDL_CreateThread(init_zoom_box,&zbi);
					zb_zoom_level = 2;
					zb_offsetX = (new_surf->w < *access_int_global(access_zoom_box_w)/zb_zoom_level)?0:
						(screen->w < new_surf->w)?
						(screen->w - *access_int_global(access_zoom_box_w)/zb_zoom_level)/2:
						(new_surf->w - *access_int_global(access_zoom_box_w)/zb_zoom_level)/2;
					zb_offsetY = (new_surf->h < *access_int_global(access_zoom_box_h)/zb_zoom_level)?0:
						(screen->h < new_surf->h)?
						(screen->h - *access_int_global(access_zoom_box_h)/zb_zoom_level)/2:
						(new_surf->h - *access_int_global(access_zoom_box_h)/zb_zoom_level)/2;
				}
			break;
			case next_page|event_end:
				if(pending_action != next_page) break;
				if((SDL_GetTicks() - start)/1000 < 2) {
					if(book->current_page != book->num_pages - 1)
						quit = next_page;
				}
				pending_action = no_event;
			break;
			case prev_page|event_end:
				if(pending_action != prev_page) break;
				if((SDL_GetTicks() - start)/1000 < 2) {
					if(book->current_page != 0)
						quit = prev_page;
				}
				pending_action = no_event;
			break;
			case quit_command|event_end: quit = quit_command; break;
			case pan_up:	offsetY_shift = -pan_amount;	break;
			case pan_down:	offsetY_shift =  pan_amount;	break;
			case pan_left:	offsetX_shift = -pan_amount;	break;
			case pan_right:	offsetX_shift =  pan_amount;	break;
			case rotate_cw|event_end: case rotate_ccw|event_end:
			if((pending_action != rotate_cw) && (pending_action != rotate_ccw))
				break;
			if((SDL_GetTicks() - start)/1000 < 2) {
				if(zbi.zoom_box_is_active) {
					{ //Begin new block...saving space on stack here.
						Uint16 centerX, centerY;
						Uint16 maxX =
							(screen->w > new_surf->w?new_surf->w:screen->w);
						Uint16 maxY =
							(screen->h > new_surf->h?new_surf->h:screen->h);
						Uint16 zb_w = *access_int_global(access_zoom_box_w);
						Uint16 zb_h = *access_int_global(access_zoom_box_h);
						centerX = zb_offsetX + zb_w/(zb_zoom_level*2);
						centerY = zb_offsetY + zb_h/(zb_zoom_level*2);
						zb_zoom_level += (pending_action == rotate_cw?-1:1);
						if(zb_zoom_level > 4) zb_zoom_level = 4;
						else if(zb_zoom_level < 2) zb_zoom_level = 2;
						if(zb_w/(zb_zoom_level*2) > centerX)
							zb_offsetX = 0;
						else if(zb_w/(zb_zoom_level*2) + centerX > maxX)
							zb_offsetX = (zb_w/zb_zoom_level > new_surf->w)?0:(maxX - zb_w/zb_zoom_level);
						else zb_offsetX = centerX - zb_w/(zb_zoom_level*2);
						if(zb_h/(zb_zoom_level*2) > centerY)
							zb_offsetY = 0;
						else if(zb_h/(zb_zoom_level*2) + centerY > maxY)
							zb_offsetY = (zb_h/zb_zoom_level > new_surf->h)?0:(maxY - zb_h/zb_zoom_level);
						else zb_offsetY = centerY - zb_h/(zb_zoom_level*2);
					}
					if((new_surf->w < screen->w) || (new_surf->h < screen->h))
						clear_screen();
					show_image(new_surf,offsetX,offsetY,0);
					draw_zoom_box(page_img,((float)scale_width)/page_img->w,*rotation,zb_offsetX,zb_offsetY,offsetX,offsetY,zb_zoom_level);
					pending_action = no_event;
					break;
				}
				float centerY = (offsetX + (new_surf->w > screen->w?screen->w:new_surf->w)/2.0f)/(float)new_surf->w;
				float centerX = (offsetY + (new_surf->h > screen->h?screen->h:new_surf->h)/2.0f)/(float)new_surf->h;
				offsetX_shift = offsetY_shift = 0;
				*rotation+=(poll_command&rotate_cw?1:3);
				*rotation %= 4;
				if(new_surf != page_img) SDL_FreeSurface(new_surf);
				if(autozoom_mode == full_width) {
					if(*rotation & 1)
						scale_width = page_img->w*(screen->w/(float)page_img->h);
					else scale_width = screen->w;
				} else if(autozoom_mode == twice_width) {
					if(*rotation & 1)
						scale_width = page_img->w*(screen->w/(float)page_img->h)*2;
					else scale_width = screen->w*2;
				} else if(autozoom_mode == autodetect_zoom) {
					if(!(*rotation & 1) && (page_img->w > page_img->h))
						scale_width = screen->w*2;
					else if((*rotation & 1) && (page_img->w < page_img->h))
						scale_width = page_img->w*(screen->w/(float)page_img->h)*2;
				}
				new_surf = rotozoom(page_img,NULL,scale_width,0,*rotation);
				if(!new_surf) return init_event(error_ev);
				if(new_surf->w <= screen->w) offsetX = 0;
				else {
					float result;
					if(poll_command & rotate_cw) {
						result = new_surf->w - (centerX * new_surf->w) - screen->w/2.0f;
					} else {
						result = (centerX * new_surf->w) - screen->w/2.0f;
					}
					offsetX = (result > 0?result:0);
					if((offsetX + screen->w) > new_surf->w) offsetX = new_surf->w - screen->w;					
				}
				if(new_surf->h <= screen->h) offsetY = 0;
				else {
					float result;
					if(poll_command & rotate_cw) {
						result = (centerY * new_surf->h) - screen->h/2;
					} else {
						result = new_surf->h - (centerY * new_surf->h) - screen->h/2;
					}
					offsetY = (result > 0?result:0);
					if((offsetY + screen->h) > new_surf->h) offsetY = new_surf->h - screen->h;					
				}
				if((new_surf->w < screen->w) || (new_surf->h < screen->h))
					clear_screen();
				show_image(new_surf,offsetX,offsetY,1);
			}
			pending_action = no_event;
			break;
			case zoom_in|event_end: case zoom_out|event_end:
			if((pending_action != zoom_in) && (pending_action != zoom_out)) break;
			if((SDL_GetTicks() - start)/1000 < 2) {
				SDL_KillThread(izb_thread); //Yeah yeah, I know.
				izb_thread = NULL;
				if(zbi.zoom_box_is_active) {
					zbi.zoom_box_is_active = 0;
					if((new_surf->w < screen->w) || (new_surf->h < screen->h))
						clear_screen();
					show_image(new_surf,offsetX,offsetY,1);
				} else if(
				  !(
					(poll_command & zoom_in) && (scale_width == (Uint16)(zoom_max*page_img->w))
				  ) && !(
				  	(poll_command & zoom_out) && (scale_width == (Uint16)(zoom_min*page_img->w))
				)) {
					*scale_ratio = ((float)scale_width)/page_img->w;
					float centerX, centerY;
					centerX = (offsetX + (new_surf->w > screen->w?screen->w:new_surf->w)/2.0f)/(float)new_surf->w;
					centerY = (offsetY + (new_surf->h > screen->h?screen->h:new_surf->h)/2.0f)/(float)new_surf->h;
					*scale_ratio += (poll_command&zoom_in?zoom_amount:-zoom_amount);
					if(*scale_ratio > zoom_max) *scale_ratio = zoom_max;
					else if(*scale_ratio < zoom_min) *scale_ratio = zoom_min;
					if(*scale_ratio == 1) autozoom_mode = full_view;
					scale_width = (Uint16)(*scale_ratio*page_img->w);
					if(new_surf != page_img) SDL_FreeSurface(new_surf);
					new_surf = rotozoom(page_img,NULL,scale_width,0,*rotation);
					if(!new_surf) return init_event(error_ev);
					offsetX_shift = offsetY_shift = 0;
					if(!new_surf) return init_event(error_ev);
					autozoom_mode = custom_zoom;
					if(new_surf->w <= screen->w) offsetX = 0;
					else {
						float result = (centerX * new_surf->w) - screen->w/2;
						offsetX = (result > 0?result:0);
						if((offsetX + screen->w) > new_surf->w) offsetX = new_surf->w - screen->w;
					}
					if(new_surf->h <= screen->h) offsetY = 0;
					else {
						float result = (centerY * new_surf->h) - screen->h/2;
						offsetY = (result > 0?result:0);
						if((offsetY + screen->h) > new_surf->h) offsetY = new_surf->h - screen->h;
					}
					if((new_surf->w < screen->w) || (new_surf->h < screen->h))
						clear_screen();
					show_image(new_surf,offsetX,offsetY,1);
				}
			}
			pending_action = no_event;
			break;
			case zoom_fixed|event_end: //Autozoom button pressed
			if(pending_action != zoom_fixed) break;
			if((SDL_GetTicks() - start)/1000 < 2) {
				if(!zbi.zoom_box_is_active) {
					float real_scale_ratio = new_surf->w / (float)page_img->w;
					float centerX = (offsetX + (new_surf->w > screen->w?screen->w:new_surf->w)/2.0f)/(float)new_surf->w;
					float centerY = (offsetY + (new_surf->h > screen->h?screen->h:new_surf->h)/2.0f)/(float)new_surf->h;
					offsetX = offsetY = 0;
					offsetX_shift = offsetY_shift = 0;
					int new_zoom_mode = *access_int_global(access_autozoom_mode);
					if(((autozoom_mode == custom_zoom) && (new_zoom_mode != full_view)) ||
							((autozoom_mode == full_view) && *scale_ratio)) {
						//Change to autozoom mode
						if(new_surf != page_img) SDL_FreeSurface(new_surf);
						*scale_ratio = 0;
						switch(new_zoom_mode) {
							case twice_width: scale_width = screen->w*2; break;
							case autodetect_zoom:
								if(!(*rotation & 1) && (page_img->w > page_img->h))
									scale_width = screen->w*2;
								else if((*rotation & 1) && (page_img->w < page_img->h))
									scale_width = screen->w*2;
								else scale_width = screen->w;
							break;
							default: scale_width = screen->w; break;
						}
						new_surf = (*rotation & 1?
							rotozoom(page_img,NULL,0,scale_width,*rotation):
							rotozoom(page_img,NULL,scale_width,0,*rotation)
						);
						if(!new_surf) return init_event(error_ev);
						scale_width = new_surf->w;
						autozoom_mode = new_zoom_mode;
						real_scale_ratio = new_surf->w / (float)page_img->w;
						if(new_surf->h <= screen->h) offsetY = 0;
						else {
							float result = (centerY * new_surf->h) - screen->h/2;
							offsetY = (result > 0?result:0);
							if((offsetY + screen->h) > new_surf->h) offsetY = new_surf->h - screen->h;
						}
						if((new_surf->w < screen->w) || (new_surf->h < screen->h))
							clear_screen();
						show_image(new_surf,0,offsetY,1);
					} else {
						//Change to fullview mode
						if(new_surf != page_img) SDL_FreeSurface(new_surf);
						*scale_ratio = 1;
						scale_width = page_img->w;
						new_surf = rotozoom(page_img,NULL,page_img->w,page_img->h,*rotation);
						autozoom_mode = full_view;
						if(new_surf->w <= screen->w) offsetX = 0;
						else {
							float result = (centerX * new_surf->w) - screen->w/2;
							offsetX = (result > 0?result:0);
							if((offsetX + screen->w) > new_surf->w) offsetX = new_surf->w - screen->w;
						}
						if(new_surf->h <= screen->h) offsetY = 0;
						else {
							float result = (centerY * new_surf->h) - screen->h/2;
							offsetY = (result > 0?result:0);
							if((offsetY + screen->h) > new_surf->h) offsetY = new_surf->h - screen->h;
						}
						if((new_surf->w < screen->w) || (new_surf->h < screen->h))
							clear_screen();
						show_image(new_surf,offsetX,offsetY,1);
					}
				}
			}
			pending_action = no_event;
			break;
			case open_menu|event_end:
			if(pending_action != open_menu) break;
			if((SDL_GetTicks() - start)/1000 < 2) {
				event menu_command = show_menu(book);
				switch(menu_command.command) {
					case close_book: case aux_command:
					case jump_page: case quit_command:
						if((menu_command.command != aux_command) || (menu_command.int_data == open_book)) {
							ret = menu_command;
							quit = menu_command.command;
						} else free_event(menu_command);
					break;
					case set_pointer:
						if(menu_command.ptr_data == access_int_global(access_pan_rate))
							pan_amount = *access_int_global(access_pan_rate);
						if((menu_command.ptr_data == access_int_global(access_zoom_box_w)) ||
							(menu_command.ptr_data == access_int_global(access_zoom_box_h))) {
							free_event(menu_command);
							zbi.zoom_box_is_active = 0;
						}
					break;
					default: free_event(menu_command); break;
				}
				if(!quit) {
					if((new_surf->w < screen->w) || (new_surf->h < screen->h))
						clear_screen();
					show_image(new_surf,offsetX,offsetY,0);
					if(zbi.zoom_box_is_active)
						draw_zoom_box(page_img,((float)scale_width)/page_img->w,*rotation,zb_offsetX,zb_offsetY,offsetX,offsetY,zb_zoom_level);
					else flip_screen();
				}
			}
			offsetX_shift = offsetY_shift = 0;
			pending_action = no_event;
			break;
			case pan_event: {
				Sint16 joyX = SDL_JoystickGetAxis(joy,0);
				Sint16 joyY = SDL_JoystickGetAxis(joy,1);
				if(joy_in_use) offsetX_shift = offsetY_shift = 0;
				joy_in_use = 0;
				if(abs(joyX) > dead_zone) {
					offsetX_shift = (char) (joyX*joy_sense);
					joy_in_use = 1;
				}
				if(abs(joyY) > dead_zone) {
					offsetY_shift = (char) (joyY*joy_sense);
					joy_in_use = 1;
				}
			} break;
			case redraw_ev:
			show_image(new_surf,offsetX,offsetY,1);
			break;
		}
		if(poll_command & event_end) {
			poll_command ^= event_end;
			if(poll_command & pan_event) {
				delay = 0;
				wait_for_input = 1;
				if(poll_command & pan_horiz) offsetX_shift = 0;
				if(poll_command & pan_vert) offsetY_shift = 0;
			}
		}
		if((offsetX_shift == 0) && (offsetY_shift == 0)) joy_in_use = 0;
		#ifdef PSP
		if(!joy_in_use) {
			if(!SDL_JoystickGetButton(joy,9) && !SDL_JoystickGetButton(joy,7) &&
			  !SDL_JoystickGetButton(joy,8) && !SDL_JoystickGetButton(joy,6)) {
					delay = 0;
					wait_for_input = 1;
			}
			if(!SDL_JoystickGetButton(joy,9) && !SDL_JoystickGetButton(joy,7)) {
					offsetX_shift = 0;
			}
			if(!SDL_JoystickGetButton(joy,8) && !SDL_JoystickGetButton(joy,6)) {
					offsetY_shift = 0;
			}
		}
		#endif
		if(offsetX_shift || offsetY_shift) {
			delay = 50;
			wait_for_input = 0;
			if(!zbi.zoom_box_is_active) {
				Uint16 offsetX_buffer = offsetX, offsetY_buffer = offsetY;
				if((offsetX_shift < 0) && (offsetX < -offsetX_shift))
					offsetX = -offsetX_shift;
				if((offsetY_shift < 0) && (offsetY < -offsetY_shift))
					offsetY = -offsetY_shift;
				if(offsetX_shift) offsetX += offsetX_shift;
				if(offsetY_shift) offsetY += offsetY_shift;
				if(screen->w <= new_surf->w) {
					if (offsetX > new_surf->w - screen->w)
						offsetX = new_surf->w - screen->w;
				}
				else offsetX = 0;
				if(screen->h <= new_surf->h) {
					if (offsetY > new_surf->h - screen->h)
						offsetY = new_surf->h - screen->h;
				}
				else offsetY = 0;
				if((offsetX_buffer != offsetX) || (offsetY_buffer != offsetY)) {
					if((new_surf->w < screen->w) || (new_surf->h < screen->h))
						clear_screen();
					show_image(new_surf,offsetX,offsetY,1);
				}
			} else {
				if((offsetX_shift < 0) && (zb_offsetX < -offsetX_shift)) {
					if(offsetX > 0) {
						Uint16 buffer = -offsetX_shift - zb_offsetX;
						if(buffer > offsetX) offsetX = 0;
						else offsetX -= buffer;
					}
					zb_offsetX = -offsetX_shift;
				}
				if((offsetY_shift < 0) && (zb_offsetY < -offsetY_shift)) {
					if(offsetY > 0) {
						Uint16 buffer = -offsetY_shift - zb_offsetY;
						if(buffer > offsetY) offsetY = 0;
						else offsetY -= buffer;
					}
					zb_offsetY = -offsetY_shift;
				}
				if(offsetX_shift) zb_offsetX += offsetX_shift;
				if(offsetY_shift) zb_offsetY += offsetY_shift;
				int real_zb_w = *access_int_global(access_zoom_box_w)/zb_zoom_level;
				if(real_zb_w > new_surf->w) real_zb_w = new_surf->w;
				int real_zb_h = *access_int_global(access_zoom_box_h)/zb_zoom_level;
				if(real_zb_h > new_surf->h) real_zb_h = new_surf->h;
				if((real_zb_w + zb_offsetX) >= new_surf->w) zb_offsetX = new_surf->w - real_zb_w;
				if((zb_offsetX + real_zb_w) > screen->w) {
					offsetX -= screen->w - zb_offsetX - real_zb_w;
					if(offsetX > (new_surf->w - screen->w)) offsetX = new_surf->w - screen->w;
					zb_offsetX = screen->w - real_zb_w;
				}
				if((real_zb_h + zb_offsetY) >= new_surf->h) zb_offsetY = new_surf->h - real_zb_h;
				if((zb_offsetY + real_zb_h) > screen->h) {
					offsetY -= screen->h - zb_offsetY - real_zb_h;
					if(offsetY > (new_surf->h - screen->h)) offsetY = new_surf->h - screen->h;
					zb_offsetY = screen->h - real_zb_h;
				}
				if((new_surf->w < screen->w) || (new_surf->h < screen->h))
					clear_screen();
				show_image(new_surf,offsetX,offsetY,0);
				draw_zoom_box(page_img,((float)scale_width)/page_img->w,*rotation,zb_offsetX,zb_offsetY,offsetX,offsetY,zb_zoom_level);
			}
		}
		SDL_Delay(delay);
	}
	if(new_surf != page_img) SDL_FreeSurface(new_surf);
	if(ret.command == no_event) ret.command = quit;
	return ret;
}

inline char* init_block(Uint16 kb) {
	Uint32 bytes = kb*1024;
	char* block = malloc(sizeof(char)*bytes);
	if(!block) return NULL;
	return memset(block,0,bytes);
}
inline event init_event(int ev) {
	event ret;
	ret.command = ev;
	ret.option_text = NULL;
	ret.ptr_data = NULL;
	ret.int_data = 0;
	ret.option_color = get_color(color_normal);
	return ret;
}
char ends_with(const char *reference, const char *against) {
	Uint32 agalen = strlen(against);
	if(agalen > strlen(reference)) return -1;
	reference += strlen(reference) - agalen;
	int i;
	for(i = 0; i < agalen; i++)
		if(tolower(against[i]) != tolower(reference[i]))
			return 0;
	return 1;
}
inline char* make_string(const char *original) {
	char* ret = malloc(sizeof(char)*(strlen(original)+1));
	if(!ret) return NULL;
	strcpy(ret,original);
	return ret;
}

comic_book open_comic_book(char *dir, char *filename) {
	comic_book book;
	book.current_page = 0;
	book.num_pages = 0;
	book.page_order = NULL;
	book.zip_file = NULL;
	book.type = invalid_comic_type;
	book.localname = sjis_to_utf8(filename);
	if(!book.localname) {
		error_message(get_message(mesg_out_of_memory));
		return book;
	}
	book.filename = malloc(sizeof(char)*(strlen(dir)+strlen(filename)+1));
	if(!book.filename) {
		free(book.localname);
		book.localname = NULL;
		error_message(get_message(mesg_out_of_memory));
		return book;
	}
	strcpy(book.filename,dir);
	strcat(book.filename,filename);
	book.filename[strlen(dir)+strlen(filename)] = 0;
	DIR *dtest = opendir(book.filename);
	if(dtest) {
		book.type = comic_book_dir;
		closedir(dtest);
	} else {
		FILE *magic_check = fopen(book.filename,"rb");
		if(!magic_check) return book;
		else {
			char magic_buf[4];
			if(fread(magic_buf,1,4,magic_check) == 4) {
				if(strncmp("PK\03\04",magic_buf,4) == 0) book.type = comic_book_zip;
				else if(strncmp("Rar!",magic_buf,4) == 0) book.type = comic_book_rar;
				else if(strncmp("RE~^",magic_buf,4) == 0) book.type = comic_book_rar;
				else if((ends_with(filename,".cbz") > 0) || (ends_with(filename,".zip") > 0))
					book.type = comic_book_zip;
				else if((ends_with(filename,".cbr") > 0) || (ends_with(filename,".rar") > 0))
					book.type = comic_book_rar;
			}
			fclose(magic_check);
		}
	}
	if(book.type == comic_book_zip) {
		book.zip_file = unzOpen(book.filename);
		if(book.zip_file == NULL) {
			error_message(get_message(mesg_open_file_error),"zip");
			book.zip_file = NULL;
			return book;
		}
		int status = UNZ_OK;
		do {
			char *block = init_block(1);
			if(!block) {
				error_message(get_message(mesg_out_of_memory));
				unzClose(book.zip_file);
				book.zip_file = NULL;
				return book;
			}
			unzGetCurrentFileInfo(book.zip_file,NULL,block,1024,NULL,0,NULL,0);
			if(block[1023] != 0) {
				error_message(get_message(mesg_zip_name_too_long_error));
				unzClose(book.zip_file);
				book.zip_file = NULL;
				free(block);
				return book;
			}
			if((ends_with(block,".jpg") > 0) || (ends_with(block,".jpeg") > 0) ||
			  (ends_with(block,".png") > 0) || (ends_with(block,".tiff") > 0) ||
			  (ends_with(block,".bmp") > 0) || (ends_with(block,".gif") > 0))
				book.num_pages++;
			free(block);
		} while((status = unzGoToNextFile(book.zip_file)) == UNZ_OK);
		if(status != UNZ_END_OF_LIST_OF_FILE) {
			error_message(get_message(mesg_read_file_error),"zip");
			unzClose(book.zip_file);
			book.zip_file = NULL;
			book.num_pages = 0;
			return book;
		} else if(book.num_pages < 1) {
			error_message(get_message(mesg_empty_book_error));
			unzClose(book.zip_file);
			book.zip_file = NULL;
			return book;
		}
		book.page_order = calloc(book.num_pages,sizeof(char*));
		if(!book.page_order) {
			book.num_pages = 0;
			free(book.localname);
			book.localname = NULL;
			free(book.filename);
			book.filename = NULL;
			error_message(get_message(mesg_out_of_memory));
			return book;
		}
		int i;
		unzGoToFirstFile(book.zip_file);
		status = UNZ_OK;
		for(i = 0; (status == UNZ_OK) && (i < book.num_pages); ++i) {
			char *block = init_block(1);
			if(!block) {
				error_message(get_message(mesg_out_of_memory));
				book.num_pages = 0;
				unzClose(book.zip_file);
				book.zip_file = NULL;
				return book;
			}
			unzGetCurrentFileInfo(book.zip_file,NULL,block,1023,NULL,0,NULL,0);
			if((ends_with(block,".jpg") > 0) || (ends_with(block,".jpeg") > 0) ||
			  (ends_with(block,".png") > 0) || (ends_with(block,".tiff") > 0) ||
			  (ends_with(block,".bmp") > 0) || (ends_with(block,".gif") > 0))
				book.page_order[i] = block;
			else i--;
			status = unzGoToNextFile(book.zip_file);
		}
		unzGoToFirstFile(book.zip_file);
	} else if(book.type == comic_book_rar) {
		void *rar_file = rar_open(book.filename,RAR_OM_LIST);
		if(!rar_file) {
			error_message(get_message(mesg_open_file_error),"rar");
			book.num_pages = 0;
			return book;
		}
		struct RARHeaderData *hd = calloc(sizeof(struct RARHeaderData),1);
		if(!hd) {
			error_message(get_message(mesg_out_of_memory));
			free(book.localname);
			book.localname = NULL;
			free(book.filename);
			book.filename = NULL;
			return book;
		}
		int status = 0;
		while(!(status = rar_get_header(rar_file,hd))) {
			char *block = hd->FileName;
			if((ends_with(block,".jpg") > 0) || (ends_with(block,".jpeg") > 0) ||
					(ends_with(block,".png") > 0) || (ends_with(block,".tiff") > 0) ||
					(ends_with(block,".bmp") > 0) || (ends_with(block,".gif") > 0))
				book.num_pages++;
			rar_next_file(rar_file);
		}
		rar_close(rar_file);
		if(status != ERAR_END_ARCHIVE) {
			error_message(get_message(mesg_read_file_error),"rar");
			free(book.localname);
			book.localname = NULL;
			free(book.filename);
			book.filename = NULL;
			book.num_pages = 0;
			return book;
		} else if(book.num_pages < 1) {
			error_message(get_message(mesg_empty_book_error));
			free(book.localname);
			book.localname = NULL;
			free(book.filename);
			book.filename = NULL;
			return book;
		}
		rar_file = rar_open(book.filename,RAR_OM_LIST);
		book.page_order = calloc(book.num_pages,sizeof(char*));
		if(!book.page_order) {
			book.num_pages = 0;
			free(book.localname);
			book.localname = NULL;
			free(book.filename);
			book.filename = NULL;
			error_message(get_message(mesg_out_of_memory));
			return book;
		}
		int i;
		for(i = 0; (i < book.num_pages) && !rar_get_header(rar_file,hd); ++i) {
			char *block = hd->FileName;
			if((ends_with(block,".jpg") > 0) || (ends_with(block,".jpeg") > 0) ||
					(ends_with(block,".png") > 0) || (ends_with(block,".tiff") > 0) ||
					(ends_with(block,".bmp") > 0) || (ends_with(block,".gif") > 0)) {
				book.page_order[i] = make_string(block);
				if(!book.page_order[i]) {
					error_message(get_message(mesg_out_of_memory));
					book.num_pages = 0;
					free(book.localname);
					book.localname = NULL;
					free(book.filename);
					book.filename = NULL;
					int j;
					for(j = 0; j < i; j++) free(book.page_order[j]);
					free(book.page_order);
					book.page_order = NULL;
					return book;
				}
			}
			else i--;
			rar_next_file(rar_file);
		}
		rar_close(rar_file);
		free(hd);
	} else {
		DIR *d = opendir(book.filename);
		if(!d) {
			book.num_pages = 0;
			free(book.localname);
			book.localname = NULL;
			free(book.filename);
			book.filename = NULL;
			return book;
		}
		book.type = comic_book_dir;
		struct dirent *filent;
		while((filent = readdir(d))) {
			char *real = dir_real_name(filent);
			if(!real) {
				closedir(d);
				book.num_pages = 0;
				free(book.localname);
				book.localname = NULL;
				free(book.filename);
				book.filename = NULL;
				return book;
			}
			if((ends_with(real,".jpg") > 0) || (ends_with(real,".jpeg") > 0) ||
					(ends_with(real,".png") > 0) || (ends_with(real,".tiff") > 0) ||
					(ends_with(real,".bmp") > 0) || (ends_with(real,".gif") > 0))
				++book.num_pages;
			free(real);
		}
		closedir(d);
		book.page_order = calloc(book.num_pages,sizeof(char*));
		if(!book.page_order) {
			book.num_pages = 0;
			free(book.localname);
			book.localname = NULL;
			free(book.filename);
			book.filename = NULL;
			error_message(get_message(mesg_out_of_memory));
			book.num_pages = 0;
			return book;
		}
		d = opendir(book.filename);
		if(!d) {
			free(book.localname);
			book.localname = NULL;
			free(book.filename);
			book.filename = NULL;
			free(book.page_order);
			book.page_order = NULL;
			book.num_pages = 0;
			return book;
		}
		int i;
		for(i = 0; (filent = readdir(d)) && (i < book.num_pages); ++i) {
			char *real = dir_real_name(filent);
			if(!real) {
				closedir(d);
				book.num_pages = 0;
				free(book.localname);
				book.localname = NULL;
				free(book.filename);
				book.filename = NULL;
				int j;
				for(j = 0; j < i; ++j) free(book.page_order[j]);
				free(book.page_order);
				return book;
			}
			if((ends_with(real,".jpg") > 0) || (ends_with(real,".jpeg") > 0) ||
					(ends_with(real,".png") > 0) || (ends_with(real,".tiff") > 0) ||
					(ends_with(real,".bmp") > 0) || (ends_with(real,".gif") > 0))
				book.page_order[i] = real;
			else {
				--i;
				free(real);
			}
		}
		closedir(d);
		book.num_pages = i;
		if((strcmp(".",book.localname) == 0) && (book.filename[0] != '/')) {
			free(book.localname);
			book.localname = strrchr(book.filename,'/');
			book.localname[0] = '\0';
			book.localname = strrchr(book.filename,'/');
			book.localname = make_string(book.localname+1);
		}
	}
	qsort(book.page_order,book.num_pages,sizeof(char*),cb_compare);
	return book;
}
void close_comic_book(comic_book book) {
	if(book.type == comic_book_zip)
		unzClose(book.zip_file);
	int i;
	for(i = 0; i < book.num_pages; i++) {
		if(book.page_order[i] != NULL) free(book.page_order[i]);
	}
	free(book.page_order);
	free(book.filename);
	free(book.localname);
}
comic_book goto_next_page(comic_book book) {
	if(book.current_page >= book.num_pages-1) {
		book.current_page = book.num_pages-1;
	} else book.current_page++;
	return book;
}
comic_book goto_prev_page(comic_book book) {
	if(book.current_page <= 0) {
		book.current_page = 0;
	} else book.current_page--;
	return book;
}
comic_book goto_page(comic_book book, Uint16 page) {
	if(page < book.num_pages) book.current_page = page;
	return book;
}

event show_menu(comic_book *book) {
	char book_is_open = book != NULL;
	{	//More stack fun! Check to make sure the cwd actually exists.
		DIR *dtest = opendir(cwd);
		if(!dtest) {
			free(cwd);
			cwd = get_full_path("/");
			if(!cwd) return init_event(error_ev);
		} else closedir(dtest);
	}
	char *lwd = make_string(cwd); //Local working directory
	if(!lwd) return init_event(error_ev);
	device acc = access_device(access_joystick);
	SDL_Joystick *joy = acc.joystick;
	acc = access_device(access_screen);
	SDL_Surface *screen = acc.screen;
	SDL_Surface *bg = get_background();
	SDL_Surface *usb = get_image(image_usb);
	if(bg) show_image(bg,0,0,1);
	else {
		clear_screen();
		flip_screen();
	}
	Uint16 current_option = 0, num_options = 0,
		current_submenu = (book_is_open?menu_in_book:menu_open_book),
		supermenu = menu_invalid, scroll_offset = 0;
	Sint32 minimum = 0, maximum = 0;
	int menu_type;
	static char run_battery_meter = 1;
		//This is static because it is passed to as a pointer to a thread that
		//may remain running after the function exits.
	run_battery_meter = 1; //It needs to be reset every time the function runs
	/*SDL_Thread *bmt = */ SDL_CreateThread(battery_meter_thread,&run_battery_meter);
	event action, *options = NULL;
	char usb_on = 0;
	action = init_event(no_event);
	while(action.command == no_event) {
		current_option = 0;
		switch(current_submenu) {
			case menu_open_book: { //Open book menu
				supermenu = (book_is_open?menu_in_book:menu_invalid);
				menu_type = option_menu;
				Uint16 num_books;
				event *options_buf = NULL;
				num_books = file_list(lwd,&options_buf,1);
				num_options = 3;
				num_options += num_books;
				if(!book_is_open) {
					if(num_books) num_options += 2;
				}
				options = malloc(sizeof(event)*num_options);
				if(!options) {
					run_battery_meter = 0;
					free(lwd);
					if(usb_on) stop_usb();
					error_message(get_message(mesg_out_of_memory));
					return init_event(error_ev);
				}

				if(num_books) {
					if(!book_is_open) {
						options[0].option_text = make_string(get_message(mesg_load_last));
						options[0].ptr_data = (int*)make_string("last_book");
						options[0].command = aux_command;
						options[0].int_data = open_book_by_id;
						options[0].option_color = get_color(color_normal);
		
						options[1].option_text = make_string(get_message(mesg_load_saved));
						options[1].ptr_data = (int*)make_string("saved_book");
						options[1].command = aux_command;
						options[1].int_data = open_book_by_id;
						options[1].option_color = get_color(color_normal);
					}
					unsigned lfod = (book_is_open?0:2);
					options[lfod].option_text = make_string(get_message(mesg_open_dir));
					options[lfod].ptr_data = (int*)make_string(".");
					options[lfod].command = aux_command;
					options[lfod].int_data = open_book;
					options[lfod].option_color = get_color(color_normal);

					memcpy(options+(book_is_open?1:3),options_buf,num_books*sizeof(event));
					free(options_buf);
				} else {
					options[0].option_text = make_string(get_message(mesg_no_comics));
					options[0].ptr_data = NULL;
					options[0].int_data = 0;
					options[0].command = no_event;
					options[0].option_color = get_color(color_normal);

					char *real_cb_dir = get_full_path(cb_dir);
					if(!real_cb_dir) {
						run_battery_meter = 0;
						free(options);
						free(lwd);
						if(usb_on) stop_usb();
						error_message(get_message(mesg_out_of_memory));
						return init_event(error_ev);
						
					}
					make_dir(real_cb_dir);
					free(real_cb_dir);
				}
				options[num_options-2].option_text = make_string(get_message(mesg_reload_menu));
				options[num_options-2].ptr_data = NULL;
				options[num_options-2].int_data = 0;
				options[num_options-2].command = open_menu;
				options[num_options-2].option_color = get_color(color_normal);
			} break;
			case menu_in_book: { //In-book menu
				supermenu = menu_invalid;
				menu_type = option_menu;
				num_options = 14;
				options = malloc(sizeof(event)*num_options);
				if(!options) {
					run_battery_meter = 0;
					reset_text_pos();
					free(lwd);
					if(usb_on) stop_usb();
					error_message(get_message(mesg_out_of_memory));
					return init_event(error_ev);
				}
				options[0].option_text = make_string(get_message(mesg_open_book));
				options[0].command = open_submenu;
				options[0].int_data = menu_open_book;
				options[0].ptr_data = NULL;
				options[0].option_color = get_color(color_normal);

				options[1].option_text = make_string(get_message(mesg_close_book));
				options[1].command = close_book;
				options[1].int_data = 0;
				options[1].ptr_data = NULL;
				options[1].option_color = get_color(color_normal);

				options[2].option_text = make_string(get_message(mesg_next_book));
				options[2].command = aux_command;
				options[2].int_data = next_book;
				options[2].ptr_data = NULL;
				options[2].option_color = get_color(color_normal);

				options[3].option_text = make_string(get_message(mesg_prev_book));
				options[3].command = aux_command;
				options[3].int_data = prev_book;
				options[3].ptr_data = NULL;
				options[3].option_color = get_color(color_normal);

				options[4].option_text = make_string(get_message(mesg_jump_page));
				options[4].command = open_submenu;
				options[4].int_data = menu_jump_page;
				options[4].ptr_data = NULL;
				options[4].option_color = get_color(color_normal);

				options[5].option_text = make_string(get_message(mesg_load_bookmark));
				options[5].command = aux_command;
				options[5].int_data = retrieve_bookmark;
				options[5].ptr_data = NULL;
				options[5].option_color = get_color(color_normal);

				options[6].option_text = make_string(get_message(mesg_set_bookmark));
				options[6].command = aux_command;
				options[6].int_data = add_bookmark;
				options[6].ptr_data = NULL;
				options[6].option_color = get_color(color_normal);

				options[7].option_text = make_string(get_message(mesg_load_saved));
				options[7].ptr_data = (int*)make_string("saved_book");
				options[7].command = aux_command;
				options[7].int_data = open_book_by_id;
				options[7].option_color = get_color(color_normal);

				options[8].option_text = make_string(get_message(mesg_set_saved));
				options[8].ptr_data = NULL;
				options[8].command = aux_command;
				options[8].int_data = save_book_command;
				options[8].option_color = get_color(color_normal);

				options[9].option_text = make_string(get_message(mesg_more_bookmark));
				options[9].command = open_submenu;
				options[9].int_data = menu_bookmark;
				options[9].ptr_data = NULL;
				options[9].option_color = get_color(color_normal);

				options[10].option_text = make_string(get_message(mesg_config));
				options[10].command = open_submenu;
				options[10].int_data = menu_config;
				options[10].ptr_data = NULL;
				options[10].option_color = get_color(color_normal);

				options[11].option_text = make_string(get_message(mesg_about));
				options[11].command = popup;
				options[11].int_data = 0;
				options[11].ptr_data = NULL;
				options[11].option_color = get_color(color_normal);

				options[12].option_text = make_string(get_message(mesg_quit));
				options[12].command = quit_command;
				options[12].int_data = 0;
				options[12].ptr_data = NULL;
				options[12].option_color = get_color(color_normal);
			} break;
			case menu_jump_page: { //Jump to page menu
				supermenu = menu_in_book;
				menu_type = option_menu;
				num_options = book->num_pages + 1;
				options = malloc(sizeof(event)*num_options);
				if(!options) {
					run_battery_meter = 0;
					reset_text_pos();
					free(lwd);
					if(usb_on) stop_usb();
					error_message(get_message(mesg_out_of_memory));
					return init_event(error_ev);
				}

				int i;
				char *buffer, *buffer2;
				for(i = 0; i < book->num_pages; i++) {
					buffer = malloc(sizeof(char)*80);
					if(!buffer) {
						run_battery_meter = 0;
						reset_text_pos();
						int j;
						for(j = 0; j < i; j++) free_event(options[j]);
						free(options);
						free(lwd);
						if(usb_on) stop_usb();
						error_message(get_message(mesg_out_of_memory));
						return init_event(error_ev);
					}
					memset(buffer,0,sizeof(char)*80);
					sprintf(buffer,get_message(mesg_page),i+1);
					buffer2 = strrchr(book->page_order[i],'/');
					if(!buffer2) buffer2 = strrchr(book->page_order[i],'\\');
					strncat(buffer,(buffer2?buffer2+1:book->page_order[i]),80-strlen(buffer));
					options[i].option_text = make_string(ellipsis(buffer,0));
					free(buffer);
					options[i].command = jump_page;
					options[i].int_data = i;
					options[i].ptr_data = NULL;
					options[i].option_color = (i == book->current_page?
						get_color(color_secondary):get_color(color_normal));
				}
			} break;
			case menu_adjust_clock: { //Adjust clock frequency
				supermenu = menu_config;
				menu_type = number_menu;
				num_options = 1;
				options = malloc(sizeof(event));
				if(!options) {
					run_battery_meter = 0;
					reset_text_pos();
					free(lwd);
					if(usb_on) stop_usb();
					error_message(get_message(mesg_out_of_memory));
					return init_event(error_ev);
				}
				options->option_text = make_string(get_message(mesg_adjust_clock_top));
				options->command = clock_adjust;
				options->int_data =
				#ifdef PSP
				scePowerGetCpuClockFrequency();
				#else
				222;
				#endif
				options->ptr_data = NULL;
				maximum = 333;
				minimum = 20;
			} break;
			case menu_config: { //Config menu
				supermenu = menu_in_book;
				menu_type = option_menu;
				num_options = 19;
				options = malloc(sizeof(event)*num_options);
				if(!options) {
					run_battery_meter = 0;
					reset_text_pos();
					free(lwd);
					if(usb_on) stop_usb();
					error_message(get_message(mesg_out_of_memory));
					return init_event(error_ev);
				}
				options[0].option_text = (*access_int_global(access_resize)?
					make_string(get_message(mesg_resize_to_nn)):
					make_string(get_message(mesg_resize_to_resample)));
				options[0].command = toggle_global;
				options[0].int_data = access_resize;
				options[0].ptr_data = NULL;
				options[0].option_color = get_color(color_normal);

				options[1].option_text = make_string(get_message(mesg_adjust_clock));
				options[1].command = open_submenu;
				options[1].int_data = menu_adjust_clock;
				options[1].ptr_data = NULL;
				options[1].option_color = get_color(color_normal);

				options[2].option_text = make_string(get_message(mesg_adjust_pan));
				options[2].command = open_submenu;
				options[2].int_data = menu_adjust_pan;
				options[2].ptr_data = NULL;
				options[2].option_color = get_color(color_normal);

				options[3].option_text = (*access_int_global(access_zoom_persist)?
					make_string(get_message(mesg_disable_zoom_persist)):
					make_string(get_message(mesg_enable_zoom_persist))
				);
				options[3].command = toggle_global;
				options[3].int_data = access_zoom_persist;
				options[3].ptr_data = NULL;
				options[3].option_color = get_color(color_normal);

				options[4].option_text = (*access_int_global(access_rotate_persist)?
					make_string(get_message(mesg_disable_rotate_persist)):
					make_string(get_message(mesg_enable_rotate_persist))
				);
				options[4].command = toggle_global;
				options[4].int_data = access_rotate_persist;
				options[4].ptr_data = NULL;
				options[4].option_color = get_color(color_normal);

				options[5].option_text = make_string(get_message(mesg_adjust_scroll_skip));
				options[5].command = open_submenu;
				options[5].int_data = menu_adjust_scroll_skip;
				options[5].ptr_data = NULL;
				options[5].option_color = get_color(color_normal);

				options[6].option_text = (*access_int_global(access_manga_mode)?
					make_string(get_message(mesg_disable_manga_mode)):
					make_string(get_message(mesg_enable_manga_mode))
				);
				options[6].command = toggle_global;
				options[6].int_data = access_manga_mode;
				options[6].ptr_data = NULL;
				options[6].option_color = get_color(color_normal);

				options[7].option_text = (*access_int_global(access_bookmark_on_load)?
					make_string(get_message(mesg_disable_bookmark_on_load)):
					make_string(get_message(mesg_enable_bookmark_on_load))
				);
				options[7].command = toggle_global;
				options[7].int_data = access_bookmark_on_load;
				options[7].ptr_data = NULL;
				options[7].option_color = get_color(color_normal);

				options[8].option_text = make_string(get_message(mesg_set_autozoom));
				options[8].command = open_submenu;
				options[8].int_data = menu_autozoom;
				options[8].ptr_data = NULL;
				options[8].option_color = get_color(color_normal);

				options[9].option_text = make_string(get_message(mesg_set_zoom_box_w));
				options[9].command = open_submenu;
				options[9].int_data = menu_adjust_zb_w;
				options[9].ptr_data = NULL;
				options[9].option_color = get_color(color_normal);

				options[10].option_text = make_string(get_message(mesg_set_zoom_box_h));
				options[10].command = open_submenu;
				options[10].int_data = menu_adjust_zb_h;
				options[10].ptr_data = NULL;
				options[10].option_color = get_color(color_normal);

				options[11].option_text = (*access_int_global(access_precaching)?
					make_string(get_message(mesg_disable_precaching)):
					make_string(get_message(mesg_enable_precaching))
				);
				options[11].command = toggle_global;
				options[11].int_data = access_precaching;
				options[11].ptr_data = NULL;
				options[11].option_color = get_color(color_normal);

				options[12].option_text = (*access_int_global(access_singlehanded)?
					make_string(get_message(mesg_disable_singlehanded)):
					make_string(get_message(mesg_enable_singlehanded))
				);
				options[12].command = toggle_global;
				options[12].int_data = access_singlehanded;
				options[12].ptr_data = NULL;
				options[12].option_color = get_color(color_normal);

				options[13].option_text = (*access_int_global(access_analog_disabled)?
					make_string(get_message(mesg_enable_analog)):
					make_string(get_message(mesg_disable_analog))
				);
				options[13].command = toggle_global;
				options[13].int_data = access_analog_disabled;
				options[13].ptr_data = NULL;
				options[13].option_color = get_color(color_normal);

				/*options[14].option_text = (*access_int_global(access_dynamic_cpu)?
					make_string(get_message(mesg_disable_dynamic_cpu)):
					make_string(get_message(mesg_enable_dynamic_cpu))
				);
				options[14].command = toggle_global;
				options[14].int_data = access_dynamic_cpu;
				options[14].ptr_data = NULL;
				options[14].option_color = get_color(color_normal);*/

				options[14].option_text = make_string(get_message(mesg_set_theme));
				options[14].command = open_submenu;
				options[14].int_data = menu_theme;
				options[14].ptr_data = NULL;
				options[14].option_color = get_color(color_normal);

				options[15].option_text = make_string(get_message(mesg_set_language));
				options[15].command = open_submenu;
				options[15].int_data = menu_language;
				options[15].ptr_data = NULL;
				options[15].option_color = get_color(color_normal);

				options[16].option_text = make_string(get_message(mesg_save_config));
				options[16].command = aux_command;
				options[16].int_data = save_config_command;
				options[16].ptr_data = NULL;
				options[16].option_color = get_color(color_normal);

				options[17].option_text = make_string(get_message(mesg_load_config));
				options[17].command = aux_command;
				options[17].int_data = load_config_command;
				options[17].ptr_data = NULL;
				options[17].option_color = get_color(color_normal);
			} break;
			case menu_adjust_pan: { //Adjust pan rate menu
				supermenu = menu_config;
				menu_type = number_menu;
				maximum = pan_max;
				minimum = pan_min;
				options = malloc(sizeof(event));
				if(!options) {
					run_battery_meter = 0;
					reset_text_pos();
					free(lwd);
					if(usb_on) stop_usb();
					error_message(get_message(mesg_out_of_memory));
					return init_event(error_ev);
				}
				num_options = 1;

				options->option_text = make_string(get_message(mesg_adjust_pan_top));
				options->command = set_pointer;
				options->ptr_data = access_int_global(access_pan_rate);
				options->int_data = *options->ptr_data;
			} break;
			case menu_adjust_scroll_skip: { //Adjust scroll skip amount
				supermenu = menu_config;
				menu_type = number_menu;
				maximum = 99;
				minimum = 1;
				options = malloc(sizeof(event));
				if(!options) {
					run_battery_meter = 0;
					reset_text_pos();
					free(lwd);
					if(usb_on) stop_usb();
					error_message(get_message(mesg_out_of_memory));
					return init_event(error_ev);
				}
				num_options = 1;

				options->option_text = make_string(get_message(mesg_adjust_scroll_skip_top));
				options->command = set_pointer;
				options->ptr_data = access_int_global(access_scroll_skip);
				options->int_data = *options->ptr_data;
			} break;
			case menu_bookmark: { //Bookmark menu
				supermenu = menu_in_book;
				menu_type = option_menu;
				num_options = 4;
				options = malloc(sizeof(event)*num_options);
				if(!options) {
					run_battery_meter = 0;
					reset_text_pos();
					free(lwd);
					if(usb_on) stop_usb();
					error_message(get_message(mesg_out_of_memory));
					return init_event(error_ev);
				}

				options[0].option_text = make_string(get_message(mesg_delete_bookmarks));
				options[0].command = aux_command;
				options[0].int_data = delete_bookmarks_command;
				options[0].ptr_data = NULL;
				options[0].option_color = get_color(color_normal);

				options[1].option_text = make_string(get_message(mesg_delete_all_bookmarks));
				options[1].command = aux_command;
				options[1].int_data = delete_all_bookmarks_command;
				options[1].ptr_data = NULL;
				options[1].option_color = get_color(color_normal);

				options[2].option_text = make_string(get_message(mesg_purge_bookmarks));
				options[2].command = aux_command;
				options[2].int_data = purge_bookmarks_command;
				options[2].ptr_data = NULL;
				options[2].option_color = get_color(color_normal);
			} break;
			case menu_adjust_zb_w: { //Set zoom box width
				supermenu = menu_config;
				menu_type = number_menu;
				maximum = 180;
				minimum = 20;
				options = malloc(sizeof(event));
				if(!options) {
					run_battery_meter = 0;
					reset_text_pos();
					free(lwd);
					if(usb_on) stop_usb();
					error_message(get_message(mesg_out_of_memory));
					return init_event(error_ev);
				}
				num_options = 1;

				options->option_text = make_string(get_message(mesg_adjust_zoom_box_w_top));
				options->command = set_pointer;
				options->ptr_data = access_int_global(access_zoom_box_w);
				options->int_data = *options->ptr_data;
			} break;
			case menu_adjust_zb_h: { //Set zoom box height
				supermenu = menu_config;
				menu_type = number_menu;
				maximum = 250;
				minimum = 20;
				options = malloc(sizeof(event));
				if(!options) {
					run_battery_meter = 0;
					reset_text_pos();
					free(lwd);
					if(usb_on) stop_usb();
					error_message(get_message(mesg_out_of_memory));
					return init_event(error_ev);
				}
				num_options = 1;

				options->option_text = make_string(get_message(mesg_adjust_zoom_box_h_top));
				options->command = set_pointer;
				options->ptr_data = access_int_global(access_zoom_box_h);
				options->int_data = *options->ptr_data;
			} break;
			case menu_autozoom: { //Autozoom mode
				supermenu = menu_config;
				menu_type = option_menu;
				num_options = 5;
				options = malloc(sizeof(event)*num_options);
				if(!options) {
					run_battery_meter = 0;
					reset_text_pos();
					free(lwd);
					if(usb_on) stop_usb();
					error_message(get_message(mesg_out_of_memory));
					return init_event(error_ev);
				}

				options[0].option_text = make_string(get_message(mesg_full_width));
				options[0].command = set_pointer;
				options[0].ptr_data = access_int_global(access_autozoom_mode);
				options[0].int_data = full_width;
				options[0].option_color = get_color(color_normal);

				options[1].option_text = make_string(get_message(mesg_full_view));
				options[1].command = set_pointer;
				options[1].ptr_data = access_int_global(access_autozoom_mode);
				options[1].int_data = full_view;
				options[1].option_color = get_color(color_normal);

				options[2].option_text = make_string(get_message(mesg_twice_width));
				options[2].command = set_pointer;
				options[2].ptr_data = access_int_global(access_autozoom_mode);
				options[2].int_data = twice_width;
				options[2].option_color = get_color(color_normal);

				options[3].option_text = make_string(get_message(mesg_autodetect));
				options[3].command = set_pointer;
				options[3].ptr_data = access_int_global(access_autozoom_mode);
				options[3].int_data = autodetect_zoom;
				options[3].option_color = get_color(color_normal);
			} break;
			case menu_theme: { //Set theme menu
				supermenu = menu_config;
				menu_type = option_menu;
				Uint16 num_themes;
				event *options_buf = NULL;
				num_themes = theme_list(&options_buf);
				num_options = (num_themes?num_themes:1)+2;
				options = malloc(sizeof(event)*num_options);
				if(!options) {
					run_battery_meter = 0;
					reset_text_pos();
					free(lwd);
					if(usb_on) stop_usb();
					error_message(get_message(mesg_out_of_memory));
					return init_event(error_ev);
				}

				if(num_themes) {
					memcpy(options,options_buf,num_themes*sizeof(event));
					free(options_buf);
				} else {
					options[0].option_text = make_string(get_message(mesg_no_themes));
					options[0].ptr_data = NULL;
					options[0].int_data = 0;
					options[0].command = no_event;
					options[0].option_color = get_color(color_normal);
				}
				options[num_options-2].option_text = make_string(get_message(mesg_reload_menu));
				options[num_options-2].ptr_data = NULL;
				options[num_options-2].int_data = 0;
				options[num_options-2].command = open_menu;
				options[num_options-2].option_color = get_color(color_normal);
			} break;
			case menu_language: { //Set language menu
				supermenu = menu_config;
				menu_type = option_menu;
				Uint16 num_langs;
				event *options_buf = NULL;
				num_langs = language_list(&options_buf);
				num_options = num_langs+3;
				options = malloc(sizeof(event)*num_options);
				if(!options) {
					run_battery_meter = 0;
					reset_text_pos();
					free(lwd);
					if(usb_on) stop_usb();
					error_message(get_message(mesg_out_of_memory));
					return init_event(error_ev);
				}

				options[0].option_text = make_string("English (built-in)");
				options[0].ptr_data = (int*)make_string("builtin");
				options[0].int_data = set_language;
				options[0].command = aux_command;
				options[0].option_color = get_color(color_normal);
				if(num_langs) {
					memcpy((event*)options+1,options_buf,num_langs*sizeof(event));
					free(options_buf);
				}
				options[num_options-2].option_text = make_string(get_message(mesg_reload_menu));
				options[num_options-2].ptr_data = NULL;
				options[num_options-2].int_data = 0;
				options[num_options-2].command = open_menu;
				options[num_options-2].option_color = get_color(color_normal);
			} break;
			default:
				if(usb_on) stop_usb();
				error_message(get_message(mesg_internal_error));
				return init_event(error_ev);
			break;
		}
		if(menu_type == option_menu) {
			event *last_option = options + num_options - 1;
			last_option->option_text = make_string(get_message(mesg_go_back));
			last_option->ptr_data = NULL;
			last_option->option_color = get_color(color_secondary);
			if(supermenu) {
				last_option->command = open_submenu;
				last_option->int_data = supermenu;
			} else {
				last_option->command = event_end;
				last_option->int_data = 0;
			}
		}
		//Menu display/interaction
		//NOTE: Do not free action if it is changed in this section of the code,
		//as it will be freed with the other options later if it is changed.
		int poll_command;
		char redraw_menu = 1, scrolling = 0, wait_for_input = 1;
		switch(menu_type) {
			case option_menu:
			while(action.command == no_event) {
				if(redraw_menu) {
					//TODO: Make work with multiline options instead of truncating?
					run_battery_meter = 2;
					if(bg) show_image(bg,0,0,0);
					else clear_screen();
					if(usb && usb_on) {
						SDL_Rect dest = { .x = (screen->w-usb->w)/2 };
						show_image2(usb,NULL,&dest,0,1,0);
					}
					battery_meter();
					reset_text_pos();
					if(scroll_offset > 0) {
						set_explicit_color(get_color(color_tertiary));
						say(get_message(mesg_more_above));
					}
					Uint16 i, i_max = (options_per_page()<(num_options-scroll_offset)
						?options_per_page():num_options-scroll_offset);
					for(i = 0; i < i_max; i++) {
						next_line();
						int j = i + scroll_offset;
						set_explicit_color(options[j].option_color);
						if(j == current_option) {
							unset_explicit_color();
							selection_on = 1;
						}
						char *ellipsised = ellipsis(make_string(options[j].option_text),0);
						say("%s",ellipsised);
						free(ellipsised);
						selection_on = 0;
					}
					if((num_options-scroll_offset)>options_per_page()) {
						next_line();
						set_explicit_color(get_color(color_tertiary));
						say(get_message(mesg_more_below));
					}
					unset_explicit_color();
					if(book) {
						char *status = init_block(2);
						char *page_info = init_block(1);
						if(status && page_info) {
							int result = snprintf(page_info,1024,": %u/%u",book->current_page + 1,book->num_pages);
							if(result == 1024) page_info[1023] = 0;
							int w1, w2;
							get_text_dimensions(page_info,&w1,NULL);
							get_text_dimensions(get_message(mesg_more_below),&w2,NULL);
							strcpy(status,book->localname);
							ellipsis(status,w1+w2);
							strncat(status,page_info,1024);
							free(page_info);
							status_message(status);
							free(status);
						} else if(status) free(status);
					}
					redraw_menu = 0;
					run_battery_meter = 1;
					flip_screen();
				}
				if(scrolling) SDL_Delay(menu_scroll_rate);
				poll_command = handle_input(wait_for_input);
				switch(poll_command) {
					case select_option|event_end: action = options[current_option]; break;
					case go_back|event_end: {
						if(current_submenu == menu_open_book) {
							free(lwd);
							lwd = make_string(cwd);
						}
						if(supermenu) {
							action.command = open_submenu;
							action.int_data = supermenu;
						} else action.command = event_end;
					} break;
					case pan_down: {
						scrolling = 1;
						wait_for_input = 0;
					} break;
					case pan_up: {
						scrolling = -1;
						wait_for_input = 0;
					} break;
					case next_page: {
						scrolling = *access_int_global(access_scroll_skip);
						if(scrolling > options_per_page()-1) scrolling = options_per_page()-1;
						wait_for_input = 0;
					} break;
					case prev_page: {
						scrolling = -*access_int_global(access_scroll_skip);
						if(-scrolling > options_per_page()-1) scrolling = -options_per_page()+1;
						wait_for_input = 0;
					} break;
					case open_menu|event_end:
						if(book) action.command = event_end;
					break;
					case rotate_ccw|event_end:
					switch(current_submenu) {
						case menu_open_book:
							if(strchr(lwd,'/') == strrchr(lwd,'/')) break;
							action.command = aux_command;
							action.int_data = open_book;
							action.ptr_data = (int*)make_string("../");
							current_option = 0;
						break;
						case menu_jump_page:
							if(book) {
								Sint32 delta = (Sint32)book->current_page - current_option;
								current_option = book->current_page;
								if(num_options > options_per_page()) {
									if(delta > 0) {
										if((current_option + 1) > (scroll_offset + options_per_page()))
											scroll_offset = (1 + current_option) - options_per_page();
										else if(((Sint32)current_option - delta) < 0) scroll_offset = 0;
									} else {
										if(current_option < scroll_offset) scroll_offset = current_option;
										else if(((Sint32)current_option - delta) >= num_options)
											scroll_offset = num_options - options_per_page();
									}
								}
							}
							redraw_menu = 1;
						break;
						case menu_theme: {
							char *current_theme = make_string(get_loaded_theme_name());
							run_battery_meter = 0;
							unload_theme();
							if(load_theme((char*) options[current_option].ptr_data) != 1) {
								if(load_theme(current_theme) != 1) {
									error_message(get_message(mesg_load_theme_error));
									show_popup(1);
									exit(2);
								}
								error_message(get_message(mesg_load_theme_error));
							}
							free(current_theme);
							bg = get_background();
							action = init_event(event_end);
						} break;
					}
					break;
					case zoom_fixed|event_end:
						if(usb_on) {
							stop_usb();
							usb_on = 0;
						} else usb_on = (start_usb() >= 0);
						redraw_menu = 1;
					break;
					case redraw_ev: redraw_menu = 1; break;
				}
				if(scrolling) {
					#ifdef PSP
					if(!SDL_JoystickGetButton(joy,8) &&
					  !SDL_JoystickGetButton(joy,6) &&
					  !SDL_JoystickGetButton(joy,4) &&
					  !SDL_JoystickGetButton(joy,5)
					) {
						scrolling = 0;
						wait_for_input = 1;
					}
					else {
					#endif
						current_option = (current_option + num_options + scrolling) % num_options;
						redraw_menu = 1;
						//Adjust menu position
						if(num_options > options_per_page()) {
							if(scrolling > 0) {
								if((current_option + 1) > (scroll_offset + options_per_page()))
									scroll_offset = (1 + current_option) - options_per_page();
								else if(((Sint32)current_option - scrolling) < 0) scroll_offset = 0;
							} else {
								if(current_option < scroll_offset) scroll_offset = current_option;
								else if(((Sint32)current_option - scrolling) >= num_options)
									scroll_offset = num_options - options_per_page();
							}
						}
					#ifndef PSP
						scrolling = 0;
						wait_for_input = 1;
					#else
					}
					#endif
				}
			} break;
			case number_menu: {
				Uint8 num_digits0, num_digits1, num_digits;
				Sint32 max_buf = maximum, min_buf = minimum,
					value = options->int_data, init_value = value;
				for(num_digits0 = 0; max_buf; max_buf /= 10) num_digits0++;
				for(num_digits1 = 0; min_buf; min_buf /= 10) num_digits1++;
				num_digits = (num_digits0 > num_digits1?num_digits0:num_digits1);
				while(action.command == no_event) {
					if(redraw_menu) {
						run_battery_meter = 2;
						if(bg) show_image(bg,0,0,0);
						else clear_screen();
						if(usb && usb_on) {
							SDL_Rect dest = { .x = (screen->w-usb->w)/2 };
							show_image2(usb,NULL,&dest,0,1,0);
						}
						battery_meter();
						if(options->option_text) {
							int w, h;
							get_text_dimensions(options->option_text,&w,&h);
							setXY((screen->w - w)/2,(screen->h - (h*7)/2)/2);
							say("%s",options->option_text);
						}
						next_line();
						char *full_string = malloc(sizeof(char)*(2+num_digits));
						char *sprintf_buffer = malloc(sizeof(char)*7);
						if(full_string && sprintf_buffer) {
							sprintf(sprintf_buffer,"%%c%%0%uu",num_digits);
							sprintf(full_string,sprintf_buffer,value>=0?'+':'-',value);
							free(sprintf_buffer);
							int w;
							get_text_dimensions(full_string,&w,NULL);
							setXY((screen->w - w)/2,getY());
							free(full_string);
						} else {
							if(full_string) free(full_string);
							if(sprintf_buffer) free(sprintf_buffer);
						}
						if(current_option == 2) selection_on = 1;
						if(value < 0) say("-");
						else say("+");
						selection_on = 0;
						int i;
						for(i = num_digits-1; i >= 0; i--) {
							int j = value, k;
							if(j < 0) j = -j;
							for(k = i; k > 0; --k) {
								j /= 10;
							}
							if(current_option == num_digits-i+2) selection_on = 1;
							char digit = j%10;
							say("%u",digit);
							selection_on = 0;
							j /= 10;
						}
						next_line();
						int w1, w2, w3;
						get_text_dimensions(get_message(mesg_accept),&w1,NULL);
						get_text_dimensions(" ",&w2,NULL);
						get_text_dimensions(get_message(mesg_cancel),&w3,NULL);
						setXY((screen->w - w1 - w2 - w3)/2,getY());
						if(current_option == 0) selection_on = 1;
						say(get_message(mesg_accept));
						selection_on = 0;
						say(" ");
						if(current_option == 1) selection_on = 1;
						say(get_message(mesg_cancel));
						selection_on = 0;
						redraw_menu = 0;
						run_battery_meter = 1;
						flip_screen();
					}
					if(scrolling) SDL_Delay(menu_scroll_rate);
					poll_command = handle_input(wait_for_input);
					switch(poll_command) {
						case select_option|event_end:
							if(current_option == 0) {
								action = *options;
								action.int_data = value;
							} else if(current_option == 1) {
								if(supermenu) {
									action.command = open_submenu;
									action.int_data = supermenu;
								} else action.command = event_end;
							} else {
								current_option = 0;
								redraw_menu = 1;
							}
						break;
						case go_back|event_end:
							if(supermenu) {
								action.command = open_submenu;
								action.int_data = supermenu;
							} else action.command = event_end;
						break;
						case open_menu|event_end: if(book) action.command = event_end; break;
						case pan_down:
							scrolling = 1;
							wait_for_input = 0;
						break;
						case pan_up:
							scrolling = -1;
							wait_for_input = 0;
						break;
						case pan_right:
							current_option = (current_option+1)%(3+num_digits);
							redraw_menu = 1;
						break;
						case pan_left:
							current_option = (current_option+2+num_digits)%(3+num_digits);
							redraw_menu = 1;
						break;
						case zoom_fixed|event_end:
							if(usb_on) {
								stop_usb();
								usb_on = 0;
							} else usb_on = (start_usb() >= 0);
							redraw_menu = 1;
						break;
						case redraw_ev: redraw_menu = 1; break;
						case rotate_ccw|event_end:
							value = init_value;
							redraw_menu = 1;
					}
					if(scrolling) {
						#ifdef PSP
						if(!SDL_JoystickGetButton(joy,8) && !SDL_JoystickGetButton(joy,6)) {
							scrolling = 0;
							wait_for_input = 1;
						}
						else {
						#endif
							if(scrolling > 0) {
								if(current_option >= 3) {
									value -= (Sint32)pow(10,num_digits-current_option+2);
								}
							} else {
								if(current_option >= 3) {
									int power = num_digits-current_option+2;
									int delta = (int)pow(10,power);
									value += delta;
								}
							}
							redraw_menu = 1;
							if(value > maximum) value = maximum;
							if(value < minimum) value = minimum;
						#ifndef PSP
							scrolling = 0;
							wait_for_input = 1;
						#else
						}
						#endif
					}
				}
			} break;
			default:
				if(usb_on) stop_usb();
				error_message(get_message(mesg_internal_error));
				return init_event(error_ev);
			break;
		}
		if(bg) show_image(bg,0,0,1);
		else {
			clear_screen();
			flip_screen();
		}
		switch(action.command) {
			case go_back:
				if(supermenu == menu_invalid) {
					action.command = event_end;
					break;
				}
			case open_submenu:
				if(current_submenu == menu_open_book) {
					free(lwd);
					lwd = make_string(cwd);
				}
				//free_event(action);
				current_submenu = action.int_data;
				current_option = 0;
				scroll_offset = 0;
			case open_menu:
				//free_event(action);
				action = init_event(no_event);
				current_option = 0;
				scroll_offset = 0;
			break;
			case popup:
				run_battery_meter = 2;
				show_popup(action.int_data);
				run_battery_meter = 1;
				free_event(action);
				action = init_event(no_event);
			break;
			case aux_command:
			switch(action.int_data) {
				case open_book:
					if(((char*)action.ptr_data)[strlen((char*)action.ptr_data)-1] == '/') {
						char *buffer;
						if(strcmp((char*)action.ptr_data,"../") != 0) {
							buffer = malloc(sizeof(char)*
								(strlen((char*)action.ptr_data)+strlen(lwd)+1));
							if(!buffer) {
								run_battery_meter = 0;
								reset_text_pos();
								free(lwd);
								free_event(action);
								if(usb_on) stop_usb();
								error_message(get_message(mesg_out_of_memory));
								return init_event(error_ev);
							}
							memset(buffer,0,strlen((char*)action.ptr_data)+strlen(lwd)+1);
							strcpy(buffer,lwd);
							strcat(buffer,(char*)action.ptr_data);
						} else {
							if(strcmp(lwd,"/") != 0) {
								lwd[strlen(lwd)-1] = 0;
								char *temp = strrchr(lwd,'/');
								temp[1] = 0;
							}
							buffer = make_string(lwd);
						}
						free(lwd);
						lwd = buffer;
						action = init_event(no_event);
						current_option = 0;
						scroll_offset = 0;
					} else {
						free(cwd);
						cwd = lwd;
					}
				break;
				case open_book_by_id: {
					size_t length = 261;
					char *full_path = malloc(sizeof(char)*length), *book, *dir;
					if(!full_path) {
						run_battery_meter = 0;
						reset_text_pos();
						free(lwd);
						free_event(action);
						if(usb_on) stop_usb();
						error_message(get_message(mesg_out_of_memory));
						return init_event(error_ev);
					}
					if((get_book_by_id((char*)action.ptr_data,full_path,&length) < 0) ||
							(length > 260) || !(book = strrchr(full_path,'/'))) {
						free(full_path);
						action = init_event(no_event);
						continue;
					}
					*(book++) = 0;
					dir = full_path;
					action = init_event(aux_command);
					action.int_data = open_book;
					action.ptr_data = (int*)make_string(book);
					*(book-1) = '/';
					*book = 0;
					if(lwd != cwd) free(lwd);
					free(cwd);
					cwd = lwd = make_string(dir);
					free(full_path);
				} break;
				case preview_theme: {
					char *current_theme = make_string(get_loaded_theme_name());
					run_battery_meter = 2;
					unload_theme();
					if(load_theme((char*) action.ptr_data) != 1) {
						if(load_theme(current_theme) != 1) {
							error_message(get_message(mesg_load_theme_error));
							show_popup(1);
							exit(2);
						}
						error_message(get_message(mesg_load_theme_error));
						action = init_event(no_event);
						current_option = 0;
						scroll_offset = 0;
						bg = get_background();
					} else {
						bg = get_background();
						clear_screen();
						show_image(bg,0,0,0);
						say_centered(get_message(mesg_theme_preview));
						flip_screen();
						int button;
						while(1) {
							button = handle_input(1);
							if((button & (zoom_in | zoom_out)) && (button & event_end)) break;
							if(button == redraw_ev) {
								clear_screen();
								show_image(bg,0,0,0);
								say_centered(get_message(mesg_theme_preview));
								flip_screen();
							}
							SDL_Delay(100);
						}
						if(button == (zoom_in|event_end)) {
							unload_theme();
							if(load_theme(current_theme) != 1) {
								error_message(get_message(mesg_load_theme_error));
								show_popup(1);
								exit(2);
							}
							bg = get_background();
							action = init_event(no_event);
							current_option = 0;
							scroll_offset = 0;
						}
					}
					free(current_theme);
					run_battery_meter = 1;
				} break;
				case set_language: {
					load_language((char*)action.ptr_data);
				} break;
			}
			break;
		}
		int i;
		for(i = 0; i < num_options; i++) {
			if((options[i].option_text != action.option_text) &&
					(options[i].ptr_data != action.ptr_data))
				free_event(options[i]);
		}
		free(options);
	}
	if(cwd != lwd) free(lwd);
	if(usb_on) stop_usb();
	switch(action.command) {
		case clock_adjust:
			set_clock(action.int_data);
		break;
		case toggle_global: {
			int *global = access_int_global(action.int_data);
			if(global) *global = !*global;
		} break;
		case set_pointer:
			*action.ptr_data = action.int_data;
		break;
		case aux_command:
		switch(action.int_data) {
			case next_book: {
				free_event(action);
				if(book) {
					action.option_text = NULL;
					action.command = aux_command;
					action.int_data = open_book;
					action.ptr_data = (int*)next_file_name(cwd,book->localname);
					if(!action.ptr_data) action = init_event(event_end);
				} else {
					action = init_event(event_end);
				}
			} break;
			case prev_book: {
				free_event(action);
				if(book) {
					action.option_text = NULL;
					action.command = aux_command;
					action.int_data = open_book;
					action.ptr_data = (int*)prev_file_name(cwd,book->localname);
					if(!action.ptr_data) action = init_event(event_end);
				} else {
					action = init_event(event_end);
				}
			}
			break;
			case load_config_command: load_config(); break;
			case save_config_command: save_config(); break;
			case add_bookmark: save_bookmark(book); break;
			case retrieve_bookmark: {
				Sint32 page = get_bookmark(book);
				free_event(action);
				if(page >= 0) {
					action = init_event(jump_page);
					action.int_data = page;
				} else action = init_event(no_event);
			} break;
			case save_book_command: set_saved_book(book->filename); break;
			case delete_bookmarks_command: delete_bookmarks(book->filename);
			break;
			case delete_all_bookmarks_command: delete_all_bookmarks(); break;
			case purge_bookmarks_command: purge_bookmarks(); break;
		} break;
	}
	run_battery_meter = 0;
	scale_down_cpu();
	return action;
}
inline void free_event(event mo) {
	if(mo.ptr_data && (mo.command != set_pointer)) free(mo.ptr_data);
	if(mo.option_text && ((char*)mo.ptr_data != mo.option_text)) free(mo.option_text);
}
Uint16 real_file_list(char *dir, event **ptr_to_list, unsigned long user,
		char (*selection_criteria)(char*, struct dirent*, unsigned long),
		event (*event_maker)(char*, struct dirent*, unsigned long),
		int (*comparator)(const void*,const void*)) {
	if(!(dir && ptr_to_list && selection_criteria && event_maker)) return 0;
	Uint16 num_options = 0;
	event *options = NULL;
	*ptr_to_list = NULL;
	DIR *dirh = opendir(dir);
	if(!dirh) return 0;
	struct dirent *filent;
	char result; 
	while((filent = readdir(dirh))) {
		result = selection_criteria(dir,filent,user);
		if(result > 0) ++num_options;
		else if(result < 0) {
			closedir(dirh);
			return 0;
		}
	}
	closedir(dirh);
	if(num_options) options = malloc(sizeof(event)*num_options);
	else return 0;
	if(!options) {
		error_message(get_message(mesg_out_of_memory));
		return 0;
	}
	dirh = opendir(dir);
	int i;
	for(i = 0; (filent = readdir(dirh)) && i < num_options; ++i) {
		result = selection_criteria(dir,filent,user);
		if(result > 0) options[i] = event_maker(dir,filent,user);
		if((result < 0) || ((result > 0) && (options[i].command == error_ev))) {
			closedir(dirh);
			int j;
			for(j = 0; j < i; ++j) free_event(options[j]);
			free(options);
			return 0;
		}
		else if(result == 0) --i;
	}
	num_options = i;
	closedir(dirh);
	if(comparator) qsort(options,num_options,sizeof(event),comparator);
	if(num_options) *ptr_to_list = options;
	else free(options);
	return num_options;
}
char cb_criteria(char *dir, struct dirent *filent, unsigned long get_dirs) {
	char *temp = dir_real_name(filent);
	if(!temp) {
		error_message(get_message(mesg_out_of_memory));
		return -1;
	}
	char result = 0;
	#if defined(PSP) && _PSP_FW_VERSION > 150
	if(strrchr(temp,(char)-2) != NULL) result = 0;	//Keep out those nasty
	else											//unopenable files
	#endif
	if(strlen(temp)+strlen(dir) <= 250) {
		if(get_dirs && dirent_is_dir(filent,dir)) {
			if(strcmp(temp,".") != 0) result = 1;
		} else if((
		  (ends_with(temp,".cbz") == 1) ||
		  (ends_with(temp,".zip") == 1) ||
		  (ends_with(temp,".cbr") == 1) ||
		  (ends_with(temp,".rar") == 1)) &&
		(get_dirs || !dirent_is_dir(filent,dir))) result = 1;
	}
	free(temp);
	return result;
}
event cb_event_maker(char *dir, struct dirent *filent, unsigned long dummy) {
	char *temp = dir_real_name(filent);
	if(!temp) {
		error_message(get_message(mesg_out_of_memory));
		return init_event(error_ev);
	}
	event ret = init_event(no_event);
	#if defined(PSP) && _PSP_FW_VERSION > 150
	if(strrchr(temp,(char)-2) != NULL) return init_event(no_event);
	else
	#endif
	if(strlen(temp)+strlen(dir) <= 250) {
		ret.option_color = get_color(color_normal);
		if(dirent_is_dir(filent,dir)) {
			if(strcmp(temp,".") == 0)
				return init_event(no_event);
			size_t namelen = strlen(temp);
			char *buffer = malloc(sizeof(char)*(namelen+2));
			if(!buffer) {
				reset_text_pos();
				free(temp);
				error_message(get_message(mesg_out_of_memory));
				return init_event(error_ev);
			}
			strcpy(buffer,temp);
			free(temp);
			buffer[namelen] = '/';
			namelen++;
			buffer[namelen] = 0;
			temp = buffer;
			ret.option_color = get_color(color_secondary);
		}
		else if(!(
				(ends_with(temp,".cbz") == 1) ||
				(ends_with(temp,".zip") == 1) ||
				(ends_with(temp,".cbr") == 1) ||
				(ends_with(temp,".rar") == 1)) ||
				dirent_is_dir(filent,dir)) {
			return init_event(no_event);
		}
		ret.ptr_data = (int*)temp;
		#if defined(PSP) || defined(_WIN32)
		ret.option_text = ellipsis(sjis_to_utf8(temp),0);
		#else
		ret.option_text = ellipsis(temp,0);
		#endif
		ret.command = aux_command;
		ret.int_data = open_book;
	}
	return ret;
}
char theme_criteria(char *dir, struct dirent *filent, unsigned long dummy) {
	char *temp = dir_real_name(filent);
	if(!temp) {
		error_message(get_message(mesg_out_of_memory));
		return -1;
	}
	if((strcmp(".",temp) == 0) || (strcmp("..",temp) == 0) || !dirent_is_dir(filent,dir)) {
		free(temp);
		return 0;
	}
	char *temp2 = get_theme_name(temp);
	free(temp);
	if(!temp2) return 0;
	free(temp2);
	return 1;
}
event theme_event_maker(char *dir, struct dirent *filent, unsigned long dummy) {
	char *temp = dir_real_name(filent);
	if(!temp) return init_event(error_ev);
	const char *loaded_theme = get_loaded_theme_name();
	event ret = init_event(aux_command);
	ret.option_color = (strcmp(temp,loaded_theme) == 0?get_color(color_secondary):get_color(color_normal));
	ret.ptr_data = (int*)make_string(temp);
	free(temp);
	temp = get_theme_name((char*)ret.ptr_data);
	if(!temp) {
		free(ret.ptr_data);
		return init_event(error_ev);
	}
	ret.option_text = ellipsis(temp,0);
	ret.int_data = preview_theme;
	return ret;
}
char language_criteria(char *dir, struct dirent *filent, unsigned long dummy) {
	char *temp = dir_real_name(filent);
	if(!temp) {
		error_message(get_message(mesg_out_of_memory));
		return -1;
	}
	char *ext = strrchr(temp,'.');
	if(!ext || (strcasecmp(".xml",ext) != 0)) {
		free(temp);
		return 0;
	}
	ext[0] = 0;
	char *temp2 = get_language_name(temp);
	if(!temp2) {
		free(temp);
		return 0;
	}
	free(temp2);
	char_ranges *ranges = malloc(sizeof(char_ranges));
	if(get_language_ranges(temp,ranges) != 0) {
		if(ranges) free(ranges);
		free(temp);
		return 0;
	}
	if(!ranges_compatible(get_font_ranges(),*ranges)) {
		free(ranges->range_begins);
		free(ranges->range_ends);
		free(ranges);
		free(temp);
		return 0;
	}
	free(temp);
	return 1;
}
event language_event_maker(char *dir, struct dirent *filent, unsigned long dummy) {
	char *temp = dir_real_name(filent);
	if(!temp) return init_event(error_ev);
	event ret = init_event(aux_command);
	ret.option_color = get_color(color_normal);
	char *ext = strrchr(temp,'.');
	if(!ext || (strcasecmp(".xml",ext) != 0)) {
		free(temp);
		return init_event(error_ev);
	}
	ext[0] = 0;
	ret.ptr_data = (int*)make_string(temp);
	free(temp);
	temp = get_language_name((char*)ret.ptr_data);
	if(!temp) {
		free(ret.ptr_data);
		return init_event(error_ev);
	}
	ret.option_text = ellipsis(temp,0);
	ret.int_data = set_language;
	return ret;
}
Uint16 file_list(char *dir, event **ptr_to_list, char get_dirs) {
	return real_file_list(dir,ptr_to_list,get_dirs,cb_criteria,cb_event_maker,cb_compare);
}
Uint16 theme_list(event **ptr_to_list) {
	char *dir = get_full_path(themes_dir);
	if(dir) {
		Uint16 ret = real_file_list(dir,ptr_to_list,0,theme_criteria,theme_event_maker,NULL);
		free(dir);
		return ret;
	}
	return 0;
}
Uint16 language_list(event **ptr_to_list) {
	char *dir = get_full_path(languages_dir);
	if(dir) {
		Uint16 ret = real_file_list(dir,ptr_to_list,0,language_criteria,language_event_maker,NULL);
		free(dir);
		return ret;
	}
	return 0;
}
char dirent_is_dir(struct dirent *filent, char *dir) {
	#ifdef PSP
	return filent->d_stat.st_attr & FIO_SO_IFDIR;
	#else
	#ifdef _WIN32
	size_t namlen = strlen(dir)+filent->d_namlen+1;
	#else
	size_t namlen = strlen(dir)+strlen(filent->d_name)+1;
	#endif
	char *temp = malloc(sizeof(char)*namlen);
	if(!temp) {
		error_message(get_message(mesg_out_of_memory));
		return -1;
	}
	memset(temp,0,namlen);
	strcpy(temp,dir);
	#ifdef _WIN32
	strncat(temp,filent->d_name,filent->d_namlen);
	#else
	strcat(temp,filent->d_name);
	#endif
	DIR *dirh = opendir(temp);
	free(temp);
	if(dirh) {
		closedir(dirh);
		return 1;
	} else return 0;
	#endif
}
void battery_meter() {
	#ifdef PSP
	unsigned color;
	font_attr attr;
	if(!get_font_attr(&attr)) return;
	if(scePowerIsPowerOnline()) color = get_color(color_battery_charging);
	else if(scePowerIsLowBattery()) color = get_color(color_battery_low);
	else color = attr.font_color;
	char battery_percent[7];
	int power = scePowerGetBatteryLifePercent();
	if(power >= 0) sprintf(battery_percent,"[%03u%%]",power);
	else sprintf(battery_percent,"[---%%]");
	device scr_acc = access_device(access_screen);
	int w;
	if(get_text_dimensions(battery_percent,&w,NULL) < 0) return;
	set_explicit_color(color);
	place_text(battery_percent,scr_acc.screen->w-w,0,color);
	unset_explicit_color();
	#endif
}
int battery_meter_thread(void *is_running) {
	#ifdef PSP
	char *running = (char*)is_running;
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = redraw_ev;
	while(*running) {
		while(*running == 2) SDL_Delay(200);
		SDL_PushEvent(&event);
		SDL_Delay(10000);
	}
	#endif
	return 0;
}
inline char *dir_real_name(struct dirent *dir) {
	size_t namelen;
	#ifndef _WIN32
	namelen = strlen(dir->d_name);
	#else
	namelen = dir->d_namlen;
	#endif
	char *temp = malloc(sizeof(char)*(namelen+1));
	if(!temp) {
		return NULL;
	}
	temp[namelen] = 0;
	strncpy(temp,dir->d_name,namelen);
	return temp;
}

char* extract_file(comic_book *book, Uint16 page, size_t *size) {
	if(page > book->num_pages) return NULL;
	if(book->type == comic_book_rar) {
		char *buffer;
		if(rar_get_file_by_name(book->filename,book->page_order[page],&buffer,size)) {
			return NULL;
		}
		return buffer;
	} else if(book->type == comic_book_zip) {
		if(unzLocateFile(book->zip_file,book->page_order[page],1) != UNZ_OK) {
			return NULL;
		}
		return extract_file_from_zip(book->zip_file, size);
	} else if(book->type == comic_book_dir) {
		char *full_name = malloc(sizeof(char)*(strlen(book->filename)+strlen(book->page_order[page])+2));
		if(!full_name) return NULL;
		sprintf(full_name,"%s/%s",book->filename,book->page_order[page]);
		FILE *f = fopen(full_name,"rb");
		if(!f) {
			free(full_name);
			return NULL;
		}
		if(fseek(f,0,SEEK_END) != 0) {
			fclose(f);
			return NULL;
		}
		long length = ftell(f);
		char *ret;
		if((length == -1) || (fseek(f,0,SEEK_SET) != 0) || !(ret = malloc(length))) {
			fclose(f);
			return NULL;
		}
		fread(ret,length,1,f);
		*size = length;
		fclose(f);
		return ret;
		
	} else {
		error_message(get_message(mesg_internal_error));
		return NULL;
	}
}
char* extract_file_from_zip(unzFile zip, size_t *size) {
	unz_file_info file_info;
	unzGetCurrentFileInfo(zip,&file_info,NULL,0,NULL,0,NULL,0);
	char *block = calloc(file_info.uncompressed_size,sizeof(char));
	if(!block) {
		error_message(get_message(mesg_out_of_memory));
		return NULL;
	}
	unzOpenCurrentFile(zip);
	int output = unzReadCurrentFile(zip, (voidp)block,file_info.uncompressed_size);
	if(output < 0) {
		error_message(get_message(mesg_read_file_error),"zip");
		unzCloseCurrentFile(zip);
		free(block);
		return NULL;
	}
	if(size) *size = output;
	unzCloseCurrentFile(zip);
	return block;
}

char* next_file_name(char *dir, char *cur_file) {
	event *list = NULL;
	char *ret = NULL;
	Uint16 num_books = file_list(dir,&list,0);
	if(num_books == 0) return NULL;
	int i, j = -1;
	for(i = 0; i < num_books; i++) {
		if((j == -1) && (cb_compare(&cur_file,&(list[i].ptr_data)) < 0)) {
			j = i;
			ret = make_string((char*)list[i].ptr_data);
		}
		if(i) free_event(list[i]);
	}
	if(j == -1) ret = make_string((char*)list[0].ptr_data);
	free_event(list[0]);
	free(list);

	return ret;
}
char* prev_file_name(char *dir, char *cur_file) {
	event *list = NULL;
	char *ret = NULL;
	Uint16 num_books = file_list(dir,&list,0);
	if(num_books == 0) return NULL;
	int i, j = -1;
	for(i = num_books - 1; i >= 0; i--) {
		if((j == -1) && (cb_compare(&cur_file,&(list[i].ptr_data)) > 0)) {
			j = i;
			ret = make_string((char*)list[i].ptr_data);
		}
		if(i != num_books - 1) free_event(list[i]);
	}
	if(j == -1) ret = make_string((char*)list[num_books - 1].ptr_data);
	free_event(list[num_books - 1]);
	free(list);

	return ret;
}

int cb_compare(const void *foo, const void *bar) {
	int result = strncasecmp(*(char**)foo,*(char**)bar,1024);
	return result;
}
int event_compare(const void *foo, const void *bar) {
	int result = strncasecmp((char*)((event*)foo)->ptr_data,(char*)((event*)bar)->ptr_data,1024);
	return result;
}
int sjis_compare(const void *foo, const void *bar) {
	t_utf8_sjis_xlate *f = (t_utf8_sjis_xlate *) foo;
	t_utf8_sjis_xlate *b = (t_utf8_sjis_xlate *) bar;
	if(f->sjis > b->sjis) return 1;
	if(f->sjis < b->sjis) return -1;
	return 0;
}
char ranges_compatible(const char_ranges bigger, const char_ranges smaller) {
	int i;
	for(i = 0; i < smaller.num_ranges; ++i) {
		//There's probably a faster way to do this, but it's not called all that
		//often, so I can optimize it later if I want to.
		int j;
		char ok = 0;
		for(j = 0; j < bigger.num_ranges; ++j) {
			if((bigger.range_begins[j] <= smaller.range_begins[i]) &&
					(bigger.range_ends[j] >= smaller.range_ends[i])) {
				ok = 1;
				break;
			}
			else if(((bigger.range_begins[j] > smaller.range_begins[i]) &&
					(bigger.range_begins[j] <= smaller.range_ends[i])) ||
					((bigger.range_ends[j] >= smaller.range_begins[i]) &&
					(bigger.range_ends[j] < smaller.range_ends[i]))) {
				break;
			}
		}
		if(!ok) return 0;
	}
	return 1;
}

inline int make_dir(const char *dir_name) {
	char *new_dir_name = make_string(dir_name);
	if(!new_dir_name) {
		errno = ENOMEM;
		return -1;
	}
	if(new_dir_name[strlen(new_dir_name)-1] == '/') new_dir_name[strlen(new_dir_name)-1] = 0;
	int ret;
	#ifdef PSP
	ret = sceIoMkdir(new_dir_name,0777);
	#elif defined(_WIN32)
	ret = mkdir(new_dir_name);
	#else /* _WIN32 */
	ret = mkdir(new_dir_name,0777);
	#endif /* PSP */
	free(new_dir_name);
	return ret;
}
inline char* get_full_path(const char *file_name) {
	char *ret = malloc(sizeof(char)*(strlen(root_dir)+strlen(file_name)+1));
	if(!ret) return NULL;
	sprintf(ret,"%s%s",root_dir,file_name);
	return ret;
}

int init_usb() {
	#if defined(PSP) && defined(USE_USB)
	//Thanks to ps2dev.org for how to do this
	int c = pspSdkLoadStartModule("flash0:/kd/chkreg.prx",PSP_MEMORY_PARTITION_KERNEL);
	int n = pspSdkLoadStartModule("flash0:/kd/npdrm.prx",PSP_MEMORY_PARTITION_KERNEL);
	int s = pspSdkLoadStartModule("flash0:/kd/semawm.prx",PSP_MEMORY_PARTITION_KERNEL);
	int u = pspSdkLoadStartModule("flash0:/kd/usbstor.prx",PSP_MEMORY_PARTITION_KERNEL);
	int g = pspSdkLoadStartModule("flash0:/kd/usbstormgr.prx",PSP_MEMORY_PARTITION_KERNEL);
	int m = pspSdkLoadStartModule("flash0:/kd/usbstorms.prx",PSP_MEMORY_PARTITION_KERNEL);
	int b = pspSdkLoadStartModule("flash0:/kd/usbstorboot.prx",PSP_MEMORY_PARTITION_KERNEL);
	return ((c >= 0) && ((_PSP_FW_VERSION > 150) && (n >= 0)) && (s >= 0) &&
		(u >= 0) && (g >= 0) && (m >= 0) && (b >= 0))?0:-1;
	#else
	return -1;
	#endif
}

int start_usb() {
	#if defined(PSP) && defined(USE_USB)
	sceUsbStart(PSP_USBBUS_DRIVERNAME,0,0);
	sceUsbStart(PSP_USBSTOR_DRIVERNAME,0,0);
	sceUsbstorBootSetCapacity(0x800000);
	return sceUsbActivate(0x1c8);
	#else
	return -1;
	#endif
}

int stop_usb() {
	#if defined(PSP) && defined(USE_USB)
	sceUsbDeactivate(0x1c8);
	sceIoDevctl("fatms0:", 0x0240D81E, NULL, 0, NULL, 0 ); //Avoid corrupted files 
	sceUsbStop(PSP_USBSTOR_DRIVERNAME,0,0);
	sceUsbStop(PSP_USBBUS_DRIVERNAME,0,0);
	return 0;
	#else
	return -1;
	#endif
}

static char get_utf8_length(int sjis) {
	t_utf8_sjis_xlate dummy = { .sjis = sjis };

	t_utf8_sjis_xlate *result = 
		bsearch(&dummy,sjis_xlate,sjis_xlate_entries,sizeof(dummy),sjis_compare);
	if(!result) return 0;
	if(result->utf8 & 0xFF000000) return 4;
	if(result->utf8 & 0x00FF0000) return 3;
	if(result->utf8 & 0x0000FF00) return 2;
	return 1;
}
char* sjis_to_utf8(const char *string) {
	unsigned int i;
	unsigned int outsize = 1;
	const unsigned char *ustring = (const unsigned char *)string;
	for(i = 0; i < strlen(string); ++i) {
		if(ustring[i] < 0x5C) ++outsize;
		else if(ustring[i] == 0x5C) outsize += 2; //Yen sign
		else if(ustring[i] < 0x7E) ++outsize;
		else if(ustring[i] == 0x7E) outsize += 3;
		else if(ustring[i] == 0x7F) ++outsize;
		else if(ustring[i] == 0x80); //Value 0x80 is reserved
		else if(ustring[i] < 0xA0) {
			outsize += get_utf8_length((ustring[i] << 8) | ustring[i+1]);
			++i;
		}
		else if(ustring[i] == 0xA0); //Value 0xA0 is reserved
		else if(ustring[i] < 0xE0) {
			outsize += 3;
			++i;
		}
		else if(ustring[i] < 0xF0) {
			outsize += get_utf8_length((ustring[i] << 8) | ustring[i+1]);
			++i;
		}
	}
	char *ret = malloc(sizeof(char)*outsize);
	if(!ret) return NULL;
	ret[outsize-1] = '\0';
	t_utf8_sjis_xlate buffer, *bsr;
	int j = 0;
	for(i = 0; i < strlen(string); ++i, ++j) {
		if(ustring[i] < 0x5C) ret[j] = string[i];
		else if(ustring[i] == 0x5C) { //Yen sign
			ret[j] = 0xC2;
			ret[++j] = 0xA5;
			++i;
		}
		else if(ustring[i] < 0x7E) ret[j] = string[i];
		else if(ustring[i] == 0x7E) { //Overline
			ret[j] = 0x82;
			ret[++j] = 0x80;
			ret[++j] = 0xBE;
			++i;
		}
		else if(ustring[i] == 0x7F) ret[j] = string[i];
		else if(ustring[i] == 0x80); //Value 0x80 is reserved
		else if(ustring[i] == 0xA0); //Value 0xA0 is reserved
		else if(ustring[i] < 0xA0 || (ustring[i] >= 0xE0 && ustring[i] < 0xF0)) {
			buffer.sjis = (ustring[i] << 8) | (ustring[i+1]);
			bsr = bsearch(&buffer,sjis_xlate,sjis_xlate_entries,sizeof(buffer),sjis_compare);
			if(bsr) {
				if(bsr->utf8 & 0xFF000000) {
					ret[j] = bsr->utf8 >> 24;
					ret[++j] = (bsr->utf8 & 0xFF0000) >> 16;
					ret[++j] = (bsr->utf8 & 0xFF00) >> 8;
					ret[++j] = bsr->utf8 & 0xFF;
				} else if(bsr->utf8 & 0x00FF0000) {
					ret[j] = bsr->utf8 >> 16;
					ret[++j] = (bsr->utf8 & 0xFF00) >> 8;
					ret[++j] = bsr->utf8 & 0xFF;
				} else if(bsr->utf8 & 0x0000FF00) {
					ret[j] = bsr->utf8 >> 8;
					ret[++j] = bsr->utf8 & 0xFF;
				} else ret[j] = bsr->utf8;
			}
			++i;
		}
		else if(ustring[i] < 0xE0) {
			ret[j] = 0x8F;
			ret[++j] = 0x80 | ((ustring[i+1] - 0x40) >> 6) | 0x0F ;
			ret[++j] = 0x80 | ((ustring[i+1] - 0x40) & 0x3F);
			++i;
		}
	}
	return ret;
}

void scale_up_cpu(void) {
	#if 0
	if(*access_int_global(access_dynamic_cpu))
		set_clock(333);
	#endif
}

void scale_down_cpu(void) {
	#if 0
	if(*access_int_global(access_dynamic_cpu))
		set_clock(16);
	#endif
}

#if defined(PSP) && !defined(DOXYGEN)
static int exit_callback(int arg1, int arg2, void *arg3) {
	exit(0);
	return 0;
}
int exit_callback_thread(SceSize args, void *argp) {
	int cbid;
	cbid = sceKernelCreateCallback("Exit Callback",exit_callback,NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
	return 0;
}
int setup_exit_callback() {
	int cbthid = sceKernelCreateThread("Exit Callback Thread",exit_callback_thread,0x11,0xFA0,0,0);
	if(cbthid >= 0) sceKernelStartThread(cbthid,0,0);
	return cbthid;
}
#endif

int main(int argc, char* argv[]) {
	#ifdef PSP
	atexit(sceKernelExitGame);
	#if _PSP_FW_VERSION > 150
	//Reserve some memory for threads by allocating it, initializing the heap,
	//and then freeing it.
	//Be very careful about what is put before this! If anything calls malloc,
	//this trick won't work!
	SceUID temp_mem = sceKernelAllocPartitionMemory(2,"temp",PSP_SMEM_Low,0x40000,0);
	free(malloc(1));
	sceKernelFreePartitionMemory(temp_mem);

	//Set charset to Shift-JIS so we can read Japanese filenames
	/*get_registry_value("/CONFIG/SYSTEM/CHARACTER_SET","ansi",&old_charset_ansi);
	get_registry_value("/CONFIG/SYSTEM/CHARACTER_SET","oem",&old_charset_oem);
	set_registry_value("/CONFIG/SYSTEM/CHARACTER_SET","ansi",0x0D);
	set_registry_value("/CONFIG/SYSTEM/CHARACTER_SET","oem",0x0D);*/
	#endif
	initial_clock = scePowerGetCpuClockFrequency();
	setup_exit_callback();
	init_usb();
	#endif
	atexit(_atexit);
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) < 0) {
		error_message(get_message(mesg_sdl_error),SDL_GetError());
		return 1;
	}
	#if !defined(PSP) && !defined(_WIN32)
	root_dir = getenv("HOME");
	#endif

	#ifdef PSP
	init_video(480,272);
	#else
	init_video(0,0);
	#endif 

	init_theme_system();
	char *real_cb_dir = get_full_path(cb_dir);
	if(!real_cb_dir) {
		error_message(get_message(mesg_out_of_memory));
		return 1;
	}

	{ //Pop the extra variable off the stack when we're done; we won't need it
		DIR *dtest;
		dtest = opendir(real_cb_dir);
		if(!dtest) make_dir(real_cb_dir);
		else closedir(dtest);
		char *real_data_dir = get_full_path(data_dir);
		if(!real_data_dir) {
			error_message(get_message(mesg_out_of_memory));
			return 1;
		}
		dtest = opendir(real_data_dir);
		if(!dtest) make_dir(real_data_dir);
		else closedir(dtest);
		free(real_data_dir);
	}

	if((load_config() == xml_er_subroutine_fail) || ((!get_loaded_theme_name())
			&& (load_theme("default") != 1))) {
		error_message(get_message(mesg_load_theme_error));
		show_popup(1);
		return 2;
	}
	save_config(); //Make sure config file is in the right place now
	load_bookmarks();
	event obtained_command;
	obtained_command.command = no_event;
	obtained_command.option_text = NULL;
	obtained_command.ptr_data = NULL;
	obtained_command.int_data = 0;
	if(argc > 1) {
		char *buffer = make_string(argv[1]), *temp, *book;
		free(real_cb_dir);
		if(!buffer) {
			error_message(get_message(mesg_out_of_memory));
			return 1;
		}
		clear_screen();
		show_image(get_background(),0,0,0);
		flip_screen();
		if(strrchr(buffer,'/')) temp = strrchr(buffer,'/');
		if(temp) {
			*temp = 0;
			cwd = malloc(sizeof(char)*(strlen(buffer)+2));
			if(!cwd) {
				free(buffer);
				error_message(get_message(mesg_out_of_memory));
				return 1;
			}
			memset(cwd,0,sizeof(char)*(strlen(buffer)+2));
			strcpy(cwd,buffer);
			strcat(cwd,"/");
			book = make_string(temp + 1);
			if(!book) {
				free(cwd);
				free(buffer);
				error_message(get_message(mesg_out_of_memory));
				return 1;
			}
		}
		else {
			temp = buffer;
			cwd = make_string("./");
			if(!cwd) {
				free(buffer);
				error_message(get_message(mesg_out_of_memory));
				return 1;
			}
			book = make_string(temp);
			if(!book) {
				free(cwd);
				free(buffer);
				error_message(get_message(mesg_out_of_memory));
				return 1;
			}
		}
		free(buffer);
		obtained_command = show_book(cwd,book);
		clear_screen();
		show_image(get_background(),0,0,0);
		flip_screen();
	} else {
		cwd = real_cb_dir;
		if(!cwd) {
			error_message(get_message(mesg_out_of_memory));
			return 1;
		}
	}
	while(obtained_command.command != quit_command) {
		if(argc <= 1) obtained_command = show_menu(NULL);
		while(!(obtained_command.command & (close_book|quit_command|error_ev|open_menu))) {
			switch(obtained_command.command) {
				case aux_command: 
				switch(obtained_command.int_data) {
					case open_book: {
						char *temp = make_string((char*)obtained_command.ptr_data);
						free_event(obtained_command);
						if(!temp) {
							free(cwd);
							error_message(get_message(mesg_out_of_memory));
							return 1;
						}
						obtained_command = show_book(cwd,temp);
						free(temp);
					}
				} break;
				default:
				obtained_command.command = quit_command;
				break;
			}
		}
		if(argc > 1) obtained_command.command = quit_command;
		free_event(obtained_command);
	}
	clear_screen();
	show_image(get_background(),0,0,0);
	reset_text_pos();
	say_centered(get_message(mesg_closing));
	flip_screen();

	free(cwd);
	return 0;
}
