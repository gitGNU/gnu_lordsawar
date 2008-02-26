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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <SDL.h>
#include "xmlhelper.h"
#include "army.h"

#include "defs.h"
#include "player.h"
#include "playerlist.h"

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

class Item
{
    public:

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
        
	//! Loading constructor.
        Item(XML_Helper* helper);

	//! Copy constructor.
        Item(const Item& orig);

	//! Creates a new Item from scratch.
        Item(std::string name, bool plantable, Player *plantable_owner);

        //! Destructor.
        ~Item();
        
        //! Save the item to the opened saved-game file.
        bool save(XML_Helper* helper) const;

        //! Returns whether or not the Item has a particular special bonus.
        bool getBonus(Item::Bonus bonus) const;

	//! Add a bonus to the Item.
	void addBonus(Item::Bonus bonus);

	//! Remove a bonus from the Item.
	void removeBonus(Item::Bonus bonus);
        
        //! Return the name of the item.
        std::string getName() const {return d_name;}

	//! Set the name of the item.
	void setName(std::string name) {d_name = name;}

        //! Return the Id of the Item.  0 means the item is a prototype.
        Uint32 getId() const {return d_id;}

	//! Return whether or not the Item is of a kind that can be vectored to.
        bool isPlantable() const {return d_plantable;}

	//! Set the planted status of the Item.
        void setPlanted(bool planted) {d_planted = planted;}

	//! Get the planted status of the Item.
        bool getPlanted() const {return d_planted;}

	//! Return the Player who can plant this particular Item.
	Player *getPlantableOwner() const 
	  {return Playerlist::getInstance()->getPlayer(d_plantable_owner_id);}

	//! Return some text describing the item's special abilities.
        std::string getBonusDescription() const;

    private:
	//! The item's bonus.
	/**
	 * This value is a bitwise OR-ing of the valuesi in Item::Bonus.
	 */
        Uint32 d_bonus;
        
	//! The name of the Item.
	/**
	 * This value does not change during gameplay.
	 */
        std::string d_name;

	//! The Id of the Item.
	/**
	 * This value is a unique Id among all other game objects.
	 * This value does not change during gameplay.
	 */
        Uint32 d_id;

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
};

#endif //ITEM_H
