MAINTAINERCLEANFILES= Makefile.in

map_bigstarkdir = $(datadir)/lordsawar/tilesets/bigstark

bigstark.lwt: bigstark.lwt.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigstark.lwt.in $(builddir)/bigstark.lwt
DISTCLEANFILES = bigstark.lwt

map_bigstark_DATA = \
    bigstark.lwt \
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
    bigstark.lwt.in \
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
