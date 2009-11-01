//  Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef REWARDLIST_H
#define REWARDLIST_H

#include <list>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include "reward.h"

class XML_Helper;

/** List of unique rewards in the game.
  * Some rewards like gold, and allies can be created whenever they're needed,
  * but other rewards are unique in nature.  This list is for those unique
  * rewards -- namely item rewards, and hidden ruins.
  *
  */

class Rewardlist : public std::list<Reward*>, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static std::string d_tag; 


	// Methods that operate on the class data and modify the class.

        //! deletes a reward from the list
        void deleteReward(Reward* s);

	//! remove one item reward from the list and return it
	Reward *popRandomItemReward();

	//! remove one ruin reward from the list and return it
	Reward *popRandomRuinReward();

	//! remove one ruin reward from the list and return it
	Reward *popRandomMapReward();

        //! Behaves like std::list::clear(), but frees pointers as well
        void flClear();

        //! Behaves like std::list::erase(), but frees pointers as well
        iterator flErase(iterator object);

        //! Behaves like std::list::remove(), but frees pointers as well
        bool flRemove(Reward* object);


	// Methods that operate on the class data and do not modify the class.

        //! Save the data. See XML_Helper for details
        bool save(XML_Helper* helper) const;


	// Static Methods

        //! Returns the singleton instance. Creates a new one if required.
        static Rewardlist* getInstance();

        //! Loads the singleton instance with a savegame.
        static Rewardlist* getInstance(XML_Helper* helper);

        //! Explicitely deletes the singleton instance.
        static void deleteInstance();
        
    protected:    

	// Constructor.
        Rewardlist();

	//! Copy constructor.
        Rewardlist(Rewardlist *rewardlist);

	//! Loading constructor.
        Rewardlist(XML_Helper* helper);

	//! Destructor.
        ~Rewardlist();

    private:
        //! Callback function for loading rewards.
        bool load(std::string tag, XML_Helper* helper);

	//! Return a random reward from the list of the given type.
        Reward *popRandomReward(Reward::Type type);

	// DATA

        static Rewardlist* s_instance;
};

#endif // REWARDLIST_H

// End of file
