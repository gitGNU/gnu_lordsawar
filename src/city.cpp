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
#include "playerlist.h"
#include "armysetlist.h"
#include "GameMap.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

City::City(PG_Point pos, string name, Uint32 gold)
    :Location(name, pos, 2), d_player(0), d_numbasic(4),
     d_production(-1),
     d_duration(-1), d_gold(gold),
     d_defense_level(1), d_burnt(false), d_vectoring(false),
     d_capital(false)

{
    // Initialise armytypes
    for (int i = 0; i < 4; i++)
        d_basicprod[i] = -1; 

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
    //initialize the city
    Uint32 ui;
    
    helper->getData(ui, "owner");
    d_player = Playerlist::getInstance()->getPlayer(ui);

    //get production types
    istringstream sbase, svect;
    string s;

    helper->getData(s, "basic_prod");
    sbase.str(s);
    for (int i = 0; i < 4; i++)
        sbase >>d_basicprod[i];

    helper->getData(d_defense_level, "defense");
    d_numbasic = 4;
    
    helper->getData(d_production, "production");
    helper->getData(d_duration, "duration");
    helper->getData(d_gold, "gold");
    helper->getData(d_burnt, "burnt");
    helper->getData(d_capital, "capital");

    helper->getData(s, "vectoring");
     svect.str(s);
    svect >> d_vector.x;
    svect >> d_vector.y;

    if (d_vector.x!=-1 && d_vector.y!=-1) 
        d_vectoring=true;
    else 
      d_vectoring=false;

    //mark the positions on the map as being occupied by a city
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            GameMap::getInstance()->getTile(d_pos.x+i, d_pos.y+j)
                                  ->setBuilding(Maptile::CITY);
}

City::City(const City& c)
    :Location(c), d_player(c.d_player), d_numbasic(c.d_numbasic),
    d_production(c.d_production), 
    d_duration(c.d_duration), d_gold(c.d_gold), d_defense_level(c.d_defense_level),
     d_burnt(c.d_burnt),d_vectoring(c.d_vectoring),d_vector(c.d_vector),
     d_capital(c.d_capital)
{
    for (int i = 0; i < 4; i++)
        d_basicprod[i] = c.d_basicprod[i];

}

City::~City()
{
}

bool City::save(XML_Helper* helper) const
{
    bool retval = true;

    stringstream sbase, svect;
    for (int i = 0; i < 4; i++)
        sbase << d_basicprod[i] <<" ";

    svect << d_vector.x << " " << d_vector.y;

    retval &= helper->openTag("city");
    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("x", d_pos.x);
    retval &= helper->saveData("y", d_pos.y);
    retval &= helper->saveData("owner", d_player->getId());
    retval &= helper->saveData("basic_prod", sbase.str());
    retval &= helper->saveData("defense", d_defense_level);
    retval &= helper->saveData("production", d_production);
    retval &= helper->saveData("duration", d_duration);
    retval &= helper->saveData("gold", d_gold);
    retval &= helper->saveData("burnt", d_burnt);
    retval &= helper->saveData("capital", d_capital);
    retval &= helper->saveData("vectoring", svect.str());
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
    d_gold = static_cast<Uint32>(d_gold*1.8);

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
    d_gold = static_cast<Uint32>(d_gold*0.66);

    // remove obsolete productions
    removeBasicProd(d_numbasic);
    
    return true;
}


int City::getFreeBasicSlot() 
{
     int index=-1;

     debug(d_name << " BASIC SLOTS=" << d_numbasic) 
     for (int i = 0; i < d_numbasic; i++)
     {
         debug(d_name << " Index Value=" << d_basicprod[i])
         if (d_basicprod[i] == -1)
         {
             index=i;
             return i;
         }         
     }

     // TODO: here the AI should choose more wisely which production is to be replaced
     // for now we return no index
     return index;
}

bool City::isAlreadyBought(const Army * army)
{
    int max=-1;

    max=d_numbasic;
 
    for (int i = 0; i < max; i++)
    {
        
        debug("Value of production slot=" << d_basicprod[i] << " of index " << i << " max index=" << max)
        if (d_basicprod[i]!=-1)
	{
	    int type1=army->getType();
	    int type2=(Armysetlist::getInstance()->getArmy(Armysetlist::getInstance()->getStandardId(), d_basicprod[i]))->getType();
	    string name=(Armysetlist::getInstance()->getArmy(Armysetlist::getInstance()->getStandardId(), d_basicprod[i]))->getName();
           
            debug("army in list " << type2 << " - " << name)          
	    debug("army in city " << type1 << " - " << army->getName())          
 
	    if(type1==type2) return true;
	}
        else debug("basic prod was -1 index=" << i);
    }
 
    return false;
}

bool City::addBasicProd(int index, int armytype)
{
    const Armysetlist* al = Armysetlist::getInstance();
    int size = al->getSize(al->getStandardId());
    
    if ((index >= d_numbasic) || (armytype >= size))
        return false;

    if (index < 0)
    {
        // try to find an unoccupied production slot. If there is none, pick 
        // the slot with the highest index.
        for (int i = 0; i < d_numbasic; i++)
            if (d_basicprod[i] == -1)
            {
                index = i;
                break;
            }

        if (index < 0)
        {
            index = d_numbasic - 1;
        }
    }
    
    removeBasicProd(index);
    d_basicprod[index] = armytype;
    return true;
}

void City::removeBasicProd(int index)
{
    if ((index < 0) || (index > 3))
        return;

    d_basicprod[index] = -1;

    if (d_production == index)
        setProduction(-1);
}

void City::conquer(Player* newowner)
{
    d_player = newowner;

    // remove vectoring info (the new player can propably not use it anyway)
    setVectoring(PG_Point(-1,-1));
}

void City::setRandomArmytypes()
{
    //always set the lowest armytype
    addBasicProd(0, 0);

    //add another armytype if the dice are lucky
    int random_nr = rand() % 3;
    if (random_nr == 1) 
        addBasicProd(1, 1 + rand() % 4);
}

void City::produceLargestArmy()
{
    debug("produceLargestArmy()");

    // We assume that the largest army is the one with the highest index
    // This is a possible assumption as this function is solely used to
    // populate the cities during map generation.

    Stack* stack = getFreeStack();
    if (stack)
    {
      for (int i = 3; i >= 0; i--)
          if (getArmytype(i) != -1)
          {
              int savep = d_production;
                
              setProduction(i);
              produceArmy();
              setProduction(savep);
              return;
          }
    }
}

void City::addHero(Hero* hero) const
{
    Stack* stack = getFreeStack();

    // No stack found so create one
    if (!stack)
    {
        stack = new Stack(d_player, d_pos);
        d_player->addStack(stack);
    }

    // add hero to stack
    stack->push_front(hero);
}

void City::nextTurn()
{
    if (d_burnt)
        return;

    // check if an army should be produced
    if (d_production >= 0 && --d_duration == 0) 
    {
        produceArmy();
    }
}

bool City::hasProduction(int type, Uint32 set) const
{
    if (set == Armysetlist::getInstance()->getStandardId())
        for (int i = 0; i < d_numbasic; i++)
            if (d_basicprod[i] == type)
                return true;

    return false;
}

int City::getArmytype(int slot) const
{
    if (slot < 0)
        return -1;
    
    if (slot >= d_numbasic)
        return -1;
    return d_basicprod[slot];
}

const Army* City::getArmy(int slot) const
{
    if (getArmytype(slot) == -1)
        return 0;

    const Armysetlist* al = Armysetlist::getInstance();
    
    return al->getArmy(al->getStandardId(), d_basicprod[slot]);
}

bool City::isFriend(Player* player) const
{
    return (d_player == player);
}

Stack* City::getFreeStack() const
{
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
        {
            Stack* stack = Stacklist::getObjectAt(d_pos.x + j, d_pos.y+ i);

            // No stack found or vectoring enabled so create one
            debug("Vectoring was " << d_vectoring<< "!!!!!!!!!!!!!!!!!!!")

            if (stack == 0 || d_vectoring)
            {
                PG_Point temp;
                temp.x = d_pos.x + j;
                temp.y = d_pos.y + i;
                stack = new Stack(d_player, temp);
                d_player->addStack(stack);
                return stack;
            }
            else if (stack->size() < 8) return stack;
        }

    return 0;
}

void City::setVectoring(PG_Point p) 
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

void City::produceArmy()
{
    debug("produce_army()\n");

    // do not produce an army if the player has no gold.
    if ((d_player->getGold() < 0) || (d_production == -1))
        return;

    Stack* stack = getFreeStack();

    if (stack)
    {
        // add produced army to stack
        const Armysetlist* al = Armysetlist::getInstance();
        Uint32 set;
        int index;
        
        set = al->getStandardId();
        index = d_basicprod[d_production];
        
        stack->push_back(new Army(*(al->getArmy(set, index)), d_player));

        if (d_vectoring && d_vector.x!=-1 && d_vector.y!=-1)
            stack->getPath()->calculate(stack, d_vector);
    }
    
    // start producing next army of same type
    setProduction(d_production);
}

// End of file
