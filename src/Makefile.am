#   Copyright (C) 2008 Barry deFreese
#   Copyright (C) 2009 Ben Asselstine
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
noinst_LTLIBRARIES = liblordsawar.la liblordsawarnet.la liblordsawargfx.la

liblordsawar_la_SOURCES = action.cpp action.h AI_Analysis.cpp AI_Analysis.h \
        AI_Allocation.cpp AI_Allocation.h AI_Diplomacy.cpp AI_Diplomacy.h \
        ai_dummy.cpp ai_dummy.h ai_fast.cpp ai_fast.h \
        ai_smart.cpp ai_smart.h AICityInfo.cpp AICityInfo.h \
	armybase.cpp armybase.h armyproto.cpp armyproto.h armyprodbase.cpp \
        armyprodbase.h army.cpp army.h armysetlist.cpp armysetlist.h \
        armyset.cpp armyset.h armyprotobase.cpp armyprotobase.h \
        bridge.cpp bridge.h bridgelist.cpp bridgelist.h \
        city.cpp city.h citylist.cpp citylist.h \
        citysetlist.cpp citysetlist.h cityset.cpp cityset.h \
        Configuration.cpp Configuration.h counter.cpp counter.h \
        CreateScenario.cpp CreateScenario.h \
	CreateScenarioRandomize.cpp CreateScenarioRandomize.h \
	fight.cpp fight.h File.cpp File.h FogMap.cpp FogMap.h \
        GameMap.cpp GameMap.h GameScenario.cpp GameScenario.h \
	GameScenarioOptions.cpp GameScenarioOptions.h \
	hero.cpp hero.h heroproto.cpp heroproto.h \
        herotemplates.cpp herotemplates.h history.cpp history.h \
        Immovable.cpp Immovable.h Item.cpp Item.h \
	ItemProto.cpp ItemProto.h \
        Itemlist.cpp Itemlist.h Location.cpp Location.h \
	LocationBox.cpp LocationBox.h LocationList.h \
        MapGenerator.cpp MapGenerator.h \
        maptile.cpp maptile.h Movable.cpp Movable.h \
        MoveResult.cpp MoveResult.h Named.cpp Named.h Namable.cpp Namable.h \
        NamedLocation.cpp NamedLocation.h network_player.cpp network_player.h \
	network-action.cpp network-action.h \
        network-history.cpp network-history.h \
        NextTurn.cpp NextTurn.h NextTurnHotseat.cpp NextTurnHotseat.h \
	NextTurnNetworked.cpp NextTurnNetworked.h \
        NextTurnPbm.cpp NextTurnPbm.h \
        Ownable.cpp Ownable.h path.cpp path.h \
        player.cpp player.h playerlist.cpp playerlist.h \
        port.cpp port.h portlist.cpp portlist.h \
        Positioned.cpp Positioned.h QCityOccupy.cpp QCityOccupy.h \
        QCityRaze.cpp QCityRaze.h QCitySack.cpp QCitySack.h \
        QEnemyArmies.cpp QEnemyArmies.h QEnemyArmytype.cpp QEnemyArmytype.h \
	QKillHero.cpp QKillHero.h QPillageGold.cpp QPillageGold.h \
        Quest.cpp Quest.h QuestsManager.cpp QuestsManager.h \
        real_player.cpp real_player.h Renamable.cpp Renamable.h \
        reward.h reward.cpp rewardlist.h rewardlist.cpp \
	road.cpp road.h roadlist.cpp roadlist.h \
        ruin.cpp ruin.h ruinlist.cpp ruinlist.h \
        shield.cpp shield.h shieldset.cpp shieldset.h rgb_shift.h \
        shieldsetlist.cpp shieldsetlist.h shieldstyle.cpp shieldstyle.h \
        signpost.cpp signpost.h signpostlist.cpp signpostlist.h \
	stack.cpp stack.h \
        stacklist.cpp stacklist.h string_tokenizer.cpp string_tokenizer.h \
        temple.cpp temple.h templelist.cpp templelist.h \
        Threat.cpp Threat.h Threatlist.cpp Threatlist.h \
        Tile.cpp Tile.h tileset.cpp tileset.h tilesetlist.cpp tilesetlist.h \
        tilestyle.cpp tilestyle.h tilestyleset.cpp tilestyleset.h \
        timing.cpp timing.h UniquelyIdentified.cpp UniquelyIdentified.h \
        vectoredunit.cpp vectoredunit.h \
        vectoredunitlist.cpp vectoredunitlist.h xmlhelper.cpp xmlhelper.h \
        rectangle.h vector.h ucompose.hpp \
	SightMap.cpp SightMap.h SmallTile.cpp SmallTile.h \
	Triumphs.cpp Triumphs.h Backpack.cpp Backpack.h \
	MapBackpack.cpp MapBackpack.h prodslot.cpp prodslot.h \
	prodslotlist.cpp prodslotlist.h namelist.cpp namelist.h

liblordsawarnet_la_SOURCES = \
	game-client.cpp game-client.h \
        game-client-decoder.cpp game-client-decoder.h \
	game-server.cpp game-server.h game-station.cpp game-station.h \
	network-connection.cpp chat-client.cpp chat-client.h \
        network-connection.h network-common.h \
        network-server.cpp network-server.h \
        pbm/pbm.cpp pbm/pbm.h \
        pbm-game-client.cpp pbm-game-client.h \
        pbm-game-server.cpp pbm-game-server.h

liblordsawargfx_la_SOURCES = \
	bigmap.cpp bigmap.h citymap.cpp citymap.h  \
        historymap.cpp historymap.h heromap.cpp heromap.h \
	heroesmap.cpp heroesmap.h \
	ruinmap.cpp ruinmap.h questmap.cpp questmap.h \
	vectormap.cpp vectormap.h overviewmap.cpp overviewmap.h \
        smallmap.cpp smallmap.h \
	MapRenderer.cpp MapRenderer.h  \
	GraphicsCache.cpp GraphicsCache.h \
	GraphicsLoader.cpp GraphicsLoader.h \
	input-events.h map-tip-position.h sdl-draw.h 


lordsawar_SOURCES = armymap.cpp armymap.h \
	callback-enums.h defs.h game-parameters.h \
        recently-played-game.cpp recently-played-game.h \
        recently-played-game-list.cpp recently-played-game-list.h \
	sidebar-stats.h game.cpp game.h gamebigmap.cpp gamebigmap.h \
	sound.cpp sound.h main.cpp 


if FL_RESRC
lordsawar_SOURCES += lordsawar.rc
endif
.rc.o:
	i586-mingw32msvc-windres -o $@ $<

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

liblordsawar_la_LIBADD = gui/liblwgui.la \
    @LTLIBOBJS@

lordsawar_LDFLAGS =
lordsawar_LDADD = liblordsawar.la \
    liblordsawarnet.la \
    liblordsawargfx.la \
    $(SDLSOUND_LIBS) \
    $(GTKMM_LIBS) \
    $(LIBGNET_LIBS) \
    $(LIBSIGC_LIBS) \
    gui/liblwgui.la \
    @LIBINTL@

lordsawar_DEPENDENCIES = \
  liblordsawar.la \
  liblordsawargfx.la \
  liblordsawarnet.la \
  gui/liblwgui.la

AM_CXXFLAGS = -Wall -DPO_PATH=\"$(prefix)/share/locale/\" -Wunused-function
INCLUDES = $(SDL_CFLAGS) \
    $(GTKMM_CFLAGS) \
    $(LIBGNET_CFLAGS) \
    $(LIBSIGC_CFLAGS) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' \
    -I$(top_srcdir)
