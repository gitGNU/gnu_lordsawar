// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
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

#ifndef HERO_H
#define HERO_H

#include <string>
#include <vector>
#include <list>

#include "army.h"
#include "Item.h"
#include "player.h"

//! A Hero is an Army unit capable of carrying items, going on quests and more.
/** 
 * The Hero class is just like the Army class except that the Hero gets
 * it's name, and gender set, and it's Army:Stat statistics are augmented by 
 * Item objects that are carried in a d_backpack.
 *
 * Heroes emerge in a City usually along with some powerful allies for a 
 * certain amount of gold pieces.
 *
 * Heroes are special because they can get a new Quest from a Temple, and
 * search Ruin objects.  They can pick up and drop Item objects, and can
 * also plant standards into the ground.
 *
 * Heroes are also unique in that they can increase in experience levels.
 */
class Hero : public Army
{
    public:

	//! The xml tag of this object in a saved-game file.
	static std::string d_tag; 

	//! The xml tag of the backpack subobject in a saved-game file.
	static std::string d_backpack_tag; 

        //! The different genders a Hero unit can have.
	/**
	 * The purpose of this enumeration is to show the correct 
	 * recruitment picture for Hero units when they emerge in a 
	 * City, and the Player has to decide if they want it or not.
	 */
        enum Gender {
	  //! The hero has no gender (Not used).
	  NONE = 0, 
	  //! The hero is male.
	  MALE = 1, 
	  //! The hero is female.
	  FEMALE = 2
	};

	static Hero::Gender genderFromString(const std::string str);
	static std::string genderToString(const Hero::Gender gender);
        /**
	 * Copies the prototype hero and creates a hero from it.
         */
	//! Default constructor.
        Hero(const HeroProto& a);

        /**
         * This performs a deep copy, including the Hero's items.
         */
	//! Copy constructor.
        Hero(Hero& h);

        /** 
	 * @param helper   The opened saved-file to read the Hero from.
	 */
	//! Load a Hero from an opened saved-game file.
        Hero(XML_Helper* helper);

	//! Destructor.
        ~Hero();

        //! Saves the Hero to a saved-game file.
        bool save(XML_Helper* helper) const;
        
        /**
	 * Returns a stat of the hero.  See Army::Stat, and Army::getStat.
         * 
         * If modified is set to false, return the "raw", i.e. inherent
         * value of the hero. Otherwise, all items are checked for a 
         * bonus.
         */
        Uint32 getStat(Army::Stat stat, bool modified = true) const;

        //! Add an Item to the backpack of the Hero.
	/**
	 * @param item      The Item to add to the d_backpack.
	 * @param position  How deep into the backpack the Item is stored.
	 *                  Subsequent Items get pushed down to make room.
	 *                  This value starts at 0.
	 *
	 * This method is usually used to add an Item to the top of the
	 * hero's backpack (e.g. position == 0).
	 *
	 * @return Always returns true.
	 */
        bool addToBackpack(Item* item, int position);

	//! Add an Item to the bottom of the hero's backpack. 
        bool addToBackpack(Item* item);

        //! Remove an Item from the backpack of the hero.
        /**
	 * Scan the hero's d_backpack for the Item, and remove it if it is
	 * found.
	 *
	 * @note This method removes the Item from the d_backpack, but does
	 *       not destroy the Item.
	 *
	 * @param item   The Item to look for.
	 *
	 * @return True if the Item was found and removed.
	 */
        bool removeFromBackpack(Item* item);

        //! Returns the backpack of the hero.
        std::list<Item*> getBackpack() {return d_backpack;}

        //! Callback to load the backpack from the opened saved-game file.
        bool loadItems(std::string tag, XML_Helper* helper);
        
	//! Return the natural command of the hero.
	/**
	 * Natural command is used for bonus calculations during a Fight.
	 *
	 * @return A number that is added to the strength to other Army and
	 *         Hero units in the Stack. 
	 */
	Uint32 calculateNaturalCommand();

	void setName(std::string name) {d_name = name;};
	virtual std::string getName() const {return d_name;};

	bool isHero() const {return true;};

        //! Set the gender of the hero.
        void setGender(Gender gender){d_gender = gender;}

        //! Return the gender of the hero.
        Uint32 getGender() const {return d_gender;}

    private:
        
	//! The hero's backpack that holds any number of Item objects.
        std::list<Item*> d_backpack;

	//! The name of the hero.
	std::string d_name;

	//! Gender of the hero
	Hero::Gender d_gender;
};

#endif //HERO_H
