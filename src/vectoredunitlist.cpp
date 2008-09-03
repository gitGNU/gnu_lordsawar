//  Copyright (C) 2007, 2008 Ben Asselstine
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

#include "vectoredunitlist.h"
#include "vectoredunit.h"
#include "citylist.h"
#include "city.h"
#include "xmlhelper.h"
#include "player.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

VectoredUnitlist* VectoredUnitlist::s_instance = 0;

VectoredUnitlist* VectoredUnitlist::getInstance()
{
    if (s_instance == 0)
        s_instance = new VectoredUnitlist();

    return s_instance;
}

VectoredUnitlist* VectoredUnitlist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new VectoredUnitlist(helper);
    return s_instance;
}

void VectoredUnitlist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

VectoredUnitlist::VectoredUnitlist()
{
}

VectoredUnitlist::~VectoredUnitlist()
{
  for (VectoredUnitlist::iterator it = begin(); it != end(); it++)
    delete *it;
}

VectoredUnitlist::VectoredUnitlist(XML_Helper* helper)
{
    helper->registerTag("vectoredunit", sigc::mem_fun(this, &VectoredUnitlist::load));
    helper->registerTag("armyprodbase", sigc::mem_fun(this, &VectoredUnitlist::load));
}

bool VectoredUnitlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("vectoredunitlist");

    for (VectoredUnitlist::const_iterator it = begin(); it != end(); it++)
        retval &= (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool VectoredUnitlist::load(std::string tag, XML_Helper* helper)
{
  if (tag == "vectoredunit")
    {
      VectoredUnit *r = new VectoredUnit(helper);
      push_back(r);
      return true;
    }

  if (tag == "armyprodbase")
    {
      VectoredUnit *vectoredunit = back();
      vectoredunit->setArmy(new ArmyProdBase (helper));
      return true;
    }

    return false;
}

void VectoredUnitlist::nextTurn(Player* p)
{
  Citylist *cl = Citylist::getInstance();
  debug("next_turn(" <<p->getName() <<")");

  for (VectoredUnitlist::iterator it = begin(); it != end(); it++)
    {
      City *c = cl->getObjectAt((*it)->getPos());
      if (c)
	{
	  if (c->getOwner() == p)
	    (*it)->nextTurn();
	}
      else //must be a standard
	(*it)->nextTurn();
    }

  for (VectoredUnitlist::iterator it = begin(); it != end();)
    {
      if ((*it)->getDuration() <= 0)
	{
	  it = flErase(it);
	  continue;
	}
      it++;
    }
}

bool VectoredUnitlist::removeVectoredUnitsGoingTo(Vector<int> pos)
{
  bool found = false;
  for (VectoredUnitlist::iterator it = begin(); it != end();)
    {
      if ((*it)->getDestination() == pos)
	{
	  found = true;
	  it = flErase(it);
	  continue;
	}
      it++;
    }
  return found;
}

bool VectoredUnitlist::removeVectoredUnitsComingFrom(Vector<int> pos)
{
  bool found = false;
  for (VectoredUnitlist::iterator it = begin(); it != end();)
    {
      if ((*it)->getPos() == pos)
	{
	  found = true;
	  it = flErase(it);
	  continue;
	}
      it++;
    }
  return found;
}

bool VectoredUnitlist::removeVectoredUnitsGoingTo(City *c)
{
  int count = 0;
  int counter = 0;
  bool found = false;
  Citylist *cl = Citylist::getInstance();
  for (VectoredUnitlist::iterator it = begin(); it != end();)
    {
      if (c->contains((*it)->getDestination()))
	{
	  found = true;
	  it = flErase(it);
	  counter++;
	  continue;
	}
      it++;
    }
  if (counter != count)
    {
      counter = 0;
  for (VectoredUnitlist::iterator it = begin(); it != end();)
    {
      if (c->contains((*it)->getDestination()))
	{
	  printf ("crap!  we found another one on the second try\n");
	  found = true;
	  it = flErase(it);
	  counter++;
	  continue;
	}
      it++;
    }
  printf ("got another %d\n", counter);
  if (counter)
    exit(0);
    }
  return found;
}

bool VectoredUnitlist::removeVectoredUnitsComingFrom(City *c)
{
  bool found = false;
  for (VectoredUnitlist::iterator it = begin(); it != end();)
    {
      if (c->contains((*it)->getPos()))
	{
	  found = true;
	  it = flErase(it);
	  continue;
	}
      it++;
    }
  return found;
}

void VectoredUnitlist::getVectoredUnitsGoingTo(City *c, std::list<VectoredUnit*>& vectored)
{
  for (VectoredUnitlist::iterator it = begin(); it != end(); it++)
    {
      if (c->contains((*it)->getDestination()))
	{
	  vectored.push_back(*it);
	}
    }
}
void VectoredUnitlist::getVectoredUnitsGoingTo(Vector<int> pos, std::list<VectoredUnit*>& vectored)
{
  for (VectoredUnitlist::iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getDestination() == pos)
	{
	  vectored.push_back(*it);
	}
    }
}
void VectoredUnitlist::getVectoredUnitsComingFrom(Vector<int> pos, std::list<VectoredUnit*>& vectored)
{
  for (VectoredUnitlist::iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getPos() == pos)
	{
	  vectored.push_back(*it);
	}
    }
}

Uint32 VectoredUnitlist::getNumberOfVectoredUnitsGoingTo(Vector<int> pos)
{
  Uint32 count = 0;
  for (VectoredUnitlist::iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getDestination() == pos)
	{
	  count++;
	}
    }
  return count;
}

void VectoredUnitlist::changeDestination(City *c, Vector<int> new_dest)
{
  for (VectoredUnitlist::iterator it = begin(); it != end(); it++)
    {
      if (c->contains((*it)->getPos()))
	(*it)->setDestination(new_dest);
    }
}

VectoredUnitlist::iterator VectoredUnitlist::flErase(iterator object)
{
  delete(*object);
  return erase (object);
}
