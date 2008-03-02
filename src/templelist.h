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

#ifndef TEMPLELIST_H
#define TEMPLELIST_H

#include <sigc++/trackable.h>
#include "LocationList.h"
#include "temple.h"

//! A list of Temple objects.
/** 
 * The templelist keeps track of the temples located on the game map. It
  * is also implemented as a singleton since many classes use it for looking 
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

        // Find the nearest temple
        Temple* getNearestVisibleTemple(const Vector<int>& pos);
        Temple* getNearestVisibleTemple(const Vector<int>& pos, int dist);

	Temple * getNearestTemple(const Vector<int>& pos, std::list<bool (*)(Temple *)> filters);
        
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
