MAINTAINERCLEANFILES= Makefile.in

cityset_defaultdir = $(datadir)/lordsawar/citysets/default

default.lwc: default.lwc.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/default.lwc.in $(builddir)/default.lwc
DISTCLEANFILES = default.lwc

cityset_default_DATA = \
	default.lwc \
	castle_razed.png \
	castles.png \
	port.png \
	ruin.png \
	signpost.png \
	temples.png \
	towers.png

EXTRA_DIST = \
	default.lwc.in \
	README \
	castle_razed.png \
	castles.png \
	port.png \
	ruin.png \
	signpost.png \
	temples.png \
	towers.png
