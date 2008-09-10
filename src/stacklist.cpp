// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2004 John Farrell
// Copyright (C) 2007, 2008 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>
#include <assert.h>
#include <algorithm>

#include "stacklist.h"
#include "stack.h"
#include "city.h"
#include "path.h"
#include "playerlist.h"
#include "xmlhelper.h"
#include "Item.h"
#include "hero.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

//the static functions first
Stack* Stacklist::getObjectAt(int x, int y)
{
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        for (const_iterator it = mylist->begin(); it !=mylist->end(); it++)
            if (((*it)->getPos().x == x) && ((*it)->getPos().y == y))
                return *it;
    }

    return 0;

}

Vector<int> Stacklist::getPosition(Uint32 id)
{
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        for (const_iterator it = mylist->begin(); it !=mylist->end(); it++)
            for (Stack::const_iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
                if ((*sit)->getId() == id)
                    return (*it)->getPos();
    }

    return Vector<int>(-1,-1);
}

//We only expect one ambiguity at a time with stacks of the same player. This
//never happens except when a stack comes to halt on another stack during
//long movements
Stack* Stacklist::getAmbiguity(Stack* s)
{
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        for (const_iterator it = mylist->begin(); it != mylist->end();it++)
            if ((s->getPos().x == (*it)->getPos().x)
                && (s->getPos().y == (*it)->getPos().y) && (s != *it))
                return (*it);
    }

    return 0;
}

//search all player's stacklists to find this stack
bool Stacklist::deleteStack(Stack* s)
{
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        for (const_iterator it = mylist->begin(); it != mylist->end(); it++)
            if ((*it) == s)
                return mylist->flRemove(s);
    }
    return false;
}

void Stacklist::payUpkeep(Player *p)
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->payUpkeep(p);
}

void Stacklist::nextTurn()
{
    debug("nextTurn()");
    for (iterator it = begin(); it != end(); it++)
      {
        (*it)->nextTurn();
      }
    for (iterator it = begin(); it != end(); it++)
      for (iterator jit = begin(); jit != end(); jit++)
	if (*jit != *it)
	  if ((*jit)->getId() == (*it)->getId())
	    {
	      fprintf (stderr, "duplicate army id %d found\n", (*it)->getId());
	      exit (1);
	    }
}

vector<Stack*> Stacklist::defendersInCity(City *city)
{
    debug("defendersInCity()");

    vector<Stack*> stackvector;
    Vector<int> pos = city->getPos();

    for (unsigned int i = pos.x; i < pos.x + city->getSize(); i++)
    {
        for (unsigned int j = pos.y; j < pos.y + city->getSize(); j++)
        {
	    Stack *stack;
	    stack = city->getOwner()->getStacklist()->getOwnObjectAt(i, j);
            if (stack)
            {
	      stackvector.push_back(stack);
            }
        }
    }

    return stackvector;
}

unsigned int Stacklist::getNoOfStacks()
{
    unsigned int mysize = 0;

    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        mysize += (*pit)->getStacklist()->size();
    }

    return mysize;
}

unsigned int Stacklist::countArmies()
{
    unsigned int mysize = 0;

    for (iterator it = begin(); it != end(); it++)
      mysize += (*it)->size();

    return mysize;
}

unsigned int Stacklist::countHeroes()
{
  std::vector<Uint32> heroes;
  getHeroes(heroes);
  return heroes.size();
}

Stacklist::Stacklist()
    :d_activestack(0)
{
}

Stacklist::Stacklist(Stacklist *stacklist)
    :d_activestack(0)
{
    for (iterator it = stacklist->begin(); it != stacklist->end(); it++)
    {
        push_back(*it);
    }
}

Stacklist::Stacklist(XML_Helper* helper)
    :d_activestack(0)
{
    helper->registerTag("stack", sigc::mem_fun((*this), &Stacklist::load));

    load("stacklist", helper);
}

Stacklist::~Stacklist()
{
}

Stack* Stacklist::getNextMovable()
{
    Player *player = Playerlist::getInstance()->getActiveplayer();
    
    iterator it = begin();
    
    //first, if we already have an active stack, loop through until we meet it
    if (d_activestack)
    {
        for (; *it != d_activestack; it++);
        it++;   //we want to start with the next stack :)
    }

    //continue looping until we meet the next not defending stack of this player
    for (; it != end(); ++it)
    {
	Stack *s = *it;
        if (s->getOwner() == player && !s->getDefending() && 
	    !s->getParked() && s->canMove())
	    return s;
    }
    
    //still not found a stack? Then start looping from the beginning until we
    //meet the activestack again. If there is no activestack, we have already
    //looped through the whole list, so stop here
    if (!d_activestack)
        return 0;

    for (it = begin(); *it != d_activestack; ++it)
    {
	Stack *s = *it;
        if (s->getOwner() == player && !s->getDefending() &&
	    !s->getParked() && s->canMove())
	    return s;
    }

    //still there? well, then we have only one stack left.
    if (d_activestack->getDefending() || d_activestack->getParked())
	return 0;
    else
	return d_activestack;
}

Stack *Stacklist::getStackById(Uint32 id)
{
  for (Stacklist::iterator i = begin(), e = end(); i != e; ++i)
    if ((*i)->getId() == id)
      return *i;
    
  return 0;
}

Stack *Stacklist::getArmyStackById(Uint32 army)
{
  for (Stacklist::iterator i = begin(), e = end(); i != e; ++i)
    if ((*i)->getArmyById(army))
      return *i;
  
  return 0;
}

void Stacklist::flClear()
{
    d_activestack = 0;

    for (iterator it = begin(); it != end(); it++)
    {
        delete (*it);
    }

    clear();
}

Stacklist::iterator Stacklist::flErase(iterator object)
{
    if (d_activestack == (*object))
        d_activestack = 0;
    delete (*object);
    return erase(object);
}

bool Stacklist::flRemove(Stack* object)
{
    debug("removing stack with id " << object->getId() << endl);
    iterator stackit = find(begin(), end(), object);
    if (stackit != end())
    {
        if (d_activestack == object)
            d_activestack = 0;
	assert (object->getId() == (*stackit)->getId());
	printf ("about to delete stack with id %d\n", object->getId());
        delete object;
        erase(stackit);
        return true;
    }
    return false;
}

bool Stacklist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("stacklist");
    if (d_activestack)
    {
        retval &= helper->saveData("active", d_activestack->getId());
    }
    else
    {
        retval &= helper->saveData("active", 0);
    }

    //save stacks
    for (const_iterator it = begin(); it != end(); it++)
        retval &= (*it)->save(helper);

    retval &= helper->closeTag();

    return retval;
}

bool Stacklist::enoughMoves() const 
{
    for (const_iterator it = begin(); it != end(); it++)
    {
        Stack* s = *it;
        if (!s->getPath()->empty() && s->enoughMoves())
            return true;
    }

    return false;
}

bool Stacklist::load(string tag, XML_Helper* helper)
{
    static Uint32 active = 0;
    
    if (tag == "stacklist")
    {
        helper->getData(active, "active");
        return true;
    }

    if (tag == "stack")
    {
        Stack* s = new Stack(helper);
        if ((active != 0) && (s->getId() == active))
        {
            d_activestack = s;
        }

        push_back(s);
        return true;
    }

    return false;
}

Stack* Stacklist::getOwnObjectAt(int x, int y)
{
  for (const_iterator it = begin(); it != end(); it++)
    if (((*it)->getPos().x == x) && ((*it)->getPos().y == y))
      return *it;
  return 0;
}

std::list<Hero*> Stacklist::getTopHeroes(unsigned int num)
{
  std::list<Hero*> result;
  std::vector<Uint32> hero_ids;

  if (num == 0)
    return result;

  //load up a stack with all the heroes we have
  Stack *stack = Stack::createNonUniqueStack(NULL, Vector<int>(1,1));
  if (!stack)
    return result;

  getHeroes(hero_ids);
  for (std::vector<Uint32>::iterator it = hero_ids.begin(); 
       it != hero_ids.end(); it++)
    {
      Stack *hero_stack = getArmyStackById(*it);
      Hero *hero = dynamic_cast<Hero*>(hero_stack->getArmyById(*it));
      stack->push_back(new Hero(*hero));
    }
  //now we yank out the top NUM heroes and put them in our list.
  if (num == -1)
    num = hero_ids.size();
  for (unsigned int i = 0 ; i < num; i++)
    {
      Hero *hero = dynamic_cast<Hero*>(stack->getStrongestHero());
      if (hero)
	{
	  result.push_back(hero);
	  stack->erase(find (stack->begin(), stack->end(), hero));
	  if (stack->size() == 0)
	    break;
	}
    }
  stack->flClear();
  delete stack;
  return result;
}

void Stacklist::getHeroes(std::vector<Uint32>& dst)
{
  for (Stacklist::iterator it = begin(); it != end(); it++)
    (*it)->getHeroes(dst);
}

void Stacklist::collectTaxes(Player *p, Uint32 num_cities)
{
  std::vector<Uint32> hero_ids;
  getHeroes(hero_ids);

  //now let's see if we have any items that give us gold per city
  for (std::vector<Uint32>::iterator it = hero_ids.begin(); 
       it != hero_ids.end(); it++)
    {
      Stack *stack = getArmyStackById(*it);
      Army *army = stack->getArmyById(*it);
      Hero *hero = static_cast<Hero*>(army);
      std::list<Item*> backpack = hero->getBackpack();
      std::list<Item*>::const_iterator item;
      for (item = backpack.begin(); item != backpack.end(); item++)
	{
	  if ((*item)->getBonus(Item::ADD2GOLDPERCITY))
	    p->addGold(2 * num_cities);
	    if ((*item)->getBonus(Item::ADD3GOLDPERCITY))
	      p->addGold(3 * num_cities);
	    if ((*item)->getBonus(Item::ADD4GOLDPERCITY))
	      p->addGold(4 * num_cities);
	    if ((*item)->getBonus(Item::ADD5GOLDPERCITY))
	      p->addGold(5 * num_cities);
	  }
    }
}

// End of file
