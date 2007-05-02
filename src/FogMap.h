// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef FOGMAP_H
#define FOGMAP_H

#include "vector.h"

class XML_Helper;

/** Class representing the *fog* map in the game
  * 
  * Map that represents fog.  Overlays regular map. Each player has its own one.
  */

class FogMap
{
    public:
        //! The three fog types: in sight range, once discovered, undiscovered
        enum TYPE {OPEN=0, UNCOVERED=1, COVERED=2};
        
        //! Standard constructor: create a given map
        FogMap();

        //! Load the map using the given XML_Helper. See xml_helper.h for details.
        FogMap(XML_Helper* helper);

        ~FogMap();

        /** Fills the whole map with a single status
          *
          * @param type             the status to use
          * @return true on success, false on error
          */
        bool fill(TYPE type);

        //! Get the tile object at position (x,y)
        TYPE getFogTile(int x, int y) const;

		/** Alter fog around a point
          * 
          * @param pt       the point around which status is altered
          * @param radius   the radius around the point where the fog is altered
          * @param new_type the type which the area gets
          */
		void alterFogRadius(Vector<int> pt, int radius, TYPE new_type);

        /** Save the contents of the map
          * 
          * @param helper           see XML_Helper for more information
          * @return true if saving went well, false otherwise
          */
        bool save(XML_Helper* helper) const;

    private:
        // Data
        int d_width;
        int d_height;

        TYPE* d_fogmap;
};

#endif

// End of file
