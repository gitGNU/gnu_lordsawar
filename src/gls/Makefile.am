#   Copyright (C) 2011, 2015 Ben Asselstine
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

if WANT_GLS
bin_PROGRAMS = lordsawar-game-list-server lordsawar-game-list-client

lordsawar_game_list_client_SOURCES = gls-client-main.cpp \
				     gls-client-tool.cpp gls-client-tool.h

lordsawar_game_list_client_LDADD = \
                                  -L$(top_builddir)/src \
                                  $(top_builddir)/src/liblordsawar.la \
                                  $(top_builddir)/src/liblordsawargamelist.la \
                                  $(top_builddir)/src/liblordsawarnet.la \
                                  $(LIBSIGC_LIBS) \
                                  $(GTKMM_LIBS) \
                                  $(XSLT_LIBS) \
                                  $(XMLPP_LIBS) \
                                  $(ARCHIVE_LIBS) \
                                  -lz

lordsawar_game_list_server_SOURCES = main.cpp \
  gamelist-server.cpp gamelist-server.h 

lordsawar_game_list_server_LDADD = \
                                  -L$(top_builddir)/src \
                                  $(top_builddir)/src/liblordsawar.la \
                                  $(top_builddir)/src/liblordsawargamelist.la \
                                  $(top_builddir)/src/liblordsawarnet.la \
                                  $(LIBSIGC_LIBS) \
                                  $(GTKMM_LIBS) \
                                  $(XSLT_LIBS) \
                                  $(XMLPP_LIBS) \
                                  $(ARCHIVE_LIBS) \
                                  -lz

endif


localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@


AM_CXXFLAGS = -Wall -Wshadow -Wextra -pedantic -Wno-deprecated-declarations \
  -DPO_PATH=\"$(prefix)/share/locale/\" -std=c++11
AM_CPPFLAGS = \
    $(SDL_CFLAGS) \
    $(LIBSIGC_CFLAGS) \
    $(GTKMM_CFLAGS) \
    $(XMLPP_CFLAGS) \
    $(XSLT_CFLAGS) \
    $(ARCHIVE_CFLAGS) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' \
    -I$(top_srcdir) -I$(top_srcdir)/src

EXTRA_DIST = README
