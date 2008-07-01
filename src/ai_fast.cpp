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

void AI_Fast::maybeBuyScout()
{
  bool hero_exists = false;
  for (Stacklist::iterator it = d_stacklist->begin(); 
       it != d_stacklist->end(); it++)
    
    if ((*it)->hasHero())
      hero_exists = true; 
    
  if (Citylist::getInstance()->countCities(this) == 1 && 
	hero_exists == false)
    {
      bool one_turn_army_exists = false;
      City *c = Citylist::getInstance()->getFirstCity(this);
      //do we already have something that can be produced in one turn?
      for (int i = 0; i < c->getMaxNoOfProductionBases(); i++)
	{
	  if (c->getArmytype(i) == -1)    // no production in this slot
	    continue;

	  const Army *proto = c->getProductionBase(i);
	  if (proto->getProduction() == 1)
	    {
	      one_turn_army_exists = true;
	      break;
	    }
	}
      if (one_turn_army_exists == false)
	{
	  const Armysetlist* al = Armysetlist::getInstance();
	  int free_slot = c->getFreeBasicSlot();
	  if (free_slot == -1)
	    free_slot = 0;
	  Army *scout = al->getScout(getArmyset());
	  cityBuyProduction(c, free_slot, scout->getType());
	}
    }
}

bool AI_Fast::startTurn()
{
    maybeBuyScout();

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
	    City *c = &*cit;
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
	setupVectoring();

    // this is a recursively-programmed quite staightforward AI,we just call:
    while (computerTurn() == true)
      {
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
		printf ("AI_FAST stack %d can still potentially move\n", s->getId());
	    
		printf("moving from %d,%d to %d,%d with %d moves left\n",
		       s->getPos().x, s->getPos().y,
		       (*s->getPath()->begin())->x, 
		       (*s->getPath()->begin())->y, s->getGroupMoves());
		found = true;
	      }
	  }
	if (found)
	  found = false;
      }

    delete d_analysis;
    d_analysis = 0;

    d_stacklist->setActivestack(0);

    // Declare war with enemies, make peace with friends
    if (GameScenarioOptions::s_diplomacy)
      d_diplomacy->makeProposals();

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

bool AI_Fast::maybeDisband(Stack *s, City *city, Uint32 min_defenders, 
			   int safe_mp, bool &stack_killed)
{
  //to prevent armies from piling up in far away places, 
  //we disband some periodically.
  if (s->size() != MAX_STACK_SIZE)
    return false;

  //is the city in danger from a city?
  if (safeFromAttack(city, safe_mp, 0) == false)
    return false;

  if (city->countDefenders() - s->size() >= min_defenders)
    {
      printf("disbanding here1\n");
    return stackDisband(s);
    }

  //okay, we need to disband part of our stack
  //find a square to travel to
  std::list<Vector<int> > diffs;
  diffs.push_back(Vector<int>(0, 1));
  diffs.push_back(Vector<int>(0, -1));
  diffs.push_back(Vector<int>(-1, -1));
  diffs.push_back(Vector<int>(-1, 1));
  diffs.push_back(Vector<int>(1, -1));
  diffs.push_back(Vector<int>(1, 1));
  diffs.push_back(Vector<int>(1, 0));
  diffs.push_back(Vector<int>(-1, 0));

  Vector<int> found = Vector<int>(-1, -1);
  for (std::list<Vector<int> >::iterator it = diffs.begin();
       it != diffs.end(); it++)
    {
      Vector<int> dest = s->getPos() + (*it);
      if (d_stacklist->getObjectAt(dest) == NULL)
	{
	  Uint32 mp = s->getPath()->calculate(s, dest);
	  if ((int)mp <= 0)
	    continue;
	  found = dest;
	  break;
	}
    }

  //no place to move to (strange)
  if (found == Vector<int>(-1, -1))
    return false;

  //before we move, ungroup the lucky ones not being disbanded
  unsigned int count = 0;
  s->group();
  for (Stack::reverse_iterator i = s->rbegin(); i != s->rend(); i++)
    {
      if (count == min_defenders)
	break;
      if ((*i)->isHero() == false)
	{
	  count++;
	  (*i)->setGrouped(false);
	}
    }
      printf("disbanding here2\n");

  stackSplit(s);
  stackMove(s);
  s = d_stacklist->getActivestack();

  if (d_stacklist->getActivestack() == 0) 
    {
      //maybe we got lucky and inadvertently attacked an enemy stack and lost.
      stack_killed = true;
      return false;
    }

  return stackDisband(s);
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
        // game ended; emergency stop
        if (Playerlist::isFinished())
            break;

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
		stack_moved = maybeVisitTempleForBlessing
		  (s, s->getGroupMoves(), s->getGroupMoves() + 7, 50.0, 
		   blessed, stack_died);
		if (stack_died)
		  return true;
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
		
	    stack_moved = maybePickUpItems(s, s->getGroupMoves(), 
					       s->getGroupMoves() + 7, 
					       picked_up, stack_died);
	    if (stack_died)
	      return true;
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
        
        // Maybe the stack vanished upon joining. If so, reiterate;
        if (!d_stacklist->getActivestack())
	  {
	    cerr <<"crapola.  i hit a bad spot.\n";
	    //remove me if this is never hit
	    exit(1);
	  return true;
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
			    disbanded = maybeDisband(s, c, 3, 18, killed);
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
		    cerr << "hit yet another bad spot.\n";
		    exit (1);
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
	    Vector<int> pos;
	    if (target)
	      {
		pos = target->getClosestPoint(s->getPos());
		debug("Maniac mode, found target at (" <<pos.x <<"," <<pos.y <<")")
	      }
	    else
	      {
		Citylist *cl = Citylist::getInstance();
		City *enemy_city = cl->getNearestForeignCity(s->getPos());
		pos  = enemy_city->getPos();
		debug("Maniac, found no targets, attacking city " << enemy_city->getName() << " at (" <<pos.x <<"," <<pos.y <<")")
	      }

	    if (pos == Vector<int>(-1,-1))
	      {
		debug("Maniac, failure to calc point")
		exit (1);
		return false;
	      }

	    int mp = s->getPath()->calculate(s, pos);
	    if (mp > 0)
	      {
	      printf ("stack %d at %d,%d moving %d with %d moves\n",
		      s->getId(), s->getPos().x, s->getPos().y,
		      mp, s->getGroupMoves());
		bool moved = stackMove(s);
		printf("result of move: %d\n", moved);
	      stack_moved |= moved;
	      s = d_stacklist->getActivestack();
	      }
	    else
	      {
		printf ("we're going the wrong way (mp is %d)!!\n", mp);
		printf ("this means we couldn't calculate a path from %d,%d to %d,%d\n", s->getPos().x, s->getPos().y, pos.x, pos.y);
		sleep (10);
		Citylist *cl = Citylist::getInstance();
		City *friendly_city = cl->getNearestFriendlyCity(s->getPos());
		s->getPath()->calculate(s, friendly_city->getPos());
		stack_moved |= stackMove(s);
	      }

	    if (!d_stacklist->getActivestack())
	      return true;
	    continue;

	  }
    }
    return stack_moved;
}

bool AI_Fast::treachery (Stack *stack, Player *player, Vector <int> pos, DiplomaticState state)
{
  bool performTreachery = true;
  return performTreachery;
}

bool AI_Fast::maybePickUpItems(Stack *s, int max_dist, int max_mp, bool &picked_up, bool &stack_died)
{
  int min_dist = -1;
  bool stack_moved = false;
  Vector<int> item_tile(-1, -1);

  // do we not have a hero?
  if (s->hasHero() == false)
    return false;

  //ok, which bag of stuff is closest?
  std::vector<Vector<int> > tiles = GameMap::getInstance()->getItems();
  std::vector<Vector<int> >::iterator it = tiles.begin();
  for(; it != tiles.end(); it++)
    {
      Vector<int> tile = *it;
      //don't consider bags of stuff that are inside enemy cities
      City *c = Citylist::getInstance()->getObjectAt(tile);
      if (c)
	{
	  if (c->getOwner() != s->getOwner())
	    continue;
	}

      int distance = dist (tile, s->getPos());
      if (distance < min_dist || min_dist == -1)
	{
	  min_dist = distance;
	  item_tile = tile;
	}
    }

  //if no bags of stuff, or the bag is too far away
  if (min_dist == -1 || min_dist > max_dist)
    return false;
  
  //are we not standing on it?
  if (s->getPos() != item_tile)
    {
      //can we really reach it?
      Vector<int> old_dest(-1,-1);
      if (s->getPath()->size())
	old_dest = *s->getPath()->back();
      Uint32 mp = s->getPath()->calculate(s, item_tile);
      if ((int)mp > max_mp)
	{
	  //nope.  unreachable.  set in our old path.
	  if (old_dest != Vector<int>(-1,-1))
	    s->getPath()->calculate(s, old_dest);
	  return false;
	}
      stack_moved = stackMove(s);
      //maybe we died -- an enemy stack was guarding the bag.
      if (!d_stacklist->getActivestack())
	{
	  stack_died = true;
	  return true;
	}
      s = d_stacklist->getActivestack();
    }

  //are we standing on it now?
  if (s->getPos() == item_tile)
    {
      Hero *hero = static_cast<Hero*>(s->getFirstHero());
      if (hero)
	picked_up = heroPickupAllItems(hero, s->getPos());
    }

  return stack_moved;
}

bool AI_Fast::maybeVisitTempleForBlessing(Stack *s, int dist, int max_mp, 
					  double percent_can_be_blessed, 
					  bool &blessed, bool &stack_died)
{
  bool stack_moved = false;
  Templelist *tl = Templelist::getInstance();

  Temple *temple = tl->getNearestVisibleAndUsefulTemple(s, 
							percent_can_be_blessed,
						       	dist);
  if (!temple)
    return false;

  //if we're not there yet
  if (s->getPos() != temple->getPos())
    {
      //can we really reach it?
      Vector<int> old_dest(-1,-1);
      if (s->getPath()->size())
	old_dest = *s->getPath()->back();
      Uint32 mp = s->getPath()->calculate(s, temple->getPos());
      if ((int)mp > max_mp)
	{
	  //nope.  unreachable.  set in our old path.
	  if (old_dest != Vector<int>(-1,-1))
	    s->getPath()->calculate(s, old_dest);
	  return false;
	}
      stack_moved = stackMove(s);
        
      //maybe we died -- an enemy stack was guarding the temple
      if (!d_stacklist->getActivestack())
	{
	  stack_died = true;
	  return true;
	}
      s = d_stacklist->getActivestack();
    }

  int num_blessed = 0;
  //are we there yet?
  if (s->getPos() == temple->getPos())
    {
      num_blessed = stackVisitTemple(s, temple);
    }

  blessed = num_blessed > 0;
  return stack_moved;
}

bool AI_Fast::safeFromAttack(City *c, Uint32 safe_mp, Uint32 min_defenders)
{
  //if there isn't an enemy city nearby to the source
  // calculate mp to nearest enemy city
  //   needs to be less than 18 mp with a scout
  //does the source city contain at least 3 defenders?

  City *enemy_city = Citylist::getInstance()->getNearestEnemyCity(c->getPos());
  if (enemy_city)
    {
      Uint32 mp = Stack::scout (c->getOwner(), c->getPos(), 
				enemy_city->getPos());
      if ((int)mp <= 0 || mp >= safe_mp)
	{
	  if (c->countDefenders() >= min_defenders)
	    return true;
	}
    }

  return false;
}

bool AI_Fast::maybeVector(City *c, Uint32 safe_mp, Uint32 min_defenders,
			  City *target, City **vector_city)
{
  if (vector_city)
    *vector_city = NULL;
  Citylist *cl = Citylist::getInstance();

  //is this city producing anything that we can vector?
  if (c->getActiveProductionSlot() == -1)
    return false;

  //is it safe to vector from this city?
  bool safe = safeFromAttack(c, 18, 3);

  if (!safe)
    return false;

  //get the nearest city to the enemy city that can accept vectored units
  City *near_city = cl->getNearestFriendlyVectorableCity(target->getPos());
  if (!near_city)
    return false;

  //if it's us then it's easier to just walk.
  if (near_city == c)
    return false;

  //is that city already vectoring?
  if (near_city->getVectoring() != Vector<int>(-1, -1))
    return false;

  //can i just walk there faster?

  //find mp from source to target city
  const Army *proto = c->getProductionBase(c->getActiveProductionSlot());
  Uint32 mp_from_source_city = Stack::scout(c->getOwner(), c->getPos(),
					    target->getPos(), proto);

  //find mp from nearer vectorable city to target city
  Uint32 mp_from_near_city = Stack::scout(c->getOwner(), near_city->getPos(),
					  target->getPos(), proto);

  Uint32 max_moves_per_turn = proto->getStat(Army::MOVES);

  double turns_to_move_from_source_city = 
    (double)mp_from_source_city / (double)max_moves_per_turn;
  double turns_to_move_from_near_city = 
    (double)mp_from_near_city / (double)max_moves_per_turn;
  turns_to_move_from_near_city += 1.0; //add extra turn to vector

  //yes i can walk there faster, so don't vector
  if (turns_to_move_from_source_city <= turns_to_move_from_near_city)
    return false;

  //great.  now do the vectoring.
  c->changeVectorDestination(near_city->getPos());

  if (vector_city)
    *vector_city = near_city;
  return true;
}

void AI_Fast::setupVectoring()
{
  Citylist *cl = Citylist::getInstance();
  //turn off vectoring where it isn't safe anymore
  //turn off vectoring for destinations that are far away from the
  //nearest enemy city

	  
  debug("setting up vectoring\n");
  for (Citylist::iterator cit = cl->begin(); cit != cl->end(); ++cit)
    {
      City *c = &*cit;
      if (c->getOwner() != this || c->isBurnt())
	continue;
      Vector<int> dest = c->getVectoring();
      if (dest == Vector<int>(-1, -1))
	continue;
      if (safeFromAttack(c, 18, 3) == false)
	{
	  City *target_city = Citylist::getInstance()->getObjectAt(dest);
	  debug("stopping vectoring from " << c->getName() <<" to " << target_city->getName() << " because it's not safe to anymore!\n")
	    c->setVectoring(Vector<int>(-1,-1));
	  continue;
	}

      City *enemy_city = cl->getNearestEnemyCity(dest);
      if (!enemy_city)
	{
	  City *target_city = Citylist::getInstance()->getObjectAt(dest);
	  debug("stopping vectoring from " << c->getName() <<" to " << target_city->getName() << " because there aren't any more enemy cities!\n")
	    c->setVectoring(Vector<int>(-1,-1));
	  continue;
	}

      Uint32 mp = Stack::scout(this, dest, enemy_city->getPos(), NULL);
      if ((int)mp <= 0 || mp > 30)
	{

	  City *target_city = Citylist::getInstance()->getObjectAt(dest);
	  debug("stopping vectoring from " << c->getName() <<" to " << target_city->getName() << " because it's too far away from an enemy city!\n")
	    c->setVectoring(Vector<int>(-1,-1));
	  continue;
	}
    }

  for (Citylist::iterator cit = cl->begin(); cit != cl->end(); ++cit)
    {
      City *c = &*cit;
      if (c->getOwner() != this || c->isBurnt())
	continue;
      City *enemy_city = cl->getNearestEnemyCity(c->getPos());
      if (!enemy_city)
	continue;
      City *vector_city = NULL;
      //if the city isn't already vectoring
      if (c->getVectoring() == Vector<int>(-1,-1))
	{
	  bool vectored = maybeVector(c, 18, 3, enemy_city, &vector_city);
	  if (vectored)
	    debug("begin vectoring from " << c->getName() <<" to " << vector_city->getName() << "!\n")
	}
    }
}
// End of file
