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
	../shield.o ../shieldset.o ../shieldstyle.o ../shieldsetlist.o \
	../citysetlist.o ../cityset.o ../CreateScenarioRandomize.o \
	../Ownable.o ../Movable.o ../Immovable.o ../Positioned.o \
	../Named.o ../Namable.o ../Renamable.o ../NamedLocation.o \
	../network_player.o ../game.o ../NextTurn.o ../gamebigmap.o \
	../NextTurnPbm.o  ../NextTurnHotseat.o ../NextTurnNetworked.o \
	../herotemplates.o ../game-server.o \
        ../game-client.o ../network-server.o \
	../network-connection.o ../GameScenarioOptions.o ../network-action.o \
	../network-history.o ../pbm-game-server.o ../pbm-game-client.o \
	../game-client-decoder.o ../chat-client.o ../game-station.o

##if WANT_PBM
bin_PROGRAMS = lordsawar-pbm

lordsawar_pbm_SOURCES =  pbm.cpp main.cpp pbm.h


lordsawar_pbm_LDADD = $(GTKMM_LIBS) \
			 $(LIBGNET_LIBS) \
			 $(DEPENDSON) \
                           $(SDL_LIBS) \
               @LIBINTL@ -L..
##endif


localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@


AM_CXXFLAGS = -Wall -DPO_PATH=\"$(prefix)/share/locale/\"
INCLUDES = $(GTKMM_CFLAGS) \
    $(SDL_CFLAGS) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' -I../..

EXTRA_DIST = README
