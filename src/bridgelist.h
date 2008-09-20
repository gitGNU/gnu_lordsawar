//  Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef BRIDGELIST_H
#define BRIDGELIST_H

#include <sigc++/trackable.h>
#include "LocationList.h"
#include "bridge.h"

//! A list of Bridge objects located on the game map.
/** 
 * The bridgelist keeps track of the bridges located on the game map. It
 * is implemented as a singleton because many classes use it for looking up
 * bridges.
 */
class Bridgelist : public LocationList<Bridge>, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static std::string d_tag; 

        //! Return the singleton instance.  Create a new one if needed.
        static Bridgelist* getInstance();

        //! Load the singleton instance from the opened saved-game file.
        static Bridgelist* getInstance(XML_Helper* helper);

        //! Explicitly delete the singleton instance.
        static void deleteInstance();

        //! Saves the list of Bridge objects to the opened saved-game file.
        bool save(XML_Helper* helper) const;

    protected:
        //! Default constructor.
        Bridgelist();

        //! Loading constructor.
	/**
	 * Load the list of Bridge objects from the opened saved-game file.
	 *
	 * @param helper  The opened saved-game file to load the Bridge objects
	 *                from.
	 */
        Bridgelist(XML_Helper* helper);

    private:
        //! Callback for loading Bridge objects into the list of bridges.
        bool load(std::string tag, XML_Helper* helper);

        //! A static pointer for the singleton instance.
        static Bridgelist* s_instance;
};

#endif
