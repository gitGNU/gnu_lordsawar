// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2005 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2012, 2014 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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
#ifndef MAPTILE_H
#define MAPTILE_H

#include <list>
#include "Tile.h"
#include "SmallTile.h"
#include "tileset.h"
#include "Item.h"
#include "MapBackpack.h"
class StackTile;

//! A single tile on the game map.
/** 
 * The Maptile class encapsulates all information one may want to get about a
 * single maptile of the game map.  Specifically, it stores the type of and the
 * buildings on the map tile.
 *
 * A remark concerning the type.  A maptile has two types. First, the type 
 * value is an index in the tileset which says which tile type this maptile 
 * has, e.g. "This maptile is of the sort of the first tile in the tileset". 
 * The maptile type says which terrain type this maptile has, 
 * e.g. grass or swamps.
 *
 * The GameMap contains on Maptile object for every cell of the map.
 *
 */
class Maptile
{
    public:
        //! Enumeration of all possible constructed objects on the maptile.
	/**
	 * Each member in the enumeration refers to a class that inherits 
	 * the Location class.
	 */
        enum Building {
	  //! Bupkiss.  Nothing built here.
	  NONE=0, 
	  //! A City is built here.
	  CITY=1, 
	  //! A Ruin is built here.
	  RUIN=2, 
	  //! A Temple is built here.
	  TEMPLE=3, 
	  //! A Signpost is built here.
	  SIGNPOST=4, 
	  //! A Road is built here.
	  ROAD=6, 
	  //! A Port is built here.
	  PORT=7, 
	  //! A Bridge is built here.
	  BRIDGE=8
	};

	//! Default constructor.
        /** 
	 * Make a new Maptile.
	 *
         * @param x                The x position of the tile.
         * @param y                The y position of the tile.
         * @param type             The terrain type (index in the tileset).
	 * @param tileStyle        The look of this tile to use.
         */
        Maptile(int x, int y, guint32 type, TileStyle *tileStyle);

	//! Slower constructor.
        /** 
	 * Make a new Maptile, but this time using the Tile::Type.
	 *
         * @param x                The x position of the tile.
         * @param y                The y position of the tile.
         * @param type             The terrain type enumeration Tile::Type.
	 * @param tileStyle        The look of this tile to use.
         */
        Maptile(int x, int y, Tile::Type type, TileStyle *tileStyle);

	//! Destructor.
        ~Maptile();

        //! Set the type of the terrain (type is an index in the tileset).
        void setIndex(guint32 index);

        //! Set which kind of building is on this maptile.
        void setBuilding(Building building){d_building = building;}

        //! Get the index of the tile type in the tileset.
        guint32 getIndex() const {return d_index;}

        //! Get which building is on the maptile.
        Building getBuilding() const {return d_building;}

        //! Get the number of moves needed to cross this maptile.
	/**
	 * This method refers to the Tile::getMoves method, but then also 
	 * takes into account the buildings on the tile.
	 * 
	 * @return The number of movement points required to cross this 
	 *         Maptile.
	 */
        guint32 getMoves() const;

        //! Get the smalltile color of this maptile.
	Gdk::RGBA getColor() const {return d_smalltile->getColor();}

	//! Get the pattern of this maptile on the smalltile.
       SmallTile::Pattern getPattern() const {return d_smalltile->getPattern();}

	//! Get the associated colour with the pattern.
       Gdk::RGBA getSecondColor() const {return d_smalltile->getSecondColor();}

	//! Get the associated colour with the pattern.
       Gdk::RGBA getThirdColor() const {return d_smalltile->getThirdColor();}

        //! Get the tile type (the type of the underlying terrain).
        Tile::Type getType() const {return d_type;}

        //! Get the list of Item objects on this maptile.
        MapBackpack *getBackpack() const {return d_backpack;};

        //! Set the backpack for this tile.
        void setBackpack(MapBackpack *bag) {if (d_backpack) delete d_backpack; d_backpack = bag;};

	//! Get the list of Stack objects on this maptile.
	StackTile *getStacks() const {return d_stacktile;};
        
	//! Whether or not this map tile considered to be "open terrain".
	/**
	 *
	 * This is used for battle bonus calculations.  An Army unit can
	 * potentially have a bonus for being `out in the open' -- and this 
	 * method defines if this maptile is `out in the open' or not.
	 */
	bool isOpenTerrain();

	//! Whether or not this map tile is considered to be "hilly terrain".
	/**
	 *
	 * This is used for battle bonus calculations.  An Army unit can 
	 * potentially have a bonus for being `in the hills' -- and this method
	 * defines if this maptile is `in the hills' or not.
	 */
        bool isHillyTerrain();

	//! Whether or not this map tile is considered to be "city terrain".
	/**
	 * This is used for battle bonus calculations.  An Army unit can 
	 * potentially have a bonus for being `in a city' -- and this method
	 * defines if this maptile is `in a city' or not.
	 */
        bool isCityTerrain();

        //! Whether or not this map tile is considered to be a road.
        /**
         * includes roads and bridges.
         */
        bool isRoadTerrain();

        //! Whether or not there is a building on this tile that belongs on land.
        bool hasLandBuilding() const;

        //! Whether or not there is a building on this tile that belongs on water.
        bool hasWaterBuilding() const;
        //! Prints some debug information about this maptile.

	bool d_blocked[8];

	//! Get the TileStyle associated with this Maptile.
	TileStyle * getTileStyle() const {return d_tileStyle;}

	//! Set the TileStyle associated with this Maptile.
	void setTileStyle(TileStyle *style) {d_tileStyle = style;}

	static Maptile::Building buildingFromString(const std::string str);
	static std::string buildingToString(const Maptile::Building bldg);
        static Glib::ustring buildingToFriendlyName(const guint32 bldg);
    private:
	//! The index of the Tile within the Tileset (GameMap::s_tileset).
	/**
	 * The Maptile has a type, in the form of a Tile.  This Tile is
	 * identified by it's index within GameMap::s_tileset.
	 */
        guint32 d_index;

        //! what kind of tile this is
        Tile::Type d_type;

        //! how many moves it takes to cross this tile type ( == Tile::d_moves).
        guint32 d_moves;

	//! The look of the maptile.
	TileStyle *d_tileStyle;

	//! The type of constructed object on this maptile.
        Building d_building;

	//! The list of pointers to items on this maptile.
	MapBackpack *d_backpack;

	//! The list of pointers to stacks on this maptile.
	StackTile *d_stacktile;

        SmallTile *d_smalltile;
};

#endif // MAPTILE_H

// End of file
