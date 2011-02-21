// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2011 Ben Asselstine
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

#include "NextTurnNetworked.h"

#include "playerlist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "stacklist.h"
#include "armysetlist.h"
#include "hero.h"
#include "vectoredunitlist.h"
#include "FogMap.h"
#include "history.h"
#include "QuestsManager.h"
#include "network_player.h"
#include "game-server.h"
#include "game-client.h"
#include "GameScenarioOptions.h"

#include "path.h"

using namespace std;
#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

NextTurnNetworked::NextTurnNetworked(bool turnmode, bool random_turns)
    :NextTurn(turnmode, random_turns)
{
  Playerlist* plist = Playerlist::getInstance();
  for (Playerlist::iterator i = plist->begin(); i != plist->end(); ++i) 
    {
      Player *p = *i;
      if (p->getType() != Player::NETWORKED)
        p->ending_turn.connect(sigc::mem_fun(this, &NextTurn::endTurn));
    }
}

NextTurnNetworked::~NextTurnNetworked()
{
}

Player* NextTurnNetworked::next()
{
  nextPlayer();
  Player *p = Playerlist::getActiveplayer();
  return p;
}

void NextTurnNetworked::start_player(Player *player)
{
  if (d_stop)
    return;
      
  abort.disconnect();
  abort = srequestAbort.connect(sigc::mem_fun(player, &Player::abortTurn));
  Playerlist::getInstance()->setActiveplayer(player);
  if (player->getType() == Player::NETWORKED)
    {
      splayerStart.emit(player);
      return;
    }
  start();
}

void NextTurnNetworked::start()
{
  Playerlist* plist = Playerlist::getInstance();
  supdating.emit();

  startTurn();

  // inform everyone about the next turn 
  snextTurn.emit(plist->getActiveplayer());

  if (plist->getNoOfPlayers() <= 2)
    {
      if (plist->checkPlayers()) //end of game detected
        return;
    }

  splayerStart.emit(plist->getActiveplayer());

  // let the player do his or her duties...
  bool continue_loop = plist->getActiveplayer()->startTurn();
  if (!continue_loop)
    return;

  //Now do some cleanup at the end of the turn.
  if (d_stop == true)
    return;
  finishTurn();

}

void NextTurnNetworked::endTurn()
{
  std::string old_name = Playerlist::getActiveplayer()->getName();
  // Finish off the player and transfers the control to the start function
  // again.
  finishTurn();
  if (Playerlist::getInstance()->checkPlayers() == true)
    {
      if (d_stop)
	return;
      if (Playerlist::getInstance()->getNoOfPlayers() <= 1)
	return;
    }
}

void NextTurnNetworked::startTurn()
{
  //this function is called before a player starts his turn. Some
  //items you could imagine to be placed here: healing/building
  //units, check for joining heroes...

  //a shortcut
  Player* p = Playerlist::getActiveplayer();

  //here we check to see if the player is about to start a new turn
  //we do this check to see if we're re-sitting down again in the game lobby
  //we want to prevent offering the player another hero, etc.
  if (p->hasAlreadyInitializedTurn() && p->hasAlreadyEndedTurn() == false)
    return;
  p->initTurn();

  //calculate upkeep and income
  p->calculateUpkeep();
  p->calculateIncome();

  //if turnmode is set, create/heal armies at player's turn
  if (d_turnmode)
    {

      p->collectTaxesAndPayUpkeep();

      //reset moves, and heal stacks
      p->stacksReset();

      //vector armies (needs to preceed city's next turn)
      VectoredUnitlist::getInstance()->nextTurn(p);

      //build new armies
      Citylist::getInstance()->nextTurn(p);

    }
  p->calculateUpkeep();

  QuestsManager::getInstance()->nextTurn(p);
}

void NextTurnNetworked::finishTurn()
{
  //Put everything that has to be done before the next player starts
  //his turn here. E.g. one could clear some caches.
  Player *p = Playerlist::getActiveplayer();
  if (p)
    {
      p->getFogMap()->smooth();
      p->endTurn();
    }
}

void NextTurnNetworked::finishRound()
{
  Playerlist *plist = Playerlist::getInstance();
  //Put everything that has to be done when a new round starts in here.
  //E.g. increase the round number in GameScenario. (this is done with
  //the snextRound signal, but useful for an example).

  if (Playerlist::getInstance()->checkPlayers() == true)
    {
      if (d_stop)
        return;
      if (Playerlist::getInstance()->getNoOfPlayers() <= 1)
        return;
    }

  printf("d_turnmode is %d\n", d_turnmode);
  if (!d_turnmode)
    {
      //do this for all players at once

      for (Playerlist::iterator it = plist->begin(); it != plist->end(); it++)
        {
          if ((*it)->isDead())
            continue;

          (*it)->collectTaxesAndPayUpkeep();

          printf("healing armies for %s\n", (*it)->getName().c_str());
          //reset, and heal armies
          (*it)->stacksReset();

          //vector armies (needs to preceed city's next turn)
          VectoredUnitlist::getInstance()->nextTurn(*it);

          //produce new armies
          Citylist::getInstance()->nextTurn(*it);

        }
    }

  // heal the stacks in the ruins
  Playerlist::getInstance()->getNeutral()->ruinsReset();

  if (d_random_turns)
    {
      plist->randomizeOrder();
      nextPlayer();
    }

}
