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

/** This class describes an item.
  * 
  * Items are defined by three values. They have a type, which determines where
  * they may be worn (Weapon, Shield, Accessoire etc.), a bonus which determines
  * what goods they bring to their wielder (strength, defense, army bonus etc.)
  * and a value which tells how much the stat is raised or what boni the user
  * gets.
  *
  * @note Items occur on two places. First, there is an itemlist which holds
  * templates of all available items. They can be identified by an id of 0.
  * Second, there are items in the actual game, which are gained by cloning an
  * item from the itemlist.
  *
  * @note Items should have not more than three boni. If they give an army or
  * movement bonus, just count each of the boni as one bonus.
  */

class Item
{
    public:
        enum Type {WEAPON=0, SHIELD=1, ARMOUR=2, ACCESSOIRE=3, NONE=4};

        /** There are only two useful constructors. This is the loading
          * constructor which loads an item description from a savegame or
          * from an item description file
          */
        Item(XML_Helper* helper);

        /** This constructor clones an item from a template for actual use in
          * the game.
          */
        Item(const Item& orig);

        //! In opposition to other classes, item actually needs its destructor.
        ~Item();
        

        //! Saves the item data
        bool save(XML_Helper* helper) const;

        
        //! Returns the type of the item (weapon, shield etc.)
        Uint32 getType() const {return d_type;}
        
        //! Returns whether the item has a special bonus
        bool getBonus(Army::Stat bonus) const;
        
        //! Returns the "size" of one of the item's boni
        int getValue(Army::Stat bonus);

        //! Returns a picture of the item
        SDL_Surface* getPic() const;

        //! Return the name of the item
        std::string getName() const {return __(d_name);}

        //! Return the id of the item. 0 means the item is a template.
        Uint32 getId() const {return d_id;}

        /** The index should be a unique number and identifies the items in the
          * itemlist. Each item there has an index. When an item enters the game
          * it stores the index of it's original item and may refer e.g. to its
          * picture anytime.
          */
        Uint32 getIndex() const {return d_index;}
        
    private:
        Uint32 d_type;
        std::map<Army::Stat, int> d_bonus;
        
        std::string d_name;
        Uint32 d_id;
        SDL_Surface* d_pic;
        Uint32 d_index;
};

#endif //ITEM_H
