MANTAINERCLEANFILES= Makefile.in

mapdir = $(datadir)/lordsawar/map/
2ndPunic37.map: 2ndPunic37.map.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/2ndPunic37.map.in $(builddir)/2ndPunic37.map
dol.map: dol.map.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/dol.map.in $(builddir)/dol.map
DISTCLEANFILES = dol.map 2ndPunic37.map
map_DATA = \
	2ndPunic37.map \
	dol.map

EXTRA_DIST = \
	README \
	2ndPunic37.map.in \
	dol.map.in
