//  Copyright (C) 2008 Ben Asselstine
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

#include <iostream>
#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "Backpack.h"

#include "xmlhelper.h"

#include "Item.h"

std::string Backpack::d_tag = "backpack";

using namespace std;

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<flush;}
#define debug(x)


Backpack::Backpack()
{
    debug("Backpack()");
}

Backpack::Backpack(XML_Helper* helper)
{
  helper->registerTag(Item::d_tag, sigc::mem_fun(this, &Backpack::loadItem));
}

Backpack::Backpack(const Backpack& backpack)
{
  for (const_iterator it = backpack.begin(); it != backpack.end(); it++)
    push_back(new Item(**it));
}

Backpack::~Backpack()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);
}

bool Backpack::saveData(XML_Helper* helper) const
{
  bool retval = true;
  for (const_iterator it = begin(); it != end(); it++)
    retval &= (*it)->save(helper);
  return true;
}

bool Backpack::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Backpack::d_tag);
    retval &= saveData(helper);
    retval &= helper->closeTag();

    return retval;
}

bool Backpack::loadItem(std::string tag, XML_Helper* helper)
{
  if (tag == Backpack::d_tag)
    return true;

  if (tag == Item::d_tag)
    {
      Item* item = new Item(helper);
      push_back(item);
      return true;
    }

  return false;
}

Uint32 Backpack::countStrengthBonuses()
{
  Uint32 bonus = 0;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getBonus(Item::ADD1STR))
	bonus += 1;
      if ((*it)->getBonus(Item::ADD2STR))
	bonus += 2;
      if ((*it)->getBonus(Item::ADD3STR))
	bonus += 3;
    }
  return bonus;
}

Uint32 Backpack::countStackStrengthBonuses()
{
  Uint32 bonus = 0;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getBonus(Item::ADD1STACK))
	bonus += 1;
      if ((*it)->getBonus(Item::ADD2STACK))
	bonus += 2;
      if ((*it)->getBonus(Item::ADD3STACK))
	bonus += 3;
    }
  return bonus;
}


Uint32 Backpack::countGoldBonuses()
{
  Uint32 bonus = 0;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getBonus(Item::ADD2GOLDPERCITY))
	bonus += 2;
      if ((*it)->getBonus(Item::ADD3GOLDPERCITY))
	bonus += 3;
      if ((*it)->getBonus(Item::ADD4GOLDPERCITY))
	bonus += 4;
      if ((*it)->getBonus(Item::ADD5GOLDPERCITY))
	bonus += 5;
    }
  return bonus;
}

Uint32 Backpack::countMovementDoublers()
{
  Uint32 bonus = 0;
  for (iterator it = begin(); it != end(); it++)
    if ((*it)->getBonus(Item::DOUBLEMOVESTACK))
      bonus++;
  return bonus;
}

Uint32 Backpack::countStackFlightGivers()
{
  Uint32 bonus = 0;
  for (iterator it = begin(); it != end(); it++)
    if ((*it)->getBonus(Item::FLYSTACK))
      bonus++;
  return bonus;
}

Uint32 Backpack::countPlantableItems()
{
  Uint32 count = 0;
  for (iterator it = begin(); it != end(); it++)
    if ((*it)->isPlantable())
      count++;
  return count;
}

Item *Backpack::getPlantableItem(Player *player)
{
  for (iterator it = begin(); it != end(); it++)
    if ((*it)->isPlantable() && (*it)->getPlantableOwner() == player)
      return *it;
  return NULL;
}
	
Item *Backpack::getItemById(Uint32 id)
{
  for (iterator it = begin(); it != end(); it++)
    if ((*it)->getId() == id)
      return *it;
  return NULL;
}

bool Backpack::addToBackpack(Item* item, int position)
{
  iterator it = begin();
  for (; position > 0; position--, it++);
  insert(it, item);
  return true;
}

bool Backpack::addToBackpack(Item* item)
{
  iterator it = end();
  insert(it, item);
  return true;
}

bool Backpack::removeFromBackpack(Item* item)
{
  for (iterator it = begin(); it != end(); it++)
    if ((*it) == item)
      {
	//FIXME: delete the item?
	erase(it);
	return true;
      }

  return false;
}
// End of file
