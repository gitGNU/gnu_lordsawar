dnl Process this file with autoconf to produce a configure script.
dnl This is useful to force using autoreconf 2.5.x
AC_PREREQ(2.59)
AC_INIT([lordsawar], [0.3.0-pre2], [lordsawar-discuss@nongnu.org], [lordsawar])
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
if test -n "$CXX"; then
  gppver=`$CXX --version 2>/dev/null | sed -e '2,$d' -e 's@^\([[^0-9.]]*\) \([[0-9.]]*\)\(.*\)@\2@'`
else
  AC_MSG_ERROR([*** C++ compiler not found.])
fi
AC_SUBST([gppver])

AC_PROG_LIBTOOL

PKG_CHECK_MODULES(GSTREAMER, gstreamermm-0.10)
AC_SUBST(GSTREAMER_CFLAGS)
AC_SUBST(GSTREAMER_LIBS)

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

dnl check for libxml and libxslt
PKG_CHECK_MODULES(XSLT, [libxslt >= 1.1.20])
AC_SUBST(XSLT_CFLAGS)


#if test $winnls = no ; then
AM_GNU_GETTEXT_VERSION([0.18.3])
AM_GNU_GETTEXT([external])
#fi

dnl Check for glib

PKG_CHECK_MODULES(GLIBMM, glibmm-2.4)

AC_SUBST(GLIBMM_CFLAGS)
AC_SUBST(GLIBMM_LIBS)

dnl Check for gtkmm

PKG_CHECK_MODULES(GTKMM, gtkmm-3.0)

AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)
			     		      
PKG_CHECK_MODULES(XMLPP, libxml++-2.6)
AC_SUBST(XMLPP_CFLAGS)
AC_SUBST(XMLPP_LIBS)

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

dnl Do we want to compile the game list server?
AC_ARG_ENABLE(gls,
            [  --enable-gls     compile game list server (default=yes)],
            use_gls="$enableval", use_gls="yes")
AM_CONDITIONAL([WANT_GLS], [test $use_gls = "yes"])

dnl Do we want to compile the game host server?
AC_ARG_ENABLE(ghs,
            [  --enable-ghs     compile game host server (default=yes)],
            use_ghs="$enableval", use_ghs="yes")
AM_CONDITIONAL([WANT_GHS], [test $use_ghs = "yes"])

dnl ...the standard screen resolution...
AC_ARG_WITH(resolution,
           [  --with-resolution=RES standard resolution for lordsawar [default=800x600]], 
            SCREEN_RESOLUTION="$withval", SCREEN_RESOLUTION="800x600")
AC_SUBST(SCREEN_RESOLUTION)

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

eval LORDSAWAR_DATADIR="$datadir/lordsawar"
AC_SUBST(LORDSAWAR_DATADIR)

AC_CONFIG_FILES([
        dat/Makefile 
        dat/icons/Makefile 
        dat/icons/32x32/Makefile 
        dat/icons/64x64/Makefile 
        dat/lordsawar.desktop.in
        dat/lordsawar-editor.desktop.in
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
        dat/shield/dol/Makefile 
        dat/shield/plain/Makefile 
        dat/citysets/Makefile 
        dat/citysets/default/Makefile 
        dat/citysets/bigdefault/Makefile 
        dat/citysets/dol/Makefile 
        dat/citysets/bigdol/Makefile 
        dat/various/Makefile 
        dat/various/items/Makefile
        dat/various/editor/Makefile
        dat/various/xslt/Makefile
        dat/map/Makefile 
        dat/map/2ndPunic37/Makefile 
        dat/map/dol/Makefile 
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
        src/gls/Makefile
        src/ghs/Makefile
        src/gui/Makefile])
AC_OUTPUT
echo ""
echo "LordsAWar! is now configured. Type 'make' to compile it."
echo ""
echo "Configuration:"
echo " Installation location : $prefix"
echo " Editor compilation    : $use_editor"
echo " Game-list Server      : $use_gls"
echo " Game-host Server      : $use_ghs"
echo ""
echo "Extra Options:"
echo " Compiler Optimizations: $use_opts"
