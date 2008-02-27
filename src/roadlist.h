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

#ifndef ROADLIST_H
#define ROADLIST_H

#include "road.h"
#include "LocationList.h"
#include <sigc++/trackable.h>

/** The roadlist just keeps track of the roads located on the game map. It
  * is also implemented as a singleton since many classes use it for looking up
  * roads.
  */

class Roadlist : public LocationList<Road>, public sigc::trackable
{
    public:
        //! Return the singleton instance. Create a new one if needed.
        static Roadlist* getInstance();

        //! Load the singleton instance with the given savegame
        static Roadlist* getInstance(XML_Helper* helper);

        //! Explicitly delete the singleton instance
        static void deleteInstance();
        
        //! Returns the road closest to pos
        Road* getNearestRoad(const Vector<int>& pos);
	Road* getNearestRoad(const Vector<int>& pos, int dist);
        //! Saves the game data. See XML_Helper for details.
        bool save(XML_Helper* helper) const;

	int calculateType (Vector<int> t);

    protected:
        //! Default constructor
        Roadlist();

        //! Loading constructor
        Roadlist(XML_Helper* helper);

    private:
        //! Callback for loading
        bool load(std::string tag, XML_Helper* helper);

        static Roadlist* s_instance;
};

#endif
