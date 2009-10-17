MAINTAINERCLEANFILES= Makefile.in

cityset_doldir = $(datadir)/lordsawar/citysets/dol

dol.lwc: dol.lwc.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/dol.lwc.in $(builddir)/dol.lwc

cityset_dol_DATA = \
	dol.lwc \
	castle_razed.png \
	castles.png \
	port.png \
	ruin.png \
	signpost.png \
	temples.png \
	towers.png

EXTRA_DIST = \
	dol.lwc.in \
	README \
	castle_razed.png \
	castles.png \
	port.png \
	ruin.png \
	signpost.png \
	temples.png \
	towers.png
