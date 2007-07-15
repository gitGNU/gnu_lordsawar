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

#ifndef TEMPLE_H
#define TEMPLE_H

#include <string>
#include "Location.h"

class Stack;
class Quest;

/** A temple is the place where heroes can get quests or have their armies
  * blessed. It doesn't extend the Location class very much...
  */

class Temple : public Location
{
    public:
        /** Default constructor
          * 
          * @param pos          the location of the temple
          * @param name         the name of the temple (AFAIR unused)
          */
        Temple(Vector<int> pos, std::string name = "Shrine", int type=0);

        //! Loading constructor. See XML_Helper
        Temple(XML_Helper* helper);
        Temple(const Temple&);
        ~Temple();

        //! Get the unique id of this temple
        Uint32 getId() const {return d_id;}

        //! Returns the type of the temple
        int getType() {return d_type;};

        //! Returns the type of the temple
        void setType(int type) {d_type=type;};

        //! Dummy function. May be extended in the future.
        bool searchable(){return true;}

        //! Save the temple data.
        bool save(XML_Helper* helper) const;

    protected:
        int d_type;
};

#endif // TEMPLE_H
