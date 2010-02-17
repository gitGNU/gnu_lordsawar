MAINTAINERCLEANFILES= Makefile.in

cityset_bigdefaultdir = $(datadir)/lordsawar/citysets/bigdefault

bigdefault.lwc: bigdefault.lwc.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigdefault.lwc.in $(builddir)/bigdefault.lwc
DISTCLEANFILES = bigdefault.lwc

cityset_bigdefault_DATA = \
	bigdefault.lwc \
	castle_razed.png \
	castles.png \
	port.png \
	ruin.png \
	signpost.png \
	temples.png \
	towers.png

EXTRA_DIST = \
	bigdefault.lwc.in \
	README \
	castle_razed.png \
	castles.png \
	port.png \
	ruin.png \
	signpost.png \
	temples.png \
	towers.png
