SUBDIRS = army tilesets various map music glade shield citysets
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

DISTCLEANFILES = citynames.xml heronames.xml templenames.xml ruinnames.xml signposts.xml

lwdesktopdir= $(datadir)/applications
lwdesktop_DATA= lordsawar.desktop

lordsawar64icondir=$(datadir)/icons/hicolor/64x64/apps
lordsawar64icon= 

lordsawar32icondir=$(datadir)/icons/hicolor/32x32/apps
lordsawar32icon= 

EXTRA_DIST = \
    README \
    citynames.xml.in \
    heronames.xml.in \
    templenames.xml.in \
    ruinnames.xml.in \
    signposts.xml.in \
    lordsawar.desktop \
    gtkrc
