//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#include "armysetlist.h"
#include "playerlist.h"
#include "army.h"
#include "city.h"
#include "GameMap.h"
#include "action.h"
#include "MapBackpack.h"

std::string VectoredUnit::d_tag = "vectoredunit";

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

    retval &= helper->openTag(VectoredUnit::d_tag);
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
  City *dest;
  // drop it in the destination city!
  dest = GameMap::getCity(d_destination);
  if (!dest)
    {
      if (d_destination == Vector<int>(-1,-1))
	{
	  printf ("destination is -1,-1??? why?\n");
	return NULL;
	}
      printf ("uhh... no city at %d,%d?\n", d_destination.x, d_destination.y);
      Maptile *tile = GameMap::getInstance()->getTile(d_destination);
      if (tile)
	{
	  if (tile->getBackpack()->getPlantedItem(d_owner))
	    {
	      //army arrives on a planted standard
	      Army *a = new Army(*d_army, d_owner);
	      LocationBox loc = LocationBox(d_destination);
	      loc.addArmy(a);
	      return a;
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
      printf ("destination city is owned by `%s', but the vectored unit is owned by `%s'\n", dest->getOwner()->getName().c_str(), d_owner->getName().c_str());
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
