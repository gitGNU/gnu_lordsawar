MAINTAINERCLEANFILES= Makefile.in

map_starkdir = $(datadir)/lordsawar/tilesets/stark

stark.lwt: stark.lwt.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/stark.lwt.in $(builddir)/stark.lwt
DISTCLEANFILES = stark.lwt

map_stark_DATA = \
    stark.lwt \
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
    stark.lwt.in \
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
