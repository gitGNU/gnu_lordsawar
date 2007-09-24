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
        //! The two fog types: discovered, undiscovered
	//! open to view, or closed to view.
	//! closed to view can be partially obscured.
        enum FogType {OPEN=0, CLOSED=1};
        
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
        bool fill(FogType type);

        //! Get the tile object at position (x,y)
        FogType getFogTile(Vector<int> pos) const;

	/** Alter fog around a point
          * 
          * @param pt       the point around which status is altered
          * @param radius   the radius around the point where the fog is altered
          * @param new_type the type which the area gets
          */
	void alterFogRadius(Vector<int> pt, int radius, FogType new_type);
	/** Alter fog in a rectangle
          * 
          * @param pt       the upper left point of the rectangle
          * @param width    the width of the rectangle
          * @param height   the height of the rectangle
          * @param new_type the type which the area gets
          */
        void alterFogRectangle(Vector<int> pt, int height, int width, FogType new_type);

	/** Sweep the fog map for squares that are fogged that are
          * surrounded by defogged squres, and remove them.
          */
	void smooth();

	/** For bigmap purposes, it helps to know when a given tile 
          * shouldn't be rendered.
          */
        bool isLoneFogTile(Vector<int> pos);
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

        FogType * d_fogmap;
};

#endif

// End of file
