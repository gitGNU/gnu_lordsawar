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

#ifndef MAP_BACKPACK_H
#define MAP_BACKPACK_H

#include "vector.h"
#include "Backpack.h"
#include "Immovable.h"

class XML_Helper;
class Item;

//! A backpack that resides on the map.
/** 
 * A MapBackpack is an object that holds items, and has a position on the
 * game map.
 */

class MapBackpack: public Backpack, public Immovable
{
 public:
     //! The xml tag of this object in a saved-game file.
     static std::string d_tag; 

     //! Default constructor.
     MapBackpack(Vector<int> pos);

     //! Copy constructor.
     MapBackpack(const MapBackpack&);

     //! Loading constructor.
     MapBackpack(XML_Helper* helper);

     //! Destructor.
    ~MapBackpack();

    //! Save the MapBackpack object to an opened saved-game file.
    bool save(XML_Helper* helper) const;

    //! Return the plantable item that is planted here.
    Item *getFirstPlantedItem();

    //! Return the plantable item owned by player that is planted here.
    Item *getPlantedItem(Player *player);
};

#endif
