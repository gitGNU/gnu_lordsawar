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

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include "city.h"
#include "path.h"
#include "army.h"
#include "hero.h"
#include "stacklist.h"
#include "stack.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "citylist.h"
#include "GameMap.h"
#include "vectoredunitlist.h"
#include "action.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

City::City(Vector<int> pos, string name, Uint32 gold)
    :Location(name, pos, 2), d_player(0), d_numbasic(4),
     d_production(-1),
     d_duration(-1), d_gold(gold),
     d_defense_level(1), d_burnt(false), d_vectoring(false),
     d_capital(false), d_capital_owner(0)

{
    // Initialise armytypes
    for (int i = 0; i < 4; i++)
        d_basicprod[i] = NULL; 

    // set the tiles to city type
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            GameMap::getInstance()->getTile(d_pos.x + i, d_pos.y + j)
                                  ->setBuilding(Maptile::CITY);
    d_vector.x=-1;
    d_vector.y=-1; 
}

City::City(XML_Helper* helper)
    :Location(helper, 2), d_numbasic(4)
{
  //note: the armies get loaded in citylist

    //initialize the city
    Uint32 ui;

    for (int i = 0; i < 4; i++)
      d_basicprod[i] = NULL;

    helper->getData(ui, "owner");
    d_player = Playerlist::getInstance()->getPlayer(ui);

    helper->getData(d_defense_level, "defense");
    
    helper->getData(d_production, "production");
    helper->getData(d_duration, "duration");
    helper->getData(d_gold, "gold");
    helper->getData(d_burnt, "burnt");
    helper->getData(d_capital, "capital");
    if (d_capital)
      {
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
            GameMap::getInstance()->getTile(d_pos.x+i, d_pos.y+j)
                                  ->setBuilding(Maptile::CITY);
    
}

City::City(const City& c)
    :Location(c), d_player(c.d_player), d_numbasic(c.d_numbasic),
    d_production(c.d_production), d_duration(c.d_duration), d_gold(c.d_gold), 
    d_defense_level(c.d_defense_level), d_burnt(c.d_burnt),
    d_vectoring(c.d_vectoring),d_vector(c.d_vector), d_capital(c.d_capital), 
    d_capital_owner(c.d_capital_owner)
{
    for (int i = 0; i < 4; i++)
      {
	if (c.d_basicprod[i] != NULL)
	  d_basicprod[i] = new Army(*c.d_basicprod[i]);
	else
	  d_basicprod[i] = NULL;
      }
}

City::~City()
{
    for (int i = 0; i < 4; i++)
      if (d_basicprod[i])
	delete d_basicprod[i];
}

bool City::save(XML_Helper* helper) const
{
    bool retval = true;

    stringstream svect;

    svect << d_vector.x << " " << d_vector.y;

    retval &= helper->openTag("city");
    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("x", d_pos.x);
    retval &= helper->saveData("y", d_pos.y);
    retval &= helper->saveData("owner", d_player->getId());
    retval &= helper->saveData("defense", d_defense_level);
    retval &= helper->saveData("production", d_production);
    retval &= helper->saveData("duration", d_duration);
    retval &= helper->saveData("gold", d_gold);
    retval &= helper->saveData("burnt", d_burnt);
    retval &= helper->saveData("capital", d_capital);
    if (d_capital)
      retval &= helper->saveData("capital_owner", d_capital_owner->getId());
    retval &= helper->saveData("vectoring", svect.str());

    for (int i = 0; i < d_numbasic; i++)
      {
	if (d_basicprod[i] == NULL)
	  continue;
	retval &= d_basicprod[i]->save(helper, Army::PRODUCTION_BASE);
      }
    retval &= helper->closeTag();
    return retval;
}

int City::getNoOfBasicProd()
{
  int i, max = 0;
  for (i = 0; i < this->getMaxNoOfBasicProd(); i++)
    {
      if (this->getArmy(i))
        max++;
    }
  return max;
}

void City::setProduction(int index)
{
    if (index == -1)
    {
        d_production = index;
        d_duration = -1;
        return;
    }

    // return on wrong data
    if (((index >= d_numbasic)) || (index >= 0 && getArmytype(index) == -1))
        return;

    d_production = index;
    const Army* a = getArmy(index);

    // set the duration to produce this armytype
    if (a)
        d_duration = a->getProduction(); 
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


int City::getFreeBasicSlot() 
{
     int index=-1;

     debug(d_name << " BASIC SLOTS=" << d_numbasic) 
     for (int i = 0; i < d_numbasic; i++)
     {
         debug(d_name << " Index Value=" << d_basicprod[i])
         if (d_basicprod[i] == NULL)
         {
             index=i;
             return i;
         }         
     }

     return index;
}

bool City::isAlreadyBought(const Army * army)
{
    int max=-1;

    max=d_numbasic;
 
    for (int i = 0; i < max; i++)
    {
        
        debug("Value of production slot=" << d_basicprod[i] << " of index " << i << " max index=" << max)
        if (d_basicprod[i] != NULL)
	{
	    int type1 = army->getType();
	    int type2 = d_basicprod[i]->getType();
	    string name = d_basicprod[i]->getName();
           
            debug("army in list " << type2 << " - " << name)          
	    debug("army in city " << type1 << " - " << army->getName())          
 
	    if(type1 == type2) return true;
	}
        else debug("basic prod was nil index=" << i);
    }
 
    return false;
}

bool City::addBasicProd(int index, Army *army)
{
    if (index >= d_numbasic)
        return false;

    army->setPlayer(this->getPlayer());

    if (index < 0)
    {
        // try to find an unoccupied production slot. If there is none, pick 
        // the slot with the highest index.
        for (int i = 0; i < d_numbasic; i++)
            if (d_basicprod[i] == NULL)
            {
                index = i;
                break;
            }

        if (index < 0)
        {
            index = d_numbasic - 1;
        }
    }
    
    bool restore_production = false;
    if (d_production == index)
      restore_production = true;
    removeBasicProd(index);
    d_basicprod[index] = army;
    if (restore_production)
      setProduction(index);
    return true;
}

void City::removeBasicProd(int index)
{
    if ((index < 0) || (index > 3))
        return;

    if (d_basicprod[index])
      delete d_basicprod[index];
    d_basicprod[index] = NULL;

    if (d_production == index)
        setProduction(-1);
}

void City::conquer(Player* newowner)
{
    d_player = newowner;

    // remove vectoring info 
    setVectoring(Vector<int>(-1,-1));

    deFog(newowner);

    VectoredUnitlist *vul = VectoredUnitlist::getInstance();
    vul->removeVectoredUnitsComingFrom(d_pos);
    vul->removeVectoredUnitsGoingTo(d_pos);
}


void City::randomlyImproveOrDegradeArmy(Army *army)
{
  if (rand() % 30 == 0) //random chance of improving strength
    {
      army->setStat(Army::STRENGTH, 
		    army->getStat(Army::STRENGTH, false) + 1);
    }
  if (rand() % 25 == 0) //random chance of improving turns
    {
      if (army->getProduction() > 1)
	army->setProduction(army->getProduction() - 1);
    }
  if (rand() % 50 == 0) //random chance of degrading strength
    {
      if (army->getStat(Army::STRENGTH, false) > 1)
	army->setStat(Army::STRENGTH, 
		      army->getStat(Army::STRENGTH, false) - 1);
    }
  if (rand() % 45 == 0) //random chance of improving turns
    {
      if (army->getProduction() < 5)
	army->setProduction(army->getProduction() + 1);
    }
}

void City::setRandomArmytypes(bool produce_allies)
{

  const Armysetlist* al = Armysetlist::getInstance();
  Uint32 set = getPlayer()->getArmyset();
  int army_type;
  int num = rand() % 10;
  if (num < 7)
    army_type = 1;
  else if (num < 9)
    army_type = 0;
  else
    army_type = 1 + (rand () % 11);
  Army *template_army = al->getArmy(set, army_type);
  if (!template_army || 
      (template_army->getAwardable() == true && produce_allies == false) ||
      template_army->isHero())
    {
      produceScout();
      return;
    }
  Army *army = new Army (*template_army);
  randomlyImproveOrDegradeArmy(army);
  addBasicProd(0, army);

  if ((rand() % 10) < 3 && !isCapital())
    return;

  army_type += 1 + (rand() % (2 + (produce_allies ? 2 : 0)));
  template_army = al->getArmy(set, army_type);
  if (!template_army ||
      (template_army->getAwardable() == true && produce_allies == false) ||
      template_army->isHero())
    return;
  army = new Army (*template_army);
  randomlyImproveOrDegradeArmy(army);
  addBasicProd(1, army);

  if ((rand() % 10) < 4 && !isCapital())
    return;

  if (army_type < 5)
    army_type += 1 + (rand() % (7 + (produce_allies ? 2 : 0)));
  else
    army_type += 1 + (rand() % (2 + (produce_allies ? 2 : 0)));
  template_army = al->getArmy(set, army_type);
  if (!template_army ||
      (template_army->getAwardable() == true && produce_allies == false) ||
      template_army->isHero())
    return;
  army = new Army (*template_army);
  randomlyImproveOrDegradeArmy(army);
  addBasicProd(2, army);

  if ((rand() % 10) < 6 && !isCapital())
    return;

  army_type += 1 + (rand() % (3 + (produce_allies ? 2 : 0)));
  template_army = al->getArmy(set, army_type);
  if (!template_army ||
      (template_army->getAwardable() == true && produce_allies == false) ||
      template_army->isHero())
    return;
  army = new Army (*template_army);
  randomlyImproveOrDegradeArmy(army);
  addBasicProd(3, army);

}

void City::produceStrongestArmy()
{
  debug("produceStrongestArmy()");

  Stack* stack = getFreeStack(d_player);
  if (stack)
    {
      unsigned int max_strength = 0;
      int strong_idx = -1;
      for (int i = 0; i < 4; i++)
	{
	  if (d_basicprod[i] == NULL)
	    continue;
	  if (getArmy(i)->getStat(Army::STRENGTH,false) > max_strength)
	    {
	      strong_idx = i;
	      max_strength = getArmy(i)->getStat(Army::STRENGTH,false);
	    }
	}
      if (strong_idx == -1)
	return;

      int savep = d_production;
      setProduction(strong_idx);
      produceArmy();
      setProduction(savep);
      return;
    }
}

void City::produceScout()
{
  const Armysetlist* al = Armysetlist::getInstance();
  Uint32 set = getPlayer()->getArmyset();
  Army *scout = al->getArmy(set, 0);
  Army *a = new Army(*scout, d_player);
  GameMap::getInstance()->addArmy(this, a);

}

void City::produceWeakestArmy()
{
  debug("produceWeakestArmy()");

  Stack* stack = getFreeStack(d_player);
  if (stack)
    {
      unsigned int min_strength = 100;
      int weak_idx = -1;
      for (int i = 0; i < 4; i++)
	{
	  if (d_basicprod[i] == NULL)
	    continue;
	  if (getArmy(i)->getStat(Army::STRENGTH,false) < min_strength)
	    {
	      weak_idx = i;
	      min_strength = getArmy(i)->getStat(Army::STRENGTH,false);
	    }
	}
      if (weak_idx == -1)
	return;

      int savep = d_production;
      setProduction(weak_idx);
      produceArmy();
      setProduction(savep);
      return;
    }
}

void City::nextTurn()
{
  if (d_burnt)
    return;

  // check if an army should be produced
  if (d_production >= 0 && --d_duration == 0) 
    {
      if (d_player->getGold() <= 0)
	{
	  //dont make or vector the unit
	  //and also stop production
	  setProduction(-1);
	  d_vectoring = false;
	  d_vector.x = -1;
	  d_vector.y = -1;
	  return;
	}

      Action_Produce *item = new Action_Produce();
      // vector the army to the new spot
      if (d_vectoring)
	{
	  VectoredUnitlist *vul = VectoredUnitlist::getInstance();
	  VectoredUnit *v = new VectoredUnit(d_pos, d_vector, 
					     new Army(*(d_basicprod[d_production])),
					     MAX_TURNS_FOR_VECTORING, getPlayer());
	  vul->push_back(v);
	  setProduction(d_production);
	  item->fillData(getArmytype(d_production), this, true);
	}
      else //or make it here
	{
	  Army *a = produceArmy();
	  item->fillData(a->getType(), this, false);
	}
      //FIXME: a cookie goes to the person who can figure out how
      //to get this action into the realplayer class.
      getPlayer()->getActionlist()->push_back(item);
    }
}

bool City::hasProduction(int type, Uint32 set) const
{
  if (type < 0)
    return false;
  for (int i = 0; i < d_numbasic; i++)
    {
      if (d_basicprod[i] == NULL)
	continue;
      if (d_basicprod[i]->getType() == (unsigned int) type)
	return true;
    }

  return false;
}

int City::getArmytype(int slot) const
{
  if (slot < 0)
    return -1;

  if (slot >= d_numbasic)
    return -1;
  if (d_basicprod[slot] == NULL)
    return -1;
  return d_basicprod[slot]->getType();
}

const Army* City::getArmy(int slot) const
{
  if (getArmytype(slot) == -1)
    return 0;
  return d_basicprod[slot];
}

bool City::isFriend(Player* player) const
{
  return (d_player == player);
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
  d_vector.x=p.x; 
  d_vector.y=p.y;
  d_vectoring=true; 

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
  if (d_production == -1)
    return NULL;

  debug("produce_army()\n");

  // do not produce an army if the player has no gold.
  // unless it's the neutrals
  if (d_player != Playerlist::getInstance()->getNeutral() && 
      d_player->getGold() < 0) 
    return NULL;

  Army *a = new Army(*(getArmy(d_production)), d_player);
  GameMap::getInstance()->addArmy(this, a);

  if (d_player == Playerlist::getInstance()->getNeutral()) 
    {
      //we're an active neutral city
      //check to see if we've made 5 or not.
      //stop producing if we've made 5 armies in our neutral city
      Stack *s = d_player->getStacklist()->getObjectAt(d_pos);
      if (!s)
	setProduction(d_production);
      else if (s->size() < 5)
	setProduction(d_production);
      else
	setProduction(-1);
    }
  else // start producing next army of same type
    setProduction(d_production);
  return a;
}

bool City::canAcceptVectoredUnit()
{
  VectoredUnitlist *vul = VectoredUnitlist::getInstance();
  if (vul->getNumberOfVectoredUnitsGoingTo(d_pos) >= 
      MAX_ARMIES_VECTORED_TO_ONE_CITY)
    return false;
  return true;
}
// End of file
