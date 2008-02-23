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
#include "citylist.h"
#include "city.h"
#include "xmlhelper.h"

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
    for (iterator it = begin(); it != end(); it++)
      delete *it;
}

VectoredUnitlist::VectoredUnitlist(XML_Helper* helper)
{
    helper->registerTag("vectoredunit", sigc::mem_fun(this, &VectoredUnitlist::load));
    helper->registerTag("army", sigc::mem_fun(this, &VectoredUnitlist::load));
}

bool VectoredUnitlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("vectoredunitlist");

    for (const_iterator it = begin(); it != end(); it++)
        retval &= (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool VectoredUnitlist::load(std::string tag, XML_Helper* helper)
{
  if (tag == "army")
    {
      VectoredUnitlist::iterator it = end();
      it--;
      VectoredUnit *vectoredunit = *it;
      vectoredunit->setArmy(new Army (helper, Army::PRODUCTION_BASE));
      return true;
    }
    
  if (tag == "vectoredunit")
    {
      VectoredUnit *r = new VectoredUnit(helper);
      push_back(r);
      return true;
    }

    return false;
}

void VectoredUnitlist::nextTurn(Player* p)
{
  Citylist *cl = Citylist::getInstance();
  City *c;
  debug("next_turn(" <<p->getName() <<")");
  bool advance;

  iterator it = begin();
  while (it != end())
    {
      advance = true;
      c = cl->getObjectAt((*it)->getPos());
      if (c)
	{
	  if (c->getOwner() == p)
	    {
	      if ((*it)->nextTurn() == true)
		{
		  iterator nextit = it;
		  nextit++;
		  erase(it);
		  advance = false;
		  it = nextit; //advance here instead of down there
		}
	    }
	}
      else //must be a standard
	{
	      if ((*it)->nextTurn() == true)
		{
		  iterator nextit = it;
		  nextit++;
		  erase(it);
		  advance = false;
		  it = nextit; //advance here instead of down there
		}
	}
      if (advance)
	++it;
    }

}

void VectoredUnitlist::removeVectoredUnitsGoingTo(Vector<int> pos)
{
  iterator it = begin();
  iterator nextit = it;
  nextit++;
  for (; nextit != end(); it++, nextit++)
    {
      if ((*it)->getDestination() == pos)
	{
	  erase(it);
	  it = nextit;
	  nextit++;
	}
    }
}

void VectoredUnitlist::removeVectoredUnitsComingFrom(Vector<int> pos)
{
  iterator it = begin();
  iterator nextit = it;
  nextit++;
  for (; nextit != end(); it++, nextit++)
    {
      if ((*it)->getPos() == pos)
	{
	  erase(it);
	  it = nextit;
	  nextit++;
	}
    }
}
void VectoredUnitlist::removeVectoredUnitsGoingTo(City *c)
{
  iterator it = begin();
  iterator nextit = it;
  nextit++;
	  
  for (; nextit != end(); it++, nextit++)
    {
      if (c->contains((*it)->getDestination()))
	{
	  erase(it);
	  it = nextit;
	  nextit++;
	}
    }
}

void VectoredUnitlist::removeVectoredUnitsComingFrom(City *c)
{
  iterator it = begin();
  iterator nextit = it;
  nextit++;
  for (; nextit != end(); it++, nextit++)
    {
      if (c->contains((*it)->getPos()))
	{
	  erase(it);
	  it = nextit;
	  nextit++;
	}
    }
}
void VectoredUnitlist::getVectoredUnitsGoingTo(City *c, std::list<VectoredUnit*>& vectored)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if (c->contains((*it)->getDestination()))
	{
	  vectored.push_back(*it);
	}
    }
}
void VectoredUnitlist::getVectoredUnitsGoingTo(Vector<int> pos, std::list<VectoredUnit*>& vectored)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getDestination() == pos)
	{
	  vectored.push_back(*it);
	}
    }
}
void VectoredUnitlist::getVectoredUnitsComingFrom(Vector<int> pos, std::list<VectoredUnit*>& vectored)
{
  for (iterator it = begin(); it != end(); it++)
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
  for (iterator it = begin(); it != end(); it++)
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
  for (iterator it = begin(); it != end(); it++)
    {
      if (c->contains((*it)->getDestination()))
	(*it)->setDestination(new_dest);
    }
}

