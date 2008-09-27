// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef TILE_H
#define TILE_H

#include <string>
#include <SDL.h>

#include "xmlhelper.h"

#include "tilestyleset.h"

class SmallTile;
//! Describes a kind of tile that a Stack can traverse.
/** 
 * Many tiles are put together to form a tileset. Thus, a tile describes a
 * single terrain type. It keeps the name, movement points, type (grass, water
 * etc.) and also keeps the images together.
 *
 * Each tile holds a list of TileStyleSet objects which hold a list of 
 * TileStyle objects.  Each TileStyleSet holds a bunch of pictures of
 * what this tile can look like.  A TileStyle is a single picture.
 * These TileStyle pictures are displayed on the BigMap using the MapRenderer.
 *
 * Tile objects are held in a Tileset object.
 * Maptile objects refer to Tile objects.
 */
class Tile : public std::list<TileStyleSet*>
{
    public:
	//! The xml tag of this object in a tileset configuration file.
	static std::string d_tag; 

        //! Enumerate the kinds of terrain that a Stack can potentially move on.
        enum Type { 
	  //! Synomymous with GRASS.
	  NONE = 0, 
	  //! Grassy plain.  Flat.  Open.  Easy to pass through.
	  GRASS = NONE, 
	  //! Lake, ocean, river, puddle, moat, or anything else watery.
	  WATER = 1, 
	  //! Trees in great abundance, also includes shrubberies.
	  FOREST = 2, 
	  //! Hilly terrain, generally passable.
	  HILLS = 4,
	  //! Very hilly terrain, generally not passable except by flight.
	  MOUNTAIN = 8, 
	  //! Marshy terrain.
	  SWAMP = 16 
	};
	static std::string tileTypeToString(const Tile::Type type);
	static Tile::Type tileTypeFromString(const std::string str);

	//! Default constructor.
	Tile();

        //! Loading constructor.
	/**
	 * Loads the tileset.tile XML entities in the tileset configuration 
	 * files.
	 * */
        Tile(XML_Helper* helper);

	//! Destructor.
        ~Tile();

        //! Get the number of movement points needed to cross this tile
        Uint32 getMoves() const {return d_moves;}

        //! Get the type (grass, hill,...) of this tile type.
        Type getType() const {return d_type;}

	void setType(Type type) {d_type = type;}
                
	static int getTypeIndexForType(Tile::Type type);

	int getTypeIndex() {return getTypeIndexForType(d_type);}

	void setTypeByIndex(int idx);

	//! Get the name of this kind of tile (used in the editor).
	std::string getName() const {return d_name;}

	//! Set the name of this kind of tile (used in the editor).
	void setName(std::string name) {d_name = name;}

	//! Save a Tile to an opened tile configuration file.
	/**
	 * @param  The opened XML tile configuration file.
	 */
	bool save(XML_Helper *helper);

	//! Lookup a random tile style for this tile.
	/**
	 * Scan the TileStyles for this Tile for a TileStyle that matches 
	 * the given style.  When there is more than one TileStyle to choose 
	 * from, randomly pick one from all of the matching TileStyle objects.
	 *
	 * @param style  The kind of style we're looking for.
	 *
	 * @return A pointer to the matching TileStyle object, or NULL if no 
	 *         TileStyle could be found with that given style.
	 */
	TileStyle *getRandomTileStyle (TileStyle::Type style);
	SmallTile * getSmallTile() {return d_smalltile;};
	void setSmallTile(SmallTile *smalltile) {d_smalltile = smalltile;};
	bool validate();

    private:
        // DATA

	//! The name of this kind of a tile.
	/**
	 * The name is taken from the tileset configuration file.
	 * This value doesn't change during gameplay.
	 * It used in the scenario editor, but not used in the game.
	 * Equates to the tileset.tile.d_name XML entities in the tileset
	 * configuration file.
	 */
	std::string d_name;

	//! The number of movement points required to cross this tile.
	/**
	 * If an Army unit cannot traverse the tile efficiently it pays
	 * this number of movement points to walk over this tile.
	 * This value doesn't change during gameplay.
	 * Equates to the tileset.tile.d_moves XML entities in the tileset
	 * configuration file.
	 */
        Uint32 d_moves;
	
	//! The kind of terrain tile this instance represents.
	/**
	 * Equates to the tileset.tile.d_type XML entities in the tileset
	 * configuration file.
	 */
        Type d_type;

	SmallTile *d_smalltile;

	bool validateGrassAndSwamp(std::list<TileStyle::Type> types);
	bool validateForestWaterAndHills(std::list<TileStyle::Type> types);
	bool validateMountains(std::list<TileStyle::Type> types);
};

#endif // TILE_H

// End of file
