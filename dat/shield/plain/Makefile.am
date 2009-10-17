MAINTAINERCLEANFILES= Makefile.in

lordsawarshieldplaindir = $(datadir)/lordsawar/shield/plain

plain.lws: plain.lws.in
	LC_ALL=C $(INTLTOOL_MERGE) -x -u -c $(top_builddir)/po/.intltool-merge-cache $(top_srcdir)/po $(srcdir)/plain.lws.in $(builddir)/plain.lws

lordsawarshieldplain_DATA = \
	plain.lws \
	small-shield.png \
	small-neutral-shield.png \
	medium-shield.png \
	medium-neutral-shield.png \
	large-shield.png \
	large-neutral-shield.png 

EXTRA_DIST = \
	README \
	plain.lws.in \
	small-shield.png \
	small-neutral-shield.png \
	medium-shield.png \
	medium-neutral-shield.png \
	large-shield.png \
	large-neutral-shield.png 
