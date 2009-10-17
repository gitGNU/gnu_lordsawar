MAINTAINERCLEANFILES= Makefile.in

cityset_bigdoldir = $(datadir)/lordsawar/citysets/bigdol

bigdol.lwc: bigdol.lwc.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigdol.lwc.in $(builddir)/bigdol.lwc

cityset_bigdol_DATA = \
	bigdol.lwc \
	castle_razed.png \
	castles.png \
	port.png \
	ruin.png \
	signpost.png \
	temples.png \
	towers.png

EXTRA_DIST = \
	bigdol.lwc.in \
	README \
	castle_razed.png \
	castles.png \
	port.png \
	ruin.png \
	signpost.png \
	temples.png \
	towers.png
