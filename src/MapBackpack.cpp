//  Copyright (C) 2008, Ben Asselstine
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

#include "MapBackpack.h"
#include "Item.h"
#include "xmlhelper.h"

std::string MapBackpack::d_tag = "itemstack";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

MapBackpack::MapBackpack(Vector<int> pos)
  :Backpack(), Immovable(pos)
{
}

MapBackpack::MapBackpack(const MapBackpack& object)
  :Backpack(object), Immovable(object)
{
}

MapBackpack::MapBackpack(XML_Helper* helper)
  :Backpack(helper), Immovable(helper)
{
}

MapBackpack::~MapBackpack()
{
}

bool MapBackpack::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(Backpack::d_tag);
  retval &= helper->saveData("x", getPos().x);
  retval &= helper->saveData("y", getPos().y);
  retval &= Backpack::saveData(helper);
  retval &= helper->closeTag();

  return retval;
}

Item *MapBackpack::getFirstPlantedItem()
{
  for (MapBackpack::iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getPlanted() == true)
	return *it;
    }
  return NULL;
}

Item *MapBackpack::getPlantedItem(Player *player)
{
  for (MapBackpack::iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getPlanted() == true &&
	  (*it)->getPlantableOwner() == player)
	return *it;
    }
  return NULL;
}
