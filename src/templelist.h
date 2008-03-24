// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2003, 2004, 2005 Ulf Lorenz
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

#ifndef TEMPLELIST_H
#define TEMPLELIST_H

#include <sigc++/trackable.h>
#include "LocationList.h"
#include "temple.h"

//! A list of Temple objects on the game map.
/** 
 * The templelist keeps track of the temples located on the game map. It
 * is implemented as a singleton because many classes use it for looking 
 * up temples.
 */
class Templelist : public LocationList<Temple>, public sigc::trackable
{
    public:
        //! Return the singleton instance.  Create a new one if needed.
        static Templelist* getInstance();

        //! Load the temple list from an opened saved-game file.
	/**
	 * @param helper  The opened saved-game file to load the list of 
	 *                temples from.
	 *
	 * @return The list of temples.
	 */
        static Templelist* getInstance(XML_Helper* helper);

        //! Explicitly delete the singleton instance.
        static void deleteInstance();

        //! Saves the temple data to an opened saved-game file.
        bool save(XML_Helper* helper) const;

        //! Find the nearest temple that is not obscured by fog.
	/**
	 * Scan through all temples, searching for the closest one that is
	 * not covered by fog-of-war on a hidden map.
	 *
	 * @param pos  The position to find the nearest temple from.
	 *
	 * @return A pointer to the nearest temple that is not obscured by fog.
	 */
        Temple* getNearestVisibleTemple(const Vector<int>& pos);

	//! Find the nearest temple that is unobscured and is not too far away.
	/**
	 * Scan through all the temples, searching for the closest one that
	 * is not covered by fog-of-war on a hidden map, but is not farther
	 * away than a given distance.
	 *
	 * @param pos  The position to find the nearest temple from.
	 * @param dist The number of tiles away that is deemed "too far".
	 *
	 * @return A pointer to the nearest temple that is not obscured by fog 
	 *         and is within the prescribed number of tiles.  Returns NULL 
	 *         if no temple could be found.
	 */
        Temple* getNearestVisibleTemple(const Vector<int>& pos, int dist);

    protected:
        //! Default constructor.
        Templelist();

        //! Loading constructor.
	/**
	 * Load the list of temples from an opened saved-game file.
	 *
	 * @param helper  The opened saved-game file to load the temples from.
	 */
        Templelist(XML_Helper* helper);

    private:
        //! Callback for loading temple objects from opened saved game files.
        bool load(std::string tag, XML_Helper* helper);

        //! A static pointer for the singleton instance.
        static Templelist* s_instance;
};

#endif
