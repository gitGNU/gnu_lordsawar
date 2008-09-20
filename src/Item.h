// Copyright (C) 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <SDL.h>
#include "xmlhelper.h"
#include "army.h"

#include "player.h"
#include "playerlist.h"
#include "UniquelyIdentified.h"

#include "ItemProto.h"

//! A carryable thing that confers special properties on it's holder.
/** 
 * This class describes an item.  Items are carried by heroes in a backpack.
 * When Items are carried they give special abilities to that hero, and 
 * perhaps the stack it is included in.
 * Items can be dropped onto the ground, and picked up from the ground.
 * When a hero dies, all of that hero's items get dropped onto the ground.
 *
 * There are "plantable items", that a hero can stick into the ground.  This
 * gives the ability to vector Army units to that location.  Every player
 * gets a plantable item, and the item is branded to be usable only by the
 * player that it belongs to.
 * 
 */


class Item: public ItemProto, public UniquelyIdentified
{
    public:
	//! The xml tag of this object in a saved-game file.
	static std::string d_tag; 

	//! Loading constructor.
        Item(XML_Helper* helper);

	//! Copy constructor.
        Item(const Item& orig);

	//! Copy constructor.  make an item from a prototype.
	Item(const ItemProto &proto);

	//! Creates a new Item from scratch.
        Item(std::string name, bool plantable, Player *plantable_owner);

	static Item* createNonUniqueItem(std::string name, bool plantable,
					 Player *plantable_owner);
        //! Destructor.
        ~Item();
        
	//! Emitted when an item is destroyed.
        sigc::signal<void, Item*> sdying;

        //! Save the item to the opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Return whether or not the Item is of a kind that can be vectored to.
        bool isPlantable() const {return d_plantable;}

	//! Set the planted status of the Item.
        void setPlanted(bool planted) {d_planted = planted;}

	//! Get the planted status of the Item.
        bool getPlanted() const {return d_planted;}

	//! Return the Player who can plant this particular Item.
	Player *getPlantableOwner() const 
	  {return Playerlist::getInstance()->getPlayer(d_plantable_owner_id);}

	//! Return the type of this item.
	Uint32 getType() const {return d_type;};

    private:

	//! non-default constructor to make an item with a particular id.
	Item(std::string name, bool plantable, Player *plantable_owner,
	     Uint32 id);

	/**
	 * This value indicates if the type of this Item can potentially be
	 * planted into the ground on the GameMap, and subsequently have 
	 * Army units vectored to that position.
	 */
	bool d_plantable;

	/**
	 * If the Item is plantable, this value is used to determine if the
	 * correct Player is attempting to plant the Item into the ground.
	 * For example, the red player cannot plant the flag belonging to the
	 * yellow player.
	 */
	//! The Id of the Player who can plant this Item.
	Uint32 d_plantable_owner_id;

	/**
	 * When the Item is planted, the player can vector Army units to 
	 * this item's position on the GameMap.
	 */
	//! Whether or not this Item is currently planted.
	bool d_planted;

	//! The item was instantiated from the item prototype that has this id.
	Uint32 d_type;
};

#endif //ITEM_H
