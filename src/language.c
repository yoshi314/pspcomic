/** \file
    Language-related functions

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

const char *messages_table[] = {
	/* Begin errors */
	"out_of_memory",
	"sdl_error",
	"sdl_image_error",
	"sdl_ttf_error",
	"open_book_error",
	"open_file_error",
	"read_file_error",
	"load_theme_error",
	"load_page_error",
	"load_language_error",
	"language_too_long_error",
	"theme_too_short_error",
	"zip_name_too_long_error",
	"empty_book_error",
	"insert_ms",
	"internal_error",
	/* End errors */
	/* Begin status messages */
	"loading",
	"closing",
	/* End status messages */
	/* Begin menu messages */
	"accept",
	"cancel",
	"more_above",
	"more_below",
	"go_back",
	"reload_menu",
	"theme_preview",
	"load_last",
	"load_saved",
	"no_comics",
	"no_themes",
	"open_dir",
	/* Main menu */
	"open_book",
	"close_book",
	"next_book",
	"prev_book",
	"jump_page",
	"load_bookmark",
	"set_bookmark",
	"set_saved",
	"more_bookmark",
	"config",
	"about",
	"quit",
	/* Config menu */
	"resize_to_nn", "resize_to_resample",
	"adjust_clock",
	"adjust_pan",
	"enable_zoom_persist", "disable_zoom_persist",
	"enable_rotate_persist", "disable_rotate_persist",
	"adjust_scroll_skip",
	"enable_manga_mode", "disable_manga_mode",
	"enable_bookmark_on_load", "disable_bookmark_on_load",
	"set_autozoom",
	"set_zoom_box_w", "set_zoom_box_h",
	"enable_precaching", "disable_precaching",
	"enable_singlehanded", "disable_singlehanded",
	"enable_analog", "disable_analog",
	/*"enable_dynamic_cpu", "disable_dynamic_cpu",*/
	"set_theme",
	"set_language",
	"save_config",
	"load_config",
	/* Bookmark menu */
	"delete_bookmarks",
	"delete_all_bookmarks",
	"purge_bookmarks",
	/* Autozoom mode menu */
	"full_width",
	"full_view",
	"twice_width",
	"autodetect",
	/* Other menus */
	"page",
	"adjust_clock_top",
	"adjust_pan_top",
	"adjust_scroll_skip_top",
	"adjust_zoom_box_w_top",
	"adjust_zoom_box_h_top"
};
const char *en_messages[] = {
	/* Begin errors */
	"Out of memory",
	"SDL Error: %s",
	"SDL_image Error: %s",
	"SDL_ttf Error: %s",
	"Error opening book",
	"Error opening %s file",
	"Error reading %s file",
	"Error loading theme",
	"Could not load page from archive. Bailing...",
	"Error loading language",
	"The loaded language does not support this theme",
	"The loaded theme does not support this language",
	"Filename in zip too long",
	"Book is empty",
	"Please insert a Memory Stick Duo",
	"Internal error",
	/* End errors */
	/* Begin status messages */
	"Loading...",
	"Closing program...",
	/* End status messages */
	/* Begin menu messages */
	"Accept",
	"Cancel",
	"[More]",
	"[More]",
	"< Go back",
	"Reload menu",
	"Press cross to select theme or triangle to go back",
	"Load last opened comic book",
	"Load saved comic book",
	"Please place comic books in the /comics folder",
	"You have no themes. Please replace the default theme.",
	"Open this directory",
	/* Main menu */
	"Open comic book...",
	"Close comic book",
	"Next comic book",
	"Previous comic book",
	"Jump to page...",
	"Load bookmark",
	"Set to bookmark",
	"Save comic book",
	"More bookmark operations...",
	"Configuration...",
	"About",
	"Quit",
	/* Config menu */
	"Change resize method to nearest neighbor", "Change resize method to resample",
	"Adjust clock frequency...",
	"Adjust pan rate...",
	"Turn on zoom level persistence", "Turn off zoom level persistence",
	"Turn on rotation persistence", "Turn off rotation persistence",
	"Adjust menu scroll skip rate...",
	"Turn on manga mode", "Turn off manga mode",
	"Jump to bookmark upon loading comic book", "Do not jump to bookmark upon loading comic book",
	"Set autozoom mode...",
	"Set zoom box width...", "Set zoom box height...",
	"Turn on precaching", "Turn off precaching",
	"Turn on single-handed mode", "Turn off single-handed mode",
	"Turn on analog nub", "Turn off analog nub",
	/*"Turn on dynamic CPU scaling", "Turn off dynamic CPU scaling",*/
	"Change theme...",
	"Change language...",
	"Save configuration",
	"Load configuration",
	/* Bookmark menu */
	"Delete bookmark for this comic book",
	"Delete all bookmarks",
	"Purge bookmarks",
	/* Autozoom mode menu */
	"Fit page to screen width",
	"View page at original width",
	"Fit page to twice screen width (good for spreads)",
	"Autodetect spread mode",
	/* Other menus */
	"Page %u: ",
	"Set clock frequency (MHz)",
	"Set pan rate:",
	"Set scroll skip:",
	"Set zoom box width:",
	"Set zoom box height:",
};

///The currently loaded language
static language *loaded_language = NULL;
///The name of the currently loaded language
static char *loaded_language_name = NULL;

const char* get_message(int mesg_number) {
	if(mesg_number >= mesg_end) return NULL;
	return ((loaded_language&&loaded_language->messages)?
		(loaded_language->messages[mesg_number]?
			loaded_language->messages[mesg_number]:en_messages[mesg_number]):
		en_messages[mesg_number]);
}

char load_language(const char *lang) {
	if(!lang) return 0;
	if(strcmp(lang,"builtin") == 0) {
		if(loaded_language) unload_language();
		return 1;
	}
	char_ranges *ranges = malloc(sizeof(char_ranges));
	//get_language_ranges will catch if ranges is NULL
	if(get_language_ranges(lang,ranges) != 0) {
		if(ranges) free(ranges);
		error_message(get_message(mesg_load_language_error));
		return 0;
	}
	if(!ranges_compatible(get_font_ranges(),*ranges)) {
		free(ranges->range_begins);
		free(ranges->range_ends);
		free(ranges);
		error_message(get_message(mesg_theme_too_short_error));
		return 0;
	}
	language *new_language = malloc(sizeof(language));
	if(!new_language) {
		free(ranges->range_begins);
		free(ranges->range_ends);
		free(ranges);
		error_message(get_message(mesg_load_language_error));
		return 0;
	}
	new_language->ranges = ranges;
	new_language->messages = get_language_messages(lang);
	if(!new_language->messages) {
		free(ranges->range_begins);
		free(ranges->range_ends);
		free(ranges);
		free(new_language);
		error_message(get_message(mesg_load_language_error));
		return 0;
	}
	if(loaded_language) unload_language();
	loaded_language = new_language;
	loaded_language_name = make_string(lang);
	return 1;
}

void unload_language() {
	if(loaded_language_name) {
		free(loaded_language_name);
		loaded_language_name = NULL;
	}
	if(!loaded_language) return;
	if(loaded_language->messages) {
		unsigned i;
		for(i = 0; i < mesg_end; ++i) {
			if(loaded_language->messages[i]) free(loaded_language->messages[i]);
		}
		free(loaded_language->messages);
	}
	if(loaded_language->ranges) {
		if(loaded_language->ranges->range_begins) free(loaded_language->ranges->range_begins);
		if(loaded_language->ranges->range_ends) free(loaded_language->ranges->range_ends);
		free(loaded_language->ranges);
	}
	free(loaded_language);
	loaded_language = NULL;
}
char_ranges get_loaded_language_ranges() {
	if(loaded_language) return *loaded_language->ranges;
	else return default_ranges;
}
const char* get_loaded_language_name() {
	if(loaded_language_name) return loaded_language_name;
	return "builtin";
}
