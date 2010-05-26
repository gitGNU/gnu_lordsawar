
MAINTAINERCLEANFILES= Makefile.in
lordsawararmysetdir = $(datadir)/lordsawar/army

bigdefault.lwa: bigdefault.lwa.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigdefault.lwa.in $(builddir)/bigdefault.lwa

default.lwa: default.lwa.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/default.lwa.in $(builddir)/default.lwa

DISTCLEANFILES = default.lwa bigdefault.lwa
lordsawararmyset_DATA = \
	default.lwa \
	bigdefault.lwa

EXTRA_DIST = \
	default.lwa.in \
	bigdefault.lwa.in \
	README 
