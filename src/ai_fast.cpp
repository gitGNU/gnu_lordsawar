// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2004 John Farrell
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <stdlib.h>
#include <algorithm>
#include <fstream>

#include "ai_fast.h"

#include "playerlist.h"
#include "armysetlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "templelist.h"
#include "ruinlist.h"
#include "path.h"
#include "GameMap.h"
#include "Threatlist.h"
#include "action.h"
#include "xmlhelper.h"
#include "AI_Diplomacy.h"
#include "stack.h"
#include "GameScenarioOptions.h"
#include "hero.h"
#include "vectoredunitlist.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

AI_Fast::AI_Fast(string name, Uint32 armyset, SDL_Color color, int width, int height, int player_no)
    :RealPlayer(name, armyset, color, width, height, Player::AI_FAST, player_no), d_join(true),
    d_maniac(false), d_analysis(0), d_diplomacy(0), d_abort_requested(false)
{
}

AI_Fast::AI_Fast(const Player& player)
    :RealPlayer(player), d_join(true), d_maniac(false), d_analysis(0), 
    d_diplomacy(0)
{
    d_type = AI_FAST;
    d_abort_requested = false;
}

AI_Fast::AI_Fast(XML_Helper* helper)
    :RealPlayer(helper), d_analysis(0), d_diplomacy(0), d_abort_requested(false)
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

    retval &= helper->openTag(Player::d_tag);
    retval &= helper->saveData("join", d_join);
    retval &= helper->saveData("maniac", d_maniac);
    retval &= Player::save(helper);
    retval &= helper->closeTag();

    return retval;
}

void AI_Fast::abortTurn()
{
  d_abort_requested = true;
}

bool AI_Fast::startTurn()
{
    sbusy.emit();
    AI_maybeBuyScout();

    sbusy.emit();
    // maniac AI's never recruit heroes, otherwise take everything we can get
    if (!d_maniac)
      maybeRecruitHero();
    

    debug(getName() << ": AI_Fast::start_turn")
    debug("being in " <<(d_maniac?"maniac":"normal") <<" mode")
    debug((d_join?"":"not ") <<"joining armies")

    d_analysis = new AI_Analysis(this);
    d_diplomacy = new AI_Diplomacy(this);

    d_diplomacy->considerCuspOfWar();
    
    if (getUpkeep() > getIncome() + getIncome())
      d_maniac = true;
    else
      d_maniac = false;

    //setup production
    if (getIncome() > getUpkeep())
      {
	Citylist *cl = Citylist::getInstance();
	for (Citylist::iterator cit = cl->begin(); cit != cl->end(); ++cit)
	  {
	    City *c = *cit;
	    if (c->getOwner() != this || c->isBurnt())
	      continue;
	    if (c->getActiveProductionSlot() == -1)
	      {
		if (c->getProductionBase(0))
		  c->setActiveProductionSlot(0);
	      }
	  }
      }

    //setup vectoring
    if (!d_maniac)
	AI_setupVectoring(18, 3, 30);

    sbusy.emit();
    // this is a recursively-programmed quite staightforward AI,we just call:
    while (computerTurn() == true)
      {
	sbusy.emit();
	bool found = false;
    
	//are there any stacks with paths that can move?
	for (Stacklist::reverse_iterator it = d_stacklist->rbegin(); it != d_stacklist->rend(); it++)
	  {
	    Stack *s = (*it);
	    if (s->getPath()->size() > 0 && s->enoughMoves())
	      {
		int mp = s->getPath()->calculate(s, *s->getPath()->back());
		if (mp <= 0)
		  continue;
		debug ("AI_FAST stack " << s->getId() << " can still potentially move");
		debug ("moving from " << s->getPos().x << "," << s->getPos().y
		       << ") to (" <<(*s->getPath()->begin())->x << "," <<
		       (*s->getPath()->begin())->y << ") with " << s->getGroupMoves() <<" left");

	    
		found = true;
	      }
	  }
	if (found)
	  found = false;
	if (d_abort_requested)
	  break;
      }

    delete d_analysis;
    d_analysis = 0;

    d_stacklist->setActivestack(0);

    // Declare war with enemies, make peace with friends
    if (GameScenarioOptions::s_diplomacy)
      d_diplomacy->makeProposals();

    if (d_abort_requested)
      aborted_turn.emit();
    return true;
}

void AI_Fast::invadeCity(City* c)
{
  debug("Invaded city " <<c->getName());

  // There are three modes: maniac razes all cities, non-maniac just
  // occupies them...
  // but even a maniac won't raze a city if money is needed
  if (getIncome() < getUpkeep())
    {
      debug("Occupying it");
      cityOccupy(c);
    }
  else if (d_maniac)
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

Stack *AI_Fast::findNearOwnStackToJoin(Stack *s, int max_distance)
{
  Stack* target = NULL;
  for (Stacklist::iterator it2 = d_stacklist->begin(); it2 != d_stacklist->end(); it2++)
    {
      if (s->size() + (*it2)->size() > MAX_STACK_SIZE || s == (*it2))
	continue;

      int distance = dist(s->getPos(), (*it2)->getPos());

      if (distance <= max_distance)
	{
	  target = (*it2);
	  break;
	}
    }
  return target;
}

bool AI_Fast::computerTurn()
{
  bool stack_moved = false;
    // we have configurable behaviour in two ways:
    // 1. join => merges close stacks
    // 2. maniac => attack at any costs, raze everything in the path
    //
    // So the basic algorithm is like
    // for all armies
    //      if !maniac, and close to a temple, visit it
    //      if (army_to_join close && d_join)
    //          join armies
    //      if (!maniac && army_damaged)
    //          resupply
    //      if (!maniac)
    //          find next enemy city
    //          if it's too far away then disband
    //      else
    //          find next enemy unit with preference to cities
    //      attack
    //
    // return true if any stack moved

    // we are using reversed order because new stacks come behind old stacks
    // and we want the freshly created stacks join the veterans and not the other
    // way round.
    for (Stacklist::reverse_iterator it = d_stacklist->rbegin(); it != d_stacklist->rend(); it++)
    {
        d_stacklist->setActivestack(*it);
        Stack* s = *it;
	s->group();
        
	//go to a temple
	if (!d_maniac)
	  {
	    bool stack_died = false;
	    bool blessed = false;
	    if (Citylist::getInstance()->getObjectAt(s->getPos()) == NULL)
	      {
		stack_moved = AI_maybeVisitTempleForBlessing
		  (s, s->getGroupMoves(), s->getGroupMoves() + 7, 50.0, 
		   blessed, stack_died);
		if (stack_died)
		  return true;
		s = d_stacklist->getActivestack();
		if (blessed && stack_moved)
		  stack_moved = false; //do this so we move it later on
		else if (stack_moved)
		  continue;
	      }
	  }

	//pick up items
	if (!d_maniac)
	  {
	    bool stack_died = false;
	    bool picked_up = false;

	    stack_moved = AI_maybePickUpItems(s, s->getGroupMoves(), 
					       s->getGroupMoves() + 7, 
					       picked_up, stack_died);
	    if (stack_died)
	      return true;
	    s = d_stacklist->getActivestack();
	    if (picked_up && stack_moved)
	      stack_moved = false; //do this so we move it later on
	    else if (stack_moved)
	      continue;
	  }

        debug(">>>> What to do with stack " <<s->getId() <<" at (" <<s->getPos().x
	       <<"," <<s->getPos().y <<") containing " <<s->size() << " armies ?")

        // join armies if close
        if (d_join && s->size() < MAX_STACK_SIZE)
        {
            Stack* target = NULL;
	    target = findNearOwnStackToJoin(s, 5);

            if (target)
            {
                debug("Joining with stack " <<target->getId() <<" at (" <<target->getPos().x
                      <<"," <<target->getPos().y <<")")
		s->getPath()->calculate(s, target->getPos());
                stack_moved |= stackMove(s);
		continue;
            }
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

		if (target->contains(s->getPos()) == false)
		  {
		    s->getPath()->calculateToCity(s, target);
		    stack_moved |= stackMove(s);

		    // the stack could have joined another stack waiting there
		    if (!d_stacklist->getActivestack())
		      return true;
		    continue;
		  }
		else if (s->getPos() != target->getPos())
		  {
		    //if we're not in the upper right corner
		    s->getPath()->calculate(s, target->getPos());
		    //then try to go there
		    stack_moved |= stackMove(s);
		    continue;
		  }
		//otherwise just stay put in the city
	    }

	    // third step: non-maniac players attack only enemy cities
	    else
	      {
		target = NULL;
		Path *target1_path = new Path();
		Path *target2_path = new Path();
		Citylist *cl = Citylist::getInstance();
		City *target1 = cl->getNearestEnemyCity(s->getPos());
		City *target2 = cl->getNearestForeignCity(s->getPos());
		if (target1)
		  target1_path->calculateToCity (s, target1);
		if (!target2)
		  return false; //it's game over and we're still moving
		target2_path->calculateToCity (s, target2);

		//no enemies?  then go for the nearest foreign city.
		//if diplomacy isn't on and we hit this, then it's game over
		if (!target1)
		  target = target2;

		//is the enemy city far enough away that a foreign city
		//outweighs it?
		else if (target1_path->size() / 13 > target2_path->size())
		  target = target2;
		else
		  target = target1;

		if (target == target2)
		  {
		    if (GameScenarioOptions::s_diplomacy == true)
		      d_diplomacy->needNewEnemy(target->getOwner());
		    // try to wait a turn until we're at war
		    if (target1)
		      target = target1;
		  }

		if (!target)    // strange situation
		  {
		    cerr << "yet another bad situation!!\n";
		    exit (1);
		    return false;
		  }

		debug("Attacking " << target->getName() << " (" << 
		      target->getPos().x <<","<< target->getPos().y << ")")
		  int moves = s->getPath()->calculateToCity(s, target);
		debug("Moves to enemy city: " << moves);

		if (moves > 60)
		  {
		    //are we in a city?
		    City *c = Citylist::getInstance()->getObjectAt(s->getPos());
		    if (c)
		      {
			//is nearest friendly city to the enemy not our city?
			if (cl->getNearestFriendlyCity(target->getPos()) != c)
			  {
			    bool disbanded;
			    bool killed = false;
			    disbanded = AI_maybeDisband(s, c, 3, 18, killed);
			    if (disbanded)
			      {
				debug ("disbanded stack in " << c->getName() <<"\n");
				return true;
			      }
			    if (killed)
			      return true;
			  }
		      }
		  }
		if (moves >= 1)
		  {
		    stack_moved |= stackMove(s);
		    s = d_stacklist->getActivestack();
		    if (!d_stacklist->getActivestack())
		      return true;
		    //if we didn't get there
		    if (target->getOwner() != s->getOwner())
		      {
			//and the target city is empty
			if (target->countDefenders() == 0)
			  {
			    //attack if if we can reach it.
			    int moved = stackSplitAndMove(s);
			    stack_moved |=  moved;
			  }
		      }
		  }
		else
		  {
		    cerr << "hit yet another bad spot." << moves << "\n";
		    printf ("stack %d is at %d,%d, target is at %d,%d\n",
			    s->getId(), s->getPos().x, s->getPos().y, target->getPos().x,
			    target->getPos().y);
		    printf ("this error means the map has an unreachable city on it, which is supposed to be impossible.\n");
		    printf ("if an enemy city is completely surrouned by other stacks, this error message is possible.\n");
		    printf ("fixme: the stack should try for somewhere else.\n");
		    return false;
		  }

		// a stack has died ->restart
		if (!d_stacklist->getActivestack())
		  return true;

		continue;
	      }
	}


	// fourth step: maniac players attack everything that is close if they can
	// reach it or cities otherwise.
	if (d_maniac)
	  {
	    const Threatlist* threats = d_analysis->getThreatsInOrder(s->getPos());
	    Threatlist::const_iterator tit = threats->begin();
	    const Threat* target = 0;

	    // prefer weak forces (take strong if neccessary) and stop after 10
	    // stacks
	    for (int i = 0; tit != threats->end() && i < 10; tit++, i++)
	      {
		// in a first step, we only look at enemy stacks
		if ((*tit)->isCity() || (*tit)->isRuin())
		  continue;

		// ignore stacks out of reach
		Vector<int> threatpos = (*tit)->getClosestPoint(s->getPos());
		if (threatpos == Vector<int>(-1, -1))
		  continue;

		Uint32 mp = s->getPath()->calculate(s, threatpos);
		if ((int)mp <= 0 || mp > s->getGroupMoves())
		  continue;

		target = *tit;
		break;

	      }

	    // now we need to choose. If we found a target, attack it, otherwise
	    // attack the closest city.
	    Vector<int> pos = Vector<int>(-1,-1);
	    if (target)
	      {
		pos = target->getClosestPoint(s->getPos());
		debug("Maniac mode, found target at (" <<pos.x <<"," <<pos.y <<")")
	      }
	    else
	      {
		Citylist *cl = Citylist::getInstance();
		City *enemy_city = cl->getNearestForeignCity(s->getPos());
		if (enemy_city)
		  {
		    pos  = enemy_city->getPos();
		    debug("Maniac, found no targets, attacking city " << enemy_city->getName() << " at (" <<pos.x <<"," <<pos.y <<")")
		  }
	      }

	    if (pos == Vector<int>(-1,-1))
	      return false;

	    int mp = s->getPath()->calculate(s, pos);
	    if (mp > 0)
	      {
	      //printf ("stack %d at %d,%d moving %d with %d moves\n",
		      //s->getId(), s->getPos().x, s->getPos().y,
		      //mp, s->getGroupMoves());
		bool moved = stackMove(s);
		//printf("result of move: %d\n", moved);
		stack_moved |= moved;
		s = d_stacklist->getActivestack();
	      }
	    else
	      {
		printf ("we're going the wrong way (mp is %d)!!\n", mp);
		printf ("this means we couldn't calculate a path from %d,%d to %d,%d\n", s->getPos().x, s->getPos().y, pos.x, pos.y);
		//sleep (10);
		Citylist *cl = Citylist::getInstance();
		City *friendly_city = cl->getNearestFriendlyCity(s->getPos());
		if (friendly_city)
		  {
		    mp = s->getPath()->calculate(s, friendly_city->getPos());
		    if (mp > 0)
		      stack_moved |= stackMove(s);
		    else
		      stack_moved |= false;
		  }
		else
		  {
		    //we can't find anyplace to move to!
		    //so we stay put.
		    stack_moved |= false;
		  }
	      }

	    if (!d_stacklist->getActivestack())
	      return true;
	    continue;

	  }
	if (d_abort_requested)
	  break;
    }
    return stack_moved;
}

bool AI_Fast::treachery (Stack *stack, Player *player, Vector <int> pos, DiplomaticState state)
{
  bool performTreachery = true;
  return performTreachery;
}


// End of file
