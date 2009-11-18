/** \file
    XML-related functions header (noticing a pattern here?)

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

#ifndef PSPComic_xmlfuncs_hpp
#define PSPComic_xmlfuncs_hpp

#include "pspcomic.h"

#ifdef __cplusplus
extern "C" {
#endif

///An enumeration of the errors that an XML-related function may return
enum xml_error {
	xml_er_unknown			= -1,
	xml_er_cannot_load		= -2,
	xml_er_malformed		= -3,
	xml_er_out_of_memory	= -4,
	xml_er_null_pointer		= -5,
	xml_er_subroutine_fail	= -6
};

//////////////////////////////////
// Configuration file functions //
//////////////////////////////////

/**
 * Load the saved configuration
 *
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char load_config();

/**
 * Save the current configuration
 *
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char save_config();

////////////////////////
// Bookmark functions //
////////////////////////
/**
 * Load bookmarks from bookmarks file
 *
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char load_bookmarks();

/**
 * Convert an older format of the bookmark file to the current version
 *
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char convert_bookmarks();

/**
 * Save the currently open page of a comic book as the bookmark
 *
 * \param book The comic book
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char save_bookmark(comic_book *book);

/**
 * Get the bookmarked page number for a comic book
 *
 * \param book The comic book
 * \return The page number, or an ::xml_error (negative) on failure
 */
Sint32 get_bookmark(comic_book *book);

/**
 * Deletes the bookmarks for a given comic book
 *
 * \param book The filename of the comic book
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char delete_bookmarks(const char *book);

/**
 * Put the name of the comic book refered to by the identifier id into
 * the buffer pointed to by book
 *
 * \param id The identifier of the comic book
 * \param book The buffer into which to put the name of the comic book
 * \param book_len A pointer to the length of the buffer. It is set to the
 * length of the file name after being called
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char get_book_by_id(const char *id, char *book, size_t *book_len);

/**
 * Put the name of the saved comic book into the buffer pointed to by book.
 * This is a wrapper around get_book_by_id
 *
 * \param book The buffer into which to put the name of the comic book
 * \param book_len A pointer to the length of the buffer. It is set to the
 * length of the file name after being called
 * \return 0 on success and an ::xml_error (negative) on failure
 */
char get_saved_book(char *book, size_t *book_len);

/**
 * Put the name of the last opened comic book into the buffer pointed to by
 * book.
 * This is a wrapper around get_book_by_id
 *
 * \param book The buffer into which to put the name of the comic book
 * \param book_len A pointer to the length of the buffer. It is set to the
 * length of the file name after being called
 * \return 0 on success and an ::xml_error (negative) on failure
 */
char get_last_book(char *book, size_t *book_len);

/**
 * Set the saved comic book for a specific identifier
 *
 * \param id The identifier
 * \param book The filename of the comic book
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char set_book_by_id(const char *id, const char *book);

/**
 * Set the saved comic book.
 * This is a wrapper around set_book_by_id
 *
 * \param book The filename of the comic book
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char set_saved_book(const char *book);

/**
 * Set the "last opened" comic book.
 * This is a wrapper around set_book_by_id
 *
 * \param book The filename of the comic book
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char set_last_book(const char *book);

/**
 * Purge the bookmark file of bookmarks which point to comic books which are no
 * longer on the filesystem
 *
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char purge_bookmarks();

/**
 * Delete all bookmarks
 *
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char delete_all_bookmarks();

//////////////////////////
// Theme file functions //
//////////////////////////
/**
 * Get the name of a theme
 *
 * \param theme The filename of the theme
 * \return The name of the theme, or NULL on error. Note that this value is
 * malloc'd and should be free'd when you are done with it.
 */
char* get_theme_name(const char *theme);

/**
 * Load the information from a theme into a theme_info struct
 *
 * \param theme The name of the theme
 * \param info The struct into which to load the information for the theme
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char get_theme_info(const char *theme, theme_info *info);

/////////////////////////////
// Language file functions //
/////////////////////////////
/**
 * Get the name of a language
 *
 * \param lang The filename of the languange
 * \return The name of the language, or NULL on error. Note that this value is
 * malloc'd and should be free'd when you are done with it.
 */
char* get_language_name(const char *lang);

/**
 * Load the messages for a language
 *
 * \param lang The name of the language
 * \return An array of the messages for that language. If the program runs out
 * of memory, the language table may not be complete when returned
 */
char** get_language_messages(const char *lang);

/**
 * Get the ranges a language uses
 *
 * \param lang The name of the language
 * \param ranges_ptr A pointer to a char_ranges struct where the ranges should
 * be stored
 * \return 0 on success, an ::xml_error (negative) on failure
 */
char get_language_ranges(const char *lang, char_ranges *ranges_ptr);


#ifdef __cplusplus
}
#endif

#endif
