/** \file
    General PSPComic-related header

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
#ifndef PSPComic_h
#define PSPComic_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <malloc.h>
#include <errno.h>
#include <stdarg.h>

#include "SDL.h"
#include "SDL_thread.h"
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#ifdef USE_GFX
#include <SDL/SDL_rotozoom.h>
#endif

#if !defined(PSP) && !defined(_WIN32)
#include <sys/stat.h>
#endif

#include "zip/unzip.h"
#include "rar.hpp"
#include "utf8_sjis.h"
#ifdef __cplusplus
}
#include "tinyxml/tinyxml.h"
extern "C" {
#endif /* __cplusplus */
#ifdef PSP
#else /* PSP */
#ifdef fflush
#undef fflush
#endif /* fflush */
#endif /* PSP */

///The sensitivity of the joystick
extern const float joy_sense;

///The delay between steps while panning
extern const Uint8 pan_delay;
///The size of the deadzone of the joystick
extern const Uint16 dead_zone;

///The delay between steps while scrolling in the menu
extern const Uint16 menu_scroll_rate;

///The delta in zoom of a single zoom in/out action
extern const float zoom_amount;
///The maximum zoom level
extern const float zoom_max;
///The minimum zoom level
extern const float zoom_min;
///The maximum number of pixels of a single pan action
extern const Uint8 pan_max;
///the minimum number of pixels of a single pan action
extern const Uint8 pan_min;
///Whether or not something is selected in the current execution
///\warning Be wary of this value and threads
extern char selection_on;
///The version of PSPComic this is, expressed as a string
extern const char pspcomic_v[];

///An array containing the names of all of the messages
extern const char *messages_table[];
///An array containing the builtin English values of all of the messages.
extern const char *en_messages[];

///Size of the default font
extern const unsigned int size_default_font;
///The default font
extern const unsigned char default_font[];

#ifdef PSP
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspsdk.h>
#include <pspmscm.h>
//#include "reg.h"
#ifdef USE_USB
#include <pspusb.h>
#include <pspusbstor.h>
#endif /* USE_USB */
///The root directory
#define root_dir "ms0:"
#elif defined(_WIN32) /* PSP */
///The root directory
#define root_dir "C:"
#else /* _WIN32 */
///The root directory (only a variable in Unix!)
extern char *root_dir;
#endif /* PSP */
///The directory for comics relative to the root directory
#define cb_dir "/comics/"
///The directory for PSPComic's data
#define data_dir cb_dir ".pspcomic/"
///The filename of the config file
#define config_file data_dir "config.xml"
///The filename for the bookmarks file
#define bookmark_file data_dir "bookmarks.xml"
///The directory for themes
#define themes_dir data_dir "themes/"
///The directory for languages
#define languages_dir data_dir "languages/"

///Commands that are used in the input system and in the menu system

///Hey kids! Never combine things like this, it only leads to pain later.
enum command {
	///No event was encountered
	no_event		= 0x00000000,
	///The end of an event was encountered (ORed with other values)
	event_end		= 0x00000001,
	///Pan up input
	pan_up			= 0x00000002,
	///Pan right input
	pan_right		= 0x00000004,
	///Pan down input
	pan_down		= 0x00000008,
	///Pan left input
	pan_left		= 0x00000010,
	///Rotate clockwise input
	rotate_cw		= 0x00000020,
	///Rotate counter-clockwise input
	rotate_ccw		= 0x00000040,
	///Zoom in input
	zoom_in			= 0x00000080,
	///Zoom out input
	zoom_out		= 0x00000100,
	///Zoom to autozoom (previously called "Zoom to fixed") input
	zoom_fixed		= 0x00000200,
	///Go back in the menu input. Note: This has the same value as ::zoom_in
	go_back			= 0x00000080,
	///Choose selected option input. Note: this has the same value as ::zoom_out
	select_option	= 0x00000100,
	///Next page input. Note: This is also used for scroll skipping up
	next_page		= 0x00000400,
	///Previous page input. Note: This is also used for scroll skipping down
	prev_page		= 0x00000800,
	///Open/Close menu input
	open_menu		= 0x00001000,
	///Quit the program
	quit_command	= 0x00002000,
	///Open a submenu
	open_submenu	= 0x00004000,
	///Close the current book
	close_book		= 0x00008000,
	///Jump to a specific page
	jump_page		= 0x00010000,
	///Set a pointer (to some value)
	set_pointer		= 0x00020000,
	///Toggle a global
	toggle_global	= 0x00040000,
	///Open a popup
	popup			= 0x00080000,
	///Adjust the clock frequency
	clock_adjust	= 0x00100000,
	///Send a custom signal to the current input handling (used in threads)
	redraw_ev		= 0x20000000,
	///Use a command that is enumerated in the auxiliary commands enumeration
	aux_command		= 0x40000000,
	///An error occurred
	error_ev		= 0x80000000
};
///All of the pan event enumerators ORed together
#define pan_event (pan_up|pan_right|pan_down|pan_left)
///All of the horizontal pan event enumerators ORed together
#define pan_horiz (pan_left|pan_right)
///All of the vertical pan event enumerators ORed together
#define pan_vert (pan_up|pan_down)
///Auxiliary commands

///Basically, commands that will never be ORed with anything
enum aux_command {
	///Load configuration command
	load_config_command,
	///Save configuration command
	save_config_command,
	///Retrieve (and then jump to) bookmark
	retrieve_bookmark,
	///Add current page as bookmark
	add_bookmark,
	///Delete bookmark for current comic book. Note: plural in case multiple
	///bookmarks per comic book are ever implemented
	delete_bookmarks_command,
	///Delete bookmarks for all comic books
	delete_all_bookmarks_command,
	///Purge bookmarks. (Delete bookmarks for comics books that no logner exist)
	purge_bookmarks_command,
	///Save comic book command
	save_book_command,
	///Open comic book command
	open_book,
	///Open comic book by saved identifier
	open_book_by_id,
	///Open next comic book command
	next_book,
	///Open previous comic book command
	prev_book,
	///Preview theme command
	preview_theme,
	///Set theme command
	set_theme,
	///Set language command
	set_language
};
///Autozoom modes
enum autozoom_mode {
	///Custom zoom (i.e., autozoom is not enabled)
	custom_zoom = 0,
	///Full view (i.e., full size)
	full_view,
	///Full width (i.e., fit to width)
	full_width,
	///Twice width
	twice_width,
	///Autodetect zoom mode (i.e., full width or twice width, when appropriate)
	autodetect_zoom
};
///Comic book file types
enum book_file_type {
	///Invalid comic book file type (this should pretty much never come up)
	invalid_comic_type = 0,
	///Zip Comic Book Archive (CBZ)
	comic_book_zip,
	///RAR Comic Book Archive (CBR)
	comic_book_rar,
	///Unarchived (directory)
	comic_book_dir
};
///Access device codes, used with ::access_device
enum access_device_code {
	///Access screen
	access_screen = 0x1,
	///Access joystick
	access_joystick = 0x2
};
///Access globals, used with ::access_int_global
enum access_global_code {
	///Access resize method
	access_resize = 1,
	///Access pan rate
	access_pan_rate,
	///Access zoom persistence mode
	access_zoom_persist,
	///Access rotation persistence mode
	access_rotate_persist,
	///Access scroll skip amound
	access_scroll_skip,
	///Access precaching mode
	access_precaching,
	///Access manga mode
	access_manga_mode,
	///Access zoom box width
	access_zoom_box_w,
	///Access zoom box height
	access_zoom_box_h,
	///Access "jump to bookmark on load" mode
	access_bookmark_on_load,
	///Access autozoom mode
	access_autozoom_mode,
	///Access single-handed mode
	access_singlehanded,
	///Access analog disabled mode
	access_analog_disabled,
	///Access dynamic CPU scaling mode
	access_dynamic_cpu
};
///Resize methods
enum resize_method {
	///Resize nearest neighbor
	resize_nn = 0,
	///Really crappy resample method
	resample,
	///Do it in hardware (probably slower than software)
	resize_hardware
};
///Menu types
enum menu_type {
	///An option (i.e., selectable different options) menu
	option_menu,
	///A number (i.e., input a number) menu
	number_menu,
	///Preview the theme
	preview_theme_menu
};
///::generate_text flags
enum generate_text_flag {
	///Generate text to be antialiased
	text_antialias		= 0x01
};
///Theme-defined colors
enum theme_color {
	///Normal color
	color_normal,
	///Item selected color
	color_selected,
	///Secondary color
	color_secondary,
	///Tertiary color
	color_tertiary,
	///Battery is low color
	color_battery_low,
	///Battery is charging color
	color_battery_charging,
	///Error text color
	color_error,
	///Zoom box color 1
	color_zb_1,
	///Zoom box color 2
	color_zb_2
};
///Messages (see ::en_messages for corresponding English values)
enum message_enum {
	/* Begin errors */
	mesg_out_of_memory = 0,
	mesg_sdl_error,
	mesg_sdl_image_error,
	mesg_sdl_ttf_error,
	mesg_open_book_error,
	mesg_open_file_error,
	mesg_read_file_error,
	mesg_load_theme_error,
	mesg_load_page_error,
	mesg_load_language_error,
	mesg_language_too_long_error,
	mesg_theme_too_short_error,
	mesg_zip_name_too_long_error,
	mesg_empty_book_error,
	mesg_insert_ms,
	mesg_internal_error,
	/* End errors */
	/* Begin status messages */
	mesg_loading,
	mesg_closing,
	/* End status messages */
	/* Begin menu messages */
	mesg_accept,
	mesg_cancel,
	mesg_more_above,
	mesg_more_below,
	mesg_go_back,
	mesg_reload_menu,
	mesg_theme_preview,
	mesg_load_last,
	mesg_load_saved,
	mesg_no_comics,
	mesg_no_themes,
	mesg_open_dir,
	/* Main menu */
	mesg_open_book,
	mesg_close_book,
	mesg_next_book,
	mesg_prev_book,
	mesg_jump_page,
	mesg_load_bookmark,
	mesg_set_bookmark,
	mesg_set_saved,
	mesg_more_bookmark,
	mesg_config,
	mesg_about,
	mesg_quit,
	/* Config menu */
	mesg_resize_to_nn, mesg_resize_to_resample,
	mesg_adjust_clock,
	mesg_adjust_pan,
	mesg_enable_zoom_persist, mesg_disable_zoom_persist,
	mesg_enable_rotate_persist, mesg_disable_rotate_persist,
	mesg_adjust_scroll_skip,
	mesg_enable_manga_mode, mesg_disable_manga_mode,
	mesg_enable_bookmark_on_load, mesg_disable_bookmark_on_load,
	mesg_set_autozoom,
	mesg_set_zoom_box_w, mesg_set_zoom_box_h,
	mesg_enable_precaching, mesg_disable_precaching,
	mesg_enable_singlehanded, mesg_disable_singlehanded,
	mesg_enable_analog, mesg_disable_analog,
	/*mesg_enable_dynamic_cpu, mesg_disable_dynamic_cpu,*/
	mesg_set_theme,
	mesg_set_language,
	mesg_save_config,
	mesg_load_config,
	/* Bookmark menu */
	mesg_delete_bookmarks,
	mesg_delete_all_bookmarks,
	mesg_purge_bookmarks,
	/* Autozoom mode menu */
	mesg_full_width,
	mesg_full_view,
	mesg_twice_width,
	mesg_autodetect,
	/* Other menus */
	mesg_page,
	mesg_adjust_clock_top,
	mesg_adjust_pan_top,
	mesg_adjust_scroll_skip_top,
	mesg_adjust_zoom_box_w_top,
	mesg_adjust_zoom_box_h_top,
	///End of messages (used internally)
	mesg_end
};
///Per-theme images
enum theme_image {
	///The image used as an indication that USB is on
	image_usb = 0,
	///Used internally
	image_end
};

///A comic book
typedef struct {
	///The ::book_file_type
	int type;
	///The zip file, open for unzipping, if it is a ::comic_book_zip
	unzFile zip_file;
	///The full name of the file
	char *filename;
	///The file name
	char *localname;
	///The currently displayed page, where the first page is 0
	Uint16 current_page;
	///Total number of pages
	Uint16 num_pages;
	///The names of the pages, in order
	char **page_order;
} comic_book;
///An event (used primarly in ::show_menu)
typedef struct {
	///The text that is displayed for the event
	char *option_text;
	///The color that is displayed for the event
	unsigned option_color;
	///The ::command that goes along with the event
	int command;
	///A number associated with the event::command (if command is
	///command::aux_command, then this should be where the ::aux_command goes)
	int int_data;
	///A pointer associated with the event::command
	int *ptr_data;
} event;
///A structure that holds pointers to devices

///With the current implementation, it can only hold one device at a time. This
///may change in the future
typedef struct {
	///The type of device contained in this struct (a ::access_device_code)

	///In the future, these may be ORed together, but this is not currently
	///supported
	int type;
	///A pointer to the screen
	SDL_Surface *screen;
	///A pointer to the joystick
	SDL_Joystick *joystick;
} device;
///Information pertaining to the zoom box that is shared between threads
typedef struct {
	///Whether or not the zoom box is actually active
	char zoom_box_is_active;
	///Stores what the current input action is, to make sure we still want the
	///zoom box to open
	int *action_status;
	///A pointer to the full-size page
	SDL_Surface *page_img;
	///A pointer to the current X offest
	
	///It is a pointer just in case the value changes while the button is being
	///held
	Uint16 *offsetX;
	///A pointer to the current Y offest
	
	///It is a pointer just in case the value changes while the button is being
	///held
	Uint16 *offsetY;
	///The current scale of the page
	float scale_ratio;
	///The current rotation of the page
	Uint8 rotation;
	///A pointer to the page scaled to what it is currently on the screen
	SDL_Surface *scaled_page;
} zoom_box_info;
///Information pertaining to precaching that is shared between threads
typedef struct {
	///Once the page is loaded, this will point to the image
	SDL_Surface *new_page;
	///The number of the page
	Uint16 page;
	///The status of whether or not it is loaded

	/** Possible values:
	 * \li 0: Not started loading
	 * \li 1: Started loading
	 * \li 2: Finished loading successfully
	 * \li -1: Wrong page was loaded
	 * \li -2: Ran out of memory
	 * \li -3: Other error
	 */
	char loaded;
	///A pointer to the comic_book that is being precached
	comic_book *book;
} precache_info;
///A structure for defining ranges of characters that are available in a font or
///theme
typedef struct {
	///The total number of rangs
	unsigned num_ranges;
	///An array of the beginning of each range
	unsigned *range_begins;
	///An array of the end of each range
	unsigned *range_ends;
} char_ranges;
///Attributes pertaining to fonts
typedef struct {
	///The height of the font loaded
	Uint16 font_height;
	///The default color of the font
	Uint32 font_color;
	///Whether or not the font should be antialiased
	char font_aa;
	///The height of the font + the extra space between lines
	Uint16 line_spacing;
	///The color for selected text
	Uint32 selected_color;
	///The color for secondary text
	Uint32 secondary_color;
	///The color for tertiary text
	Uint32 tertiary_color;
	///The color for battery low warning text
	Uint32 battery_low_color;
	///The color for battery charging text
	Uint32 battery_charging_color;
	///The color for error text
	Uint32 error_color;
} font_attr;
///Information pertaining to themes
typedef struct {
	///The internal name of the theme
	char *theme_name;
	///The name of the font file
	char *font_file;
	///Attributes pertaining to the font
	font_attr font_attributes;
	///Ranges that are valid for the font
	char_ranges *ranges;
	///The background file
	char *bg_file;
	///The color to be used if the background can't be loaded
	Uint32 bg_fallback;
	///Images preloaded for the theme
	SDL_Surface **images;
	///Zoom box color 1 (moving section)
	Uint32 zb_color_1;
	///Zoom box color 2 (static section)
	Uint32 zb_color_2;
} theme_info;
///A loaded language file
typedef struct {
	///The text in the language file
	char **messages;
	///The ranges that the language requires
	char_ranges *ranges;
} language;

///The default ranges that are returned if something doesn't specify its ranges
extern const char_ranges default_ranges;

#include "xmlfuncs.hpp" //XML functions must be included after typedefs

/**
 * Display the last error SDL generated
 *
 * \warning Due to the fact that this function internally uses ::say_centered,
 * the color of the text is also subject to be colored by ::selection_on and
 * ::set_explicit_color
 * 
 * \sa error_message
 */
inline void sdl_error(void);

/**
 * Output an error message
 *
 * The screen is automatically cleared and the error message is printed centered
 * on the screen using ::say_centered over the background. If a theme is not
 * loaded, the string is printed to stderr. The output message must not be
 * greater than 1023 characters, or else it will be truncated. The program then
 * pauses for two and a half seconds.
 *
 * \param message A printf-formatted string
 * \param ... The additional arguments for the printf-formatted string
 *
 * \warning Due to the fact that this function internally uses ::say_centered,
 * the color of the text is also subject to be colored by ::selection_on and
 * ::set_explicit_color
 *
 * \sa say
 * \sa say_centered
 */
#ifndef DOXYGEN
__attribute__((format(printf,1,2)))
#endif
void error_message(const char *message, ...);

/**
 * Print a string on the screen
 *
 * Print a string starting from the X and Y stored (which can be reset by
 * ::reset_text_pos or ::setXY) and continuing until the end of the text is
 * reached (and may run over beyond the bottom of the screen). Text is wrapped,
 * if necessary. It is printed in the current color, which is either the theme's
 * font_attr::font_color, the theme's font_attr::selected_color,
 * if ::selection_on is non-zero, or the color set by ::set_explicit_color.
 *
 * \param message A printf-formatted string
 * \param ... The additional arguments for the printf-formatted string
 *
 * \sa error_message
 * \sa say_centered
 */
#ifndef DOXYGEN
__attribute__((format(printf,1,2)))
#endif
void say(const char *message, ...);

/**
 * Initialize the screen
 *
 * \param w The width of the screen
 * \param h The height of the screen
 *
 * \sa access_device
 */
void init_video(Uint16 w, Uint16 h);

/**
 * Access a device which will contain a pointer that can be used with other SDL
 * functions
 *
 * \param device_code An ::access_device_code
 * \return A ::device struct containing the pointer
 *
 * \note Only one ::access_device_code may be specified at the current time,
 * despite the fact that multiple codes can be conceivably ORed together. This
 * will not be recognized and you will get an empty struct back. This may change
 * in the future
 *
 * \sa access_int_global
 */
device access_device(int device_code);

/**
 * Access an int global
 *
 * Rather than have globals clog up the global namespace, they are all static in
 * this function and can be returned as pointers. There is a global
 * corresponding to every value of ::access_global_code, although in the future,
 * some new values may be for other data types
 *
 * \param global_code The ::access_global_code
 * \return The pointer corresponding to the value, or NULL if the code is
 * invalid
 */
int* access_int_global(int global_code);

/**
 * Set the clock frequency of the PSP's processor, etc.
 *
 * \param new_clock The frequency to which the proccesor should be set
 *
 * \note The value must be between 10 and 333, inclusive
 * \note This has no effect if it is called on any system other than the PSP
 * \return Whatever scePowerSetClockFrequency returns, or 0 on systems other
 * than the PSP
 */
int set_clock(int new_clock);

/**
 * Switch the display and draw buffers
 */
void flip_screen(void);
/**
 * Fill the draw buffer with the current theme's theme_info::bg_fallback color
 *
 * \note 0x7F7F7F is used if no theme is loaded (which should never happen in
 * the first place)
 */
void clear_screen(void);

/**
 * Show an image
 *
 * If the image is less in one dimension than the screen, it is centered,
 * otherwise it is place offsetX or offsetY from the origin in the top left
 * corner. For example, if offsetX is 10, the first 10 rows of pixels are not
 * shown. If the image is alpha transparent, ::clear_screen is called before
 * drawing. If the image is smaller that the screen in one dimension, you should
 * call ::clear_screen yourself before calling this function.
 *
 * \param surf A pointer to the image to show
 * \param offsetX The offset to use in the X axis
 * \param offsetY The offset to use in the Y axis
 * \param flip Whether or not the screens should be flipped after drawing (0 is 
 * no, anything else is yes)
 * \return 1 on success, 0 on failure
 *
 * \bug If offsetX or offsetY are non-zerp for an image that's smaller than the
 * screen in that direction, things break. Make sure to check first.
 *
 * \sa show_image2
 */
char show_image(SDL_Surface *surf, Uint16 offsetX, Uint16 offsetY, char flip);

/**
 * Show a portion of an image on a portion of the screen, resising and rotating
 * as needed
 *
 * This function is useful for cropping and resizing images as needed, but
 * should be used sparingly because it is quite slow. It will resize the src
 * rectangle so that it has the same dimensions as the dst rectangle after
 * rotating clockwise the input number of 90 degree rotations.
 *
 * \param surf A pointer to the image to show
 * \param src The source rectangle to clip out and use. If null, the whole image
 * is used
 * \param dst The destination rectangle on the screen to which to resize and
 * show the src rectangle. If null, the top-left corner is assumed
 * \param rotation The number of 90 degree clockwise turns the clipped region
 * should be rotated before resizing
 * \param alpha Whether or not alpha should be enabled (0 is no, anything else
 * is yes)
 * \param flip Whether or not the screens should be flipped after drawing (0 is 
 * no, anything else is yes)
 * \return 1 on success, 0 on failure
 *
 * \sa show_image
 */
char show_image2(SDL_Surface *surf, SDL_Rect *src, SDL_Rect *dst, char rotation, char alpha, char flip);

/* In gfx.c */
/**
 * Get a pointer to the pixel at x and y on the surface speficied
 *
 * \param surf The surface from which to get the pixel
 * \param x The x coordinate of the pixel to get
 * \param y The y coordinate of the pixel to get
 * \return The pixel at the coordinates specified
 *
 * \warning Make sure surface is locked first!
 * \warning This function does not do boundary checking!
 */
inline void* access_pixel(const SDL_Surface* surf, const Uint16 x, const Uint16 y);

/**
 * Copy the value of one pixel to another pixel of the same format
 *
 * \param src The source pixel
 * \param dst The destination pixel
 * \param bpp The the number of bytes of each pixel
 *
 * \warning This function does not check the validity of the pixels!
 * \note This function is basically a memcpy, but it is implemented slightly
 * differently on the PSP
 */
inline void copy_pixel(const void *src, void *dst, Uint8 bpp);

/**
 * Blend the specified color into a pixel of the specified format
 *
 * \param pixel The pixel on which to operate
 * \param fmt The format of the specified pixel
 * \param r The value of the red that should be used
 * \param b The value of the blue that should be used
 * \param g The value of the green that should be used
 * \param a The value of the alpha that should be used
 *
 * \note This function only works on little endian machines
 */
inline void blend(void *pixel, SDL_PixelFormat *fmt, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/**
 * Rotate and/or resize a surface
 *
 * \param old The surface to rotate and/or resize
 * \param clip The region to rotate and/or resize
 * \param newW The width after resizing but before rotating
 * \param newH The height after resizing but before rotating
 * \param rotation The number of 90 degree clockwise turns to make
 * \return The new surface, or NULL if an error was encountered
 *
 * \note The surface will either be resized via nearest neighbor or a very (bad)
 * basic interpolation method depending on what the resize_method global is set
 * as
 * \warning The new surface will may be the same pointer as the old surface, so
 * make sure to check if they are different before freeing the old one
 */
SDL_Surface* rotozoom(SDL_Surface *old, SDL_Rect *clip, Uint16 newW, Uint16 newH, char rotation);

/**
 * Copy the palette from one surface to another
 *
 * \param old The surface from which to copy the palette
 * \param new_surf The destination surface
 * \return 1 on success
 * \return 0 if the SDL_SetPalette call fails
 * \return -1 if one or both of the surfaces are not 8-bit
 * \return -2 if memory runs out
 *
 * \note This function has not been tested on 
 */
int copy_palette(SDL_Surface *old, SDL_Surface *new_surf);

/**
 * Draw a rectangular outline in the given location on the given surface in the
 * given color
 *
 * \param surf The surface on which to draw the rectangle
 * \param rect The location and dimensions of the rectangle to draw
 * \param r The red component of the color to draw the rectangle
 * \param g The blue component of the color to draw the rectangle
 * \param b The green component of the color to draw the rectangle
 * \param a The alpha component of the color to draw the rectangle
 *
 * \warning This function does not do any bounds checking
 */
void blend_rect(SDL_Surface *surf, SDL_Rect rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
/* End in gfx.c */

/**
 * Draw the zoom box
 *
 * \param page_img The unscaled image
 * \param scale_ratio The ratio to which the image is currently scaled
 * \param rotation The current rotation of the image
 * \param x The x location of the zoom box relative to offsetX
 * \param y The y location of the zoom box relative to offsetY
 * \param offsetX The x offset of the page
 * \param offsetY The y offset of the page
 * \param zoom_level The scale factor for the zoom box (1 <= zoom_level <= 4)
 */
void draw_zoom_box(SDL_Surface *page_img, float scale_ratio, Uint8 rotation, Sint32 x, Sint32 y, Uint16 offsetX, Uint16 offsetY, Uint8 zoom_level);

/**
 * Allocates and initialize a block of memory to 0
 *
 * \param kb The number of kilobytes to allocate
 * \return A pointer to the memory, initialized as all 0s, or NULL on error
 *
 * \note This memory is malloced, so make sure to use free to free it
 *
 * \sa make_string
 */
inline char* init_block(Uint16 kb);

/**
 * Get an initialized event struct of a specific type
 *
 * \param ev The ::command code
 * \return An event struct with the command set and everything else as 0/NULL
 */
inline event init_event(int ev);

/**
 * Check if a given string ends with another string
 *
 * \param reference The string to check
 * \param against The string to see if it is the end.
 * \return 1 if reference does end with against
 * \return 0 if reference does not end with against
 * \return -1 if reference is shorter than against
 */
char ends_with(const char *reference, const char *against);

/**
 * Deep copy a string into a fresh memory block of the exact right size
 *
 * \param original The string to copy
 * \return The copied string, or NULL on error
 *
 * \warning Make sure the string is sanitized before passing to this function
 * \note Memory is allocated with malloc, so free with free
 *
 * \sa init_block
 */
inline char* make_string(const char *original);

/**
 * Open a comic book for reading
 *
 * \param dir The directory of the comic book
 * \param filename The filename of the comic book
 * \return A comic book. If there was some error loading the comic book,
 * comic_book::num_pages will equal zero
 *
 * \warning Always check to make sure that comic_book::num_pages is not zero
 * before continuing!
 *
 * \sa close_comic_book
 * \sa goto_next_page
 * \sa goto_prev_page
 * \sa show_book
 */
comic_book open_comic_book(char *dir, char *filename);

/**
 * Free resources corresponding to a comic book
 *
 * \param book The comic book to close
 *
 * \warning Obviously, do not try to access the contents of the comic book after
 * this call
 */
void close_comic_book(comic_book book);

/**
 * Go to the next page in a comic book, or leave the page the same if there is
 * no next page
 *
 * \param book The comic book
 * \return The comic book after going to the next page
 */
comic_book goto_next_page(comic_book book);

/**
 * Go to the previous page in a comic book, or leave the page the same if there
 * is no previous page
 *
 * \param book The comic book
 * \return The comic book after going to the previous page
 */
comic_book goto_prev_page(comic_book book);

/**
 * Jump to a page of a specific number
 *
 * \param book The comic book
 * \param page The number of the page to which to jump
 * \return The book after changing the page
 *
 * \note If the page is out of bounds, the page is not changed
 */
comic_book goto_page(comic_book book, Uint16 page);

/**
 * Display a page
 *
 * \param page_img The image to show
 * \param book The book the page is from
 * \param rotation A pointer to the current rotation (used for rotation
 * persistence)
 * \param scale_ratio A pointer to the current scale ratio, where a pointed
 * value of 0 represents autozoom (used for zoom persistence)
 * \return An event telling the caller what should be done next
 */
event show_page(SDL_Surface *page_img, comic_book *book, Uint8 *rotation, float *scale_ratio);

/**
 * Load and show a comic book from a filename
 *
 * \param dir The directory in which the comic book is located
 * \param book_name The filename of the comic book in its directory
 * \return An ::event signalling what to do next
 */
event show_book(char *dir, char *book_name);

/**
 * Cache a page
 *
 * \param pci The ::precache_info struct containing what to cache. The cached
 * page is also stored in this struct
 * \return 1 if the caching finished successfully
 * \return -1 if the page loaded is the wrong page (which may happen if the page
 * is switched while the page is loading)
 * \return -2 if the file can't be loaded from the archive
 * \return -3 if the image can't be loaded from the file
 * \return -4 if the page is out of bounds
 */
int cache_page(void *pci);

/**
 * Show the menu
 *
 * \param book The comic book that is currently open
 * \return Either the action that was taken, or the action to be taken. 
 * Actions that are taken inside of the menu code should not be acted upon again
 * and are: clock_adjust, toggle_global, set_pointer, load_config_command,
 * save_config_command, add_bookmark, retrieve_bookmark, save_book_command,
 * delete_bookmarks command, delete_all_bookmarks_command and
 * purge_bookmarks_command
 */
event show_menu(comic_book *book);

/**
 * Free the contents of an event
 *
 * \param cmd The event to free
 */
inline void free_event(event cmd);

/**
 * Generate a list of events for use in a menu based on the specified criteria
 * and in the format specified.
 *
 * \param dir The directory in which to search for files
 * \param ptr_to_list A pointer that gets set to point to the list
 * \param user A user-specified parameter that is later passed to the passed
 * functions
 * \param selection_criteria A function that is passed the directory name, the
 * dirent, and user, and returns positive if the dirent should be included in
 * the list, 0 if not, and negative on error
 * \param event_maker A function that is passed the directory name, the dirent,
 * and user, and returns an event corresponding to the dirent. The
 * event::command should be set to error_ev if an error was encountered
 * \param comparator A function that compares two events of the type returned by
 * event_maker and returns 0 if they are equivalent, negative if the first one
 * is less than the second one, or positive if the first one is greater than the
 * second one. NULL can be passed if the events shouldn't be sorted
 * \return The number of files in the directory
 */
Uint16 real_file_list(char *dir, event **ptr_to_list, unsigned long user,
	char (*selection_criteria)(char*, struct dirent*, unsigned long),
	event (*event_maker)(char*, struct dirent*, unsigned long),
	int (*comparator)(const void*,const void*));
/**
 * Generate a list of the comic books in a directory
 *
 * \param dir The directory to look in
 * \param ptr_to_list A pointer that gets set to point to the list of comic
 * books
 * \param get_dirs 0 if directories should be ignored, 1 if they should be
 * included
 * \return The number of comic books (and directories, if get_dirs is 1) in the
 * directory
 */
Uint16 file_list(char *dir, event **ptr_to_list, char get_dirs);

/**
 * Get the name of the next comic book in a directory
 *
 * \param dir The name of the directory
 * \param cur_file The name of the current file
 * \return The name of the next file
 *
 * \note This function will wrap around to the beginning, if necessary
 */
char* next_file_name(char *dir, char *cur_file);

/**
 * Get the name of the previous comic book in a directory
 *
 * \param dir The name of the directory
 * \param cur_file The name of the current file
 * \return The name of the previous file
 *
 * \note This function will wrap around to the end, if necessary
 */
char* prev_file_name(char *dir, char *cur_file);

/**
 * Return whether or not a dirent in a directory is a directory itself
 *
 * \param filent The dirent
 * \param dir The directory in which the dirent is located
 * \return 1 if the dirent is a directory, 0 otherwise
 */
char dirent_is_dir(struct dirent *filent, char *dir);

/**
 * Display the battery meter
 */
void battery_meter(void);

/**
 * The function used as the thread for running the battery meter
 *
 * This function will push a command::redraw_ev event onto SDL's command stack
 * every ten seconds if the value pointed to by *is_running is 1, which tells
 * ::show_menu to redraw. If the value is 2, it waits until it is one or zero.
 * If it is zero, the thread exits
 *
 * \param is_running The pointer to the state of the battery meter
 * thread. Handled internally as type char
 * \return Always zero
 */
int battery_meter_thread(void *is_running);

/**
 * Obtain the real name of a dirent
 *
 * \param dir The dirent
 * \return The real name
 *
 * \note This is primarily used because Windows is stupid and doesn't
 * NULL-terminate dirent::d_name
 */
inline char* dir_real_name(struct dirent *dir);

/**
 * Generate a list of themes
 *
 * \param ptr_to_list A pointer that gets set to point to the list of themes
 * \return The number of themes
 */
Uint16 theme_list(event **ptr_to_list);

/**
 * Generate a list of languages
 *
 * \param ptr_to_list A pointer that gets set to point to the list of languages
 * \return The number of themes
 */
Uint16 language_list(event **ptr_to_list);

/**
 * Extract a file from a comic book
 *
 * \param book The comic book
 * \param page Which page for which to get the file
 * \param size A pointer to a size_t that is later set to the size of the file
 * \return The file, or NULL on error
 *
 * \note RARs are handled internally, but ZIPs are handled in
 * ::extract_file_from_zip for no good reason
 */
char* extract_file(comic_book *book, Uint16 page, size_t *size);

/**
 * Extract the current file from a ZIP
 *
 * \param zip The ZIP file
 * \param size A pointer to a size_t that is set to the size of the file
 * \return The file, or NULL on error
 */
char* extract_file_from_zip(unzFile zip, size_t *size);

/**
 * Get the current input
 *
 * \param wait Specifies whether handle_input should wait for a change in input
 * (1 is yes, 0 is no)
 * \return The current input as a ::command. Note that it may be
 * command::no_event, even if wait is 1! If the event is ending, it will be ORed
 * with command::event_end
 */
int handle_input(char wait);

/* In gui.c */
/**
 * Load a theme
 *
 * \param theme The name of the theme to load. It is the name of the folder in
 * the ::themes_dir directory
 * \return 0 if the theme could not be loaded properly
 * \return -1 if the theme could not be loaded because of range incompatibility
 * \return 1 if the theme was loaded properly
 *
 * \warning The current theme is unloaded first, and if loading the theme fails,
 * no theme will be loaded after this. Therefore, the return of this function
 * should always be checked and a default theme should be manually loaded if
 * this call fails
 */
char load_theme(const char *theme);

/**
 * Unload the current theme
 */
void unload_theme(void);

/**
 * Free the contents of a ::theme_info struct
 *
 * \param theme The ::theme_info struct
 */
void free_theme_info(theme_info *theme);

/**
 * Obtain the background image from the current theme
 *
 * \return The background image, or NULL if there is no background loaded
 */
SDL_Surface* get_background(void);

/**
 * Get the number of lines per page, which depends on the height of the font
 *
 * \return the number of lines per page
 */
Uint16 options_per_page(void);

/**
 * Place a line of text using the font loaded in the theme
 *
 * \param string The string of text to render
 * \param x The X position
 * \param y The Y position
 * enumerators
 * \param color The color to use (RRGGBB)
 * \return 1 on success, 0 on failure
 */
char place_text(const char *string, unsigned short x, unsigned short y, unsigned color);

/**
 * Move the text cursor to the next line
 */
void next_line(void);

/**
 * Reset the X and Y of the text cursor to 0
 */
void reset_text_pos(void);

/**
 * Print a message on the bottom right corner of the screen
 *
 * \param message The message to print
 */
void status_message(const char *message);

/**
 * Print a message in the center of the screen
 *
 * \param message The message to print
 */
void say_centered(const char *message);

/**
 * Trim off the end of a string and replace it with ellipses
 *
 * This function behaves relative to the amount of space remaining on the screen
 *
 * \param string The string to trim. This string will be modified
 * \param offset The number of pixels from the left of the screen that the
 * string will be printed
 * \return The string after it is trimmed
 */
char* ellipsis(char *string, Uint16 offset);

/**
 * Get the X position of the text cursor
 *
 * \return the X position of the text cursor
 */
Uint16 getX(void);

/**
 * Get the Y position of the text cursor
 *
 * \return the Y position of the text cursor
 */
Uint16 getY(void);
 
/**
 * Set the X and Y positions of the text cursor
 *
 * \param x The X position
 * \param y The Y position
 */
void setXY(Uint16 x, Uint16 y);

/**
 * Get the dimensions of a string
 *
 * \param string The string from which to obtain the dimensions
 * \param w A pointer to an int in which to store the width. Can be null
 * \param h A pointer to an int in which to store the height. Can be null
 * \return 0 on success, -1 on error
 */
char get_text_dimensions(const char *string, int *w, int *h);

/**
 * Get the attributes of the loaded font
 *
 * \param attrs A pointer to where to store the attributes
 * \return 1 on success, 0 if no font is loaded or attrs is NULL
 */
char get_font_attr(font_attr* attrs);

/**
 * Get the ranges for the current font
 *
 * \return The ranges
 */
char_ranges get_font_ranges(void);

/**
 * Get the color that corresponds to a certain context
 *
 * \param context The context for which to get the color
 * \return The color, in ARGB format. If the first byte is equal to
 * 0xFF, it means that no theme is currently loaded
 */
Uint32 get_color(int context);

/**
 * Get the red component from a color
 *
 * \param argb The color in ARGB format
 * \return The color's red component
 */
static inline Uint8 get_red(Uint32 argb) {
	return argb >> 16;
}

/**
 * Get the green component from a color
 *
 * \param argb The color in ARGB format
 * \return The color's green component
 */
static inline Uint8 get_green(Uint32 argb) {
	return argb >> 8;
}

/**
 * Get the blue component from a color
 *
 * \param argb The color in ARGB format
 * \return The color's bluen component
 */
static inline Uint8 get_blue(Uint32 argb) {
	return argb;
}

/**
 * Get the alpha component from a color
 *
 * \param argb The color in ARGB format
 * \return The color's alpha component
 * \note Due to poor initial planning, 0xFF and 0x00 are switched when in ARGB
 * format. If this returns 0x00, it means that the theme was not loaded when the
 * color was loaded;
 */
static inline Uint8 get_alpha(Uint32 argb) {
	Uint8 color = argb >> 24;
	if(color == 0xFF) return 0;
	if(color == 0x00) return 0xFF;
	return color;
}

/**
 * Set an explicit color for ::say or ::say_centered to use instead of the
 * default or selected color
 *
 * \param color The color to use, in RGB format
 */
void set_explicit_color(Uint32 color);

/**
 * Cease using an explicit color
 */
void unset_explicit_color(void);

/**
 * Get the name of the loaded theme
 *
 * \return The name of the loaded theme, or NULL if no theme is loaded
 */
const char* get_loaded_theme_name(void);

/**
 * Show a hard-coded popup
 *
 * \param which Which popup to show. 0 is about.png and 1 is notheme.png
 * \return 1 on success, 0 on failure
 */
char show_popup(int which);

/**
 * Get an image from the theme
 * \param image The identifier for the image
 * \return The image, or null on error
 */
SDL_Surface *get_image(int image);
/* End in gui.c */

/* In language.c */
/**
 * Get the text corresponding to a message in the currently loaded language
 *
 * \param mesg_number The message to get. (Should be a value out of
 * ::message_enum but less than message_enum::mesg_end.)
 * \return The message, or NULL if out of bounds
 */
const char* get_message(int mesg_number);

/**
 * Load a language
 *
 * \param lang The name of the language to load
 * \return 1 on success, 0 on failure
 */
char load_language(const char *lang);

/**
 * Unload the current language
 */
void unload_language(void);

/**
 * Get the ranges of a loaded language
 *
 * \return The ranges
 */
char_ranges get_loaded_language_ranges(void);

/**
 * Get the name of the loaded language
 *
 * \return The name of the language, or "builtin" if no language loaded. This
 * will never return NULL
 */
const char* get_loaded_language_name(void);
/* End in language.c */

/**
 * Compare two comic book names or names of files in a comic book
 *
 * \param foo The first name, as a char**
 * \param bar The second name, as a char**
 * \return -1 if the first is less than the second, 1 if the first is greater
 * than the second, or 0 if they are equal
 */
int cb_compare(const void *foo, const void *bar);

/**
 * Compare two events based on their event::option_text member
 *
 * \param foo The first event, as an event*
 * \param bar The second event, as an event*
 * \return -1 if the first is less than the second, 1 if the first is greater
 * than the second, or 0 if they are equal
 */
int event_compare(const void *foo, const void *bar);

/**
 * Compare two t_utf8_sjis_xlate based on their t_utf8_sjis_xlate:sjis member
 *
 * \param foo The first pointer to a t_utf8_sjis_xlate
 * \param bar The second pointer to a t_utf8_sjis_xlate
 * \return -1 if the first is less than the second, 1 if the first is greater
 * than the second, or 0 if they are equal
 */
int sjis_compare(const void *foo, const void *bar);

/**
 * See if two ranges are compatible
 *
 * \param bigger The larger range set
 * \param smaller The smaller range set
 * \return 1 if compatible, 0 otherwise
 */
char ranges_compatible(const char_ranges bigger, const char_ranges smaller);

/**
 * Make a directory
 *
 * \param dir_name The name of the directory
 * \return 0 on success, -1 on error
 */
inline int make_dir(const char *dir_name);

/**
 * Get the full path of a file relative to PSPComic's root directory
 *
 * \param file_name The name of the file
 * \return A malloc'd string with the full path to the file, or NULL on error
 */
inline char* get_full_path(const char *file_name);

/**
 * Determine whether a specified file is a comic book
 *
 * \param dir The directory in which the file is located
 * \param filent A pointer to a dirent containing information on the file
 * \param get_dirs Specifies whether or not a directory should be considered a
 * comic book
 * \return 1 if the file is a comic book, 0 otherwise, or -1 on error
 */
char cb_criteria(char *dir, struct dirent *filent, unsigned long get_dirs);

/**
 * Create an event struct for the given comic book
 *
 * \param dir The directory in which the file is located
 * \param filent A pointer to a dirent containing information on the file
 * \param dummy Unused
 * \return A struct that can be used for telling the menu code to load the comic
 * book. Note that if the event::command is equal to command::error_ev, there
 * was an error
 */
event cb_event_maker(char *dir, struct dirent *filent, unsigned long dummy);

/**
 * Determine whether a specified file is a theme
 *
 * \param dir The directory in which the file is located
 * \param filent A pointer to a dirent containing information on the file
 * \param dummy Unused
 * \return 1 if the file is a theme, 0 otherwise, or -1 on error
 */
char theme_criteria(char *dir, struct dirent *filent, unsigned long dummy);

/**
 * Create an event struct for the given theme
 *
 * \param dir The directory in which the file is located
 * \param filent A pointer to a dirent containing information on the file
 * \param dummy Unused
 * \return A struct that can be used for telling the menu code to load the
 * theme. Note that if the event::command is equal to command::error_ev, there
 * was an error
 */
event theme_event_maker(char *dir, struct dirent *filent, unsigned long dummy);

/**
 * Determine whether a specified file is a language
 *
 * \param dir The directory in which the file is located
 * \param filent A pointer to a dirent containing information on the file
 * \param dummy Unused
 * \return 1 if the file is a language, 0 otherwise, or -1 on error
 */
char language_criteria(char *dir, struct dirent *filent, unsigned long dummy);

/**
 * Create an event struct for the given language
 *
 * \param dir The directory in which the file is located
 * \param filent A pointer to a dirent containing information on the file
 * \param dummy Unused
 * \return A struct that can be used for telling the menu code to load the 
 * language. Note that if the event::command is equal to command::error_ev, there
 * was an error
 */
event language_event_maker(char *dir, struct dirent *filent, unsigned long dummy);

/**
 * Initialize USB
 *
 * \return 0 on success, negative on error
 */
int init_usb();

/**
 * Enable USB
 *
 * \return non-negative on success, negative on error
 */
int start_usb();

/**
 * Disable USB
 *
 * \return non-negative on success, negative on error
 */
int stop_usb();

/**
 * Initialize the theme system
 *
 * \return 0 on failure, non-zero on success
 */
char init_theme_system();

/**
 * Convert a Shift-JIS string to UTF-8
 *
 * \param string The input string
 * \return A malloc'd string of just the right size
 */
char* sjis_to_utf8(const char *string);

/**
 * Speed up the CPU if dynamic CPU frequency is enabled
 */
void scale_up_cpu(void);

/**
 * Slow down the CPU if dynamic CPU frequency is enabled
 */
void scale_down_cpu(void);
#ifdef __cplusplus
}
#endif

#endif
