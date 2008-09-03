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

#include "vectoredunit.h"
#include <stdlib.h>
#include <string.h>
#include <xmlhelper.h>

#include "citylist.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "army.h"
#include "city.h"
#include "GameMap.h"
#include "action.h"

VectoredUnit::VectoredUnit(Vector<int> pos, Vector<int> dest, ArmyProdBase *army, int duration, Player *player)
    :Ownable(player), LocationBox(pos), d_destination(dest), 
    d_duration(duration)
{
  if (army)
    d_army = new ArmyProdBase(*army);
  else
    d_army = NULL;
}

VectoredUnit::VectoredUnit(const VectoredUnit& v)
    :Ownable(v), LocationBox(v), d_destination(v.d_destination),
     d_duration(v.d_duration)
{
  if (v.d_army)
    d_army = new ArmyProdBase(*v.d_army);
  else
    d_army = NULL;
}

VectoredUnit::VectoredUnit(XML_Helper* helper)
    :Ownable(helper), LocationBox(helper), d_army(NULL)
{
    helper->getData(d_duration, "duration");
    helper->getData(d_destination.x, "dest_x");
    helper->getData(d_destination.y, "dest_y");
    //army is loaded via callback in vectoredunitlist
}

VectoredUnit::~VectoredUnit()
{
  if (d_army)
    delete d_army;
}

bool VectoredUnit::save(XML_Helper* helper) const
{
    bool retval = true;
    std::string name = "";

    retval &= helper->openTag("vectoredunit");
    retval &= helper->saveData("x", getPos().x);
    retval &= helper->saveData("y", getPos().y);
    retval &= helper->saveData("name", name);
    retval &= helper->saveData("duration", d_duration);
    retval &= helper->saveData("dest_x", d_destination.x);
    retval &= helper->saveData("dest_y", d_destination.y);
    if (d_owner)
        retval &= helper->saveData("owner", d_owner->getId());
    else
        retval &= helper->saveData("owner", -1);
    retval &= d_army->save(helper);
    retval &= helper->closeTag();

    return retval;
}

Army *VectoredUnit::armyArrives()
{
  Citylist *cl = Citylist::getInstance();

  City *dest;
  // drop it in the destination city!
  dest = cl->getObjectAt(d_destination);
  if (!dest)
    {
      if (d_destination == Vector<int>(-1,-1))
	return NULL;
      Maptile *tile = GameMap::getInstance()->getTile(d_destination);
      if (tile)
	{
	  std::list<Item*> items = tile->getItems();
	  for (std::list<Item*>::iterator it = items.begin(); 
	       it != items.end(); it++)
	    {
	      if ((*it)->getPlanted() == true &&
		  (*it)->getPlantableOwner() == d_owner)
		{
		  //army arrives on a planted standard
		  Army *a = new Army(*d_army, d_owner);
		  LocationBox loc = LocationBox(d_destination);
		  loc.addArmy(a);
		  return a;
		}

	    }
	}
    }
  else
    {
      if (!dest->isBurnt() && dest->getOwner() == d_owner)
	{
	  //army arrives in a city
	  Army *a = new Army(*d_army, d_owner);
	  dest->addArmy(a);
	  return a;
	}
    }
  return NULL;
}

bool VectoredUnit::nextTurn()
{
  d_duration--;
  if (d_duration == 0)
    return d_owner->vectoredUnitArrives(this);
  return false;
}

// End of file
