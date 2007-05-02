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

#ifndef ROAD_H
#define ROAD_H

#include "Location.h"

/** A road is just a thing on the map to differentiate space
  */

class Road: public Location
{
    public:
        /** Default constructor
          * 
          * @param pos          the location of the road
          * @param type 	the type of road.  0=nw, 7=w
          */
        Road(Vector<int> pos, std::string name = "Road", int type = 7);

        //! Loading constructor. See XML_Helper
        Road(XML_Helper* helper);
        Road(const Road&);
        ~Road();

        //! Returns the type of the road
        int getType() {return d_type;};

        //! Sets the type of the road
        void setType(int type) {d_type = type;};

        //! Save the road data.
        bool save(XML_Helper* helper) const;

    protected:
	int d_type;

};

#endif // ROAD_H
