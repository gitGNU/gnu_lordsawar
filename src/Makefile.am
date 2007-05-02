SUBDIRS = server events gui . utils
MAINTAINERCLEANFILES= Makefile.in

bin_PROGRAMS = lordsawar

lordsawar_SOURCES = army.cpp hero.cpp stack.cpp GameScenario.cpp action.cpp \
	player.cpp real_player.cpp ai_dummy.cpp ai_fast.cpp ai_smart.cpp \
	Object.cpp Location.cpp city.cpp temple.cpp ruin.cpp Tile.cpp \
	maptile.cpp Item.cpp armysetlist.cpp stacklist.cpp playerlist.cpp \
	citylist.cpp templelist.cpp ruinlist.cpp GameMap.cpp \
	Itemlist.cpp TileSet.cpp Threat.cpp Threatlist.cpp MoveResult.cpp \
	AICityInfo.cpp AI_Analysis.cpp AI_Allocation.cpp path.cpp QuestsManager.cpp \
	Quest.cpp QKillHero.cpp QRuinSearch.cpp QEnemyArmies.cpp File.cpp main.cpp \
	xmlhelper.cpp string_tokenizer.cpp NextTurn.cpp FogMap.cpp \
	counter.cpp fight.cpp CreateScenario.cpp MapGenerator.cpp Configuration.cpp \
	sound.cpp GraphicsCache.cpp  MapRenderer.cpp \
	 \
	bigmap.cpp   \
	netggz.cpp  \
	signpost.cpp signpostlist.cpp  stone.cpp stonelist.cpp \
	road.cpp roadlist.cpp \
        overviewmap.cpp overviewmap.h \
        smallmap.cpp smallmap.h \
        game.cpp game.h \
        timing.cpp timing.h \
        callback-enums.h \
        sidebar-stats.h game-parameters.h vector.h rectangle.h \
        tmp.cpp tmp.h 

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
