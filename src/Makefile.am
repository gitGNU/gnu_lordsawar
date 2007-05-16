SUBDIRS = server events gui . utils
MAINTAINERCLEANFILES= Makefile.in

bin_PROGRAMS = lordsawar

lordsawar_SOURCES = army.cpp army.h hero.cpp hero.h stack.cpp stack.h \
        GameScenario.cpp GameScenario.h action.cpp action.h \
	player.cpp player.h real_player.cpp real_player.h ai_dummy.cpp \
        ai_dummy.h ai_fast.cpp ai_fast.h ai_smart.cpp ai_smart.h \
	Object.cpp Object.h Location.cpp Location.h city.cpp city.h \
        temple.cpp temple.h ruin.cpp ruin.h Tile.cpp Tile.h \
	maptile.cpp maptile.h Item.cpp Item.h armysetlist.cpp armysetlist.h \
        stacklist.cpp stacklist.h playerlist.cpp playerlist.h \
	citylist.cpp citylist.h templelist.cpp templelist.h \
        ruinlist.cpp ruinlist.h GameMap.cpp GameMap.h \
	Itemlist.cpp Itemlist.h TileSet.cpp TileSet.h Threat.cpp Threat.h \
        Threatlist.cpp Threatlist.h MoveResult.cpp MoveResult.h \
	AICityInfo.cpp AICityInfo.h AI_Analysis.cpp AI_Analysis.h \
        AI_Allocation.cpp AI_Allocation.h path.cpp path.h \
        QuestsManager.cpp QuestsManager.h Quest.cpp Quest.h \
	QKillHero.cpp QKillHero.h QRuinSearch.cpp QRuinSearch.h \
        QEnemyArmies.cpp QEnemyArmies.h QCitySack.cpp QCitySack.h \
        QCityRaze.cpp QCityRaze.h File.cpp File.h main.cpp \
	xmlhelper.cpp xmlhelper.h string_tokenizer.cpp string_tokenizer.h \
        NextTurn.cpp NextTurn.h FogMap.cpp FogMap.h \
	counter.cpp counter.h fight.cpp fight.h \
        CreateScenario.cpp CreateScenario.h \
        MapGenerator.cpp MapGenerator.h Configuration.cpp Configuration.h \
	sound.cpp sound.h GraphicsCache.cpp GraphicsCache.h \
        MapRenderer.cpp MapRenderer.h \
	vectormap.cpp vectormap.h \
	bigmap.cpp bigmap.h  \
	netggz.cpp netggz.h \
	signpost.cpp signpost.h signpostlist.cpp signpostlist.h \
        stone.cpp stone.h stonelist.cpp stonelist.h \
	road.cpp road.h roadlist.cpp roadlist.h \
        overviewmap.cpp overviewmap.h \
        smallmap.cpp smallmap.h \
        game.cpp game.h \
        timing.cpp timing.h \
        callback-enums.h \
        sidebar-stats.h game-parameters.h vector.h rectangle.h \
        tmp.cpp tmp.h ucompose.hpp defs.h ObjectList.h \
        map-tip-position.h input-events.h sdl-draw.h   


# old: ArmyLevelDialog.cpp ItemDialog.cpp PG_FileDialog.cpp Popup.cpp
# splash.cpp w_edit.cpp TempleInfo.cpp QuestCompletedDialog.cpp
# ArmyMedalDialog.cpp VectorDialog.cpp ScenariosDialog.cpp
# MainWindow.cpp QuestExpiredDialog.cpp LangDialog.cpp
# CityOccupationDialog.cpp RuinSearchDialog.cpp FightDialog.cpp
# MapCreationDialog.cpp MapConfDialog.cpp ServerDialog.cpp
# MultiPlayerModeDialog.cpp ArmyDialog.cpp OptionsDialog.cpp
# GamePreferencesDialog.cpp d_buy_production.cpp
# stackreport.cpp citiesreport.cpp questsreport.cpp goldreport.cpp
# hero_offer.cpp ArmyInfo.cpp stackinfo.cpp cityinfo.cpp
# about.cpp scroller.cpp ruininfo.cpp SignpostInfo.cpp

if FL_RESRC
lordsawar_SOURCES += lordsawar.rc
endif
.rc.o:
	i586-mingw32msvc-windres -o $@ $<

localedir = $(datadir)/locale
DEFS = -DFL_VERSION=\"@VERSION@\"  -DLOCALEDIR=\"$(localedir)\" @DEFS@

lordsawar_LDFLAGS = $(GGZMOD_LDFLAGS) $(GGZDMOD_LDFLAGS)
lordsawar_LDADD = $(SDL_LIBS) \
    $(GTKMM_LIBS) \
    $(LIBSIGC_LIBS) \
    $(LIB_GGZMOD) \
    $(LIB_GGZDMOD) \
    ./server/liblwserver.a \
    ./events/liblwevents.a \
    ./gui/liblwgui.a \
    @LIBINTL@
lordsawar_DEPENDENCIES = \
  ./server/liblwserver.a ./events/liblwevents.a  ./gui/liblwgui.a

AM_CXXFLAGS = -Wall -DPO_PATH=\"$(prefix)/share/locale/\"
INCLUDES = $(SDL_CFLAGS) \
    $(GTKMM_CFLAGS) \
    $(LIBSIGC_CFLAGS) \
    $(LIBGGZ_INCLUDES) \
    $(GGZDMOD_INCLUDES) \
    -DLORDSAWAR_DATADIR='"$(LORDSAWAR_DATADIR)"' \
    -Iserver -I..
