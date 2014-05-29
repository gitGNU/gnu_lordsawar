// Copyright (C) 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2004 John Farrell
// Copyright (C) 2004 Bryan Duff
// Copyright (C) 2006, 2007, 2008, 2009, 2014 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <fstream>
#include <algorithm>
#include <stdlib.h>

#include "real_player.h"
#include "action.h"
#include "history.h"
#include "playerlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "city.h"
#include "herotemplates.h"
#include "game.h"
#include "xmlhelper.h"
#include "GameScenarioOptions.h"
#include "SightMap.h"
#include "Sage.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

RealPlayer::RealPlayer(Glib::ustring name, guint32 armyset, Gdk::RGBA color, 
                       int width, int height, Player::Type type, int player_no)
    :Player(name, armyset, color, width, height, type, player_no),
    d_abort_requested(false)
{
}

RealPlayer::RealPlayer(const Player& player)
    :Player(player)
{
    d_type = HUMAN;
    d_abort_requested = false;
}

RealPlayer::RealPlayer(XML_Helper* helper)
    :Player(helper), d_abort_requested(false)
{
}

RealPlayer::~RealPlayer()
{
}

bool RealPlayer::save(XML_Helper* helper) const
{
    // This may seem a bit dumb, but allows derived players (especially
    // AI's) to save additional data, such as character types or so.
    bool retval = true;
    retval &= helper->openTag(Player::d_tag);
    retval &= Player::save(helper);
    retval &= helper->closeTag();

    return retval;
}

void RealPlayer::abortTurn()
{
  aborted_turn.emit();
}

bool RealPlayer::startTurn()
{
  return false;
}

void RealPlayer::endTurn()
{
  reportEndOfTurn();
  pruneActionlist();
}

void RealPlayer::invadeCity(City* c)
{
    // For the realplayer, this function doesn't do a lot. However, an AI
    // player has to decide here what to do (occupy, raze, pillage)
}

bool RealPlayer::chooseHero(HeroProto *hero, City* c, int gold)
{
    // For the realplayer, this function doesn't do a lot. However, an AI
    // player has to decide here what to do (accept/deny hero)
    return false;
}

Reward *RealPlayer::chooseReward(Ruin *ruin, Sage *sage, Stack *stack)
{
    // For the realplayer, this function doesn't do a lot. However, an AI
    // player has to decide here what to do (pick a reward from sage)
    return NULL;
}

bool RealPlayer::chooseTreachery (Stack *stack, Player *player, Vector <int> pos)
{
    // For the realplayer, this function doesn't do a lot. However, an AI
    // player has to decide here what to do (fight a friend or not)
  return true;
}

Army::Stat RealPlayer::chooseStat(Hero *hero)
{
    // For the realplayer, this function doesn't do a lot. However, an AI
    // player has to decide here what to do (pick strength/moves/sight stat)
  return Army::STRENGTH;
}

bool RealPlayer::chooseQuest(Hero *hero)
{
  //we decide interactively with the gui, not by this method.
  // For the realplayer, this function doesn't do a lot. However, an AI
  // player has to decide here what to do (get a quest for the hero or not)
  return true;
}

void RealPlayer::heroGainsLevel(Hero* a)
{
    // the standard human player just asks the GUI what to do
    Army::Stat stat = sheroGainsLevel.emit(a);
    doHeroGainsLevel(a, stat);

    Action_Level* item = new Action_Level();
    item->fillData(a, stat);
    addAction(item);
}

bool RealPlayer::computerChooseVisitRuin(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  //this decision callback is only for computer players
  return true;
}

bool RealPlayer::computerChoosePickupBag(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  //this decision callback is only for computer players
  return true;
}

bool RealPlayer::computerChooseVisitTempleForBlessing(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  //this decision callback is only for computer players
  return true;
}

bool RealPlayer::computerChooseVisitTempleForQuest(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  //this decision callback is only for computer players
  return true;
}

bool RealPlayer::computerChooseContinueQuest(Stack *stack, Quest *quest, Vector<int> dest, guint32 moves, guint32 turns)
{
  //this decision callback is only for computer players
  return true;
}
// End of file
