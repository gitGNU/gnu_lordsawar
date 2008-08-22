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

##if WANT_PBM
bin_PROGRAMS = lordsawar-pbm

lordsawar_pbm_SOURCES =  pbm.cpp main.cpp pbm.h


lordsawar_pbm_LDADD = $(LIBSIGC_LIBS) \
                         $(GLIBMM_LIBS) \
                         $(top_builddir)/src/liblordsawar.la \
                         @LIBINTL@ \
                         -L$(top_builddir)/src
##endif


localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@


AM_CXXFLAGS = -Wall -DPO_PATH=\"$(prefix)/share/locale/\"
INCLUDES = $(GTKMM_CFLAGS) \
    $(SDL_CFLAGS) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' \
    -I$(top_srcdir) -I$(top_srcdir)/src

EXTRA_DIST = README
