dnl Process this file with autoconf to produce a configure script.
dnl This is useful to force using autoreconf 2.5.x
AC_PREREQ(2.59)
AC_INIT([lordsawar], [0.0.1], [benasselstine@gmail.com], [lordsawar])
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER(config.h)
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE
    
dnl Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB

dnl The minimum SDL version required
SDL_VERSION=1.2.0

dnl Check for SDL
AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)
	
AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LIBS)
	
dnl Now check for SDL_image
AC_CHECK_LIB(SDL_image, IMG_Load, [],
            AC_MSG_ERROR([*** SDL_image not found!]))
AC_CHECK_HEADER(SDL_image.h, IMG_Load, [],
            AC_MSG_ERROR([*** SDL_image not found!]))


dnl And check for SDL_mixer. In detail, we need .ogg support
dnl we need some extended cflags or get problems
ac_save_cflags=$CFLAGS
ac_save_ldflags=$LDFLAGS
ac_save_libs=$LIBS

use_sound=1
AC_ARG_ENABLE(sound, [  --enable-sound         Enable or disable sound usage (default on)],
                use_sound="$enableval", use_sound="yes")

if test $use_sound = yes; then

    CFLAGS="$CFLAGS $SDL_CFLAGS"
    LDFLAGS="$LDFLAGS -lSDL_mixer"
    LIBS="-lSDL_mixer $LIBS"
    
    dnl check if ogg vorbis support is enabled
    dnl only under linux the SDL_mixer under windows supports it
    if test $CC = "gcc" || test $CXX = "g++"; then

       AC_TRY_RUN([
       #include <SDL/SDL_mixer.h>
       #include <stdio.h> 

       int main()
       {
           if (!Mix_LoadMUS("${srcdir}" "/dat/music/victory.ogg"))
               return -1;
           return 0;
       }
       ],, AC_MSG_ERROR([*** SDL_mixer is not installed or has not been compiled
                         *** with ogg vorbis support]),[])

       AC_MSG_RESULT([Test SDL_Mixer Run OK.])

    else AC_MSG_WARN([Skipping SDL_Mixer Run. We are cross-compiling for windows!])
    fi

    AC_DEFINE([FL_SOUND], 1, [enables sound])
else
    CFLAGS=$ac_save_CFLAGS
    LDFLAGS="$ac_save_LDFLAGS"
    LIBS="$ac_save_libs"
fi

	
dnl Check for expat
expat=0
AC_ARG_WITH(expat, [  --with-expat=<dir>      Use expat from here],
                      [
                      expat=$withval
                      CPPFLAGS="$CPPFLAGS -I$withval/include"
                      LDFLAGS="$LDFLAGS -L$withval/lib"
                      LIBS="$LIBS -lexpat"
                      ]
                      )
#echo "EXPAT=$expat"
if test $expat = 0 ; then
   AC_CHECK_HEADERS(expat.h,[],AC_MSG_ERROR([*** Expat not found!]))
   AC_CHECK_LIB(expat,XML_ParserCreate,[],[AC_MSG_ERROR([*** Expat not found!])])
fi


dnl this is needed in cross-compile with mingw32
AC_ARG_ENABLE(winlibs,             
              [  --enable-winlibs         enable windows cross-compiling libs],
              LIBS="-lmingw32 -lws2_32 -lintl -lphysfs -lz -lSDL -lpthreadGC2 -lpthreadGCE2 $LIBS"
              flrc=true;
              ,flrc=false
             )

AM_CONDITIONAL(FL_RESRC, test x$flrc = xtrue)

dnl Now check gettext.h
#winnls=no
#AC_ARG_ENABLE(winnls,             
#              [  --enable-winnls         enable windows cross-compiling nls],
#              USE_NLS=yes
#              winnls=yes
#              AC_SUBST(USE_NLS)
#             )

#if test $winnls = no ; then
AM_GNU_GETTEXT_VERSION(0.14.1)
AM_GNU_GETTEXT
#fi

AC_ARG_WITH(intl, [  --with-intl=<dir>      Use int from here],
                      [
                      LDFLAGS="$LDFLAGS -L$withval/intl"
                      LIBS="$LIBS -lintl"
                      LIBINTL=""
		      INTLLIBS=""
                      LTLIBINTL=""
                      ]
                      )

dnl Check for gtkmm

PKG_CHECK_MODULES(GTKMM, gtkmm-2.4 libglademm-2.4)

AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)
			     		      
		      
dnl Finally, check for libsigc++
PKG_CHECK_MODULES(LIBSIGC, sigc++-2.0)

AC_SUBST(LIBSIGC_CFLAGS)
AC_SUBST(LIBSIGC_LIBS)
AC_SUBST(HOME)

	
dnl----------------------------------------------------------------------
dnl Below here are options to modify default behaviour

dnl Do we want to compile the editor?
AC_ARG_ENABLE(editor,
            [  --enable-editor  compile editor (default=yes)],
            use_editor="$enableval", use_editor="yes")
AM_CONDITIONAL([WANT_EDITOR], [test $use_editor = "yes"])

dnl Should GGZ support be compiled in for server and client?
AC_ARG_ENABLE(ggz,
             [  --enable-ggz    enable ggz support (default=no)],,
             enable_ggz=no
)
	
if test x$enable_ggz = xyes; then
	AC_GGZ_INIT(defaults, export)
	AC_GGZ_GGZDMOD([], [enable_ggz=no])
	AC_GGZ_SERVER([], [enable_ggz=no])
	AC_GGZ_GGZMOD([], [enable_ggz=no])
	AC_GGZ_CONFIG([], [enable_ggz=no])
	if test x$enable_ggz = xyes; then
		AC_GGZ_VERSION(0, 0, 12)
		AC_DEFINE([WITH_GGZ], 1, [Enable GGZ support])
	fi
else
AC_GGZ_CHECK
fi
AM_CONDITIONAL([GGZSUPPORT], [test "x$enable_ggz" = "xyes"])


dnl ...the standard screen resolution...
AC_ARG_WITH(resolution,
           [  --with-resolution=RES standard resolution for lordsawar [default=800x600]], 
            SCREEN_RESOLUTION="$withval", SCREEN_RESOLUTION="800x600")
AC_SUBST(SCREEN_RESOLUTION)

dnl ...do we want to zip (and by the way obfuscate) the save files...
AC_ARG_ENABLE(zipfiles,
             [  --enable-zipping    zip save files (experimental) [default=no]],
             zipfiles="$enableval", zipfiles="no")

ZIP_FILES="false"
if test x$zipfiles = xyes; then
    ZIP_FILES="true"
fi
AC_SUBST(ZIP_FILES)


dnl ..do we want to disable optimizations? (useful only for some debugging cases)...
AC_ARG_ENABLE(optimizations,
             [  --enable-optimizations    enable compiler optimizations (disable for debugging) [default=yes]],
             use_opts="$enableval", use_opts="yes")

if test x$use_opts = xno; then
    AC_MSG_WARN([Optimizations have been disabled])
    CXXFLAGS=-g
fi


dnl ...and finally a workaround to get the absolute data path
if test x$prefix = xNONE; then
    prefix="/usr/local"
fi

rm -f src/main.o

LORDSAWAR_DATADIR="$datadir/lordsawar"
LORDSAWAR_DATADIR=${LORDSAWAR_DATADIR/'${prefix}'/$prefix}
AC_SUBST(LORDSAWAR_DATADIR)

AC_CONFIG_FILES([
        dat/Makefile 
        dat/army/Makefile 
        dat/army/default/Makefile 
        dat/army/imperial/Makefile 
        dat/army/orcs/Makefile 
        dat/army/heroes/Makefile 
        dat/army/monsters/Makefile 
        dat/tilesets/Makefile
        dat/tilesets/default/Makefile
        dat/tilesets/default/misc/Makefile
        dat/various/Makefile 
        dat/various/items/Makefile
        dat/various/borders/Makefile
        dat/various/editor/Makefile
        dat/map/Makefile 
        dat/ggz/Makefile 
        dat/ggz/lordsawar.dsc
        dat/ggz/lordsawar-server.dsc	
        dat/music/Makefile
        dat/theme/Makefile
        dat/theme/lordsawar/Makefile
        dat/theme/generic/Makefile
        dat/glade/Makefile
        intl/Makefile
        po/Makefile.in
        po/Makefile 
        Makefile 
        src/Makefile
        src/utils/Makefile 
        src/server/Makefile 
        src/editor/Makefile
        src/gui/Makefile])
AC_OUTPUT
echo ""
echo "LordsAWar is now configured. Type 'make' to compile it."
echo ""
echo "Configuration:"
echo " Installation location : $prefix"
echo " Editor compilation    : $use_editor"
echo " GGZ support           : $enable_ggz"
echo " Sound support         : $use_sound"
echo ""
echo "Extra Options:"
echo " Compiler Optimizations: $use_opts"
