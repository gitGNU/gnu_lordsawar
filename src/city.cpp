//  Copyright (C) 2000, 2001, 2003 Michael Bartl
//  Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2002 Mark L. Amidon
//  Copyright (C) 2005 Andrea Paternesi
//  Copyright (C) 2006, 2007, 2008, 2009 Ben Asselstine
//  Copyright (C) 2008 Ole Laursen
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

#include <stdio.h>
#include <algorithm>
#include <stdlib.h>
#include <sstream>
#include "city.h"
#include "path.h"
#include "army.h"
#include "armyprodbase.h"
#include "hero.h"
#include "stacklist.h"
#include "stack.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "citylist.h"
#include "GameMap.h"
#include "vectoredunitlist.h"
#include "vectoredunit.h"
#include "action.h"

std::string City::d_tag = "city";

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

City::City(Vector<int> pos, string name, Uint32 gold, Uint32 numslots)
    :Ownable((Player *)0), Location(pos, CITY_TILE_WIDTH), Renamable(name),
    ProdSlotlist(numslots), d_gold(gold), d_defense_level(1), d_burnt(false), 
    d_vectoring(false), d_vector(Vector<int>(-1,-1)), 
    d_capital(false), d_capital_owner(0)

{
  // set the tiles to city type
  for (unsigned int i = 0; i < getSize(); i++)
    for (unsigned int j = 0; j < getSize(); j++)
      {
	Vector<int> pos = getPos() + Vector<int>(i, j);
	GameMap::getInstance()->getTile(pos)->setBuilding(Maptile::CITY);
      }
}

City::City(XML_Helper* helper)
    :Ownable(helper), Location(helper, CITY_TILE_WIDTH), Renamable(helper),
    ProdSlotlist(helper)
{
    //initialize the city

    helper->getData(d_defense_level, "defense");
    
    helper->getData(d_gold, "gold");
    helper->getData(d_burnt, "burnt");
    helper->getData(d_capital, "capital");
    if (d_capital)
      {
	Uint32 ui;
        helper->getData(ui, "capital_owner");
        d_capital_owner = Playerlist::getInstance()->getPlayer(ui);
      }


    istringstream svect;
    string s;

    helper->getData(s, "vectoring");
     svect.str(s);
    svect >> d_vector.x;
    svect >> d_vector.y;

    if (d_vector.x!=-1 && d_vector.y!=-1) 
        d_vectoring=true;
    else 
      d_vectoring=false;

    //mark the positions on the map as being occupied by a city
    for (unsigned int i = 0; i < d_size; i++)
        for (unsigned int j = 0; j < d_size; j++)
            GameMap::getInstance()->getTile(getPos().x+i, getPos().y+j)
                                  ->setBuilding(Maptile::CITY);
}

City::City(const City& c)
    :Ownable(c), Location(c), Renamable(c), ProdSlotlist(c),
    d_gold(c.d_gold), d_defense_level(c.d_defense_level), d_burnt(c.d_burnt),
    d_vectoring(c.d_vectoring),d_vector(c.d_vector), d_capital(c.d_capital), 
    d_capital_owner(c.d_capital_owner)
{
}

City::~City()
{
}

bool City::save(XML_Helper* helper) const
{
    bool retval = true;

    stringstream svect;

    svect << d_vector.x << " " << d_vector.y;

    retval &= helper->openTag(City::d_tag);
    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("x", getPos().x);
    retval &= helper->saveData("y", getPos().y);
    retval &= helper->saveData("name", getName(false));
    retval &= helper->saveData("owner", d_owner->getId());
    retval &= helper->saveData("defense", d_defense_level);
    retval &= helper->saveData("gold", d_gold);
    retval &= helper->saveData("burnt", d_burnt);
    retval &= helper->saveData("capital", d_capital);
    if (d_capital)
      retval &= helper->saveData("capital_owner", d_capital_owner->getId());
    retval &= helper->saveData("vectoring", svect.str());

    retval &= ProdSlotlist::save(helper);
    retval &= helper->closeTag();
    return retval;
}

bool City::raiseDefense()
{
    if (d_defense_level == (int) CITY_LEVELS)
    {
        // no further progress possible
        return false;
    }

    // increase the defense level, the income and the possible production
    d_defense_level++;

    return true;
}

bool City::reduceDefense()
{
    if (d_defense_level == 1)
    {
        // no further reduction possible
        return false;
    }

    // the same as raiseDefense, but the other way round
    d_defense_level--;

    return true;
}

void City::conquer(Player* newowner)
{
  Citylist::getInstance()->stopVectoringTo(this);

  setOwner(newowner);

    // remove vectoring info 
    setVectoring(Vector<int>(-1,-1));

    deFog(newowner);

    VectoredUnitlist *vul = VectoredUnitlist::getInstance();
    vul->removeVectoredUnitsGoingTo(this);
    vul->removeVectoredUnitsComingFrom(this);
}

void City::produceStrongestProductionBase()
{
  debug("produceStrongestProductionBase()");

  if (getNoOfProductionBases() == 0)
    return;

  Stack* stack = getFreeStack(d_owner);
  if (stack)
    {
      unsigned int max_strength = 0;
      int strong_idx = -1;
      for (unsigned int i = 0; i < getMaxNoOfProductionBases(); i++)
	{
	  if ((*this)[i]->getArmyProdBase() == NULL)
	    continue;
	  if (getProductionBase(i)->getStrength() > max_strength)
	    {
	      strong_idx = i;
	      max_strength = getProductionBase(i)->getStrength();
	    }
	}
      if (strong_idx == -1)
	return;

      int savep = d_active_production_slot;
      setActiveProductionSlot(strong_idx);
      produceArmy();
      setActiveProductionSlot(savep);
      return;
    }
}

void City::produceScout()
{
  const Armysetlist* al = Armysetlist::getInstance();
  Uint32 set = d_owner->getArmyset();
  ArmyProto *scout = al->getScout(set);
  Army *a = new Army(*scout, d_owner);
  GameMap::getInstance()->addArmy(this, a);

}

void City::produceWeakestProductionBase()
{
  debug("produceWeakestProductionBase()");

  if (getNoOfProductionBases() == 0)
    return;

  Stack* stack = getFreeStack(d_owner);
  if (stack)
    {
      unsigned int min_strength = 100;
      int weak_idx = -1;
      for (unsigned int i = 0; i < getMaxNoOfProductionBases(); i++)
	{
	  if ((*this)[i]->getArmyProdBase() == NULL)
	    continue;
	  if (getProductionBase(i)->getStrength() < min_strength)
	    {
	      weak_idx = i;
	      min_strength = getProductionBase(i)->getStrength();
	    }
	}
      if (weak_idx == -1)
	return;

      int savep = d_active_production_slot;
      setActiveProductionSlot(weak_idx);
      produceArmy();
      setActiveProductionSlot(savep);
      return;
    }
}

const Army *City::armyArrives()
{
  // vector the army to the new spot
  if (d_vectoring)
    {
      VectoredUnitlist *vul = VectoredUnitlist::getInstance();
      VectoredUnit *v = new VectoredUnit 
	(getPos(), d_vector, 
	 (*this)[d_active_production_slot]->getArmyProdBase(),
	 MAX_TURNS_FOR_VECTORING, d_owner);
      vul->push_back(v);
      d_owner->cityChangeProduction(this, d_active_production_slot);
      //we don't return an army when we've vectored it.
      //it doesn't really exist until it lands at the destination.
      return NULL;
    }
  else //or make it here
    {
      return produceArmy();
    }
  return NULL;
}

void City::nextTurn()
{
  if (d_burnt)
    return;

  // check if an army should be produced
  if (d_active_production_slot >= 0 && --d_duration == 0) 
    {
      if (d_owner->getGold() <= 0)
	{
	  //dont make or vector the unit
	  //and also stop production
	  d_owner->cityChangeProduction(this, -1);
	  d_owner->vectorFromCity(this, Vector<int>(-1,-1));
	  return;
	}

      d_owner->cityProducesArmy(this);

    }
}

int City::getGoldNeededForUpgrade() const
{
  if (d_defense_level == 1)
    return 1000;
  else if (d_defense_level == 2)
    return 2000;
  else if (d_defense_level == 3)
    return 3000;
  return -1;
}

void City::setVectoring(Vector<int> p) 
{
  d_vector = p;
  d_vectoring = true;

  if (p.x == -1 || p.y == -1)
    {
      d_vectoring=false;
      d_vector.x = -1;
      d_vector.y = -1;
    }
}

Army *City::produceArmy()
{
  // add produced army to stack
  if (d_active_production_slot == -1)
    return NULL;

  debug("produce_army()\n");

  // do not produce an army if the player has no gold.
  // unless it's the neutrals
  if (d_owner != Playerlist::getInstance()->getNeutral() && 
      d_owner->getGold() < 0) 
    return NULL;

  Army *a = new Army(*(getProductionBase(d_active_production_slot)), d_owner);
  GameMap::getInstance()->addArmy(this, a);

  if (d_owner == Playerlist::getInstance()->getNeutral()) 
    {
      //we're an active neutral city
      //check to see if we've made 5 or not.
      //stop producing if we've made 5 armies in our neutral city
      if (countDefenders() >= MAX_ARMIES_PRODUCED_IN_ACTIVE_NEUTRAL_CITY)
	setActiveProductionSlot(-1);
      else
	setActiveProductionSlot(d_active_production_slot);
    }
  else // start producing next army of same type
    setActiveProductionSlot(d_active_production_slot);
  return a;
}

bool City::canAcceptMoreVectoring()
{
  return canAcceptMoreVectoring(0);
}

bool City::canAcceptMoreVectoring(Uint32 number_of_cities)
{
  //here we presume that it's one unit per city
  Uint32 num = Citylist::getInstance()->countCitiesVectoringTo(this);
  if (num + number_of_cities >= MAX_CITIES_VECTORED_TO_ONE_CITY)
    return false;
  return true;
}

bool City::changeVectorDestination(Vector<int> dest)
{
  VectoredUnitlist *vul = VectoredUnitlist::getInstance();
  setVectoring(dest);
  vul->changeDestination(this, dest);
  return true;
}

Uint32 City::countDefenders()
{
  std::vector<Stack*> defenders;
  defenders = getOwner()->getStacklist()->defendersInCity(this);

  Uint32 armies = 0;
  std::vector<Stack*>::iterator it = defenders.begin();
  for (;it != defenders.end(); it++)
    armies += (*it)->size();

  return armies;
}

void City::randomlyImproveOrDegradeArmy(ArmyProdBase *army)
{
  if (rand() % 30 == 0) //random chance of improving strength
      army->setStrength(army->getStrength() + 1);
  if (rand() % 25 == 0) //random chance of improving turns
    {
      if (army->getProduction() > 1)
	army->setProduction(army->getProduction() - 1);
    }
  if (rand() % 50 == 0) //random chance of degrading strength
    {
      if (army->getStrength() > 1)
	army->setStrength(army->getStrength() - 1);
    }
  if (rand() % 45 == 0) //random chance of improving turns
    {
      if (army->getProduction() < 5)
	army->setProduction(army->getProduction() + 1);
    }
}

bool armyCompareStrength (const ArmyProdBase *lhs, const ArmyProdBase *rhs)
{
  Uint32 lhs_strength = lhs->getStrength();
  Uint32 rhs_strength = rhs->getStrength();
  return lhs_strength < rhs_strength; 
}

void City::sortProduction()
{
  //sort them by strength
  if (getNoOfProductionBases() > 1)
    {
      std::list<ArmyProdBase*> productibles;
      unsigned int j;
      for (j = 0; j < getMaxNoOfProductionBases(); j++)
	{
	  if ((*this)[j]->getArmyProdBase())
	    productibles.push_back((*this)[j]->getArmyProdBase());
	}
      productibles.sort(armyCompareStrength);
      j = 0;
      for (std::list<ArmyProdBase*>::iterator it = productibles.begin();
	   it != productibles.end(); it++, j++)
       	(*this)[j]->setArmyProdBase(*it);
    }
  return;
}

void City::setRandomArmytypes(bool produce_allies, int likely)
{
  //remove armies any that happen to be being produced
  for (unsigned int i = 0; i < getMaxNoOfProductionBases(); i++)
    removeProductionBase(i);

  const Armysetlist* al = Armysetlist::getInstance();
  Uint32 set = d_owner->getArmyset();

  int army_type;
  int num = rand() % 10;
  if (num < 7)
    army_type = 1;
  else if (num < 9 && likely == 0)
    army_type = 0;
  else
    army_type = 1 + likely + (rand () % 11);
  ArmyProto *template_army = al->getArmy(set, army_type);
  if (!template_army || 
      (template_army->getAwardable() == true && produce_allies == false) ||
      template_army->isHero())
    {
      produceScout();
      return;
    }
  ArmyProdBase *army = new ArmyProdBase (*template_army);
  randomlyImproveOrDegradeArmy(army);
  addProductionBase(0, army);

  if (((rand() % 10) < 3 && !isCapital() && likely < 1) ||
      template_army->getAwardable())
    {
      sortProduction();
      return;
    }

  army_type += 1 + (rand() % (2 + (produce_allies ? 2 : 0)));
  template_army = al->getArmy(set, army_type);
  if (!template_army ||
      (template_army->getAwardable() == true && produce_allies == false) ||
      template_army->isHero())
    {
      sortProduction();
      return;
    }
  army = new ArmyProdBase (*template_army);
  randomlyImproveOrDegradeArmy(army);
  addProductionBase(1, army);

  if (((rand() % 10) < 4 && !isCapital() && likely < 2) ||
      template_army->getAwardable())
    {
      sortProduction();
      return;
    }

  if (army_type < 5)
    army_type += 1 + (rand() % (7 + (produce_allies ? 2 : 0)));
  else
    army_type += 1 + (rand() % (2 + (produce_allies ? 2 : 0)));
  template_army = al->getArmy(set, army_type);
  if (!template_army ||
      (template_army->getAwardable() == true && produce_allies == false) ||
      template_army->isHero())
    {
      sortProduction();
      return;
    }
  army = new ArmyProdBase (*template_army);
  randomlyImproveOrDegradeArmy(army);
  addProductionBase(2, army);

  if (((rand() % 10) < 6 && !isCapital() && likely < 3) ||
      template_army->getAwardable())
    {
      sortProduction();
      return;
    }

  army_type += 1 + (rand() % (3 + (produce_allies ? 2 : 0)));
  template_army = al->getArmy(set, army_type);
  if (!template_army ||
      (template_army->getAwardable() == true && produce_allies == false) ||
      template_army->isHero())
    {
      sortProduction();
      return;
    }
  army = new ArmyProdBase (*template_army);
  randomlyImproveOrDegradeArmy(army);
  addProductionBase(3, army);
  sortProduction();
}

// End of file
