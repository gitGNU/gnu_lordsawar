SUBDIRS = army tilesets various music glade shield citysets map icons
MANTAINERCLEANFILES= Makefile.in

lordsawardatadir = $(datadir)/lordsawar

@INTLTOOL_XML_RULE@

lordsawardata_DATA = \
    citynames.xml \
    heronames.xml \
    templenames.xml \
    ruinnames.xml \
    signposts.xml \
    gtkrc

DISTCLEANFILES = citynames.xml heronames.xml templenames.xml ruinnames.xml signposts.xml lordsawar.desktop lordsawar.desktop.in


lordsawar.desktop: lordsawar.desktop.in $(wildcard $(top_srcdir)/po/*.po)
		sed -e 's|.[{]prefix[}]|${prefix}|' lordsawar.desktop.in > desktop-foo \
		&& $(INTLTOOL_MERGE) $(top_srcdir)/po desktop-foo $(@) -d -u -c $(top_builddir)/po/.intltool-merge-cachemake \
		&& rm -r desktop-foo

lwdesktopdir= $(datadir)/applications
lwdesktop_DATA= lordsawar.desktop

EXTRA_DIST = \
    README \
    citynames.xml.in \
    heronames.xml.in \
    templenames.xml.in \
    ruinnames.xml.in \
    signposts.xml.in \
    lordsawar.desktop.in.in \
    gtkrc
