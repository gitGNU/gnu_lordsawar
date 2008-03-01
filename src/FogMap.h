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

#ifndef FOGMAP_H
#define FOGMAP_H

#include "vector.h"

class XML_Helper;

//! What a player can see on a hidden map.
/**
 * 
 * Map that represents fog.  Overlays regular SmallMap. 
 */
class FogMap
{
    public:
        //! The two fog types.
        enum FogType {
	  //! Completely open to view.
	  OPEN = 0, 
	  //! Closed to view can be partially obscured.
	  CLOSED = 1
	};
        
        //! Standard constructor: create a given map
	/**
	 * @param width   The GameMap is this wide.
	 * @param height  The GameMap is this hight.
	 */
        FogMap(int width, int height);

	//! Loading constructor.
	/**
	 * Load the fogmap from a file.
	 * FogMaps are stored in the saved-game file at:
	 * lordsawar.playerlist.player.fogmap.
	 *
	 * @param helper  The opened saved-game file to load the fogmap from.
	 */
        FogMap(XML_Helper* helper);

	//! Copy constructor.
	FogMap(const FogMap&);

	//! Destructor.
        ~FogMap();

	//! Fill the fogmap with a status.
        /** 
         * @param type             The status to use.
	 *
         * @return True on success, false on error.
         */
        bool fill(FogType type);

        //! Get the foggedness of a given position.
        FogType getFogTile(Vector<int> pos) const;

	//! Alter the fog around a given position in the fog map.
	/** 
         * @param pt       The point around which status is altered.
         * @param radius   The radius around the point where the fog is altered.
         * @param new_type The type which the area gets.
         */
	void alterFogRadius(Vector<int> pt, int radius, FogType new_type);

	//! Alter the fog in a rectangle at a given position on the fog map.
	/** 
         * @param pt       The upper left point of the rectangle.
         * @param width    The width of the rectangle.
         * @param height   The height of the rectangle.
         * @param new_type The fog type which the area gets.
         */
        void alterFogRectangle(Vector<int> pt, int height, int width, FogType new_type);

	//! Smooth the fogmap.
	/** 
	 * Sweep the fog map for squares that are fogged that are
         * surrounded by defogged squres, and remove them.
         */
	void smooth();

	//! Returns whether or not the fog on a tile is surrounded by openness.
	/** 
	 * This method is for BigMap purposes, it helps to know when a given 
	 * fog tile shouldn't be rendered.
         */
        bool isLoneFogTile(Vector<int> pos);

	//! Save a fogmap.
        /**
         * @param helper  The opened saved-game file to save the fogmap to.
	 *
         * @return True if saving went well, false otherwise.
         */
        bool save(XML_Helper* helper) const;

	//! Is a tile fogged?
	/**
	 * Assists the BigMap and SmallMap in knowing if a given tile is
	 * obscured or not.
	 *
	 * @param pos  The position in the fogmap to query.
	 *
	 * @return True if the position is obscured due to fog, false if not.
	 */
	static bool isFogged(Vector <int> pos);

    private:
        // Data
	//! The width of the fog map.
        int d_width;

	//! The height of the fog map.
        int d_height;

	//! An array of tiles that describe how a tile is fogged.
        FogType * d_fogmap;
};

#endif

// End of file
