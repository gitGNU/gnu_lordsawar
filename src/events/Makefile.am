MAINTAINERCLEANFILES= Makefile.in

noinst_LIBRARIES = liblwevents.a

liblwevents_a_SOURCES = \
	Condition.cpp Condition.h CPlayer.cpp CPlayer.h \
        CCounter.cpp CCounter.h CLiving.cpp CLiving.h CDead.cpp CDead.h \
        CArmy.cpp CArmy.h Reaction.cpp Reaction.h RMessage.cpp RMessage.h \
        RAddGold.cpp RAddGold.h RAddUnit.cpp RAddUnit.h \
        RDelUnit.cpp RDelUnit.h RUpdate.cpp RUpdate.h RCenter.cpp RCenter.h \
        RCenterObj.cpp RCenterObj.h RCreateItem.cpp RCreateItem.h \
	RWinGame.cpp RWinGame.h RLoseGame.cpp RLoseGame.h \
        RRaiseEvent.cpp RRaiseEvent.h RActEvent.cpp RActEvent.h \
	RRevivePlayer.cpp RRevivePlayer.h RKillPlayer.cpp RKillPlayer.h \
        RTransferCity.cpp RTransferCity.h Event.cpp Event.h \
        EKillAll.cpp EKillAll.h EPlayerDead.cpp EPlayerDead.h \
        ECityConq.cpp ECityConq.h EArmyKilled.cpp EArmyKilled.h \
	ERound.cpp ERound.h ERuinSearch.cpp ERuinSearch.h \
        ETempleSearch.cpp ETempleSearch.h EDummy.cpp EDummy.h \
        ENextTurn.cpp ENextTurn.h EStackKilled.cpp EStackKilled.h \
        EStackMove.cpp EStackMove.h


INCLUDES = \
    $(LIBSIGC_CFLAGS) \
    $(SDL_CFLAGS)

AM_CXXFLAGS = -Wall
