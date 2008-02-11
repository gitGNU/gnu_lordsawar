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

#ifndef MAPTILE_H
#define MAPTILE_H

#include <list>
#include "Tile.h"
#include "tileset.h"
#include "Item.h"

/** A single tile on the map
  * 
  * The Maptile class encapsulates all information one may want to get about a
  * single maptile of the game map. Specifically, it stores the type of and the
  * buildings on the map tile.
  *
  * A remark concerning the type. A maptile has two types. First, the type value
  * is an index in the tileset which says which tile type this maptile has, e.g.
  * "This maptile is of the sort of the first tile in the tileset". The maptile
  * type says which terrain type this maptile has, e.g. grass or swamps.
  */

class Maptile
{
    public:
        //! Enum of possible buldings on the tile
        enum Building {NONE=0, CITY=1, RUIN=2, TEMPLE=3, SIGNPOST=4, ROAD=6, PORT=7, BRIDGE=8};

        /** Default constructor
          * 
          * @param tileSet          the tileset to use
          * @param x                the x position of the tile
          * @param y                the y position of the tile
          * @param type             the terrain type (index in the tileset)
	  * @param tileStyle        the look of this tile to use
          */
        Maptile(Tileset* tileSet, int x, int y, Uint32 type, TileStyle *tileStyle);
        ~Maptile();

        //! Set the type of the terrain (type is an index in the tileset)
        void setType(Uint32 index){d_index = index;}

        //! Set which building is on this maptile
        void setBuilding(Building building){d_building = building;}

        
        //! Get the index of the tile type in the tileset
        Uint32 getType() const {return d_index;}

        //! Get which building is on the maptile
        Building getBuilding() const {return d_building;}

        //! Get the number of moves needed to cross this maptile
        Uint32 getMoves() const;

        //! Get the smallmap color of this maptile
        SDL_Color getColor() const;

	//! Get the pattern of this maptile on the smallmap
        Tile::Pattern getPattern() const;

	//! Get the associated colour with the pattern
        SDL_Color getSecondColor() const;

	//! Get the associated colour with the pattern
        SDL_Color getThirdColor() const;

        //! Get the tile type (the type of the underlying terrain)
        Tile::Type getMaptileType() const;

        //! Add an item to the maptile at a specific position in the list (<0 => to the end)
        void addItem(Item*, int position=-1);

        //! Remove an item from the maptile withut deleting it (!)
        void removeItem(Item* item);

        //! Get the items located at this maptile
        std::list<Item*> getItems() const;
        
	//! is this map tile considered to be "open terrain".
	//! this is used for bonus calculations
	bool isOpenTerrain();

	//! is this map tile considered to be "hilly terrain".
	//! this is used for bonus calculations
        bool isHillyTerrain();

	//! is this map tile considered to be "city terrain".
	//! this is used for bonus calculations
        bool isCityTerrain();

        //prints some debug information
        void printDebugInfo() const;
                
	bool d_blocked[8];

	TileStyle * getTileStyle() const {return d_tileStyle;}

	void setTileStyle(TileStyle *style) {d_tileStyle = style;}

    private:
        Tileset* d_tileSet;
        Uint32 d_index;
	TileStyle *d_tileStyle;
        Building d_building;    // which building is on this maptile
        std::list<Item*> d_items;
};

#endif // MAPTILE_H

// End of file
