MAINTAINERCLEANFILES= Makefile.in

map_wesnothdir = $(datadir)/lordsawar/tilesets/wesnoth

wesnoth.lwt: wesnoth.lwt.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/wesnoth.lwt.in $(builddir)/wesnoth.lwt

map_wesnoth_DATA = \
    wesnoth.lwt \
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
    wesnoth.lwt.in \
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
