dnl Process this file with autoconf to produce a configure script.
dnl This is useful to force using autoreconf 2.5.x
AC_PREREQ(2.59)
AC_INIT([lordsawar], [0.1.7-pre2], [http://sv.nongnu.org/bugs/?group=lordsawar], [lordsawar])
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER(config.h)
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE

IT_PROG_INTLTOOL([0.35.5])

GETTEXT_PACKAGE=lordsawar
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"],
                   [The domain to use with gettext])
AM_GLIB_GNU_GETTEXT

PROGRAMNAME_LOCALEDIR=[${datadir}/locale]
AC_SUBST(PROGRAMNAME_LOCALEDIR)
    
dnl Checks for programs.
AC_PROG_CXX
AC_PROG_LIBTOOL

dnl The minimum SDL version required
SDL_VERSION=1.2.0

ac_save_ldflags=$LDFLAGS
dnl Check for SDL
AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)
	
LDFLAGS=$ac_save_ldflags
AC_SUBST(SDL_CFLAGS)

dnl And check for SDL_mixer. In detail, we need .ogg support
dnl we need some extended cflags or get problems
ac_save_cflags=$CFLAGS
ac_save_ldflags=$LDFLAGS
ac_save_libs=$LIBS

use_sound=1
AC_ARG_ENABLE(sound, [  --enable-sound         Enable or disable sound usage (default on)],
                use_sound="$enableval", use_sound="yes")

SDLSOUND_LIBS=
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

    SDLSOUND_LIBS="-lSDL_mixer"

    CFLAGS=$ac_save_CFLAGS
    LDFLAGS="$ac_save_LDFLAGS"
    LIBS="$ac_save_libs"
fi
AC_SUBST(SDLSOUND_LIBS)

	
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

dnl Check for libtar
libtar=0
AC_ARG_WITH(libtar, [  --with-libtar=<dir>      Use libtar from here],
                      [
                      libtar=$withval
                      CPPFLAGS="$CPPFLAGS -I$withval/include"
                      LDFLAGS="$LDFLAGS -L$withval/lib"
                      LIBS="$LIBS -ltar"
                      ]
                      )
if test $libtar = 0 ; then
   AC_CHECK_HEADERS(libtar.h,[],AC_MSG_ERROR([*** Libtar not found!]))
   AC_CHECK_LIB(tar,tar_open,[],[AC_MSG_ERROR([*** Libtar not found!])])
fi

dnl this is needed in cross-compile with mingw32
AC_ARG_ENABLE(winlibs,             
              [  --enable-winlibs         enable windows cross-compiling libs],
              LIBS="-lmingw32 -lws2_32 -lintl -lz -lpthreadGC2 -lpthreadGCE2 $LIBS"
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
AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])
#fi

dnl Check for glib

PKG_CHECK_MODULES(GLIBMM, glibmm-2.4)

AC_SUBST(GLIBMM_CFLAGS)
AC_SUBST(GLIBMM_LIBS)

dnl Check for gtkmm

PKG_CHECK_MODULES(GTKMM, gtkmm-2.4)

AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)
			     		      
dnl Check for libgnet

PKG_CHECK_MODULES(LIBGNET, gnet-2.0)

AC_SUBST(LIBGNET_CFLAGS)
AC_SUBST(LIBGNET_LIBS)


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

dnl Do we want to compile the play by mail server?
AC_ARG_ENABLE(pbm,
            [  --enable-pbm     compile play-by-mail (default=yes)],
            use_pbm="$enableval", use_pbm="yes")
AM_CONDITIONAL([WANT_PBM], [test $use_pbm = "yes"])

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


AC_DEFUN([AC_CHOOSE_BOOST],[
    am_boost_dir_default=ifelse([$1], , /usr/include, $1)
    BOOST_CPPFLAGS=''
    AC_ARG_WITH(boost_includedir,
                AS_HELP_STRING([--with-boost-includedir=DIR],
                               [Use Boost library installed at specified path (default /usr/include)]),
                [am_boost_dir=$withval],
                [am_boost_dir=$am_boost_dir_default])

    AC_MSG_CHECKING([for boost directory])
    if test -d $am_boost_dir/boost; then
        AC_MSG_RESULT([$am_boost_dir/boost])
        BOOST_CPPFLAGS="-I$am_boost_dir"
        old_CPPFLAGS=$CPPFLAGS
        CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
        AC_LANG_PUSH(C++)
        AC_CHECK_HEADERS([boost/version.hpp],
                         [ifelse([$2], , :, [$2])],
                         [ifelse([$3], , :, [$3])])
        AC_LANG_POP(C++)
        CPPFLAGS=$old_CPPFLAGS
        AC_SUBST(BOOST_CPPFLAGS)
    else
        AC_MSG_RESULT([not found])
        AC_MSG_ERROR([*** Can't find directory $am_boost_dir/boost])
        ifelse([$3], , :, [$3])
    fi
])

AC_CHOOSE_BOOST(,,[AC_MSG_ERROR([*** Boost library not found! Install it.])])


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
        dat/army/bigdefault/Makefile 
        dat/tilesets/Makefile
        dat/tilesets/default/Makefile
        dat/tilesets/bigdefault/Makefile
        dat/tilesets/stark/Makefile
        dat/tilesets/bigstark/Makefile
        dat/tilesets/wesnoth/Makefile
        dat/tilesets/bigwesnoth/Makefile
        dat/shield/Makefile 
        dat/shield/default/Makefile 
        dat/shield/plain/Makefile 
        dat/shield/dol/Makefile 
        dat/citysets/Makefile 
        dat/citysets/default/Makefile 
        dat/citysets/bigdefault/Makefile 
        dat/citysets/dol/Makefile 
        dat/citysets/bigdol/Makefile 
        dat/various/Makefile 
        dat/various/items/Makefile
        dat/various/editor/Makefile
        dat/map/Makefile 
        dat/music/Makefile
        dat/glade/Makefile
        dat/glade/editor/Makefile
        po/Makefile.in
        Makefile 
        src/Makefile
        doc/Makefile
        help/Makefile
        help/figures/Makefile
        src/editor/Makefile
        src/pbm/Makefile
        src/gui/Makefile])
AC_OUTPUT
echo ""
echo "LordsAWar is now configured. Type 'make' to compile it."
echo ""
echo "Configuration:"
echo " Installation location : $prefix"
echo " Editor compilation    : $use_editor"
echo " Play-by-mail          : $use_pbm"
echo " Sound support         : $use_sound"
echo ""
echo "Extra Options:"
echo " Compiler Optimizations: $use_opts"
