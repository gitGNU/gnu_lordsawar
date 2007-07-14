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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef HERO_H
#define HERO_H

#include <string>
#include <vector>
#include <list>

#include "army.h"
#include "Item.h"
#include "player.h"

/** A hero is a special army, as heroes are rather special, they have names, an
  * inventory....
  *
  * The current hero implementation gives heroes a name and an inventory. The
  * inventory is split into a backpack (items that are just carried around) and
  * equipment (items that are active and provide a bonus). The class
  * automatically overloads the getxxx (with xxx e.g. Strength) functions and
  * adds all item boni to it. Note however that the stats can never drop below
  * 1 (else we may have strange behaviour).
  */

class Hero : public Army
{
    public:
        /** Standard constructor for creating a new hero
          * 
          * Copies the prototype hero and creates a hero from it.
          */
        Hero(const Army& a, std::string name, Player *owner);

        /** Copy constructor
          * 
          * This also copies the hero's equipment etc. Don't use this to
          * create new heroes!
          */
        Hero(Hero& h);

        /** Loading constructor. See XML_Helper as well.
          * 
          * @param helper           the XML_Helper instance of the savefile
          */
        Hero(XML_Helper* helper);

        ~Hero();

        //! Saves the hero data
        bool save(XML_Helper* helper) const;

        bool isHero() const {return true;}

        
        /** Returns a stat of the hero. See also army.h
          * 
          * If modified is set to false, return the "raw", i.e. inherent
          * value of the hero. Otherwise, all items are checked for a 
          * bonus or malus.
          */
        Uint32 getStat(Army::Stat stat, bool modified=true) const;

        //! Blesses the hero (strength+1, once per temple)
	//! Returns whether or not the hero was blessed.
        //bool bless();

        //! Add an item to the backpack of the hero. Returns true on success
        bool addToBackpack(Item* item, int position);
        bool addToBackpack(Item* item);

        //! Remove an item from the backpack of the hero (don't delete it!)
        //! Returns true on success.
        bool removeFromBackpack(Item* item);

        //! Returns the backpack of the hero
        std::list<Item*> getBackpack() {return d_backpack;}


        //! Callback needed during loading of the hero (see xmlhelper.h)
        bool loadItems(std::string tag, XML_Helper* helper);
        
	//! natural command is used for bonus calculations during battle
	//! Returns a number that is added to the strength to everyone
	//! in the stack.
	Uint32 calculateNaturalCommand();

    private:
        
        static std::vector<std::string> s_namelist;

        std::list<Item*> d_backpack;
};

#endif //HERO_H
