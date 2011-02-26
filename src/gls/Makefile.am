#   Copyright (C) 2011 Ben Asselstine
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

##if WANT_GLS
bin_PROGRAMS = lordsawar-game-list-server

lordsawar_game_list_server_SOURCES = main.cpp \
  gamelist-server.cpp gamelist-server.h 

lordsawar_game_list_server_LDADD = $(LIBSIGC_LIBS) $(GTKMM_LIBS) -ltar -lz \
                                   $(top_builddir)/src/liblordsawar.la \
                                   $(top_builddir)/src/liblordsawargamelist.la \
                                   $(top_builddir)/src/liblordsawarnet.la \
                                   @LIBINTL@ \
                                   -L$(top_builddir)/src
##endif


localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@


AM_CXXFLAGS = -Wall -DPO_PATH=\"$(prefix)/share/locale/\"
INCLUDES = \
    $(SDL_CFLAGS) \
    $(LIBSIGC_CFLAGS) \
    $(GTKMM_CFLAGS) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' \
    -I$(top_srcdir) -I$(top_srcdir)/src

EXTRA_DIST = README
