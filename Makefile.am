SUBDIRS = src dat po doc help
MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~

lordsawarconfigdir = $(LORDSAWAR_DATADIR)

#ACLOCAL_AMFLAGS = -I ./m4

EXTRA_DIST = DEPENDENCIES README $(INTLTOOL_FILES)

INTLTOOL_FILES = intltool-extract.in \
                 intltool-merge.in \
                 intltool-update.in

DISTCLEANFILES = intltool-extract \
                 intltool-merge \
                 intltool-update \
                 po/.intltool-merge-cache

ACLOCAL_AMFLAGS = -I m4
