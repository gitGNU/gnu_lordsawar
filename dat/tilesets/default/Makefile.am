MAINTAINERCLEANFILES= Makefile.in

map_defaultdir = $(datadir)/lordsawar/tilesets/default

default.lwt: default.lwt.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/default.lwt.in $(builddir)/default.lwt
DISTCLEANFILES = default.lwt

map_default_DATA = \
    default.lwt \
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
    default.lwt.in \
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
