MAINTAINERCLEANFILES= Makefile.in

lordsawartilebigwesnothdir = $(datadir)/lordsawar/tilesets/bigwesnoth

bigwesnoth.lwt: bigwesnoth.lwt.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigwesnoth.lwt.in $(builddir)/bigwesnoth.lwt
DISTCLEANFILES = bigwesnoth.lwt

lordsawartilebigwesnoth_DATA = \
    bigwesnoth.lwt \
    grass.png \
    water.png \
    forest.png \
    hills.png \
    mountains.png \
    swamp.png \
    flags.png \
    roads.png \
    bridges.png \
    fog.png \
    selector.png \
    small_selector.png \
    explosion.png

EXTRA_DIST = \
    bigwesnoth.lwt.in \
    README \
    grass.png \
    water.png \
    forest.png \
    hills.png \
    mountains.png \
    swamp.png \
    flags.png \
    roads.png \
    bridges.png \
    fog.png \
    selector.png \
    small_selector.png \
    explosion.png
