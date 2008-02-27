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

#ifndef SIGNPOSTLIST_H
#define SIGNPOSTLIST_H

#include <sigc++/trackable.h>
#include "LocationList.h"
#include "signpost.h"

/** The signpostlist just keeps track of the signs located on the game map. It
  * is also implemented as a singleton since many classes use it for looking up
  * signposts.
  */

class Signpostlist : public LocationList<Signpost>, public sigc::trackable
{
    public:
        //! Return the singleton instance. Create a new one if needed.
        static Signpostlist* getInstance();

        //! Load the singleton instance with the given savegame
        static Signpostlist* getInstance(XML_Helper* helper);

        //! Explicitly delete the singleton instance
        static void deleteInstance();
        

        //! Saves the game data. See XML_Helper for details.
        bool save(XML_Helper* helper) const;

    protected:
        //! Default constructor
        Signpostlist();

        //! Loading constructor
        Signpostlist(XML_Helper* helper);

    private:
        //! Callback for loading
        bool load(std::string tag, XML_Helper* helper);

        static Signpostlist* s_instance;
};

#endif
