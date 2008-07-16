// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004 John Farrell
// Copyright (C) 2005 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#include <sigc++/functors/mem_fun.h>

#include "citylist.h"
#include "city.h"
#include "playerlist.h"
#include <limits.h>
#include "xmlhelper.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Citylist* Citylist::s_instance = 0;


Citylist* Citylist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Citylist();

    return s_instance;
}

Citylist* Citylist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Citylist(helper);
    return s_instance;
}

void Citylist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Citylist::Citylist()
{
}

Citylist::Citylist(XML_Helper* helper)
{
    // simply ask the helper to inform us when a city tag is opened
    helper->registerTag("city", sigc::mem_fun(this, &Citylist::load));
    helper->registerTag("army", sigc::mem_fun(this, &Citylist::load));
    helper->registerTag("slot", sigc::mem_fun(this, &Citylist::load));
}

Citylist::~Citylist()
{
}

int Citylist::countCities(Player* player) const
{
    int cities = 0;
    
    for (const_iterator it = begin(); it != end(); it++)
    {
        if ((*it).isBurnt())
          continue;
        if ((*it).getOwner() == player) cities++;
    }
    
    return cities;
}

void Citylist::collectTaxes(Player* p)
{
  // Collect the taxes
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it).getOwner() == p)
      p->addGold((*it).getGold());
}

void Citylist::nextTurn(Player* p)
{
    debug("next_turn(" <<p->getName() <<")");

    // Because players are nextTurn'd before cities, the income, treasury, 
    // and upkeep are calculated already for the upcoming round.

    //we've already collected taxes this round, so hopefully our
    //treasury has enough money to pay our city upkeep.
    if (p->getGold() < p->getUpkeep())
      {
	int diff = p->getUpkeep() - p->getGold();
	//then we have to turn off enough production to make up for diff
	//gold pieces.
	for (iterator it = begin(); it != end(); it++)
	  {
	    if ((*it).isBurnt() == true)
	      continue;
	    int slot =(*it).getActiveProductionSlot();
	    if (slot == -1)
	      continue;
	    const Army *a = (*it).getProductionBase(slot);
	    diff -= a->getUpkeep();
    
	    p->cityTooPoorToProduce(&(*it), slot);
	    if (diff < 0)
	      break;
	  }
      }

    // This iteration adds the city production to the player    
    for (iterator it = begin(); it != end(); it++)
      {
        if ((*it).getOwner() == p)
            (*it).nextTurn();
      }

}
static bool isFogged(void *object)
{
  return ((City*)object)->isFogged();
}

static bool isBurnt(void *object)
{
  return ((City*)object)->isBurnt();
}

static bool isNotOwnedByNeutral(void *object)
{
  return ((City*)object)->getOwner() != Playerlist::getInstance()->getNeutral();
}

static bool isNotOwnedByActivePlayer(void *object)
{
  return ((City*)object)->getOwner() != Playerlist::getActiveplayer();
}

static bool isOwnedByActivePlayer(void *object)
{
  return ((City*)object)->getOwner() == Playerlist::getActiveplayer();
}

static bool isNotOwnedByEnemy(void *object)
{
  Player *p = Playerlist::getActiveplayer();
  if (!p)
    return false;
  City *city = ((City*)object);
  if (city->getOwner() != p &&
	    p->getDiplomaticState(city->getOwner()) == Player::AT_WAR)
    return false;
  return true;
}

static bool canNotAcceptVectoring(void *object)
{
  City *c = ((City*)object);
  Uint32 num = c->countCitiesVectoringToHere();
  if (num < MAX_ARMIES_VECTORED_TO_ONE_CITY)
    return false;
  return true;
}

City* Citylist::getNearestEnemyCity(const Vector<int>& pos)
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isNotOwnedByEnemy);
  return getNearestObject(pos, &filters);
}


City* Citylist::getNearestForeignCity(const Vector<int>& pos)
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isOwnedByActivePlayer);
  return getNearestObject(pos, &filters);
}

City* Citylist::getNearestCity(const Vector<int>& pos, int dist)
{
  City *c = getNearestCity(pos);
  if (!c)
    return c;
  if (c->getPos().x <= pos.x + dist && c->getPos().x >= pos.x - dist &&
      c->getPos().y <= pos.y + dist && c->getPos().y >= pos.y - dist)
    return c;
  return NULL;
}

City* Citylist::getNearestFriendlyCity(const Vector<int>& pos, int dist)
{
  City *c = getNearestFriendlyCity(pos);
  if (!c)
    return c;
  if (c->getPos().x <= pos.x + dist && c->getPos().x >= pos.x - dist &&
      c->getPos().y <= pos.y + dist && c->getPos().y >= pos.y - dist)
    return c;
  return NULL;
}

City* Citylist::getNearestFriendlyCity(const Vector<int>& pos)
{
    Player* p = Playerlist::getInstance()->getActiveplayer();
    return getNearestCity (pos, p);
}

City* Citylist::getNearestCity(const Vector<int>& pos, Player *p)
{
    int diff = -1;
    iterator diffit;
    
    for (iterator it = begin(); it != end(); ++it)
    {
        if ((*it).isBurnt())
            continue;

        if ((*it).getOwner() == p)
        {
            Vector<int> p = (*it).getPos();
            int delta = abs(p.x - pos.x);
            if (delta < abs(p.y - pos.y))
                delta = abs(p.y - pos.y);
            
            if ((diff > delta) || (diff == -1))
            {
                diff = delta;
                diffit = it;
            }
        }
    }
    
    if (diff == -1) return 0;
    return &(*diffit);
}

City* Citylist::getNearestCity(const Vector<int>& pos)
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  return getNearestObject(pos, &filters);
}

City* Citylist::getNearestVisibleCity(const Vector<int>& pos)
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isFogged);
  return getNearestObject(pos, &filters);
}

City* Citylist::getNearestVisibleCity(const Vector<int>& pos, int dist)
{
  City *c = getNearestVisibleCity(pos);
  if (!c)
    return c;
  if (c->getPos().x <= pos.x + dist && c->getPos().x >= pos.x - dist &&
      c->getPos().y <= pos.y + dist && c->getPos().y >= pos.y - dist)
    return c;
  return NULL;
}

City* Citylist::getNearestVisibleFriendlyCity(const Vector<int>& pos, int dist)
{
  City *c = getNearestFriendlyCity(pos);
  if (!c)
    return c;
  if (c->getPos().x <= pos.x + dist && c->getPos().x >= pos.x - dist &&
      c->getPos().y <= pos.y + dist && c->getPos().y >= pos.y - dist)
    return c;
  return NULL;
}


City* Citylist::getNearestVisibleFriendlyCity(const Vector<int>& pos)
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isFogged);
  filters.push_back(isNotOwnedByActivePlayer);
  return getNearestObject(pos, &filters);
}

City* Citylist::getNearestNeutralCity(const Vector<int>& pos)
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isNotOwnedByNeutral);
  return getNearestObject(pos, &filters);
}

City* Citylist::getNearestFriendlyVectorableCity(const Vector<int>& pos)
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  filters.push_back(isNotOwnedByActivePlayer);
  filters.push_back(canNotAcceptVectoring);
  return getNearestObject(pos, &filters);
}

City* Citylist::getFirstCity(Player* p)
{
    for (iterator it = begin(); it != end(); it++)
        if ((*it).getOwner() == p)
            return &(*it);

    return 0;
}

bool Citylist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("citylist");

    for (const_iterator it = begin(); it != end(); it++)
        (*it).save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Citylist::load(std::string tag, XML_Helper* helper)
{
    if (tag == "army")
      {
	//add it to the right city
	//how do i add it to the right slot?
	Citylist::iterator it = end();
	it--;
	City *city = &*it;
	Army *a = new Army (helper, Army::PRODUCTION_BASE);
	int slot = city->getMaxNoOfProductionBases() - 1;
	city->addProductionBase(slot, a);
	return true;
      }
    if (tag == "slot")
      {
	Citylist::iterator it = end();
	it--;
	City *city = &*it;
	city->setMaxNoOfProductionBases(city->getMaxNoOfProductionBases() + 1);
	return true;
      }
    if (tag == "city")
      {
	City c(helper);
	push_back(c);
	return true;
      }
    return false;
}

void Citylist::changeOwnership(Player *old_owner, Player *new_owner)
{
  for (iterator it = begin(); it != end(); it++)
    if ((*it).getOwner() == old_owner)
      {
	(*it).setOwner(new_owner);
	if ((*it).isCapital())
	  if ((*it).getCapitalOwner() == old_owner)
	    (*it).setCapitalOwner(new_owner);
      }
}

void Citylist::stopVectoringTo(City *c)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it).isBurnt() == true)
	continue;
      if ((*it).getOwner() != c->getOwner())
	continue;
      if ((*it).getVectoring() == Vector<int>(-1,-1))
	continue;
      if (c->contains((*it).getVectoring()))
	{
	  (*it).setVectoring(Vector<int>(-1,-1));
	}
      return;
    }
}

Vector<int> Citylist::calculateCenterOfTerritory (Player *p)
{
  int n = INT_MAX;
  int s = 0;
  int w = INT_MAX;
  int e = 0;
  int count = 0;
  for (iterator it = begin(); it != end(); it++)
    {
      if (p && (*it).getOwner() == p)
	continue;
      Vector<int> pos = (*it).getPos();
      count++;
      if (pos.x > e)
	e = pos.x;
      if (pos.x < w)
	w = pos.x;
      if (pos.y > s)
	s = pos.y;
      if (pos.y < n)
	n = pos.y;
    }
  if (count == 0)
    return Vector<int>(-1, -1);
  return Vector<int>((s-n)/2, (e-w)/2);
}

bool Citylist::isVectoringTarget(City *target)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it).getOwner() != target->getOwner())
	continue;
      if (target->contains((*it).getVectoring()))
	return true;
    }
  return false;
}

std::list<City*> Citylist::getCitiesVectoringTo(City *target)
{
  std::list<City*> cities;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it).getOwner() != target->getOwner())
	continue;
      if (target->contains((*it).getVectoring()))
	cities.push_back(&(*it));
    }
  return cities;
}

City* Citylist::getNearestCityPast(const Vector<int>& pos, int dist)
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isBurnt);
  return getNearestObjectAfter(pos, dist, &filters);
}

// End of file
