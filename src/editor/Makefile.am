MAINTAINERCLEANFILES= Makefile.in

DEPENDSON = ../GameScenario.o ../army.o ../GameMap.o ../templelist.o ../ruinlist.o \
        ../hero.o ../citylist.o ../Location.o ../Object.o ../stacklist.o \
        ../File.o ../fight.o ../armysetlist.o ../temple.o ../ruin.o ../stack.o \
	../city.o ../path.o ../action.o ../Configuration.o ../MapGenerator.o \
	../ai_dummy.o ../ai_fast.o ../ai_smart.o ../xmlhelper.o ../counter.o ../CreateScenario.o \
	../maptile.o ../real_player.o ../PG_FileDialog.o ../player.o \
	../MapRenderer.o ../Tile.o ../TileSet.o ../playerlist.o \
	../GraphicsCache.o ../QuestsManager.o ../Quest.o ../QKillHero.o \
        ../QRuinSearch.o ../QEnemyArmies.o ../string_tokenizer.o \
        ../Item.o ../Itemlist.o ../Threat.o ../Threatlist.o ../AI_Analysis.o \
	../AI_Allocation.o ../MoveResult.o ../AICityInfo.o \
        ../MapCreationDialog.o ../FightDialog.o ../QuestExpiredDialog.o \
        ../QuestCompletedDialog.o ../MapConfDialog.o ../citiesreport.o \
        ../stackreport.o ../FogMap.o ../Popup.o ../signpostlist.o ../signpost.o\
	../stone.o ../stonelist.o ../sound.o ../road.o ../roadlist.o

if WANT_EDITOR
bin_PROGRAMS = lordsawar_editor

lordsawar_editor_SOURCES = main.cpp MainScreen.cpp NewMapDialog.cpp Bigmap.cpp \
                             Smallmap.cpp MapGenDialog.cpp TempleDialog.cpp \
                             ArmyDialog.cpp CityDialog.cpp StackDialog.cpp \
                             RuinDialog.cpp PlayerDialog.cpp FindDialog.cpp \
                             ConditionDialog.cpp ReactionDialog.cpp \
                             SelectionDialog.cpp EventDialog.cpp ScenarioDialog.cpp \
                             SignpostDialog.cpp 

if FL_RESRC
lordsawar_editor_SOURCES += editor.rc
endif
.rc.o:
	i586-mingw32msvc-windres -o $@ $<

lordsawar_editor_LDADD = $(PARAGUI_LIBS) \
	                   $(DEPENDSON) \
                           $(LIBSIGC_LIBS) \
                   ../events/liblwevents.a \
               @LIBINTL@ -L..

  lordsawar_editor_DEPENDENCIES = ../events/liblwevents.a
endif



localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@


AM_CXXFLAGS = -Wall -DPO_PATH=\"$(prefix)/share/locale/\"
INCLUDES = $(PARAGUI_CFLAGS) \
    $(LIBSIGC_CFLAGS) \
    -DFREELORDS_DATADIR='"$(FREELORDS_DATADIR)"' -I../..
