MAINTAINERCLEANFILES= Makefile.in

noinst_LIBRARIES = liblwgui.a

liblwgui_a_SOURCES = \
    driver.cpp driver.h \
    city-window.cpp city-window.h \
    error-utils.cpp error-utils.h \
    fight-window.cpp fight-window.h \
    game-preferences-dialog.cpp game-preferences-dialog.h \
    game-window.cpp game-window.h \
    main.cpp main.h \
    splash-window.cpp splash-window.h \
    image-helpers.cpp image-helpers.h \
    gtksdl.c gtksdl.h

INCLUDES = \
   -DLORDSAWAR_GLADEDIR=\""$(gladedir)/"\" \
   $(GTKMM_CFLAGS) \
   $(SDL_CFLAGS)

AM_CXXFLAGS = -Wall

glade_DATA = \
city-defeated-dialog.glade     hero-offer-dialog.glade \
city-sacked-dialog.glade       city-window.glade \
city-pillaged-dialog.glade     quest-assigned-dialog.glade \
fight-window.glade             ruin-searched-dialog.glade \
game-preferences-dialog.glade  splash-window.glade \
game-window.glade              temple-visit-dialog.glade \
city-looted-dialog.glade \
ruinfight-started-dialog.glade ruinfight-finished-dialog.glade

gladedir = $(datadir)/$(PACKAGE)/glade

EXTRA_DIST = $(glade_DATA)
