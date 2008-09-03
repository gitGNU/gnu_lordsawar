// Copyright (C) 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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

#include <sstream>
#include <map>
#include "Item.h"
#include "ItemProto.h"
#include "File.h"
#include "playerlist.h"
#include "ucompose.hpp"

using namespace std;

Item::Item(XML_Helper* helper)
	: ItemProto(helper), UniquelyIdentified(helper)
{
    
    helper->getData(d_plantable, "plantable");
    if (d_plantable)
      {
        helper->getData(d_plantable_owner_id, "plantable_owner");
        helper->getData(d_planted, "planted");
      }
    else
      {
	d_plantable_owner_id = MAX_PLAYERS;
	d_planted = false;
      }

    helper->getData(d_type, "type");

}

Item::Item(std::string name, bool plantable, Player *plantable_owner)
	: ItemProto(name, 0), UniquelyIdentified()
{
  d_type = 0;
  d_bonus = 0;
  d_plantable = plantable;
  if (plantable_owner)
    d_plantable_owner_id = plantable_owner->getId();
  else
    d_plantable_owner_id = MAX_PLAYERS;
  d_planted = false;
  //std::cerr << "item created with id " << d_id << std::endl;
}

Item::Item(std::string name, bool plantable, Player *plantable_owner, Uint32 id)
	: ItemProto(name, 0), UniquelyIdentified(id)
{
  d_type = 0;
  d_bonus = 0;
  d_plantable = plantable;
  if (plantable_owner)
    d_plantable_owner_id = plantable_owner->getId();
  else
    d_plantable_owner_id = MAX_PLAYERS;
  d_planted = false;
  //std::cerr << "item created with id " << d_id << std::endl;
}

Item::Item(const Item& orig)
:ItemProto(orig), UniquelyIdentified(orig), 
    d_plantable(orig.d_plantable), 
    d_plantable_owner_id(orig.d_plantable_owner_id), d_planted(orig.d_planted),
    d_type(orig.d_type)
{
}

Item::Item(const ItemProto &proto)
:ItemProto(proto), UniquelyIdentified()
{
  d_type = proto.getTypeId();
  d_plantable = false;
  d_plantable_owner_id = MAX_PLAYERS;
  d_planted = false;
}

Item::~Item()
{
  if (d_unique)
    sdying.emit(this);
}

bool Item::save(XML_Helper* helper) const
{
  bool retval = true;

  // A template is never saved, so we assume this class is a real-life item
  retval &= helper->openTag("item");
  retval &= helper->saveData("name", getName());
  retval &= helper->saveData("plantable", d_plantable);
  if (d_plantable)
    {
      retval &= helper->saveData("plantable_owner", d_plantable_owner_id);
      retval &= helper->saveData("planted", d_planted);
    }
  retval &= helper->saveData("id", d_id);
  retval &= helper->saveData("type", d_type);

  std::string bonus_str = ItemProto::bonusFlagsToString(d_bonus);
  retval &= helper->saveData("bonus", bonus_str);

  retval &= helper->closeTag();

  return retval;
}

Item* Item::createNonUniqueItem(std::string name, bool plantable, 
				Player *plantable_owner)
{
  return new Item(name, plantable, plantable_owner, 0);
}
