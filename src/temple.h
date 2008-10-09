// Copyright (C) 2001, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
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

#ifndef TEMPLE_H
#define TEMPLE_H

#define DEFAULT_TEMPLE_NAME  "Shrine"

#include <string>
#include "NamedLocation.h"

class Stack;
class Quest;

//! A temple on the game map.
/** 
 * A temple is the place where heroes can get quests or have their armies
 * blessed.
 */
class Temple : public NamedLocation
{
    public:
	//! The xml tag of this object in a saved-game file.
	static std::string d_tag; 

	//! Default constructor.
        /**
         * @param pos          The location of the temple on the game map.
         * @param name         The name of the temple.
	 * @param type         The type of the temple.  This should always
	 *                     be 0.
         */
        Temple(Vector<int> pos, std::string name = DEFAULT_TEMPLE_NAME, 
	       int type = 0);
	//! Copy constructor.
        Temple(const Temple&);
        //! Loading constructor.
	/**
	 * @param helper  The opened saved-game file to load the temple from.
	 */
        Temple(XML_Helper* helper);
	//! Destructor.
        ~Temple();

        //! Returns the type of the temple.
        int getType() {return d_type;};

        //! Returns the type of the temple.
        void setType(int type) {d_type=type;};

        //! Returns whether or not the temple can be searched.
	/**
	 * @note Temples can always be searched in this game.
	 */
        bool searchable() {return true;}

        //! Save the temple to the opened saved-game file.
        bool save(XML_Helper* helper) const;

	bool unnamed() {return getName() == getDefaultName() ? true : false;};

	static std::string getDefaultName() {return DEFAULT_TEMPLE_NAME;};
    protected:
	
	//! The type of the temple.
	/**
	 * The temple always has a type of 0, because there is only one kind
	 * of temple in the game.
	 */
        int d_type;
};

#endif // TEMPLE_H
