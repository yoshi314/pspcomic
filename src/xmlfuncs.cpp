/** \file
    XML-related functions

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

static TiXmlDocument *bookmarks = 0;

char save_config() {
	TiXmlDocument confile;
	TiXmlDeclaration *dec = new TiXmlDeclaration("1.0","","");
	if(!dec) return xml_er_out_of_memory;
	confile.LinkEndChild(dec);
	TiXmlElement *root = new TiXmlElement("config");
	if(!root) return xml_er_out_of_memory;
	root->SetAttribute("version",pspcomic_v);

	TiXmlElement *item;
	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","resize_method");
	item->SetAttribute("value",*access_int_global(access_resize));
	root->LinkEndChild(item);

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","zoom_persist");
	item->SetAttribute("value",*access_int_global(access_zoom_persist));
	root->LinkEndChild(item);

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","rotate_persist");
	item->SetAttribute("value",*access_int_global(access_rotate_persist));
	root->LinkEndChild(item);

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","pan_rate");
	item->SetAttribute("value",*access_int_global(access_pan_rate));
	root->LinkEndChild(item);

	#ifdef PSP
	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","clock_frequency");
	item->SetAttribute("value",scePowerGetCpuClockFrequency());
	root->LinkEndChild(item);
	#endif

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","scroll_skip");
	item->SetAttribute("value",*access_int_global(access_scroll_skip));
	root->LinkEndChild(item);

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","manga_mode");
	item->SetAttribute("value",*access_int_global(access_manga_mode));
	root->LinkEndChild(item);

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","zoom_box_w");
	item->SetAttribute("value",*access_int_global(access_zoom_box_w));
	root->LinkEndChild(item);

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","zoom_box_h");
	item->SetAttribute("value",*access_int_global(access_zoom_box_h));
	root->LinkEndChild(item);

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","bookmark_on_load");
	item->SetAttribute("value",*access_int_global(access_bookmark_on_load));
	root->LinkEndChild(item);

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","autozoom_mode");
	item->SetAttribute("value",*access_int_global(access_autozoom_mode));
	root->LinkEndChild(item);

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","precaching");
	item->SetAttribute("value",*access_int_global(access_precaching));
	root->LinkEndChild(item);

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","singlehanded");
	item->SetAttribute("value",*access_int_global(access_singlehanded));
	root->LinkEndChild(item);

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","analog_disabled");
	item->SetAttribute("value",*access_int_global(access_analog_disabled));
	root->LinkEndChild(item);

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","theme");
	item->SetAttribute("value",get_loaded_theme_name());
	root->LinkEndChild(item);

	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","language");
	item->SetAttribute("value",get_loaded_language_name());
	root->LinkEndChild(item);
	
	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","resx");
	item->SetAttribute("value",*access_int_global(access_current_resx));
	root->LinkEndChild(item);
	
	item = new TiXmlElement("item");
	if(!item) return xml_er_out_of_memory;
	item->SetAttribute("id","resy");
	item->SetAttribute("value",*access_int_global(access_current_resy));
	root->LinkEndChild(item);

	confile.LinkEndChild(root);
	char *filename = get_full_path(config_file);
	if(!filename) return xml_er_unknown;
	confile.SaveFile(filename);
	free(filename);
	return 0;
}
char load_config() {
	char *filename = get_full_path(config_file);
	if(!filename) return xml_er_unknown;
	TiXmlDocument confile(filename);
	free(filename);
	if(!confile.LoadFile()) {
		confile = TiXmlDocument("config.xml");
		if(!confile.LoadFile()) return xml_er_cannot_load;
	}
	TiXmlElement *root = confile.FirstChildElement("config");
	if(!root) return xml_er_malformed;
	char theme_loaded = 0;
	for(TiXmlElement *item = root->FirstChildElement("item");
			item != 0;
			item = item->NextSiblingElement("item")) {
		const char *id = item->Attribute("id");
		if(!id) return xml_er_malformed;
		else if(!strcmp(id,"resize_method")) {
			int val;
			if(
			  item->Attribute("value",&val) && 
			  (val >= resize_nn) && (val <= resize_hardware)
			)
				*access_int_global(access_resize) = val;
		} else if(!strcmp(id,"zoom_persist"))
			item->Attribute("value",access_int_global(access_zoom_persist));
		else if(!strcmp(id,"rotate_persist"))
			item->Attribute("value",access_int_global(access_rotate_persist));
		else if(!strcmp(id,"pan_rate")) {
			int val;
			if(
			  item->Attribute("value",&val) &&
			  (val >= pan_min) && (val <= pan_max)
			)
				*access_int_global(access_pan_rate) = val;
		} else if(!strcmp(id,"clock_frequency")) {
			int val;
			if(item->Attribute("value",&val)) set_clock(val);
		} else if(!strcmp(id,"scroll_skip")) {
			int val;
			if(
			  item->Attribute("value",&val) &&
			  (val > 0) && (val <= 99)
			)
				*access_int_global(access_scroll_skip) = val;
		} else if(!strcmp(id,"manga_mode"))
			item->Attribute("value",access_int_global(access_manga_mode));
		else if(!strcmp(id,"bookmark_on_load"))
			item->Attribute("value",access_int_global(access_bookmark_on_load));
		else if(!strcmp(id,"fullview_mode")) {
			int is_fullview;
			item->Attribute("value",&is_fullview);
			*access_int_global(access_autozoom_mode) = (is_fullview?full_view:full_width);
		} else if(!strcmp(id,"autozoom_mode")) {
			int val;
			if(
			  item->Attribute("value",&val) &&
			  (val > 0) && (val <= autodetect_zoom)
			)
				*access_int_global(access_autozoom_mode) = val;
		} else if(!strcmp(id,"precaching"))
			item->Attribute("value",access_int_global(access_precaching));
		else if(!strcmp(id,"singlehanded"))
			item->Attribute("value",access_int_global(access_singlehanded));
		else if(!strcmp(id,"analog_disabled"))
			item->Attribute("value",access_int_global(access_analog_disabled));
		else if(!strcmp(id,"zoom_box_w")) {
			int val;
			if(
			  item->Attribute("value",&val) &&
			  (val >= 20) && (val <= 180)
			)
				*access_int_global(access_zoom_box_w) = val;
		} else if(!strcmp(id,"zoom_box_h")) {
			int val;
			if(
			  item->Attribute("value",&val) &&
			  (val >= 20) && (val <= 250)
			)
				*access_int_global(access_zoom_box_h) = val;
		} else if(!strcmp(id,"theme")) {
			const char *theme = item->Attribute("value");
			if(!theme) return xml_er_malformed;
			if(theme_loaded) return xml_er_malformed;
			if(load_theme(theme)) theme_loaded = 1;
		} else if(!strcmp(id,"language")) {
			const char *lang = item->Attribute("value");
			if(!lang) 
				return xml_er_malformed;
			load_language(lang);
		} else if(!strcmp(id,"resx")) {
			int val = 0;
			const SDL_VideoInfo* videoinfo = SDL_GetVideoInfo();
			if (
				item->Attribute("value",&val) && 
					(val <= videoinfo->current_w) && (val >= 0)
				)
				*access_int_global(access_current_resx) = val;
			fprintf(stderr,"loaded resolution x : %i\n",val);
		} else if(!strcmp(id,"resy")) {
			int val = 0;
			const SDL_VideoInfo* videoinfo = SDL_GetVideoInfo();
			if (
				item->Attribute("value",&val) && 
					(val <= videoinfo->current_h) && (val >= 0)
				)
				*access_int_global(access_current_resy) = val;
			fprintf(stderr,"loaded resolution y : %i\n",val);
				
		}
	}
	if(!theme_loaded) return (load_theme("default")?0:xml_er_subroutine_fail);
	return 0;
}

char load_bookmarks() {
	char *filename = get_full_path(bookmark_file);
	if(!filename) return xml_er_unknown;
	bookmarks = new TiXmlDocument(filename);
	free(filename);
	if(!bookmarks) return xml_er_out_of_memory;
	if(!bookmarks->LoadFile()) {
		delete bookmarks;
		bookmarks = new TiXmlDocument("bookmarks.xml");
		if(!bookmarks->LoadFile()) {
			delete bookmarks;
			bookmarks = 0;
			return xml_er_cannot_load;
		}
	}
	TiXmlElement *root = bookmarks->FirstChildElement("bookmarks");
	if(root) {
		if(root->Attribute("version") && (root->Attribute("version")[0] == '0'))
			convert_bookmarks();
		return 0;
	}
	delete bookmarks;
	bookmarks = NULL;
	return xml_er_malformed;
}

char convert_bookmarks() {
	TiXmlElement *root = bookmarks->FirstChildElement("bookmarks");
	if(!root || !root->Attribute("version")) return xml_er_malformed;
	if(root->Attribute("version")[0] == '1') return 0;
	for(TiXmlElement *item = root->FirstChildElement("bookmark");
			item != 0;
			item = item->NextSiblingElement("bookmark")) {
		if(!item->Attribute("book")) return xml_er_malformed;
		if(!item->Attribute("page")) continue;
		char *dir = make_string(item->Attribute("book"));
		if(!dir) return xml_er_out_of_memory;
		FILE *ftest = fopen(dir,"rb");
		if(ftest) fclose(ftest);
		else {
			DIR *dtest = opendir(dir);
			if(dtest) closedir(dtest);
			else {
				root->RemoveChild(item);
				continue;
			}
		}
		char *file = strrchr(dir,'/');
		file[0] = '\0';
		++file;
		file = make_string(file);
		dir[strlen(dir)+1] = '\0';
		dir[strlen(dir)] = '/';
		comic_book booktest = open_comic_book(dir,file);
		int i;
		if((item->QueryIntAttribute("page",&i) != TIXML_SUCCESS) || (i >= booktest.num_pages)) {
			root->RemoveChild(item);
			continue;
		}
		item->SetAttribute("page",booktest.page_order[i]);
		free(file);
		free(dir);
	}
	root->SetAttribute("version",pspcomic_v);
	bookmarks->SaveFile();
	return 0;
}

char save_bookmark(comic_book *book) {
	if(!book) return xml_er_null_pointer;
	char *filename = get_full_path(bookmark_file);
	if(!filename) return xml_er_unknown;
	delete_bookmarks(book->filename);
	if(!bookmarks) {
		bookmarks = new TiXmlDocument();
		if(!bookmarks) return xml_er_out_of_memory;
		TiXmlDeclaration *dec = new TiXmlDeclaration("1.0","","");
		if(!dec) return xml_er_out_of_memory;
		bookmarks->LinkEndChild(dec);
		TiXmlElement *root = new TiXmlElement("bookmarks");
		if(!root) return xml_er_out_of_memory;
		root->SetAttribute("version",pspcomic_v);
		bookmarks->LinkEndChild(root);
		TiXmlElement *bookmark = new TiXmlElement("bookmark");
		if(!bookmark) return xml_er_out_of_memory;
		bookmark->SetAttribute("book",book->filename);
		bookmark->SetAttribute("page",book->page_order[book->current_page]);
		root->LinkEndChild(bookmark);
		bookmarks->SaveFile(filename);
		free(filename);
		return 0;
	} else {
		TiXmlNode *root = bookmarks->FirstChildElement("bookmarks");
		if(!root) return xml_er_malformed;
		root->ToElement()->SetAttribute("version",pspcomic_v);
		TiXmlElement *bookmark = new TiXmlElement("bookmark");
		if(!bookmark) return xml_er_out_of_memory;
		bookmark->SetAttribute("book",book->filename);
		bookmark->SetAttribute("page",book->page_order[book->current_page]);
		root->LinkEndChild(bookmark);
		bookmarks->SaveFile(filename);
		free(filename);
		return 0;
	}
}
char delete_bookmarks(const char *book) {
	load_bookmarks();
	if(!bookmarks) return 0;
	TiXmlNode *root = bookmarks->FirstChildElement("bookmarks");
	if(!root) return xml_er_malformed;
	for(TiXmlElement *item = root->FirstChildElement("bookmark");
			item != 0;
			item = item->NextSiblingElement("bookmark")) {
		if(item->Attribute("book")) {
			if(!strcmp(item->Attribute("book"),book) && item->Attribute("page"))
				root->RemoveChild(item);
		} else return xml_er_malformed;
	}
	char *filename = get_full_path(bookmark_file);
	if(!filename) return xml_er_unknown;
	bookmarks->SaveFile(filename);
	free(filename);
	return 0;
}
Sint32 get_bookmark(comic_book *book) {
	if(load_bookmarks() < 0) return xml_er_subroutine_fail;
	if(!book) return xml_er_null_pointer;
	for(
	  TiXmlElement *root = bookmarks->FirstChildElement("bookmarks");
	  root != 0;
	  root = root->NextSiblingElement("bookmarks")
	) {
		for(
		  TiXmlElement *item = root->FirstChildElement("bookmark");
		  item != 0;
		  item = item->NextSiblingElement("bookmark")
		) {
			if(item->Attribute("book")) {
				if(!strcmp(item->Attribute("book"),book->filename)) {
					const char *page = item->Attribute("page");
					if(!page) continue;
					char **ret = (char**)bsearch(&page,book->page_order,
						book->num_pages,sizeof(char*),cb_compare);
					if(!ret) return xml_er_unknown;
					return ret-book->page_order;
				}
			} else return xml_er_malformed;
		}
		return xml_er_unknown;
	}
	return xml_er_malformed;
}
char delete_all_bookmarks() {
	char status = load_bookmarks();
	if(status < 0) return status;
	TiXmlElement *root = bookmarks->FirstChildElement("bookmarks");
	if(!root) return xml_er_malformed;
	root->Clear();
	char *filename = get_full_path(bookmark_file);
	if(!filename) return xml_er_unknown;
	bookmarks->SaveFile(filename);
	free(filename);
	return 0;
}
char purge_bookmarks() {
	char status = load_bookmarks();
	if(status < 0) return status;
	TiXmlElement *root = bookmarks->FirstChildElement("bookmarks");
	if(!root) return xml_er_malformed;
	for(TiXmlElement *item = root->FirstChildElement("bookmark");
			item != 0;
			item = item->NextSiblingElement("bookmark")) {
		if(item->Attribute("book")) {
			FILE* temp = fopen(item->Attribute("book"),"rb");
			if(temp) fclose(temp);
			else root->RemoveChild(item);
		} else return xml_er_malformed;
	}
	char *filename = get_full_path(bookmark_file);
	if(!filename) return xml_er_unknown;
	bookmarks->SaveFile(filename);
	free(filename);
	return 0;
}
char get_book_by_id(const char *id, char *book, size_t *book_len) {
 	char status = load_bookmarks();
	if(status < 0) return status;
	TiXmlElement *root = bookmarks->FirstChildElement("bookmarks");
	if(!root) return xml_er_malformed;
	for(TiXmlElement *item = root->FirstChildElement("bookmark");
			item != 0;
			item = item->NextSiblingElement("bookmark")) {
		if(item->Attribute("id")) {
			if(!strcmp(item->Attribute("id"),id)) {
				const char *bookTemp = item->Attribute("book");
				if(bookTemp) {
					strncpy(book,bookTemp,*book_len-1);
					book[*book_len-1] = '\0';
					*book_len = strlen(book);
					return 0;
				}
				return xml_er_malformed;
			}
		}
	}
	return xml_er_unknown;
}
char get_saved_book(char *book, size_t *book_len) {
	return get_book_by_id("saved_book", book, book_len);
}
char get_last_book(char *book, size_t *book_len) {
	return get_book_by_id("last_book", book, book_len);
}

char set_book_by_id(const char *id, const char *book) {
 	load_bookmarks();
	if(!bookmarks) {
		bookmarks = new TiXmlDocument();
		if(!bookmarks) return xml_er_out_of_memory;
		TiXmlDeclaration *dec = new TiXmlDeclaration("1.0","","");
		if(!dec) return xml_er_out_of_memory;
		bookmarks->LinkEndChild(dec);
		TiXmlElement *root = new TiXmlElement("bookmarks");
		if(!root) return xml_er_out_of_memory;
		root->SetAttribute("version",pspcomic_v);
		bookmarks->LinkEndChild(root);
	}
	TiXmlElement *root = bookmarks->FirstChildElement("bookmarks");
	if(!root) return xml_er_malformed;
	char *filename = get_full_path(bookmark_file);
	if(!filename) return xml_er_unknown;
	for(TiXmlElement *item = root->FirstChildElement("bookmark");
			item != 0;
			item = item->NextSiblingElement("bookmark")) {
		if(item->Attribute("id")) {
			if(!strcmp(item->Attribute("id"),id)) {
				item->SetAttribute("book",book);
				bookmarks->SaveFile(filename);
				free(filename);
				return 0;
			}
		}
	}
	TiXmlElement *el = new TiXmlElement("bookmark");
	if(!el) return xml_er_out_of_memory;
	el->SetAttribute("id",id);
	el->SetAttribute("book",book);
	root->LinkEndChild(el);
	bookmarks->SaveFile(filename);
	free(filename);
	return 0;
}
char set_saved_book(const char *book) {
	return set_book_by_id("saved_book", book);
}
char set_last_book(const char *book) {
	return set_book_by_id("last_book",book);
}

/**
 * Extract the ARGB value of a color given a color element
 *
 * \param color_element The color element
 * \param color A reference to the variable which is to hold the color
 * \return 0 on success, an ::xml_error (negative) on failure
 */
static inline char extract_rgba(TiXmlElement *color_element, unsigned &color) {
	if(!color_element) return xml_er_null_pointer;
	int r, g, b, a;
	if(color_element->Attribute("red",&r) && (r <= 255) && (r >= 0) &&
			color_element->Attribute("blue",&b) && (b <= 255) && (b >= 0) &&
			color_element->Attribute("green",&g) && (g <= 255) && (g >= 0)) {
		color = (r << 16)|(g << 8)|b;
		if(color_element->Attribute("alpha",&a) && (a <= 255) && (a >= 0))
			color |= a << 24;
		return 0;
	}
	return xml_er_malformed;
}

/**
 * Given a TiXmlElement that is the ranges root, extract the char_ranges
 *
 * \param ranges The root element of the ranges
 * \return A char_ranges struct containing the ranges or NULL on error. It
 * should be destroyed with free_ranges when you are done with it.
 */
static char_ranges* extract_ranges(TiXmlElement *ranges) {
	if(!ranges) return NULL;
	char_ranges *the_ranges = (char_ranges*) malloc(sizeof(char_ranges));
	if(!the_ranges) return NULL;
	unsigned i = 0;
	for(TiXmlElement *range = ranges->FirstChildElement("range");
			range; range = range->NextSiblingElement("range")) ++i;
	the_ranges->range_begins = (unsigned*) malloc(sizeof(unsigned)*i);
	the_ranges->range_ends = (unsigned*) malloc(sizeof(unsigned)*i);
	if(!the_ranges->range_begins || !the_ranges->range_ends) {
		if(the_ranges->range_begins) free(the_ranges->range_begins);
		if(the_ranges->range_ends) free(the_ranges->range_ends);
		free(the_ranges);
		return NULL;
	}
	the_ranges->num_ranges = i;
	int j = 0;
	for(TiXmlElement *range = ranges->FirstChildElement("range");
			range; range = range->NextSiblingElement("range"), ++j) {
		const char *value_str = range->Attribute("begin");
		if(!value_str) {
			if(the_ranges->range_begins) free(the_ranges->range_begins);
			if(the_ranges->range_ends) free(the_ranges->range_ends);
			free(the_ranges);
			return NULL;
		}					
		the_ranges->range_begins[j] = strtoul(value_str,NULL,10);
		value_str = range->Attribute("end");
		if(!value_str || (strtoul(value_str,NULL,10) < the_ranges->range_begins[j])) {
			if(the_ranges->range_begins) free(the_ranges->range_begins);
			if(the_ranges->range_ends) free(the_ranges->range_ends);
			free(the_ranges);
			return NULL;
		}
		the_ranges->range_ends[j] = strtoul(value_str,NULL,10);
	}
	return the_ranges;
}

char* get_theme_name(const char *theme) {
	if(!theme) return NULL;
	char *theme_filename = new char[strlen(root_dir)+strlen(themes_dir)+strlen(theme)+strlen("/theme.xml")+1];
	sprintf(theme_filename,"%s%s%s/theme.xml",root_dir,themes_dir,theme);
	TiXmlDocument theme_file(theme_filename);
	if(!theme_file.LoadFile()) {
		delete [] theme_filename;
		return NULL;
	}
	delete [] theme_filename;
	TiXmlElement *root = theme_file.FirstChildElement("theme");
	if(!root) return NULL;
	const char *name = root->Attribute("name");
	if(name) return make_string(name);
	return NULL;
}

char get_theme_info(const char *theme, theme_info *info) {
	if(!info || !theme) return xml_er_null_pointer;
	char *theme_filename = new char[strlen(root_dir)+strlen(themes_dir)+strlen(theme)+strlen("/theme.xml")+1];
	sprintf(theme_filename,"%s%s%s/theme.xml",root_dir,themes_dir,theme);
	TiXmlDocument theme_file(theme_filename);
	if(!theme_file.LoadFile()) {
		delete [] theme_filename;
		fprintf(stderr,"%s\n",theme_file.ErrorDesc());
		return xml_er_cannot_load;
	}
	delete [] theme_filename;
	TiXmlElement *root = theme_file.FirstChildElement("theme");
	if(!root) return xml_er_malformed;
	const char *name = root->Attribute("name");
	if(name) info->theme_name = make_string(name);
	TiXmlElement *font_info = root->FirstChildElement("font");
	TiXmlElement *background_info = root->FirstChildElement("background");
	if(strcasecmp(theme,"default") != 0) {
		theme_info default_theme;
		if(get_theme_info("default",&default_theme) < 0) return xml_er_subroutine_fail;
		*info = default_theme;
	} else {
		if(!font_info) return xml_er_malformed;
		info->theme_name = NULL;
		info->font_file = NULL;
		info->bg_file = NULL;
		info->bg_fallback = 0x808080;
		info->ranges = NULL;
		info->images = NULL;
		memset(&info->font_attributes,0,sizeof(font_attr));
	}
	if(info->theme_name) free(info->theme_name);
	info->theme_name = make_string(root->Attribute("name"));
	if(font_info) {
		const char *font_file = font_info->Attribute("filename");
		if(font_file && ((strcmp(font_file,"builtin") == 0) || !strpbrk(font_file,"/:"))) {
			if(info->font_file) free(info->font_file);
			if(strcmp(font_file,"builtin") != 0) {
				info->font_file = (char*) malloc(sizeof(char)*
					(strlen(font_file)+strlen(theme)+2));
				if(!info->font_file) {
					free_theme_info(info);
					return xml_er_out_of_memory;
				}
				sprintf(info->font_file,"%s/%s",theme,font_file);
			} else {
				info->font_file = make_string("builtin");
				if(!info->font_file) {
					free_theme_info(info);
					return xml_er_out_of_memory;
				}
			}
			TiXmlElement *ranges = font_info->FirstChildElement("ranges");
			if(ranges) {
				if(info->ranges) free(info->ranges);
				info->ranges = extract_ranges(ranges);
				if(!info->ranges) {
					free_theme_info(info);
					return xml_er_null_pointer;
				}
			} else {
				if(info->ranges) {
					free(info->ranges);
					info->ranges = NULL;
				}
			}
		} else if(strcasecmp(theme,"default") == 0) return xml_er_malformed;
		const char *font_aa = font_info->Attribute("antialias");
		if(font_aa) {
			if(strcmp(font_aa,"yes") == 0) info->font_attributes.font_aa = 1;
			else if(strcmp(font_aa,"no") == 0) info->font_attributes.font_aa = 0;
		}
		int size = 0;
		if(font_info->Attribute("size",&size) && (size > 0) && (size < 65536))
			info->font_attributes.font_height = size;
		size = 0;
		if(font_info->Attribute("lspacing",&size) && (size >= 0) && (size < 65536)) 
			info->font_attributes.line_spacing = size;
		else info->font_attributes.line_spacing = 0;
		Uint32 colors_used = 0;
		for(TiXmlElement *color = font_info->FirstChildElement("color");
				color; color = color->NextSiblingElement("color")) {
			const char *context = color->Attribute("context");
			if(!context) continue;
			if(strcmp(context,"normal") == 0)
				extract_rgba(color,info->font_attributes.font_color);
			if((strcmp(context,"selected") == 0) &&
					(extract_rgba(color,info->font_attributes.selected_color) == 0))
				colors_used |= 1<<color_selected;
			if((strcmp(context,"secondary") == 0) &&
					(extract_rgba(color,info->font_attributes.secondary_color) == 0))
				colors_used |= 1<<color_secondary;
			if((strcmp(context,"tertiary") == 0) &&
					(extract_rgba(color,info->font_attributes.tertiary_color) == 0))
				colors_used |= 1<<color_tertiary;
			if((strcmp(context,"batterylow") == 0) &&
					(extract_rgba(color,info->font_attributes.battery_low_color) == 0))
				colors_used |= 1<<color_battery_low;
			if((strcmp(context,"batterycharging") == 0) &&
					(extract_rgba(color,info->font_attributes.battery_charging_color) == 0))
				colors_used |= 1<<color_battery_charging;
			if((strcmp(context,"error") == 0) &&
					(extract_rgba(color,info->font_attributes.error_color) == 0))
				colors_used |= 1<<color_error;
		}
		if(!(colors_used & (1<<color_selected)))
			info->font_attributes.selected_color = info->font_attributes.font_color;
		if(!(colors_used & (1<<color_secondary)))
			info->font_attributes.secondary_color = info->font_attributes.font_color;
		if(!(colors_used & (1<<color_tertiary)))
			info->font_attributes.tertiary_color = info->font_attributes.font_color;
		if(!(colors_used & (1<<color_battery_low)))
			info->font_attributes.battery_low_color = info->font_attributes.font_color;
		if(!(colors_used & (1<<color_battery_charging)))
			info->font_attributes.battery_charging_color = info->font_attributes.font_color;
		if(!(colors_used & (1<<color_error)))
			info->font_attributes.error_color = info->font_attributes.font_color;
	}
	if(background_info) {
		const char *bg_file = background_info->Attribute("filename");
		if(bg_file && !strpbrk(bg_file,"/:")) {	
			if(info->bg_file) free(info->bg_file);
			info->bg_file = (char*) malloc(sizeof(char)*
				(strlen(bg_file)+strlen(theme)+2));
			if(!info->bg_file) {
				free_theme_info(info);
				return xml_er_out_of_memory;
			}
			sprintf(info->bg_file,"%s/%s",theme,bg_file);
		}
		for(TiXmlElement *color = background_info->FirstChildElement("color");
				color; color = color->NextSiblingElement("color")) {
			const char *context = color->Attribute("context");
			if(!context) continue;
			if(strcmp(context,"fallback") == 0)
				extract_rgba(color,info->bg_fallback);
		}
	}
	for(TiXmlElement *image = root->FirstChildElement("image");
			image; image = image->NextSiblingElement("image")) {
		if(!info->images) {
			info->images = (SDL_Surface**) calloc(image_end,sizeof(SDL_Surface*));
		}
		if(!info->images) return xml_er_out_of_memory;
		const char *img_file = image->Attribute("filename");
		if(image->Attribute("id") && (strcmp(image->Attribute("id"),"image_usb") != 0))
			continue;
		if(img_file && !strpbrk(img_file,"/:")) {
			//Don't lookup at the moment because there's only one image
			if(info->images[0]) {
				SDL_FreeSurface(info->images[0]);
				info->images[0] = NULL;
			}
			char *real_img_file = (char*)
				malloc(sizeof(char)*(strlen(root_dir)+strlen(themes_dir)+strlen(theme)+strlen(img_file)+2));
			if(!real_img_file) {
				free_theme_info(info);
				return xml_er_out_of_memory;
			}
			sprintf(real_img_file,"%s%s%s/%s",root_dir,themes_dir,theme,img_file);
			info->images[0] = IMG_Load(real_img_file);
		}
	}
	for(TiXmlElement *color = root->FirstChildElement("color");
			color; color = color->NextSiblingElement("color")) {
		const char *context = color->Attribute("context");
		if(!context) continue;
		if(strcmp(context,"zoombox1") == 0)
			extract_rgba(color,info->zb_color_1);
		if(strcmp(context,"zoombox2") == 0)
			extract_rgba(color,info->zb_color_2);
	}
	return 0;
}

char* get_language_name(const char *lang) {
	if(!lang) return NULL;
	char *language_filename = new char[strlen(root_dir)+strlen(languages_dir)+strlen(lang)+strlen(".xml")+1];
	sprintf(language_filename,"%s%s%s.xml",root_dir,languages_dir,lang);
	TiXmlDocument theme_file(language_filename);
	if(!theme_file.LoadFile()) {
		delete [] language_filename;
		return NULL;
	}
	delete [] language_filename;
	TiXmlElement *root = theme_file.FirstChildElement("language");
	if(!root) return NULL;
	const char *name = root->Attribute("name");
	if(name) return make_string(name);
	return NULL;
}

char get_language_ranges(const char *lang, char_ranges *ranges_ptr) {
	if(!lang || !ranges_ptr) return xml_er_null_pointer;
	char *language_filename = new char[strlen(root_dir)+strlen(languages_dir)+strlen(lang)+strlen(".xml")+1];
	sprintf(language_filename,"%s%s%s.xml",root_dir,languages_dir,lang);
	TiXmlDocument theme_file(language_filename);
	if(!theme_file.LoadFile()) {
		delete [] language_filename;
		return xml_er_cannot_load;
	}
	delete [] language_filename;
	TiXmlElement *root = theme_file.FirstChildElement("language");
	if(!root) return xml_er_malformed;
	char_ranges *ret = extract_ranges(root->FirstChildElement("ranges"));
	if(ret) *ranges_ptr = *ret;
	else return xml_er_subroutine_fail;
	return 0;
}

char** get_language_messages(const char *lang) {
	if(!lang) return NULL;
	char *language_filename = new char[strlen(root_dir)+strlen(languages_dir)+strlen(lang)+strlen(".xml")+1];
	sprintf(language_filename,"%s%s%s.xml",root_dir,languages_dir,lang);
	TiXmlDocument theme_file(language_filename);
	if(!theme_file.LoadFile()) {
		delete [] language_filename;
		return NULL;
	}
	delete [] language_filename;
	TiXmlElement *root = theme_file.FirstChildElement("language");
	if(!root) return NULL;
	TiXmlElement *messages_root = root->FirstChildElement("messages");
	if(!messages_root) return NULL;
	char **messages = (char**)malloc(sizeof(char*)*mesg_end);
	if(!messages) return NULL;
	memset(messages,0,sizeof(char*)*mesg_end);
	for(TiXmlElement *message = messages_root->FirstChildElement("message");
			message; message = message->NextSiblingElement("message")) {
		const char *message_name = message->Attribute("id");
		if(!message_name) continue;
		for(int i = 0; i < mesg_end; ++i) {
			if(strcasecmp(message_name,messages_table[i]) == 0) {
				const char *value = message->Attribute("value");
				if(!value) break;
				char *temp = make_string(value);
				if(!temp) break;	//Notice! If PSPComic runs out of memory
									//here, you will get an incomplete language!
				//MAKE SURE THERE ARE THE RIGHT ESCAPE CHARACTERS FOR *PRINTFS!
				const char *percent_in_en = strchr(en_messages[i],'%');
				if(percent_in_en) {
					bool good = true;
					for(char *percent_in_for = strchr(temp,'%');
							percent_in_en || percent_in_for;) {
						if(!(percent_in_en && percent_in_for) ||
								(!percent_in_for &&
								(*(percent_in_en+1) != '%')) ||
								(*(percent_in_for+1) != *(percent_in_en+1))) {
							good = false;
							break;
						}
						if(*(++percent_in_for) == '%') ++percent_in_for;
						if(*(++percent_in_en) == '%') ++percent_in_en;
						percent_in_for = strchr(percent_in_for,'%');
						percent_in_en = strchr(percent_in_en,'%');
					}
					if(!good) break;
				}
				if(messages[i]) free(messages[i]);
				messages[i] = temp;
				break;
			}
		}
	}
	return messages;
}
