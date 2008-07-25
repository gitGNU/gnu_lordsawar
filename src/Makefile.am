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
SUBDIRS = gui . editor pbm
MAINTAINERCLEANFILES= Makefile.in

bin_PROGRAMS = lordsawar

lordsawar_SOURCES = army.cpp army.h hero.cpp hero.h stack.cpp stack.h \
        GameScenario.cpp GameScenario.h action.cpp action.h \
	player.cpp player.h real_player.cpp real_player.h ai_dummy.cpp \
        ai_dummy.h ai_fast.cpp ai_fast.h ai_smart.cpp ai_smart.h \
	UniquelyIdentified.cpp UniquelyIdentified.h Location.cpp Location.h \
	city.cpp city.h NamedLocation.cpp NamedLocation.h \
        temple.cpp temple.h ruin.cpp ruin.h Tile.cpp Tile.h \
	tilestyle.cpp tilestyle.h tilestyleset.cpp tilestyleset.h \
	maptile.cpp maptile.h Item.cpp Item.h armysetlist.cpp armysetlist.h \
        armyset.cpp armyset.h tilesetlist.cpp tilesetlist.h \
        stacklist.cpp stacklist.h playerlist.cpp playerlist.h \
	MapRenderer.cpp MapRenderer.h \
	citylist.cpp citylist.h templelist.cpp templelist.h \
        ruinlist.cpp ruinlist.h GameMap.cpp GameMap.h \
	Itemlist.cpp Itemlist.h tileset.cpp tileset.h Threat.cpp Threat.h \
        Threatlist.cpp Threatlist.h MoveResult.cpp MoveResult.h \
	AICityInfo.cpp AICityInfo.h AI_Analysis.cpp AI_Analysis.h \
        AI_Allocation.cpp AI_Allocation.h path.cpp path.h \
        QuestsManager.cpp QuestsManager.h Quest.cpp Quest.h \
        reward.h reward.cpp rewardlist.h rewardlist.cpp \
	QKillHero.cpp QKillHero.h \
        QEnemyArmies.cpp QEnemyArmies.h QCitySack.cpp QCitySack.h \
        QCityRaze.cpp QCityRaze.h QCityOccupy.cpp QCityOccupy.h \
        QEnemyArmytype.cpp QEnemyArmytype.h QPillageGold.cpp QPillageGold.h \
        File.cpp File.h main.cpp \
	xmlhelper.cpp xmlhelper.h string_tokenizer.cpp string_tokenizer.h \
        NextTurn.cpp NextTurn.h \
	NextTurnPbm.cpp NextTurnPbm.h \
	NextTurnHotseat.cpp NextTurnHotseat.h \
	NextTurnNetworked.cpp NextTurnNetworked.h \
	FogMap.cpp FogMap.h \
	counter.cpp counter.h fight.cpp fight.h \
        CreateScenario.cpp CreateScenario.h \
        MapGenerator.cpp MapGenerator.h Configuration.cpp Configuration.h \
	sound.cpp sound.h GraphicsCache.cpp GraphicsCache.h \
	vectormap.cpp vectormap.h \
	heromap.cpp heromap.h \
	armymap.cpp armymap.h \
	citymap.cpp citymap.h \
	questmap.cpp questmap.h \
	ruinmap.cpp ruinmap.h \
	historymap.cpp historymap.h \
	bigmap.cpp bigmap.h  \
	history.cpp history.h  \
	signpost.cpp signpost.h signpostlist.cpp signpostlist.h \
        port.cpp port.h portlist.cpp portlist.h \
        bridge.cpp bridge.h bridgelist.cpp bridgelist.h \
	road.cpp road.h roadlist.cpp roadlist.h \
        overviewmap.cpp overviewmap.h \
        smallmap.cpp smallmap.h \
        gamebigmap.cpp gamebigmap.h \
        game.cpp game.h \
        timing.cpp timing.h \
        callback-enums.h \
        sidebar-stats.h game-parameters.h vector.h rectangle.h \
        ucompose.hpp defs.h LocationList.h \
        map-tip-position.h input-events.h sdl-draw.h  \
        vectoredunit.cpp vectoredunit.h \
        vectoredunitlist.cpp vectoredunitlist.h \
        AI_Diplomacy.cpp AI_Diplomacy.h \
	shield.cpp shield.h shieldset.cpp shieldset.h \
	shieldstyle.cpp shieldstyle.h \
	shieldsetlist.cpp shieldsetlist.h \
	citysetlist.cpp citysetlist.h cityset.cpp cityset.h \
	CreateScenarioRandomize.cpp CreateScenarioRandomize.h \
	Ownable.cpp Ownable.h Immovable.cpp Immovable.h \
	Movable.cpp Movable.h Positioned.cpp Positioned.h \
	Named.cpp Named.h Namable.cpp Namable.h Renamable.cpp Renamable.h \
	network_player.cpp network_player.h \
	herotemplates.cpp herotemplates.h \
	network-connection.cpp network-connection.h network-common.h \
	network-server.cpp network-server.h \
	game-server.cpp game-server.h \
	game-client.cpp game-client.h \
	GameScenarioOptions.cpp GameScenarioOptions.h \
	network-action.cpp network-action.h \
	network-history.cpp network-history.h pbm/pbm.cpp pbm/pbm.h \
	pbm-game-server.cpp pbm-game-server.h

if FL_RESRC
lordsawar_SOURCES += lordsawar.rc
endif
.rc.o:
	i586-mingw32msvc-windres -o $@ $<

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

lordsawar_LDFLAGS =
lordsawar_LDADD = $(SDL_LIBS) \
    $(GTKMM_LIBS) \
    $(LIBGNET_LIBS) \
    $(LIBSIGC_LIBS) \
    ./gui/liblwgui.a \
    @LIBINTL@
lordsawar_DEPENDENCIES = \
  ./gui/liblwgui.a

AM_CXXFLAGS = -Wall -DPO_PATH=\"$(prefix)/share/locale/\" -Wunused-function
INCLUDES = $(SDL_CFLAGS) \
    $(GTKMM_CFLAGS) \
    $(LIBGNET_CFLAGS) \
    $(LIBSIGC_CFLAGS) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' \
    -Iserver -I..
