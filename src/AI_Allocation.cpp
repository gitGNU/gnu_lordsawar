// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005, 2006, 2007 Ulf Lorenz
// Copyright (C) 2008 Ben Asselstine
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

#include <iostream>
#include "AI_Analysis.h"
#include "AI_Allocation.h"
#include "player.h"
#include "playerlist.h"
#include "citylist.h"
#include "stacklist.h"
#include "stack.h"
#include "city.h"
#include "Threat.h"
#include "ruin.h"
#include "path.h"
#include "ruinlist.h"
#include "GameMap.h"
#include "GameScenarioOptions.h"
#include "Threatlist.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

AI_Allocation* AI_Allocation::s_instance = 0;


AI_Allocation::AI_Allocation(AI_Analysis *analysis, Player *owner)
    :d_owner(owner), d_analysis(analysis)
{
    s_instance = this;
}

AI_Allocation::~AI_Allocation()
{
    s_instance = 0;
}

void AI_Allocation::deleteStack(Stack* s)
{
    // we need to remove collaterally eradicated stacks before they make
    // trouble
    if (s_instance)
        s_instance->d_stacks->remove(s);
}

int AI_Allocation::move()
{
    Citylist *allCities = Citylist::getInstance();

    // move stacks
    d_stacks = new Stacklist(d_owner->getStacklist());

    debug(d_owner->getName() << " starts with " << d_stacks->size() << " stacks to do something with")

    int count = allocateDefensiveStacks(allCities);
    debug(d_owner->getName() << " has " << d_stacks->size() << " stacks after assigning defenders")

    //do we have any left for offense?
    if (d_stacks->size())
      {
	// stacks which are assigned to defence are no longer in 'stacks',
	// so we can tell the rest to do something
	count += allocateStacksToThreats();
	debug(d_owner->getName() << " still has " << d_stacks->size() << " stacks after allocating stacks to threats")
	  
	if (d_stacks->size())
	  count += defaultStackMovements();

	d_stacks->clear();
	delete d_stacks;
	d_stacks = 0;

	if (Playerlist::isFinished())
	  return 0;
      }

    return count;
}

// for each city, if it is in danger at all, allocate a stack to be its
// defender. These stacks sit in the NW corner and don't move out of the city.
int AI_Allocation::allocateDefensiveStacks(Citylist *allCities)
{
  int count = 0;
  for (Citylist::iterator cit = allCities->begin(); cit != allCities->end(); ++cit)
    {
      City *city = &(*cit);
      if (!city->isFriend(d_owner) || city->isBurnt())
	continue;

      float cityDanger = d_analysis->getCityDanger(city);
      //if city is not endangered, don't move any existing stacks into it
      if (cityDanger <= 0.0) continue;

      // Look how many defenders the city already has
      std::vector<Stack*> defenders = Stacklist::defendersInCity(city);
      vector<Stack*>::iterator it;
      float totalDefenderStrength = 0.0;
      for (it = defenders.begin(); it != defenders.end(); it++)
	{
	  Stack *defender = *it;
	  d_stacks->remove(defender);
	  float stackStrength = d_analysis->assessStackStrength(defender);
	  debug("assign stack " << defender->getId() << " with strength " << stackStrength
		<< " to " << city->getName() << " because its danger is " << cityDanger)

	    totalDefenderStrength += stackStrength;
	  if (totalDefenderStrength > cityDanger)
	    break;
	  // if we get here, we have assigned defenders but not enough to
	  // counter the danger that the city is in
	}

      // allocate nearby stacks to come back to the city,
      // because we don't have enough defence
      while (totalDefenderStrength < cityDanger)
	{
	  Stack *s = findClosestStackToCity(city, 2);
	  if (!s) 
	    {
	      debug("City " << city->getName() << " is endangered but no stacks are close enough to go defend it (or no more space available in city).")
	      break;
	    }
	  debug("Stack " << s->getId() << " should return to " << city->getName() << " to defend")
	  Vector<int> dest = getFreeSpotInCity(city, s->size());

	  d_stacks->remove(s);
	  d_owner->getStacklist()->setActivestack(s);

	  int mp = s->getPath()->calculate(s, dest);
	  if (mp >= 0 && d_owner->stackMove(s))
	    count++;
	}

      if (totalDefenderStrength < cityDanger)
	{
	  debug(city->getName() << " cannot be adequately defended")
	}
    }
  return count;
}

int AI_Allocation::allocateStacksToThreats()
{
  const Threatlist *threats ;
  int count = 0;
  City *city = Citylist::getInstance()->getFirstCity(d_owner);
  if (!city)
    threats = d_analysis->getThreatsInOrder();
  else
    threats = d_analysis->getThreatsInOrder(city->getPos());

  //debug("Threats to " << d_owner->getName() << " are " << threats->toString())
  debug("We start with " <<threats->size() <<" threats.")

    for (Threatlist::const_iterator it = threats->begin(); it != threats->end(); ++it)
      {
	if (d_stacks->size() == 0)
	  break;

	Threat *threat = (*it);
	while (true)
	  {
	    Stack *attacker = findBestAttackerFor(threat);
	    // if there is nobody to attack the threat, go onto the next one
	    if (!attacker) break;

	    d_stacks->remove(attacker);

	    d_owner->getStacklist()->setActivestack(attacker);
	    Vector<int> dest = threat->getClosestPoint(attacker->getPos());

	    int mp = attacker->getPath()->calculate(attacker, dest);
	    if (mp >= 0 && d_owner->stackMove(attacker))
	      count++;

	    // if the threat has been removed, go onto the next one
	    if (threat->strength() == 0.0)
		break;
	  }
      }
  return count;
}

Vector<int> AI_Allocation::getFreeSpotInCity(City *city, int stackSize)
{
  // northwest
  Vector<int> result(city->getPos());
  Stack *s = Stacklist::getObjectAt(result);
  if (!s)
    return result;
  else
    if (s->size() + stackSize <= MAX_STACK_SIZE) return result;

  // northeast
  result.x++;
  s = Stacklist::getObjectAt(result);
  if (!s)
    return result;
  else
    if (s->size() + stackSize <= MAX_STACK_SIZE) return result;

  // southeast
  result.y++;
  s = Stacklist::getObjectAt(result);
  if (!s)
    return result;
  else
    if (s->size() + stackSize <= MAX_STACK_SIZE) return result;

  // southwest
  result.x--;
  s = Stacklist::getObjectAt(result);
  if (!s)
    return result;
  else
    if (s->size() + stackSize <= MAX_STACK_SIZE) return result;

  // no room
  return Vector<int>(-1, -1);
}

Stack *AI_Allocation::findClosestStackToCity(City *city, int limitInMoves)
{
  Vector<int> pos = city->getPos();
  Stack *best = 0;
  int lowest_mp = -1;
  for (Stacklist::iterator it = d_stacks->begin(); it != d_stacks->end(); ++it)
    {
      Stack* s = *it;
      //don't consider the stack if it's already in the city
      Vector<int> spos = s->getPos();
      if (city->contains(spos))
	continue;
      //don't consider the city if we can't fit this stack anywhere inside it.
      Vector<int> dest = getFreeSpotInCity(city, s->size());
      if (dest == Vector<int>(-1, -1))
	continue;
      //don't consider the stack if it's in an endangered city
      City *stack_city = Citylist::getInstance()->getObjectAt(s->getPos());
      if (stack_city)
	{
	  if (Player::safeFromAttack(stack_city, 16, 3) == false)
	    continue;
	}

      //don't consider stacks that are too many tiles away
      int distToThreat = dist(pos, spos);
      if (distToThreat > 20)
	continue;

      int mp = Stack::scout(s, dest);
      if (mp <= 0)
	continue;
      if (mp < lowest_mp || lowest_mp == -1)
	{
	  best = s;
	  lowest_mp = mp;
	}
    }
  return best;
}

Stack *AI_Allocation::findBestAttackerFor(Threat *threat)
{
  Stack *best = NULL;
  float best_chance = -1.0;
  for (Stacklist::iterator it = d_stacks->begin(); it != d_stacks->end(); ++it)
    {
      Stack* s = *it;
      Vector<int> closestPoint = threat->getClosestPoint(s->getPos());
      // threat has been destroyed anyway
      if (closestPoint.x == -1)
	return 0;
      Vector<int> spos = s->getPos();

      int distToThreat = dist(closestPoint, spos);
      if (distToThreat > 27)
	continue;

      //don't consider the stack if it's in an endangered city
      City *stack_city = Citylist::getInstance()->getObjectAt(s->getPos());
      if (stack_city)
	{
	  if (Player::safeFromAttack(stack_city, 16, 3) == false)
	    continue;
	}

      int mp = Stack::scout(s, closestPoint);
      if (mp <= 0)
	continue;
      if (s->getGroupMoves() < mp)
	continue;

      //don't consider the stack if we're probably going to lose
      float chance = d_owner->stackFightAdvise
	(s, closestPoint, GameScenarioOptions::s_intense_combat);

      if (chance > best_chance || best_chance == -1.0)
	{
	  best = s;
	  best_chance = chance;
	}
    }
  return best;
}

int AI_Allocation::defaultStackMovements()
{
  int count = 0;
  Citylist *allCities = Citylist::getInstance();
  debug("Default movement for " <<d_stacks->size() <<" stacks");

  for (Stacklist::iterator it = d_stacks->begin(); it != d_stacks->end(); ++it)
    {
	Stack* s = *it;
	debug("thinking about stack " << s->getId() <<" at ("<<s->getPos().x<<","<<s->getPos().y<<")")
	  d_owner->getStacklist()->setActivestack(s);
	MoveResult *result = 0;
	if (s->size() >= MAX_STACK_SIZE)
	  {
	    bool moved = false;
	    City* enemyCity = allCities->getNearestEnemyCity(s->getPos());
	    if (enemyCity)
	      {
		int mp = s->getPath()->calculateToCity(s, enemyCity);
		debug("let's attack " <<enemyCity->getName() << " that is "
		      << mp << " movement points away")
		if (mp > 0)
		  {
		    d_owner->getStacklist()->setActivestack(s);
		    moved = d_owner->stackMove(s);
		    if (s !=d_owner->getStacklist()->getActivestack())
		      {
			d_stacks->remove(s);
			break;
		      }
		    s = d_owner->getStacklist()->getActivestack();
		    if (moved)
		      count++;
		  }
	      }
	    else
	      {
		enemyCity = allCities->getNearestForeignCity(s->getPos());
		if (enemyCity)
		  {
		    s->getOwner()->proposeDiplomacy(Player::PROPOSE_WAR,
						    enemyCity->getOwner());
		    debug("let's attack " <<enemyCity->getName())
		    int mp = s->getPath()->calculateToCity(s, enemyCity);
		    if (mp > 0)
		      {
			d_owner->getStacklist()->setActivestack(s);
			moved = d_owner->stackMove(s);
			if (s != d_owner->getStacklist()->getActivestack())
			  {
			    d_stacks->remove(s);
			    break;
			  }
			s = d_owner->getStacklist()->getActivestack();
			if (moved)
			  count++;
		      }
		  }
	      }

	    if (!moved)
	      {
		// for some reason (islands are one bet), we could not attack the
		// enemy city. Let's iterator through all cities and attack the first
		// one we can lay our hands on.
		debug("Mmmh, did not work.")
		  //sleep (10);
/*
		  for (Citylist::iterator cit = allCities->begin(); cit != allCities->end(); cit++)
		    if ((*cit).getOwner() != d_owner)
		      {
			debug("Let's try "<<(*cit).getName() <<" instead.")
			  result = moveStack(s, (*cit).getPos());
			if (result && result->moveSucceeded())
			  {
			    debug("Worked")
			      count++;
			    break;
			  }
		      }
		      */
	      }
	  }
	else
	  {
	    City *c = Citylist::getInstance()->getObjectAt(s->getPos());
	    bool moved;
	    if (!c)
		moved = false; //stackReinforce(s);
	    else
		moved = shuffleStacksWithinCity (c, s, Vector<int>(0,0));
		
	    if (moved)
	      count++;
	    if (s != d_owner->getStacklist()->getActivestack())
	      {
		d_stacks->remove(s);
		break;
	      }
	  }
    }
  return count;
}

bool AI_Allocation::stackReinforce(Stack *s)
{
  Citylist *allCities = Citylist::getInstance();
  float mostNeeded = -1000.0;
  City *cityNeeds = 0;
  int moves = 1000;
  for (Citylist::iterator it = allCities->begin(); it != allCities->end(); ++it)
    {
      City *city = &(*it);
      int distToCity = dist(s->getPos(), city->getPos());

      //if the city already contains the given stack, then disregard it
      //hopefully it will be shuffled later
      if (city->contains(s->getPos()))
	return false;

      //disregard if the city is too far away
      int movesToCity = (distToCity + 6) / 7;
      if (movesToCity > 3) continue;
      
      //disregard if the city can't hold our stack
      Vector<int> dest = getFreeSpotInCity(city, s->size());
      if (dest == Vector<int>(-1,-1))
	continue;

      //pick the city that needs us the most
      float need = d_analysis->reinforcementsNeeded(city);
      if (need > mostNeeded)
	{
	  cityNeeds = city;
	  mostNeeded = need;
	  moves = movesToCity;
	}
    }

  if (cityNeeds) {
    debug("stack is sent to reinforce " << cityNeeds->getName() <<" if possible")
    // don't forget to send the stack to a free field within the city
    Vector<int> dest = getFreeSpotInCity(cityNeeds, s->size());
    if (dest != Vector<int>(-1,-1))
      {
	d_analysis->reinforce(cityNeeds, s, moves);
	int mp = s->getPath()->calculate(s, dest);
	if (mp <= 0)
	  return false;
	else 
	  return d_owner->stackMove(s);
      }
  }

  //okay, no city needed us, just try to reinforce our nearest city
  City *target = allCities->getNearestFriendlyCity(s->getPos());
  if (!target) // no friendly city?
    return false;
  //are we already there?
  if (target->contains(s->getPos()))
    {
      return false;
    }
  else
    {
      Vector<int> dest = getFreeSpotInCity(target, s->size());
      if (dest == Vector<int>(-1, -1))
	return false;
      int mp = s->getPath()->calculate(s, dest);
      if (mp <= 0)
	return false;
      else 
	return d_owner->stackMove(s);
    }
  return 0;
}

void AI_Allocation::searchRuin(Stack *stack, Ruin *ruin)
{
  d_owner->stackSearchRuin(stack, ruin);
  // what to do if the ruin search fails?
}

bool AI_Allocation::shuffleStacksWithinCity(City *city, Stack *stack,
						   Vector<int> diff)
{
  if (city->getPos() + diff == stack->getPos())
    // already in the preferred position
    return false;
  int mp = stack->getPath()->calculate(stack, city->getPos() + diff);
  if (mp <= 0)
    return false;
  Stack *join = Stacklist::getObjectAt(city->getPos() + diff);
  if (!join)
    {
      return d_owner->stackMove(stack);
    }
  else if (stack->canJoin(join))
    {
      return d_owner->stackMove(stack);
    }
  else if (join->size() >= MAX_STACK_SIZE)
    {
      //recurse, but prefer a different tile.
      if (diff == Vector<int>(0,0))
	diff = Vector<int>(0,1);
      else if (diff == Vector<int>(0,1))
	diff = Vector<int>(1,0);
      else if (diff == Vector<int>(1,0))
	diff = Vector<int>(1,1);
      else if (diff == Vector<int>(1,1))
	return false;
      return shuffleStacksWithinCity(city, stack, diff);
    }
  else
    {
      //take armies from the stack and split them off to
      //join the stack at the preferred position
      return d_owner->stackSplitAndMove(stack);
    }
  return false;
}

MoveResult *AI_Allocation::moveStack(Stack *stack, Vector<int> pos)
{
  return d_owner->stackMove(stack, pos, false);
}
// End of file
