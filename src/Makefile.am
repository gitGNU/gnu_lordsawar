SUBDIRS = server events . editor utils
MAINTAINERCLEANFILES= Makefile.in

bin_PROGRAMS = freelords

freelords_SOURCES = army.cpp hero.cpp stack.cpp GameScenario.cpp action.cpp \
	player.cpp real_player.cpp ai_dummy.cpp ai_fast.cpp ai_smart.cpp \
	Object.cpp Location.cpp city.cpp temple.cpp ruin.cpp Tile.cpp \
	maptile.cpp Item.cpp armysetlist.cpp stacklist.cpp playerlist.cpp \
	citylist.cpp templelist.cpp ruinlist.cpp GameMap.cpp \
	Itemlist.cpp TileSet.cpp Threat.cpp Threatlist.cpp MoveResult.cpp \
	AICityInfo.cpp AI_Analysis.cpp AI_Allocation.cpp path.cpp QuestsManager.cpp \
	Quest.cpp QKillHero.cpp QRuinSearch.cpp QEnemyArmies.cpp File.cpp main.cpp \
	xmlhelper.cpp string_tokenizer.cpp NextTurn.cpp FogMap.cpp \
	counter.cpp fight.cpp CreateScenario.cpp MapGenerator.cpp Configuration.cpp \
	sound.cpp GraphicsCache.cpp PG_FileDialog.cpp Popup.cpp scroller.cpp tooltip.cpp MapRenderer.cpp \
	stackreport.cpp citiesreport.cpp questsreport.cpp goldreport.cpp \
	overviewmap.cpp smallmap.cpp vectormap.cpp bigmap.cpp ruininfo.cpp TempleInfo.cpp \
	about.cpp OptionsDialog.cpp GamePreferencesDialog.cpp player_preferences.cpp \
	hero_offer.cpp ArmyInfo.cpp stackinfo.cpp cityinfo.cpp CityOccupationDialog.cpp \
	RuinSearchDialog.cpp FightDialog.cpp MapCreationDialog.cpp MapConfDialog.cpp \
	d_buy_production.cpp ServerDialog.cpp MultiPlayerModeDialog.cpp ArmyDialog.cpp \
	QuestCompletedDialog.cpp ArmyLevelDialog.cpp ItemDialog.cpp \
	ArmyMedalDialog.cpp QuestExpiredDialog.cpp LangDialog.cpp splash.cpp w_edit.cpp \
	MainWindow.cpp netggz.cpp VectorDialog.cpp ScenariosDialog.cpp \
	signpost.cpp signpostlist.cpp SignpostInfo.cpp stone.cpp stonelist.cpp \
	road.cpp roadlist.cpp

if FL_RESRC
freelords_SOURCES += freelords.rc
endif
.rc.o:
	i586-mingw32msvc-windres -o $@ $<

localedir = $(datadir)/locale
DEFS = -DFL_VERSION=\"@VERSION@\"  -DLOCALEDIR=\"$(localedir)\" @DEFS@

freelords_LDFLAGS = $(GGZMOD_LDFLAGS) $(GGZDMOD_LDFLAGS)
freelords_LDADD = $(PARAGUI_LIBS) \
    $(LIBSIGC_LIBS) \
    $(LIB_GGZMOD) \
    $(LIB_GGZDMOD) \
    ./server/libflserver.a \
	./events/libflevents.a \
    @LIBINTL@
freelords_DEPENDENCIES = ./server/libflserver.a ./events/libflevents.a

AM_CXXFLAGS = -Wall -DPO_PATH=\"$(prefix)/share/locale/\"
INCLUDES = $(PARAGUI_CFLAGS) \
    $(LIBSIGC_CFLAGS) \
    $(LIBGGZ_INCLUDES) \
    $(GGZDMOD_INCLUDES) \
    -DFREELORDS_DATADIR='"$(FREELORDS_DATADIR)"' \
    -Iserver -I..
