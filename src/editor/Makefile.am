#   Copyright (C) 2007, 2008, Ole Laursen
#   Copyright (C) 2007, 2008 Ben Asselstine
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

DEPENDSON = ../GameScenario.o ../army.o ../GameMap.o ../templelist.o ../ruinlist.o \
        ../hero.o ../citylist.o ../Location.o ../UniquelyIdentified.o \
	../stacklist.o \
        ../File.o ../fight.o ../armysetlist.o ../temple.o ../ruin.o ../stack.o \
	../city.o ../path.o ../action.o ../Configuration.o ../MapGenerator.o \
	../ai_dummy.o ../ai_fast.o ../ai_smart.o ../xmlhelper.o ../counter.o \
        ../CreateScenario.o ../history.o ../armyset.o \
	../maptile.o ../real_player.o  ../player.o ../tilesetlist.o \
	../Tile.o ../tileset.o ../playerlist.o \
	../tilestyle.o ../tilestyleset.o \
	../GraphicsCache.o ../QuestsManager.o ../Quest.o ../QKillHero.o \
        ../QEnemyArmies.o ../QCityOccupy.o \
        ../QCitySack.o ../QCityRaze.o ../QPillageGold.o ../QEnemyArmytype.o \
        ../string_tokenizer.o ../reward.o ../rewardlist.o \
        ../Item.o ../Itemlist.o ../Threat.o ../Threatlist.o ../AI_Analysis.o \
	../AI_Allocation.o ../AI_Diplomacy.o ../MoveResult.o ../AICityInfo.o \
        ../FogMap.o ../signpostlist.o ../signpost.o\
	../sound.o ../road.o ../roadlist.o \
        ../bridge.o ../bridgelist.o ../port.o ../portlist.o \
        ../vectoredunitlist.o ../vectoredunit.o \
        ../bigmap.o ../smallmap.o ../overviewmap.o ../timing.o \
        ../gui/image-helpers.o ../gui/error-utils.o ../gui/gtksdl.o \
        ../gui/army-info-tip.o ../MapRenderer.o \
	../shield.o ../shieldset.o ../shieldsetlist.o \
	../citysetlist.o ../cityset.o ../CreateScenarioRandomize.o \
	../Ownable.o ../Movable.o ../Immovable.o ../Positioned.o \
	../Named.o ../Namable.o ../Renamable.o ../NamedLocation.o \
	../network_player.o ../game.o ../NextTurn.o ../gamebigmap.o \
	../herotemplates.o ../game-server.o \
        ../game-client.o ../network-server.o \
	../network-connection.o

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

# NewMapDialog.cpp Bigmap.cpp \
# Smallmap.cpp MapGenDialog.cpp TempleDialog.cpp \
# ArmyDialog.cpp CityDialog.cpp StackDialog.cpp \
# RuinDialog.cpp PlayerDialog.cpp FindDialog.cpp \
# ConditionDialog.cpp ReactionDialog.cpp \
# SelectionDialog.cpp EventDialog.cpp ScenarioDialog.cpp \
# SignpostDialog.cpp 

if FL_RESRC
lordsawar_editor_SOURCES += editor.rc
endif
.rc.o:
	i586-mingw32msvc-windres -o $@ $<

lordsawar_editor_LDADD = $(GTKMM_LIBS) \
			 $(LIBGNET_LIBS) \
			 $(DEPENDSON) \
                           $(SDL_LIBS) \
               @LIBINTL@ -L..
lordsawar_army_editor_LDADD = $(GTKMM_LIBS) \
		              $(LIBGNET_LIBS) \
	                      $(DEPENDSON) \
                              $(SDL_LIBS) \
               @LIBINTL@ -L..

lordsawar_tile_editor_LDADD = $(GTKMM_LIBS) \
		              $(LIBGNET_LIBS) \
	                      $(DEPENDSON) \
                              $(SDL_LIBS) \
               @LIBINTL@ -L..
endif


localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@


AM_CXXFLAGS = -Wall -DPO_PATH=\"$(prefix)/share/locale/\"
INCLUDES = $(GTKMM_CFLAGS) \
    $(SDL_CFLAGS) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' -I../..
