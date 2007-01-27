MAINTAINERCLEANFILES= Makefile.in

noinst_LIBRARIES = libflevents.a

libflevents_a_SOURCES = \
	Condition.cpp CPlayer.cpp CCounter.cpp CLiving.cpp CDead.cpp CArmy.cpp \
	Reaction.cpp RMessage.cpp RAddGold.cpp RAddUnit.cpp RDelUnit.cpp \
	RUpdate.cpp RCenter.cpp RCenterObj.cpp RCreateItem.cpp \
	RWinGame.cpp RLoseGame.cpp RRaiseEvent.cpp RActEvent.cpp \
	RRevivePlayer.cpp RKillPlayer.cpp RTransferCity.cpp \
	Event.cpp EKillAll.cpp EPlayerDead.cpp ECityConq.cpp EArmyKilled.cpp \
	ERound.cpp ERuinSearch.cpp ETempleSearch.cpp EDummy.cpp ENextTurn.cpp \
	EStackKilled.cpp EStackMove.cpp

INCLUDES = \
    $(LIBSIGC_CFLAGS) \
    $(PARAGUI_CFLAGS)

AM_CXXFLAGS = -Wall
