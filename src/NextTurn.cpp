//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "NextTurn.h"

#include "playerlist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "stacklist.h"
#include "armysetlist.h"
#include "hero.h"
#include "vectoredunitlist.h"
#include "FogMap.h"
#include "history.h"

#include "path.h"

using namespace std;
#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

NextTurn::NextTurn(bool turnmode)
    :d_turnmode(turnmode), d_stop(false)
{
}

NextTurn::~NextTurn()
{
}

void NextTurn::start()
{
    //We need the playerlist a lot, so maintain a copy of it.
    Playerlist* plist = Playerlist::getInstance();

    //set first player as active if no active player exists
    if (!plist->getActiveplayer())
        plist->nextPlayer();
	
    plist->calculateDiplomaticRankings();

    while (!d_stop)
    {
        // do various start-up tasks
        startTurn();
        supdating.emit();
       
        // inform everyone about the next turn 
        snextTurn.emit(plist->getActiveplayer());
        
	if (plist->getNoOfPlayers() <= 2)
	{
          if (plist->checkPlayers() == true)
	    {
	      if (plist->getNoOfPlayers() <= 1)
		return;
	    }
	}

	bool break_loop = splayerStart.emit(plist->getActiveplayer());

        //Let the player do his duties...
        plist->getActiveplayer()->startTurn();

        if (break_loop)
            return;
	
        //Now do some cleanup at the end of the turn.
        finishTurn();

        //...and initiate the next one.
        plist->nextPlayer();
        
        //if it is the first player's turn now, a new round has started
        if (plist->getActiveplayer() == plist->getFirstLiving())
        {
          if (plist->checkPlayers() == true)
	    {
	      if (plist->getNoOfPlayers() <= 1)
		break;
	      if (plist->getActiveplayer()->isDead())
		plist->nextPlayer();
	    }
	  finishRound();
	  snextRound.emit();
	}
	  
    }
}

void NextTurn::endTurn()
{
  // Finish off the player and transfers the control to the start function
  // again.
  finishTurn();
  Playerlist::getInstance()->nextPlayer();

  if (Playerlist::getActiveplayer() == Playerlist::getInstance()->getFirstLiving())
    {
      finishRound();
      snextRound.emit();
    }

  start();
}

void NextTurn::startTurn()
{
  //this function is called before a player starts his turn. Some
  //items you could imagine to be placed here: healing/building
  //units, check for joining heroes...

  //a shortcut
  Player* p = Playerlist::getActiveplayer();

  p->initTurn();
      
  //if turnmode is set, create/heal armies at player's turn
  if (d_turnmode)
    {
      //pay upkeep for stacks, reset moves, and heal stacks
      p->getStacklist()->nextTurn();

      //vector armies (needs to preceed city's next turn)
      VectoredUnitlist::getInstance()->nextTurn(p);

      //build armies
      Citylist::getInstance()->nextTurn(p);

    }

  //calculate upkeep
  p->calculateUpkeep();

}

void NextTurn::finishTurn()
{
  //Put everything that has to be done before the next player starts
  //his turn here. E.g. one could clear some caches.
  Playerlist::getActiveplayer()->getFogMap()->smooth();
}

void NextTurn::finishRound()
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

	  //produce armies
	  Citylist::getInstance()->nextTurn(*it);

	  //heal armies
	  (*it)->getStacklist()->nextTurn();

	}
    }

      
  // hold diplomatic talks, and determine diplomatic outcomes
  for (Playerlist::iterator pit = plist->begin(); pit != plist->end(); pit++)
    {
      if ((*pit)->isDead())
	continue;

      if ((*pit) == plist->getNeutral())
	continue;
  
      for (Playerlist::iterator it = plist->begin(); it != plist->end(); it++)
	{
      
	  if ((*it)->isDead())
	    continue;

	  if ((*it) == plist->getNeutral())
	    continue;

	  if ((*it) == (*pit))
	    break;
  
	  Player::DiplomaticState old_state = (*pit)->getDiplomaticState(*it);
	  Player::DiplomaticState new_state = (*pit)->negotiateDiplomacy (*it);
	  (*pit)->declareDiplomacy (new_state, (*it));
	  (*it)->declareDiplomacy (new_state, (*pit));
	  if (old_state != new_state)
	    {
	      if (new_state == Player::AT_PEACE)
		{
		  History_DiplomacyPeace *item = new History_DiplomacyPeace();
		  item->fillData(*it);
		  (*pit)->getHistorylist()->push_back(item);
		}
	      else if (new_state == Player::AT_WAR)
		{
		  History_DiplomacyWar *item = new History_DiplomacyWar();
		  item->fillData(*it);
		  (*pit)->getHistorylist()->push_back(item);
		}
	    }
	}
    }

  Playerlist::getInstance()->calculateDiplomaticRankings();
  Playerlist::getInstance()->calculateWinners();

  // heal the stacks in the ruins
  Ruinlist* rl = Ruinlist::getInstance();
  for (Ruinlist::iterator it = rl->begin(); it != rl->end(); it++)
    {
      Stack* keeper = (*it).getOccupant();
      if (keeper)
	keeper->nextTurn();
    }
}
