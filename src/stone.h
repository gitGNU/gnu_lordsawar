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

#ifndef STONE_H
#define STONE_H

#include "Location.h"

/** A stone is just a thing on the map to differentiate space
  */

class Stone: public Location
{
    public:
        /** Default constructor
          * 
          * @param pos          the location of the stone
          * @param type         the type of stone.  0=nw,3=e,4=center,5=se,7=w
          */
        Stone(PG_Point pos, std::string name = "Stone", int type=8);

        //! Loading constructor. See XML_Helper
        Stone(XML_Helper* helper);
        Stone(const Stone&);
        ~Stone();

        //! Returns the type of the stone
        int getType() {return d_type;};

        //! Sets the type of the stone
        void setType(int type) {d_type = type;};

        //! Save the stone data.
        bool save(XML_Helper* helper) const;

    protected:
	int d_type;

};

#endif // STONE_H
