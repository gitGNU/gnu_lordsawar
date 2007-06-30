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

#ifndef RUINLIST_H
#define RUINLIST_H

#include "ruin.h"
#include "ObjectList.h"
#include <sigc++/trackable.h>

/** An object list which keeps track of all ruins. It cannot do much more than
  * saving and loading the elements. Implemented as a singleton again.
  */

class Ruinlist : public ObjectList<Ruin>, public sigc::trackable
{
    public:
        //! Returns the singleton instance. Creates a new one if required.
        static Ruinlist* getInstance();

        //! Loads the singleton instance with a savegame.
        static Ruinlist* getInstance(XML_Helper* helper);

        //! Explicitely deletes the singleton instance.
        static void deleteInstance();
        
        
        //! Save function. See XML_Helper for details.
        bool save(XML_Helper* helper) const;

        // Find the nearest ruin which has not been searched
        Ruin* getNearestUnsearchedRuin(const Vector<int>& pos);
        // Find the nearest ruin
        Ruin* getNearestRuin(const Vector<int>& pos);
        Ruin* getNearestRuin(const Vector<int>& pos, int dist);
        
    protected:
        Ruinlist();
        Ruinlist(XML_Helper* helper);

    private:
        //! Loading callback. See XML_Helper as well.
        bool load(std::string tag, XML_Helper* helper);

        static Ruinlist* s_instance;
};

#endif
