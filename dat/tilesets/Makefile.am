MAINTAINERCLEANFILES= Makefile.in
lordsawartilesetdir = $(datadir)/lordsawar/tilesets

default.lwt: default.lwt.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/default.lwt.in $(builddir)/default.lwt

bigdefault.lwt: bigdefault.lwt.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigdefault.lwt.in $(builddir)/bigdefault.lwt

wesnoth.lwt: wesnoth.lwt.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/wesnoth.lwt.in $(builddir)/wesnoth.lwt

bigwesnoth.lwt: bigwesnoth.lwt.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigwesnoth.lwt.in $(builddir)/bigwesnoth.lwt

stark.lwt: stark.lwt.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/stark.lwt.in $(builddir)/stark.lwt

bigstark.lwt: bigstark.lwt.in
	LC_ALL=C $(top_srcdir)/intltool-tarmerge $(INTLTOOL_MERGE) $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/bigstark.lwt.in $(builddir)/bigstark.lwt

DISTCLEANFILES = default.lwt bigdefault.lwt wesnoth.lwt bigwesnoth.lwt stark.lwt bigstark.lwt

lordsawartileset_DATA = \
	bigdefault.lwt \
	bigstark.lwt \
	bigwesnoth.lwt \
	default.lwt \
	stark.lwt \
	wesnoth.lwt

EXTRA_DIST = \
	bigdefault.lwt.in \
	bigstark.lwt.in \
	bigwesnoth.lwt.in \
	default.lwt.in \
	stark.lwt.in \
	wesnoth.lwt.in \
	README 
