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

#ifndef ROAD_H
#define ROAD_H

#include "Location.h"

//! A single tile on the map that has a road on it.
/**
 * Stack objects move more efficently on roads, and they often interconnect 
 * many City objects.
 * A road object is built on a Tile with any terrain kind except Tile::WATER.
 */
class Road: public Location
{
    public:
	//! Default constructor.
        /**
          * @param pos          The location of the road.
          * @param type 	The type of road.
          */
        Road(Vector<int> pos, int type = 7);

	//! Copy constructor.
        Road(const Road&);
        //! Loading constructor.
	/**
	 * Make a new road object by reading lordsawar.roadlist.road XML 
	 * entities from the saved-game file.
	 *
	 * @param helper  The opened saved-game file to load the road from.
	 */
        Road(XML_Helper* helper);
	//! Destructor.
        ~Road();

        //! Returns the type of the road.
        int getType() {return d_type;};

        //! Sets the type of the road.
        void setType(int type) {d_type = type;};

        //! Save the road data to an opened saved-game file.
        bool save(XML_Helper* helper) const;

    protected:
	//! The type of the road.
	/**
	 * The type of road refers to the look of the road on the map.  It 
	 * can be any one of the following values:
	 * 0 = a road that connects to other roads to the east and the west.
	 * 1 = a road that connects to other roads to the north and the south.
	 * 2 = a road that connects to other roads to all directions.
	 * 3 = a road that connects to other roads to the north and the west.
	 * 4 = a road that connects to other roads to the north and the east.
	 * 5 = a road that connects to other roads to the south and the east.
	 * 6 = a road that connects to other roads to the west and the south.
	 * 7 = a road that connects to roads to the north, south and east.
	 * 8 = a road that connects to roads to east, west and north.
	 * 9 = a road that connects to roads to the east, west, and south.
	 * 10 = a road that connects to roads to the north, south and west.
	 *
	 * The Roadlist::calculateType method can sometimes set this value.
	 */
	int d_type;

};

#endif // ROAD_H
