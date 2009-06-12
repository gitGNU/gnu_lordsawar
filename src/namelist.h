//  Copyright (C) 2009, Ben Asselstine
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

#ifndef NAMELIST_H
#define NAMELIST_H

#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"


//! A list of names for ruins, temples and cities available to the game.
/** 
 * This class holds all of the possible names in the game for ruins, temples,
 * and cities.
 *
 */
class NameList : public std::vector<std::string>, public sigc::trackable
{
    public:

        //! Default Constructor.
	/**
	 * Loads all names it can find in the given file, and
	 * makes a new NameList object from what it finds.
	 */
        NameList(std::string filename, std::string item_tag);

	std::string popRandomName();

    private:
        
        //! Destructor.
        ~NameList();

        //! Callback for loading names into the NameList.
	bool load(std::string tag, XML_Helper *helper);

	std::string d_item_tag;
};

#endif // NAMELIST_H
