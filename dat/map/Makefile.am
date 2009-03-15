MANTAINERCLEANFILES= Makefile.in

%.map:       %.map.in       $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*.po) ; LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $< $@

mapdir = $(datadir)/lordsawar/map/
map_DATA = \
	2ndPunic37.map \
	dol.map

EXTRA_DIST = \
	README \
	2ndPunic37.map.in \
	dol.map.in
