MAINTAINERCLEANFILES= Makefile.in

noinst_LIBRARIES = liblwgui.a

liblwgui_a_SOURCES = \
    driver.cpp driver.h \
    army-gains-level-dialog.cpp army-gains-level-dialog.h \
    buy-production-dialog.cpp buy-production-dialog.h \
    destination-dialog.cpp destination-dialog.h \
    city-window.cpp city-window.h \
    error-utils.cpp error-utils.h \
    fight-window.cpp fight-window.h \
    game-preferences-dialog.cpp game-preferences-dialog.h \
    game-window.cpp game-window.h \
    hero-dialog.cpp hero-dialog.h \
    main.cpp main.h \
    splash-window.cpp splash-window.h \
    image-helpers.cpp image-helpers.h \
    gtksdl.c gtksdl.h \
    glade-helpers.h \
    input-helpers.h

INCLUDES = \
   $(GTKMM_CFLAGS) \
   $(SDL_CFLAGS)

AM_CXXFLAGS = -Wall
