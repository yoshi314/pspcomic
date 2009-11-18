PSPComic version 1.0.1
A Comic Book Reader for the PSP!
Copyright (C) 2007 - 2008 Jeffrey P. (Archaemic) & Christophe Rudyj (Kip)
========================================
Table of Contents
I.	Usage
     a. Installing
     b. Adding comic books
     c. Controls
	1. Comic book (normal mode)
	2. Comic book (zoom box mode)
	3. Menu
     d. Menu options
     e. Themes and languages
II.	Compiling from source
III.	Notes
IV.	Frequently Asked Questions
V.	History
VI.	Legal
VII.	Credits
========================================
I.	Usage
========================================
     a. Installing
----------------------------------------

To install this program, you must have either firmware 1.50, any custom
firmware, or any other firmware that can emulate firmware 1.50 through DevHook
(such as 2.71 running Homebew Enabler (HEN)). To install this program, place the
folders pspcomic and pspcomic% from the 1.50 folder in the /PSP/GAME folder if
you are running or emulating firmware 1.50 or if you have your custom firmware
set to 1.50 kernel mode. If you are using the 3.xx version, place the folder
pspcomic from the 3.xx folder in the /PSP/GAME folder. If you have your custom
firmware in 3.xx mode, put the folders pspcomic and pspcomic% from the folder
1.50 in the /PSP/GAME150 folder. If you have your custom firmware in 1.50 mode
and wish to use the 3.xx version, put it in the /PSP/GAME3xx folder where xx is
the last two digits of whichever custom firmware you are using. E.g., if you are
using 3.90 M33, the folder would be /PSP/GAME390. You must use the 3.xx version
if you are using a Slim PSP.

You must also copy the comics folder into the root of your memory stick.

Please note that if you have a Slim PSP, you must have firmware 3.71 M33-3, 3.72
HX-2, or newer to use the extra RAM.

To run the program, go to the Game option in the top menu of the Cross Media Bar
(XMB), sroll down to the Memory Stick option, and select PSPComic from the menu
It should now run.

Recent versions of iR Shell already come with PSPComic, but they may include an
old version. To install this program as an iR Shell plugin for iR Shell 3.80 and
below, copy the contents of the 1.50/pspcomic folder to one of the folders in
/IRSHELL/EXTAPP15 and the 3.xx/pspcomic folder to one of the folders in
/IRSHELL/EXTAPP3X. If you are using iR Shell 3.90 or newer, copy the contents of
the 3.xx/pspcomic folder to one of the folders in /IRSHELL/EXTAPP, renmae the
file 1.50/pspcomic/EBOOT.PBP to EBOOT15.PBP and copy it to the same folder as
before. Follow the instructions provided with iR Shell for using file extension
plugins.

----------------------------------------
     b. Adding comic books
----------------------------------------

The process of adding comic books to the memory card is quite simple. Just put
the comics in the folder "comics" at the root of the memory stick. The file must
also not have a name longer than 200 characters, or else it will also be
ingnored by the program, due to the limitations of the UnRAR library. Files are
sorted in such a way that 10 directly follows 1, and is before 2. Therefore,
numbers should be prefixed by zeros to attain proper sorting. That is, for a
folder with up to 99 comics, single digit comic book names should be 01 through
09 instead of 1 through 9, et cetera.

Supported archive formats are: .cbz, .cbr, .zip, .rar
Supported image formats are: .jpeg/.jpg/.jpe, .png, .gif, .bmp
Opening a folder as if it were an archive has some basic support.

----------------------------------------
     c. Controls
----------------------------------------
	1. Comic book (normal mode)
----------------------------------------

D-Pad:	  Pan
Triangle: Zoom in (hold to enable zoom box mode)
Cross:	  Zoom out
Circle:	  Rotate clockwise
Square:	  Rotate counterclockwise
Select:	  See below
Start:	  Open menu
Left shoulder:	Previous page
Right shoulder:	Next page

The select button performs multiple actions. If you are currently at a custom
zoom level, it will go to your selected autozoom level. If you are at the
autozoom level when you press it, it will either go to full view or autodetect
mode.

----------------------------------------
	2. Comic book (zoom box mode)
----------------------------------------

D-Pad:	  Move zoom box
Triangle: Exit zoom box mode
Cross:	  Exit zoom box mode
Circle:	  Shrink zoom box
Square:	  Enlarge zoom box
Select:	  No action
Start:	  Open menu
Left shoulder:	Previous page and exit zoom box mode
Right shoulder:	Next page and exit zoom box mode

----------------------------------------
	3. Menu
----------------------------------------

D-Pad:	  Scroll
Triangle: Go back
Cross:	  Select
Circle:	  No action
Square:	  Per menu; see below
Select:	  Toggle USB
Start:	  Close menu
Left shoulder:	Scroll up skipping some options
Right shoulder:	Scroll down skipping some options

The square button performs one of several functions depending on which menu
you're currently in:

Open book:       Go up one directory
Jump to page:    Select currently open page
Set theme:       Select theme without previewing it first
Number set menu: Reset number to previous value
All other menus: No action

----------------------------------------
     d. Menu options
----------------------------------------

- Load last opened comic book
	Load the last comic book that was opened
- Open this directory
	Open the current directory as a comic book
- Open comic book...
	Open a menu to select which comic book to which to switch
- Close comic book
	Close the current comic book and return to the top menu
- Next comic book
	Open the next comic book in the list
- Previous comic book
	Open the previous comic book in the list
- Jump to page...
	Select a page to which to jump
- Load bookmark
	Jumps to the page currently set as the bookmark for the current comic
	book
- Set to bookmark
	Set the currently open page as the bookmark for the current comic book.
	Each comic book can currently only have one bookmark
- Load saved comic book
	Load the comic book that was saved previously
- Save comic book
	Set the currently comic book as the saved comic book
- More bookmark operations...
	Further operations that can be done on bookmarks
  - Delete bookmark for this comic book
	Delete the bookmark for this comic book
  - Delete all bookmarks
	Delete the bookmarks for all comic books
  - Purge bookmarks
	Delete the bookmarks for comic books that are no longer on the memory
	stick
- Configuration...
	Open the menu of configurable options
  - Change resize method to nearest neighbor/resample
	Switch method with with to resize the page. Options are nearest neighbor
	(default) which is blockier but faster, and simple resample, which is
	slightly slower and blurrier, but often makes text easier to read. Please 
	note that the page you're currently on is not reprocessed, and the change
	doesn't take effect until you zoom, rotate or change the page.
  - Adjust clock frequency...
	Change the frequency of the clock. If it is higher, panning will be
	smoother and page operations such as resizing and rotating will be
	faster, however, more battery will be consumed. The default clock
	frequency of the PSP is 222 MHz. Speeds under this are not recommended
	because the program will slow down immensely, but will save battery.
  - Adjust pan rate...
	Change the rate at which comic books pan using the D-Pad. Units are
	pixels per pan increment (about 50 times per second, ideally; much less in
	practice). The default is 20.
  - Turn on/off zoom level persistence
	Toggle whether or not the zoom level is retained between pages. The
	default is off.
  - Turn on/off rotation persistence
	Toggle whether or not the rotation level is retained between pages. The
	default is off.
  - Adjust menu scroll skip rate...
	Change the number of items skipped in the menu when using the shoulder
	buttons. The default is 5.
  - Turn on/off manga mode
	Toggle whether the comic starts on the left (off) or the right (on) side
	when changing zoom or rotation levels. The default is off.
  - (Do not) jump to bookmark upon loading comic book
	Toggles whether the program should automatically jump to the bookmark
	set in the current comic book (if applicable) upon loading a comic book.
  - Set autozoom mode...
	Set the autozoom mode to one of the following modes
    - Fit page to screen width
	Resize the page so that the width of the page is the same as the width
	of the screen
    - View page at original width
	Do not resize page; display at original size
    - Fit page to twice screen width (good for spreads)
	Resize the page so that its width is twice the width of the screen. This
	is mostly useful for two-page spreads, and can be very slow
    - Autodetect spread mode
	Resize page to the width of the screen unless the page is wider than it
	is large, in which case resize it to twice the width of the screen
  - Set zoom box width...
	Set the width of the zoom box. The default is 150 pixels.
  - Set zoom box height...
	Set the height of the zoom box. The default is 200 pixels.
  - Turn on/off precaching
	Toggles whether or not the program should load the following page in the
	comic book before you actually change to that page. This will cause the
	program to run out of memory more easily, but can drastically reduce
	loading times. The default is off
  - Turn on/off single-handed mode
	Toggles whether the program should use single-handled mode. Single-
	handed mode switches L and R so that the program can be used effectively
	with one hand
  - Turn on/off analog nub
	Set whether or not motion of the analog nub should be ignored
  - Change theme
  	Set the theme for PSPComic to use. See the Themes and languages section
	for more information
  - Set language
  	Sets the language for PSPComic to use. See the Themes and languages
	section for more information
  - Save configuration
	Save the current configuration
  - Load configuration
	Load the configuraion currently on the memory stick
- About
	Display the about screen
- Quit
	Exit the program
- Reload menu
	Reload the list of files for the current menu (for if you change the
	files on the memory stick while running the program)
- < Go back
	Close the menu or return to a higher menu

----------------------------------------
     e. Themes and languages
----------------------------------------

Beginning with PSPComic v1.0, theming and language support have been added.
Themes are stored in the folder /comics/.pspcomic/themes and languages are
stored in the folder /comics/.pspcomic/languages. For more information on
themes, please see themespec.txt. Languages are simple XML files, and
documentation on creating a translation is included in the file themes/en.xml.

Themes and languages can be loaded from the configuration menu. However, not all
themes are compatible with all languages due to the restrictions of the fonts.
PSPComic should automatically detect if a language and a theme are compatible,
but in some cases may fail. If this happens, just restart the program without
saving your configuration settings. Please note that only languages that are
detected to be compatible compatible with the current theme are displayed in the
language menu.

All of the PSPComic translations, except for the Canadian French translation,
are unofficial, and were performed by volunteers.

========================================
II.	Compiling from source
========================================

To compile this program, you'll need the PSP Toolchain, which can be found at
svn.ps2dev.org/trunk/psp/psptoolchain/. For compiling a version for the 3.xx
firmwares, you must have at least revision 2320 (October 15, 2007), and to build
a 3.xx version that takes advantage of the PSP Slim's extra memory, you must
have at least revision 2333 (November 1, 2007). The source code can be compiled
if you have zlib, libpng (for PNG support), SDL, jpeg (for jpeg support) and
SDL_image installed for the PSPSDK by going to the main folder of the source
code, which is "src", and typing the following commands:

	make psplibs
	make psp

This will compile both the 3.xx and 1.50 kernel versions. To compile these
individually, run these commands instead of make psp:

	make 3xx

or

	make 150

If you wish to compile for the other kernel or for a different operating system
after having compiled it for one kernel or OS, you must first run this command:

	make clean

This program has not been tested on any operating systems other than the PSP and
Windows and Linux for i386. Some aspects of this program will not work on big
endian systems such as PPC Macs. However, in general, the application should
work, but is untested, and bugs will occur.

To compile for a computer operating system, first compile the libraries by using
this command:

	make rar tinyxml
	
Then you can compile PSPComic by entering either this command for Windows:

	make windows

Or this command for a Unix-like operating system:

	make unix

You can also generate the documenation by running this command (you need doxygen
installed):

	make docs

========================================
III.	Notes
========================================

- Archaemic's PSP Development blog, Arc/PSP can be found at
  http://archaemic.awardspace.com/psp/. This blog will always be updated with
  news and information about upcoming versions of PSPComic as well as a download
  link for the latest version.
- Please send all bug reports to the email address archaemic.spam at gmail.com.
  Please attempt to explain the circumstances that led up to the crash as well
  as possible, and, if reproduceable, the exact steps that lead up to the crash
  along with files that may be causing the crash.
- The PSP will not realize that there is a memory stick inserted 
- Filenames that contain non-ASCII characters do not work on the 3.xx version
  due to what appears to be a bug with the PSP SDK and how it handles Unicode
  characters when reading from a directory.
- Large pages, when made full size, may cause the PSP to run out of memory if
  attempting to rotate, or sometimes if just loaded. Make sure not to use very
  high resolution comic books with this program. If memory runs out, the page
  will "freeze", meaning it will no longer be able to be operated on, but you
  can still change pages in the comic book.
- If your file or folder is not shown in the file list, that means that the file
  is either of the wrong type, or that the filename is too long (over 250 bytes,
  including the name of the folder).
- DO NOT ATTEMPT PUT THE PSP INTO SUSPEND MODE WHILE A PAGE IS LOADING! THIS MAY
  CAUSE THE PSP TO CRASH.
- This program is statically linked to SDL_image, which in turn is statically
  linked to Independant JPEG Group's (IJG) implementation for reading JPEGs.
- This program is statically linked to zlib and a portion of minizip.
- The version of UnRAR distributed with this source code is slightly modified so
  as to work with the PSP. It is not the original source code, although it may
  be able to be compiled in an identical manner to the original source code and
  would likely be able to be used in the stead of the original source code.

========================================
IV.	Frequently Asked Questions
========================================

Q. When I try to load up some pages in a comic book, I get a notice saying
   "SDL_image error: JPEG loading error". What does this mean and can I fix it?
A. This usually means that the PSP ran out of memory when trying to load the
   JPEG in the comic book archive. Usually this is caused by a progressive JPEG
   being loaded that is too big. Often, these progressive JPEGs can be converted
   to baseline JPEGs, which PSPComic does not choke on as easily. To convert
   JPEGs, please use the bundled utility No Bull Moose.

Q. Where can I find CBZ/CBRs?
A. Have you tried Google?

Q. How do I make CBZ/CBRs?
A. Take a collection of image files, make sure they're ordered in alphabetical
   order (NOTE: make sure to pad numbers with zeroes because page 1 will be
   followed by page 10 etc. before page 2 otherwise), then put the folder into a
   .zip or .rar and rename the file to .cbz or .cbr depending on which format
   you used.

========================================
V.	History
========================================

v0.9 Beta (2007-04-16)
- Initial release

v0.9.5 Beta (2007-04-27)
- [Feature] Subdirectory support added
- [Feature] Added optional zoom level or rotation persistence between pages
- [Feature] Adjustable pan rate
- [Feature] Select button now switches between fit to width and full size
- [Feature] Ability to save/load configuration
- [Bugfix] Resample algorithms now work as selected, and no more garbling for 8-
  bit images
- [Bugfix] Next/previous comic book menu option now works even if comics on the
  memory stick are changed while menu is open
- [Bugfix] Fixed crash bug when resizing a full-sized page to fit to width
- And many more minor changes...

v0.9.6 Beta (2007-04-29)
- [Feature] L and R now scroll more than one item at a time in the menu
- [Bugfix] Crash when comics folder is absent has been fixed
- [Bugfix] When going to a higher menu from a menu that has been scrolled
  through, the menu should now appear correctly
- And some other minor changes...

v0.9.7 Beta (2007-05-05)
- [Feature] Configuration file is now XML
- [Feature] New manga mode, which makes comics start from the right instead of
  from the left
- [Feature] Menu now shows current state of togglable items
- [Feature] New battery meter shows remaining battery percentage in the menu. If
  the text is red, the battery is low. If it is yellow, the power is plugged in.
- [Feature] The clock frequency menu now allows any value between 20 MHz and 333
  MHz to be selected
- [Feature] Bookmarks have been added. For more information, please see the menu
  options section
- [Feature] 8-bit images now support resampling
- [Bugfix] Fixed scrolling bug, which made the program stop scrolling after
  some time with the D-Pad
- [Bugfix] Removing an open RAR or CBR file while the program is using that file
  will no longer cause a crash when the page is changed
- Sped up resampling
- And many more minor changes...

v0.9.7 Beta 2 (2007-05-05)
- [Bugfix] Fixed a bug where all bookmarks got deleted while adding a new
  bookmark

v0.9.8 Beta (2007-06-28)
- [Feature] Menu now shows name and current page of opened comic book
- [Feature] When zooming, the image is centered on the same place, instead of
  resetting to one of the corners
- [Feature] Added full view mode
- [Feature] Added option to load bookmark upon loading comic book
- [Feature] Alpha-transparent PNGs now display properly
- [Feature] The comics folder is now created if it is not initially present
- [Feature] Jump to page menu now shows the names of the pages
- [Feature] Really glitchy precaching. Use at your own risk! It must be enabled
  manually by editing the config.xml file
- [Feature] Zoom box. Hold Triangle to activate a box that shows a zoomed in
  region of the page. Very slow currently
- [Feature] Holding a button for several seconds now cancels the action before
  it is taken
- [Feature] Battery meter now refreshes every couple dozen seconds instead of
  just when a different option is selected
- [Bugfix] Fixed several bugs relating to zooming
- Sped up resampling
- The menu no longer shows the open comic book in the background. This is to
  save memory.
- Scrolling menus no longer flicker
- Some other stuff I forgot
- And many more minor changes...

v0.9.8 Beta 2 (2007-06-30)
- [Feature] Precaching works better and was thus added to the menu
- [Bugfix] The menu no longer lingers in the background after closing the menu
  for a small image
- [Bugfix] Loading bookmark upon loading the comic book now actually works
- [Bugfix] Huge memory leak in precaching patched

v0.9.9 Beta (2007-08-17)
- [Feature] One command line argument is now recognized. The full path of a
  comic book may now be specified, and it will load when the program starts.
  When the book is closed, the program will exit. THIS ALLOWS A PSPComic EBOOT
  TO BE USED AS AN iR Shell PLUGIN!
- [Feature] New single-handed mode switches the L and R buttons allowing the
  program to be useful when used with only one hand
- [Feature] Square button in menu now performs several functions. See controls
  section for more information
- [Feature] The last comic book being read is now remembered. However, the page
  that was being read is not remembered
- [Feature] A comic book can now be saved to be read later. However, the page is
  not saved, but may be bookmarked separately using the previously existing
  bookmark feature
- [Feature] The bookmarks and configuration file are now stored in a common
  folder so that the iR Shell plugin can share bookmarks and configuration with
  the standalone version. The configuration file is migrated when the program is
  opened and the bookmark file is migrated when the first comic is opened with
  this version
- [Feature] Full view mode has been replaced with various autozoom modes. See
  the menu options section for more information
- [Feature] The zoom box now works on small images
- [Bugfix] Precaching option in menu now properly displays precaching state
- [Bugfix] One no longer has to hold down the start button to get a menu when on
  a page that couldn't load
- [Bugfix] Battery now redraws while a number menu is on the screen
- [Bugfix] Number menus now always draw immediately without input
- [Bugfix] Image no longer pops up in the background occasionally after enabling
  the zoom box
- [Bugfix] Fixed two memory leaks associated with precaching
- [Dev note] The TinyXML makefile has changed. If you intend to compile PSPComic
  and have compiled the TinyXML library for PSP previously, please recompile it
  with the makefile provided.
- Sluggishness in menus has been fixed
- Number menus no longer flicker
- Overall stability improvement
- And many more minor changes...

v0.9.9 Beta 2 (2007-08-20)
- [Feature] One now has the ability to disable the analog nub entirely while
  using PSPComic
- [Bugfix] Fixed a crash in precaching that was introduced in v0.9.9
- [Bugfix] 8-bit color-keyed surfaces now work as expected
- Rotozoom algorithms optimized. They are now much, much faster

v1.0 (2008-02-29)
- [Feature] Themes! You can now select different themes consisting of a
  background, font and text colors!
- [Feature] Languages! Now one can use the interface in a language other than
  English!
- [Feature] Directories can now be opened as if they were comic books. Note that
  previous/next comic book does not support opening directories
- [Feature] The Slim's extra memory is now used, so much larger images may be
  opened
- [Feature] USB mode. One can now change around the files on the memory stick by
  pressing select in the menu. Changes are not automatically registered, and a
  menu must be reloaded by using the reload menu option
- [Bugfix] Fixed outstanding bug and memory leak if a filename is too long
- [Bugfix] 8-bit non-color-keyed surfaces no longer display black as transparent
- [Bugfix] Fixed an off-by-one error leading to a buffer overflow when
  resampling images
- [Bugfix] One no longer has to restart the program after attempting to open a
  comic book in a non-existent directory
- [Bugfix] Fixed minor bug with battery meter when battery is not inserted
- Further rotozoom optimization
- And many more minor changes...
- /!\ Caution! Comic books with bookmarks in them must be on the memory stick
 while upgrading from a previous version of PSPComic for the bookmarks to be
 retained.

v1.0.1 (2008-09-10)
- [Feature] Archives are now detected when opening by contents instead of
  extension. However, menus are still generated by extension
- [Feature] Shift-JIS support for filenames when the PSP's character set is set
  to Shift-JIS
- [Feature] Zoom box colors can now be customized in themes, as well as error
  text color
- [Bugfix] Fixed a RAR loading bug
- [Bugfix] No more weird behavior on resume or when the memory stick is removed,
  however the program will claim that no memory stick is inserted for a small
  period of time after the program is resumed
- [Bugfix] Fixed file handle leak in directory reading
- And many more minor changes...

========================================
VI.	Legal
========================================

This program is copyrighted (C) by Jeffrey P. and Christophe Rudyj. Portions of
this program are licensed under the terms of the GNU General Public License
(GPL), and other portions under the terms of both the UnRAR license and the GPL,
as specified later in this section. The source code distribution of this program
includes three libraries, a portion of the minizip library, which is licensed
under the terms of the zlib license, a modified version of the UnRAR
library, which is licensed under the terms of the UnRAR license, and the TinyXML
library with modified Makefiles, which is also licensed under the zlib license.
For the GPL, please read the file license.txt. For the UnRAR license, please
read the file src/rar/license.txt. For the zlib license, please refer to
http://www.zlib.net/zlib_license.html.

The files in the main folder and the src folder are licensed under the GPL
exclusively, including the images, which are counted as source code, and
excluding the file rar.hpp and the file rar.cpp. The files in the src/rar folder
and the files rar.cpp and rar.hpp in the main folder are licensed under the
UnRAR license exclusively. The files in the src/zip folder are a portion of the
minizip library by Gilles Vollant and are licensed under the zlib license
exclusively. The files in the src/tinyxml folder are the TinyXML library, with
modified Makefiles, which are licensed under the zlib license. In addition, as a
special exception, Jeffrey P. gives permission to link the code of this program
with the UnRAR, minizip and TinyXML libraries and the files rar.cpp and rar.hpp
(or with modified versions of UnRAR, minizip, TinyXML, rar.cpp and rar.hpp that
use the same license as UnRAR, minizip, TinyXML rar.cpp and rar.hpp,
respectively), and distribute linked combinations including the five. You must
obey the GNU General Public License in all respects for all of the code used
other than UnRAR, minizip, TinyXML, rar.cpp and rar.hpp. If you modify the
program, you may extend this exception to your version of the file, but you are
not obligated to do so. If you do not wish to do so, delete this exception
statement from your version.

This means that the user is free to redistribute and/or modify the program
under the terms of the GPL, so long as the user freely provides any
modifications to the source code and uses this same license upon redistribution
and does not use the source code to create an implementation of the RAR
compression algorithm.

THIS PROGRAM, AS IT IS FREE SOFTWARE, IS NOT COVERED BY A WARRANTY TO THE EXTENT
PERMITTED BY LAW. IT IS PROVIDED IN AN 'AS-IS' CONDITION, AND THE PSPCOMIC
PROJECT IS NOT RESPONSIBLE FOR ANY DAMAGES OR DATA LOSS THAT MAY OCCUR, HOWEVER
UNLIKELY IT IS FOR ANY DAMAGES OR DATA LOSS TO OCCUR.

========================================
VII.	Credits
========================================

All of the programming and the PSPComic icons were done by Jeffrey P. Main
background, ideas and encouragement were graciously provided by Christophe
Rudyj. Testing was done by Jeffrey P., Christophe Rudyj and "Chirishman",
"Chrizzly" and "HoweyH". Initial version of the command line interpreter
(iRShell plugin mode) was done by "suloku", although no code from "suloku"'s
version is used in current versions. Languages other than English are credited
in the respective language files. Some help was obtained from the Comical
project (http://comical.sourceforge.net/) in the writing of the RAR reading
code. Comical is a good free software comic book reader for Windows, Mac or
Linux (and possibly other POSIX operating systems) if you are looking for a
program with which to view comic books on the computer. The font used in the
graphics is Komika, which can be found at
http://www.dafont.com/komika-poster.font