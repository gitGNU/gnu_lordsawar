#   Copyright (C) 2007, 2008 Ole Laursen
#   Copyright (C) 2007, 2008, 2009, 2010 Ben Asselstine
#   Copyright (C) 2008 Barry deFreese
# 
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Library General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
#   02110-1301, USA.
MAINTAINERCLEANFILES= Makefile.in

if WANT_EDITOR
bin_PROGRAMS = lordsawar-editor lordsawar-army-editor lordsawar-tile-editor lordsawar-city-editor lordsawar-shield-editor

noinst_LTLIBRARIES = libshieldsetwindow.la libarmysetwindow.la libcitysetwindow.la libtilesetwindow.la libeditutil.la

libeditutil_la_SOURCES = \
	recently-edited-file.cpp recently-edited-file.h \
	recently-edited-file-list.cpp recently-edited-file-list.h \
	editor-quit-dialog.cpp editor-quit-dialog.h \
	editor-recover-dialog.cpp editor-recover-dialog.h \
	tile-size-editor-dialog.cpp tile-size-editor-dialog.h \
	glade-helpers.h

libshieldsetwindow_la_SOURCES = \
	shieldset-window.cpp shieldset-window.h \
	shieldset-info-dialog.cpp shieldset-info-dialog.h \
	masked-image-editor-dialog.cpp masked-image-editor-dialog.h

libarmysetwindow_la_SOURCES = \
	armyset-window.cpp armyset-window.h  \
	armyset-info-dialog.cpp armyset-info-dialog.h \
	masked-image-editor-dialog.cpp masked-image-editor-dialog.h \
	image-editor-dialog.cpp image-editor-dialog.h 

libcitysetwindow_la_SOURCES = \
	cityset-window.cpp cityset-window.h  \
	cityset-info-dialog.cpp cityset-info-dialog.h \
	image-editor-dialog.cpp image-editor-dialog.h 

libtilesetwindow_la_SOURCES = \
	tileset-window.cpp tileset-window.h \
	tileset-info-dialog.cpp tileset-info-dialog.h \
	tile-preview-dialog.cpp tile-preview-dialog.h \
	tile-preview-scene.cpp tile-preview-scene.h \
	tileset-selector-editor-dialog.cpp tileset-selector-editor-dialog.h \
	tileset-flag-editor-dialog.cpp tileset-flag-editor-dialog.h \
	tileset-explosion-picture-editor-dialog.cpp \
	tileset-explosion-picture-editor-dialog.h \
	tilestyle-organizer-dialog.cpp tilestyle-organizer-dialog.h \
	tileset-smallmap-building-colors-dialog.cpp \
	tileset-smallmap-building-colors-dialog.h\
	image-editor-dialog.cpp image-editor-dialog.h

lordsawar_army_editor_SOURCES = \
	armyset-main.cpp 

lordsawar_city_editor_SOURCES = \
	cityset-main.cpp

lordsawar_shield_editor_SOURCES = \
	shieldset-main.cpp

lordsawar_tile_editor_SOURCES = \
	tileset-main.cpp

lordsawar_editor_SOURCES = \
	main.cpp \
	main-window.cpp main-window.h \
	signpost-editor-dialog.cpp signpost-editor-dialog.h \
	temple-editor-dialog.cpp temple-editor-dialog.h \
	ruin-editor-dialog.cpp ruin-editor-dialog.h \
	stack-editor-dialog.cpp stack-editor-dialog.h \
	select-army-dialog.cpp select-army-dialog.h \
	players-dialog.cpp players-dialog.h \
	city-editor-dialog.cpp city-editor-dialog.h \
	map-info-dialog.cpp map-info-dialog.h \
	new-map-dialog.cpp new-map-dialog.h \
	switch-sets-dialog.cpp switch-sets-dialog.h \
	editorbigmap.cpp editorbigmap.h \
	editablesmallmap.cpp editablesmallmap.h \
	smallmap-editor-dialog.cpp smallmap-editor-dialog.h \
	itemlist-dialog.cpp itemlist-dialog.h \
	reward-editor-dialog.cpp reward-editor-dialog.h \
	select-item-dialog.cpp select-item-dialog.h \
	select-reward-dialog.cpp select-reward-dialog.h \
	rewardlist-dialog.cpp rewardlist-dialog.h \
	select-hidden-ruin-dialog.cpp select-hidden-ruin-dialog.h \
	RenamableLocation.cpp RenamableLocation.h  \
	backpack-editor-dialog.cpp backpack-editor-dialog.h \
	hero-editor-dialog.cpp hero-editor-dialog.h \
	editor-splash-window.cpp editor-splash-window.h
	
if FL_RESRC
lordsawar_editor_SOURCES += editor.rc
endif
.rc.o:
	i586-mingw32msvc-windres -o $@ $<

lordsawar_editor_LDADD = $(top_builddir)/src/gui/liblwgui.la \
                         $(top_builddir)/src/liblordsawargfx.la \
                         $(top_builddir)/src/liblordsawar.la \
			 libshieldsetwindow.la \
			 libarmysetwindow.la \
			 libcitysetwindow.la \
			 libtilesetwindow.la \
			 libeditutil.la \
                         $(GTKMM_LIBS) \
                         $(SDL_LIBS) -ltar -lz \
                         @LIBINTL@ -L$(top_builddir)/src

lordsawar_shield_editor_LDADD =  \
			 libshieldsetwindow.la \
                         $(top_builddir)/src/gui/liblwgui.la \
                         $(top_builddir)/src/liblordsawargfx.la \
                         $(top_builddir)/src/liblordsawar.la \
                         $(SDL_LIBS) -ltar -lz \
                         $(GTKMM_LIBS) \
			 libeditutil.la \
                         @LIBINTL@ -L$(top_builddir)/src

lordsawar_city_editor_LDADD = \
			 libcitysetwindow.la \
                         $(top_builddir)/src/gui/liblwgui.la \
                         $(top_builddir)/src/liblordsawargfx.la \
                         $(top_builddir)/src/liblordsawar.la \
                         $(SDL_LIBS) -ltar -lz \
                         $(GTKMM_LIBS) \
			 libeditutil.la \
                         @LIBINTL@ -L$(top_builddir)/src

lordsawar_army_editor_LDADD = \
			 libarmysetwindow.la \
                         $(top_builddir)/src/gui/liblwgui.la \
                         $(top_builddir)/src/liblordsawargfx.la \
                         $(top_builddir)/src/liblordsawar.la \
                         $(SDL_LIBS) -ltar -lz \
                         $(GTKMM_LIBS) \
			 libeditutil.la \
                         @LIBINTL@ -L$(top_builddir)/src

lordsawar_tile_editor_LDADD = \
			 libtilesetwindow.la \
			 $(top_builddir)/src/gui/liblwgui.la \
                         $(top_builddir)/src/liblordsawargfx.la \
                         $(top_builddir)/src/liblordsawar.la \
                         $(SDL_LIBS) -ltar -lz \
                         $(GTKMM_LIBS) \
			 libeditutil.la \
                         @LIBINTL@ -L$(top_builddir)/src
endif


localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@


AM_CXXFLAGS = -Wall -DPO_PATH=\"$(prefix)/share/locale/\"
INCLUDES = $(GTKMM_CFLAGS) \
    $(SDL_CFLAGS) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/src/gui/
