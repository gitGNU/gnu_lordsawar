MAINTAINERCLEANFILES= Makefile.in

DEPENDSON = ../GameScenario.o ../army.o ../GameMap.o ../templelist.o ../ruinlist.o \
        ../hero.o ../citylist.o ../Location.o ../Object.o ../stacklist.o \
        ../File.o ../fight.o ../armysetlist.o ../temple.o ../ruin.o ../stack.o \
	../city.o ../path.o ../action.o ../Configuration.o ../MapGenerator.o \
	../ai_dummy.o ../ai_fast.o ../ai_smart.o ../xmlhelper.o ../counter.o \
        ../CreateScenario.o \
	../maptile.o ../real_player.o  ../player.o \
	../MapRenderer.o ../Tile.o ../TileSet.o ../playerlist.o \
	../GraphicsCache.o ../QuestsManager.o ../Quest.o ../QKillHero.o \
        ../QEnemyArmies.o ../QCityOccupy.o \
        ../QCitySack.o ../QCityRaze.o ../QPillageGold.o ../QEnemyArmytype.o \
        ../string_tokenizer.o ../reward.o \
        ../Item.o ../Itemlist.o ../Threat.o ../Threatlist.o ../AI_Analysis.o \
	../AI_Allocation.o ../MoveResult.o ../AICityInfo.o \
        ../FogMap.o ../signpostlist.o ../signpost.o\
	../stone.o ../stonelist.o ../sound.o ../road.o ../roadlist.o \
        ../vectoredunitlist.o ../vectoredunit.o \
        ../bigmap.o ../smallmap.o ../overviewmap.o ../timing.o \
        ../gui/image-helpers.o ../gui/error-utils.o ../gui/gtksdl.o \
        ../gui/army-info-tip.o


if WANT_EDITOR
bin_PROGRAMS = lordsawar-editor

lordsawar_editor_SOURCES = \
main.cpp \
main-window.cpp main-window.h \
signpost-dialog.cpp signpost-dialog.h \
temple-dialog.cpp temple-dialog.h \
ruin-dialog.cpp ruin-dialog.h \
stack-dialog.cpp stack-dialog.h \
select-army-dialog.cpp select-army-dialog.h \
players-dialog.cpp players-dialog.h \
editorbigmap.cpp editorbigmap.h \
glade-helpers.h

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
