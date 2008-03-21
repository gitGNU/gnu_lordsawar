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

#include <stdlib.h>
#include <algorithm>
#include <fstream>

#include "ai_fast.h"

#include "playerlist.h"
#include "armysetlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "path.h"
#include "GameMap.h"
#include "Threatlist.h"
#include "action.h"
#include "xmlhelper.h"
#include "AI_Diplomacy.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

AI_Fast::AI_Fast(string name, Uint32 armyset, SDL_Color color, int width, int height, int player_no)
    :RealPlayer(name, armyset, color, width, height, Player::AI_FAST, player_no), d_join(true),
    d_maniac(false), d_analysis(0), d_diplomacy(0)
{
}

AI_Fast::AI_Fast(const Player& player)
    :RealPlayer(player), d_join(true), d_maniac(false), d_analysis(0), 
    d_diplomacy(0)
{
    d_type = AI_FAST;
}

AI_Fast::AI_Fast(XML_Helper* helper)
    :RealPlayer(helper), d_analysis(0), d_diplomacy(0)
{
    helper->getData(d_join, "join");
    helper->getData(d_maniac, "maniac");
}

AI_Fast::~AI_Fast()
{
    if (d_analysis)
        delete d_analysis;
    if (d_diplomacy)
        delete d_diplomacy;
}

bool AI_Fast::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("player");
    retval &= helper->saveData("join", d_join);
    retval &= helper->saveData("maniac", d_maniac);
    retval &= Player::save(helper);
    retval &= helper->closeTag();

    return retval;
}

bool AI_Fast::startTurn()
{
    // maniac AI's never recruit heroes, otherwise take everything we can get
    if (!d_maniac)
      maybeRecruitHero();
    
    debug(getName() << ": AI_Fast::start_turn")
    debug("being in " <<(d_maniac?"maniac":"normal") <<" mode")
    debug((d_join?"":"not ") <<"joining armies")

    d_analysis = new AI_Analysis(this);
    d_diplomacy = new AI_Diplomacy(this);

    d_diplomacy->considerCuspOfWar();
    
    // this is a recursively-programmed quite staightforward AI,we just call:
    computerTurn();

    delete d_analysis;
    d_analysis = 0;

    d_stacklist->setActivestack(0);

    // Declare war with enemies, make peace with friends
    d_diplomacy->makeProposals();
    return true;
}

void AI_Fast::invadeCity(City* c)
{
  debug("Invaded city " <<c->getName());

  // There are two modes: maniac razes all cities, non-maniac just
  // occupies them
  if (d_maniac)
  {
    debug ("Razing it");
    cityRaze(c);
  }
  else
  {
    debug("Occupying it");
    cityOccupy(c);
  }
}

void AI_Fast::levelArmy(Army* a)
{
    debug("Army raised a level, id = " <<a->getId())
    
    //advancing a level
    // increase the strength attack (uninnovative, but enough here)
    Army::Stat stat = Army::STRENGTH;
    doLevelArmy(a, stat);

    Action_Level* item = new Action_Level();
    item->fillData(a, stat);
    addAction(item);
}

void AI_Fast::computerTurn()
{
    // we have configurable behaviour in two ways:
    // 1. join => merges close stacks
    // 2. maniac => attack at any costs, raze everything in the path
    //
    // So the basic algorithm is like
    // for all armies
    //      if (army_to_join close && d_join)
    //          join armies
    //      if (!maniac && army_damaged)
    //          resupply
    //      if (!maniac)
    //          find next enemy city
    //      else
    //          find next enemy unit with preference to cities
    //      attack
    //

    // we are using reversed order because new stacks come behind old stacks
    // and we want the freshly created stacks join the veterans and not the other
    // way round.
    for (Stacklist::reverse_iterator it = d_stacklist->rbegin(); it != d_stacklist->rend(); it++)
    {
        // game ended; emergency stop
        if (Playerlist::isFinished())
            break;

        d_stacklist->setActivestack(*it);
        Stack* s = *it;
        
        debug(">>>> What to do with stack " <<s->getId() <<" at (" <<s->getPos().x
	       <<"," <<s->getPos().y <<")?")

        // join armies if close
        if (d_join && s->size() < MAX_STACK_SIZE)
        {
            Stack* target = 0;
            for (Stacklist::iterator it2 = d_stacklist->begin(); it2 != d_stacklist->end(); it2++)
            {
                if (s->size() + (*it2)->size() > MAX_STACK_SIZE || s == (*it2))
                    continue;
                
                int dist = abs(s->getPos().x - (*it2)->getPos().x);
                if (dist < abs(s->getPos().y - (*it2)->getPos().y))
                    dist = abs(s->getPos().y - (*it2)->getPos().y);
                    
                if (dist <= 5)
                {
                    target = (*it2);
                    break;
                }
            }

            if (target)
            {
                debug("Joining with stack " <<target->getId() <<" at (" <<target->getPos().x
                      <<"," <<target->getPos().y <<")")
		s->getPath()->calculate(s, target->getPos());
                stackMove(s);
		continue;
            }
        }
        
        // Maybe the stack vanished upon joining. If so, reiterate;
        if (!d_stacklist->getActivestack())
        {
            computerTurn();
            return;
        }

                    
        // second step: try to resupply
        if (!d_maniac)
        {
            City *target = Citylist::getInstance()->getNearestFriendlyCity(s->getPos());
            if (s->size() < MAX_STACK_SIZE && target)
            {
                debug("Restocking in " <<target->getName())
                // try to move to the north west part of the city (where the units
                // move after production), otherwise just wait and stand around


                if (target->getPos().x != s->getPos().x
                    || target->getPos().y != s->getPos().y)
                {
                    s->getPath()->calculate(s, target->getPos());
                    stackMove(s);

                    // the stack could have joined another stack waiting there
                    if (!d_stacklist->getActivestack())
                    {
                        computerTurn();
                        return;
                    }
                }
            }

            // third step: non-maniac players attack only enemy cities
            else
            {
	        City *target1;
	        City *target2;
		Path *target1_path = new Path();
		Path *target2_path = new Path();
	        Citylist *cl = Citylist::getInstance();
                target1 = cl->getNearestEnemyCity(s->getPos());
		target2 = cl->getNearestForeignCity(s->getPos());
		if (target1)
		  target1_path->calculate (s, target1->getPos());
		if (!target2)
		  return; //it's game over and we're still moving
		target2_path->calculate (s, target2->getPos());
		if (!target1)
		  target = target2;
		else if (target1_path->size() / 13 > target2_path->size())
		  target = target2;
		else
		  target = target1;

		if (target == target2)
		  {
		    d_diplomacy->needNewEnemy(target->getOwner());
		    // try to wait a turn until we're at war
		    if (target1)
		      target = target1;
		  }

                if (!target)    // strange situation
                    return;

                debug("Attacking " <<target->getName())
                int moves = s->getPath()->calculate(s, target->getPos());
		debug("Moves to enemy city: " << moves);
		if (s->getPath()->checkPath(s) == true)
		  stackMove(s);
		else
		  {
		    Citylist *cl = Citylist::getInstance();
		    City *friendly_city = 
		      cl->getNearestFriendlyCity(s->getPos());
		    s->getPath()->calculate(s, friendly_city->getPos());
		    stackMove(s);
		    //we should *always* be able to reach a target city!
		    //this is an error in map generation.
		  }

                // a stack has died ->restart
                if (!d_stacklist->getActivestack())
                {
                    computerTurn();
                    return;
                }
            }
        }


        // fourth step: maniac players attack everything that is close if they can
        // reach it or cities otherwise.
        if (d_maniac)
        {
            const Threatlist* threats = d_analysis->getThreatsInOrder(s->getPos());
            Threatlist::const_iterator tit = threats->begin();
            float mystrength = AI_Analysis::assessStackStrength(s);
            const Threat* target = 0;
            
            // prefer weak forces (take strong if neccessary) and stop after 10
            // stacks
            for (int i = 0; tit != threats->end() && i < 10; tit++, i++)
            {
                // in a first step, we only look at enemy stacks
                if ((*tit)->isCity() || (*tit)->isRuin())
                    continue;

                // ignore stacks out of reach
                Uint32 mp = s->getPath()->calculate(s, (*tit)->getClosestPoint(s->getPos()));
                if (mp == 0 || mp > s->getGroupMoves())
                    continue;

                // if the target is weaker than us, attack it!
                if ((*tit)->strength() < mystrength)
                {
                    target = *tit;
                    break;
                }

                // otherwise take the weakest target we can get
                if (!target || target->strength() > (*tit)->strength())
                    target = *tit;
            }

            // now we need to choose. If we found a target, attack it, otherwise
            // attack the closest city.
            Vector<int> pos;
            if (target)
            {
                pos = target->getClosestPoint(s->getPos());
                debug("Maniac mode, found target at (" <<pos.x <<"," <<pos.y <<")")
            }
            else
            {
                pos  = Citylist::getInstance()->getNearestForeignCity(s->getPos())->getPos();
                debug("Maniac, found no targets, attacking city at (" <<pos.x <<"," <<pos.y <<")")
            }

            s->getPath()->calculate(s, pos);
	    if (s->getPath()->checkPath(s) == true)
	      stackMove(s);
	    else
	      {
		Citylist *cl = Citylist::getInstance();
		City *friendly_city = cl->getNearestFriendlyCity(s->getPos());
		s->getPath()->calculate(s, friendly_city->getPos());
		stackMove(s);
	      }

            if (!d_stacklist->getActivestack())
            {
                computerTurn();
                break;
            }
        }
    }
}

bool AI_Fast::treachery (Stack *stack, Player *player, Vector <int> pos, DiplomaticState state)
{
  bool performTreachery = true;
  return performTreachery;
}
// End of file
