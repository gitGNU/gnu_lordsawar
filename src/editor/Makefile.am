#   Copyright (C) 2007, 2008, Ole Laursen
#   Copyright (C) 2007, 2008 Ben Asselstine
#   Copyright (C) 2008 Barry deFreese
# 
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
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
bin_PROGRAMS = lordsawar-editor lordsawar-army-editor lordsawar-tile-editor

lordsawar_army_editor_SOURCES = \
armyset-main.cpp \
armyset-window.cpp armyset-window.h  \
armyset-info-dialog.cpp armyset-info-dialog.h \
glade-helpers.h

lordsawar_tile_editor_SOURCES = \
tileset-main.cpp \
tileset-window.cpp tileset-window.h \
tileset-info-dialog.cpp tileset-info-dialog.h \
tile-preview-dialog.cpp tile-preview-dialog.h \
tile-preview-scene.cpp tile-preview-scene.h \
tileset-selector-editor-dialog.cpp tileset-selector-editor-dialog.h \
glade-helpers.h

lordsawar_editor_SOURCES = \
main.cpp \
main-window.cpp main-window.h \
signpost-dialog.cpp signpost-dialog.h \
temple-dialog.cpp temple-dialog.h \
ruin-dialog.cpp ruin-dialog.h \
stack-dialog.cpp stack-dialog.h \
select-army-dialog.cpp select-army-dialog.h \
players-dialog.cpp players-dialog.h \
city-dialog.cpp city-dialog.h \
map-info-dialog.cpp map-info-dialog.h \
new-map-dialog.cpp new-map-dialog.h \
editorbigmap.cpp editorbigmap.h \
glade-helpers.h \
itemlist-dialog.cpp itemlist-dialog.h \
reward-dialog.cpp reward-dialog.h \
select-item-dialog.cpp select-item-dialog.h \
select-reward-dialog.cpp select-reward-dialog.h \
rewardlist-dialog.cpp rewardlist-dialog.h \
select-hidden-ruin-dialog.cpp select-hidden-ruin-dialog.h \
RenamableLocation.cpp RenamableLocation.h 

if FL_RESRC
lordsawar_editor_SOURCES += editor.rc
endif
.rc.o:
	i586-mingw32msvc-windres -o $@ $<

lordsawar_editor_LDADD = $(top_builddir)/src/gui/liblwgui.la \
                         $(top_builddir)/src/liblordsawar.la \
                         $(GTKMM_LIBS) \
                         $(SDL_LIBS) \
                         @LIBINTL@ -L$(top_builddir)/src

lordsawar_army_editor_LDADD = $(top_builddir)/src/gui/liblwgui.la \
                         $(top_builddir)/src/liblordsawar.la \
                         $(SDL_LIBS) \
                         $(GTKMM_LIBS) \
                         @LIBINTL@ -L$(top_builddir)/src

lordsawar_tile_editor_LDADD = $(top_builddir)/src/gui/liblwgui.la \
                         $(top_builddir)/src/liblordsawar.la \
                         $(GTKMM_LIBS) \
                         $(SDL_LIBS) \
                         @LIBINTL@ -L$(top_builddir)/src
endif


localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@


AM_CXXFLAGS = -Wall -DPO_PATH=\"$(prefix)/share/locale/\"
INCLUDES = $(GTKMM_CFLAGS) \
    $(SDL_CFLAGS) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' -I$(top_srcdir) -I$(top_srcdir)/src
