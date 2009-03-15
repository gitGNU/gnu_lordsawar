// Copyright (C) 2008, 2009 Ben Asselstine
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

#ifndef ITEM_PROTO_H
#define ITEM_PROTO_H

#include <string>
#include <SDL.h>
#include "xmlhelper.h"
#include "army.h"

#include "Renamable.h"

//! A carryable type of thing that confers special properties on it's holder.
/** 
 * This class describes an item prototype.  Items are carried by heroes in a 
 * backpack, and each item has a "kind".  The item prototype is this kind.
 * When Items are carried they give special abilities to that hero, and 
 * perhaps the stack it is included in.
 * Items can be dropped onto the ground, and picked up from the ground.
 * When a hero dies, all of that hero's items get dropped onto the ground.
 * 
 */

class ItemProto: public Renamable
{
    public:

	//! The xml tag of this object in an itemlist configuration file.
	/**
	 * @note This tag appears in the item configuration file, and in
	 * saved-game files.
	 */
	static std::string d_tag;

	// The item can confer these special properties.
        enum Bonus {

	  //! Add 1 to the strength of the wearer.
	  ADD1STR         = 0x00000001,
	  //! Add 2 to the strength of the wearer.
	  ADD2STR         = 0x00000002,
	  //! Add 3 to the strength of the wearer.
	  ADD3STR         = 0x00000004,
	  //! Add 1 to the strength of the Stack.
	  ADD1STACK       = 0x00000008, 
	  //! Add 2 to the strength of the Stack.
	  ADD2STACK       = 0x00000010,
	  //! Add 3 to the strength of the Stack.
	  ADD3STACK       = 0x00000020, 
	  //! Provides the gift of flight to the Stack.
	  FLYSTACK        = 0x00000040,
	  //! Makes the stack go two times as far.
	  DOUBLEMOVESTACK = 0x00000080,
	  //! Add 2 gold to the Player's treasury per City it holds.
	  ADD2GOLDPERCITY = 0x00000100,
	  //! Add 3 gold to the Player's treasury per City it holds.
	  ADD3GOLDPERCITY = 0x00000200,
	  //! Add 4 gold to the Player's treasury per City it holds.
	  ADD4GOLDPERCITY = 0x00000400,
	  //! Add 5 gold to the Player's treasury per City it holds.
	  ADD5GOLDPERCITY = 0x00000800, 

        };
	static Uint32 bonusFlagsFromString(const std::string str);
	static std::string bonusFlagsToString(const Uint32 bonus);
        
	//! Loading constructor.
        ItemProto(XML_Helper* helper);

	//! Copy constructor.
        ItemProto(const ItemProto& orig);

	//! Creates a new Item Prototype from scratch.
        ItemProto(std::string name, Uint32 id);

        //! Destructor.
        ~ItemProto();
        
        //! Save the item to the opened saved-game file.
        bool save(XML_Helper* helper) const;

        //! Returns whether or not the Item has a particular special bonus.
        Uint32 getBonus() const {return d_bonus;};

        //! Returns whether or not the Item has a particular special bonus.
        bool getBonus(ItemProto::Bonus bonus) const;

	//! Add a bonus to the Item.
	void addBonus(ItemProto::Bonus bonus);

	//! Remove a bonus from the Item.
	void removeBonus(ItemProto::Bonus bonus);
        
        //! Return the Id of the Item prototype.
        Uint32 getTypeId() const {return d_type_id;};

	//! Set the id of the item prototype.
	void setTypeId(Uint32 id) {d_type_id = id;};

	//! Return some text describing the item's special abilities.
        std::string getBonusDescription() const;

    protected:
	//! The item's bonus.
	/**
	 * This value is a bitwise OR-ing of the values in ItemProto::Bonus.
	 */
        Uint32 d_bonus;
        

    private:

	//! The Id of the Item.
	/**
	 * This value is a unique Id among all other game objects.
	 * This value does not change during gameplay.
	 */
        Uint32 d_type_id;

	static std::string bonusFlagToString(ItemProto::Bonus type);
	static ItemProto::Bonus bonusFlagFromString(std::string str);


};

#endif //ITEM_PROTOTYPE_H
