MAINTAINERCLEANFILES= Makefile.in
lordsawarcitysetdir = $(datadir)/lordsawar/citysets

bigdefault.lwc: bigdefault.lwc.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigdefault.lwc.in $(builddir)/bigdefault.lwc

bigdol.lwc: bigdol.lwc.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigdol.lwc.in $(builddir)/bigdol.lwc

default.lwc: default.lwc.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/default.lwc.in $(builddir)/default.lwc

dol.lwc: dol.lwc.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/dol.lwc.in $(builddir)/dol.lwc

DISTCLEANFILES = default.lwc dol.lwc bigdefault.lwc bigdol.lwc
lordsawarcityset_DATA = \
	default.lwc \
	bigdefault.lwc \
	dol.lwc \
	bigdol.lwc

EXTRA_DIST = \
	default.lwc.in \
	bigdefault.lwc.in \
	dol.lwc.in \
	bigdol.lwc.in \
	README 
