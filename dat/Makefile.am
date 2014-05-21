SUBDIRS = army tilesets various music glade shield citysets map icons
MANTAINERCLEANFILES= Makefile.in

lordsawardatadir = $(datadir)/lordsawar

@INTLTOOL_XML_RULE@

lordsawardata_DATA = \
    citynames.xml \
    heronames.xml \
    templenames.xml \
    ruinnames.xml \
    signposts.xml

DISTCLEANFILES = citynames.xml heronames.xml templenames.xml ruinnames.xml signposts.xml lordsawar.desktop lordsawar.desktop.in lordsawar-editor.desktop lordsawar-editor.desktop.in

CLEANFILES=desktop-foo desktop-bar

lordsawar.desktop: lordsawar.desktop.in $(wildcard $(top_srcdir)/po/*.po)
		sed -e 's|.[{]prefix[}]|${prefix}|' lordsawar.desktop.in > desktop-foo \
		&& $(INTLTOOL_MERGE) $(top_srcdir)/po desktop-foo $(@) -d -u -c $(top_builddir)/po/.intltool-merge-cache \
		&& rm -r desktop-foo

lordsawar-editor.desktop: lordsawar-editor.desktop.in $(wildcard $(top_srcdir)/po/*.po)
		sed -e 's|.[{]prefix[}]|${prefix}|' lordsawar-editor.desktop.in > desktop-bar \
		&& $(INTLTOOL_MERGE) $(top_srcdir)/po desktop-bar $(@) -d -u -c $(top_builddir)/po/.intltool-merge-cache \
		&& rm -r desktop-bar

lwdesktopdir= $(datadir)/applications
lwdesktop_DATA= lordsawar.desktop lordsawar-editor.desktop

EXTRA_DIST = \
    README \
    citynames.xml.in \
    heronames.xml.in \
    templenames.xml.in \
    ruinnames.xml.in \
    signposts.xml.in \
    lordsawar.desktop.in.in \
    lordsawar-editor.desktop.in.in
