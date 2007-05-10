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
    gtksdl.c gtksdl.h \
    glade-helpers.h

INCLUDES = \
   $(GTKMM_CFLAGS) \
   $(SDL_CFLAGS)

AM_CXXFLAGS = -Wall
