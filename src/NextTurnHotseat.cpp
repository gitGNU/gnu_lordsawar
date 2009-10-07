// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include "NextTurnHotseat.h"

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
#include "ai_fast.h" //remove me

#include "path.h"

using namespace std;
#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

NextTurnHotseat::NextTurnHotseat(bool turnmode, bool random_turns)
    :NextTurn(turnmode, random_turns)
{
  continuing_turn = false;
  
  Playerlist* plist = Playerlist::getInstance();
  for (Playerlist::iterator i = plist->begin(); i != plist->end(); ++i) {
    Player *p = *i;
    p->ending_turn.connect(sigc::mem_fun(this, &NextTurn::endTurn));
  }
}

NextTurnHotseat::~NextTurnHotseat()
{
}

void NextTurnHotseat::start()
{
    //We need the playerlist a lot, so maintain a copy of it.

    //set first player as active if no active player exists
    if (!Playerlist::getActiveplayer())
      nextPlayer();
	
    while (!d_stop)
    {
      supdating.emit();

        // do various start-up tasks
        if (continuing_turn)
	  {
	    continuing_turn = false;
	    return;
	  }

	startTurn();
       
	// inform everyone about the next turn 
	snextTurn.emit(Playerlist::getActiveplayer());
    
	if (Playerlist::getInstance()->getNoOfPlayers() <= 2)
	  {
	    abort = srequestAbort.connect(sigc::mem_fun(Playerlist::getInstance()->getFirstLiving(), &Player::abortTurn));
	    if (Playerlist::getInstance()->checkPlayers()) //end of game detected
	      return;
	  }


	splayerStart.emit(Playerlist::getActiveplayer());

	// let the player do his or her duties...
	bool continue_loop = Playerlist::getActiveplayer()->startTurn();
	if (!continue_loop || d_stop)
	  return;
	
	//Now do some cleanup at the end of the turn.
	finishTurn();

        //...and initiate the next one.
	nextPlayer();
        
        //if it is the first player's turn now, a new round has started
        if (Playerlist::getInstance()->getActiveplayer() == 
	    Playerlist::getInstance()->getFirstLiving())
        
	  {
	    if (Playerlist::getInstance()->checkPlayers() == true)
	      {
		if (Playerlist::getInstance()->getNoOfPlayers() <= 1)
		  break;
		if (Playerlist::getActiveplayer()->isDead())
		  nextPlayer();
	      }
	    finishRound();
	    snextRound.emit();
	  }
    }
}

void NextTurnHotseat::endTurn()
{
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
  nextPlayer();

  //this is problematic when the first player dies.
  //end the turn if all living players have ended their turn.
  if (Playerlist::getInstance()->isEndOfRound() == true)
    {
      finishRound();
      snextRound.emit();
    }

  start();
}

void NextTurnHotseat::startTurn()
{
  //this function is called before a player starts his turn. Some
  //items you could imagine to be placed here: healing/building
  //units, check for joining heroes...

  //a shortcut
  Player* p = Playerlist::getActiveplayer();

  p->initTurn();

  //calculate upkeep and income
  p->calculateUpkeep();
  p->calculateIncome();

  //if turnmode is set, create/heal armies at player's turn
  if (d_turnmode)
    {

      //if (p->getType() != Player::NETWORKED)
      
      //collect taxes
      Citylist::getInstance()->collectTaxes(p);

      guint32 num_cities = Citylist::getInstance()->countCities(p);
      p->getStacklist()->collectTaxes(p, num_cities);

      //vector armies (needs to preceed city's next turn)
      VectoredUnitlist::getInstance()->nextTurn(p);

      //pay upkeep for existing stacks
      p->getStacklist()->payUpkeep(p);

      //reset moves, and heal stacks
      p->getStacklist()->nextTurn();

      //build new armies
      Citylist::getInstance()->nextTurn(p);

    }
  p->calculateUpkeep();

  QuestsManager::getInstance()->nextTurn(p);
}

void NextTurnHotseat::finishTurn()
{
  //Put everything that has to be done before the next player starts
  //his turn here. E.g. one could clear some caches.
  Player *p = Playerlist::getActiveplayer();
  p->getFogMap()->smooth();
  p->endTurn();
}

void NextTurnHotseat::finishRound()
{
  Playerlist *plist = Playerlist::getInstance();
  //Put everything that has to be done when a new round starts in here.
  //E.g. increase the round number in GameScenario. (this is done with
  //the snextRound signal, but useful for an example).

  if (!d_turnmode)
    {
      //do this for all players at once
      
      for (Playerlist::iterator it = plist->begin(); it != plist->end(); it++)
	{
	  if ((*it)->isDead())
	    continue;

	  //collect monies from cities
	  Citylist::getInstance()->collectTaxes(*it);

	  guint32 num_cities = Citylist::getInstance()->countCities(*it);
	  (*it)->getStacklist()->collectTaxes((*it), num_cities);

	  //vector armies (needs to preceed city's next turn)
	  VectoredUnitlist::getInstance()->nextTurn(*it);

	  //pay for existing armies
	  (*it)->getStacklist()->payUpkeep(*it);

	  //reset, and heal armies
	  (*it)->getStacklist()->nextTurn();

	  //produce new armies
	  Citylist::getInstance()->nextTurn(*it);

	}
    }

  // heal the stacks in the ruins
  Ruinlist* rl = Ruinlist::getInstance();
  for (Ruinlist::iterator it = rl->begin(); it != rl->end(); it++)
    {
      Stack* keeper = (*it)->getOccupant();
      if (keeper)
	keeper->nextTurn();
    }
    
  if (d_random_turns)
    {
      plist->randomizeOrder();
      nextPlayer();
    }

}
