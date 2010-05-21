MAINTAINERCLEANFILES= Makefile.in
lordsawarshielddir = $(datadir)/lordsawar/shield

default.lws: default.lws.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/default.lws.in $(builddir)/default.lws

dol.lws: dol.lws.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/dol.lws.in $(builddir)/dol.lws

plain.lws: plain.lws.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/plain.lws.in $(builddir)/plain.lws

DISTCLEANFILES = default.lws dol.lws plain.lws
lordsawarshield_DATA = \
	default.lws \
	dol.lws \
	plain.lws

EXTRA_DIST = \
	default.lws.in \
	dol.lws.in \
	plain.lws.in \
	README 
