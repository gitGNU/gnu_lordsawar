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
#include "defs.h"

#include "tilestyleset.h"

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

	//! The terrain tile's appearance as seen on the OverviewMap.
	enum Pattern { 

	  //! The terrain feature is shown as a single solid colour.
	  SOLID = 0, 

	  //! The terrain feature is checkered with two alternating colours.
	  /**
	   * The stippled pattern looks something like this:
	   * @verbatim
xoxoxoxo
oxoxoxox
xoxoxoxo
oxoxoxox
@endverbatim
	   *
	   * It is currently used for Type::FOREST, and Type::HILLS.
	   */
	  STIPPLED = 1, 

	  //! The feature is random pixels with three different colours.
	  /**
	   * The random pattern looks something like this:
	   * @verbatim
xoexooxo
exoxxeox
xoeoxoxx
eoxeooex
@endverbatim
	   *
	   * It is currently used for Type::MOUNTAINS.
	   */
	  RANDOMIZED = 2, 

	  //! The feature is shaded on the bottom and on the left.
	  /**
	   * The sunken pattern looks something like this:
	   * @verbatim
xxxxxxxo
xxxxxxxo
xxxxxxxo
oooooooo
@endverbatim
	   *
	   * It is currently used for Type::WATER.
	   */
	  SUNKEN = 3,

	  //! The feature is shown as a 3 colour pattern.
	  /**
	   * The tablecloth pattern looks something like this:
	   * @verbatim
xexexexe
eoeoeoeo
xexexexe
eoeoeoeo
@endverbatim
	   *
	   * It is currently used for Type::SWAMP.
	   */
	  TABLECLOTH = 4
	};
                    

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

        //! Get the colour associated with this tile for the smallmap.
        SDL_Color getColor() const {return d_color;}

        //! Set the colour associated with this tile for the smallmap.
	void setColor(SDL_Color clr) {d_color = clr;}

        //! Get the type (grass, hill,...) of this tile type.
        Type getType() const {return d_type;}

	void setType(Type type) {d_type = type;}
                
	static int getTypeIndexForType(Tile::Type type);

	int getTypeIndex() {return getTypeIndexForType(d_type);}

	void setTypeByIndex(int idx);
        //! Get the pattern (solid, stippled, random) of this type.
        Pattern getPattern() const {return d_pattern;}

        //! set the pattern (solid, stippled, random) of this type.
	void setPattern(Pattern pattern) {d_pattern = pattern;}

	//! Get the name of this kind of tile (used in the editor).
	std::string getName() const {return d_name;}

	//! Set the name of this kind of tile (used in the editor).
	void setName(std::string name) {d_name = name;}

        //! Get the alternate colour associated with this tile's pattern.
	/**
	 * This "second" colour gets used when Tile::Pattern is
	 * Tile::STIPPLED, Tile::RANDOMIZED, Tile::SUNKEN, or Tile::TABLECLOTH.
	 */
        SDL_Color getSecondColor() const {return d_second_color;}

        //! Set the alternate colour associated with this tile's pattern.
        void setSecondColor(SDL_Color color) {d_second_color = color;}

        //! Get another alternate colour associated with this tile's pattern.
	/**
	 * This "third" colour gets used when Tile::Pattern is
	 * Tile::RANDOMIZED, or Tile::TABLECLOTH.
	 */
        SDL_Color getThirdColor() const {return d_third_color;}

        //! Set another alternate colour associated with this tile's pattern.
        void setThirdColor(SDL_Color color) {d_third_color = color;}

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

	//! The general appearance of the terrain tile on the OverviewMap.
	/**
	 * Equates to the tileset.tile.smallmap.d_pattern XML entities in the 
	 * tileset configuration file.
	 */
	Pattern d_pattern;

	//! First colour.
	/**
	 * Equates to the following XML entities in the tileset configuration
	 * file:
	 * tileset.tile.smallmap.d_red 
	 * tileset.tile.smallmap.d_green
	 * tileset.tile.smallmap.d_blue
	 */
        SDL_Color d_color;

	//! Second colour.
	/**
	 * Only used when Tile::Pattern is one of: Tile::STIPPLED, 
	 * Tile::RANDOMIZED, Tile::SUNKEN, or Tile::TABLECLOTH.
	 *
	 * Equates to the following XML entities in the tileset configuration
	 * file:
	 * tileset.tile.smallmap.d_2nd_red 
	 * tileset.tile.smallmap.d_2nd_green
	 * tileset.tile.smallmap.d_2nd_blue
	 */
        SDL_Color d_second_color;

	//! Third colour.
	/**
	 * Only used when Tile::Pattern is Tile::RANDOMIZED, or 
	 * Tile::TABLECLOTH.
	 *
	 * Equates to the following XML entities in the tileset configuration
	 * file:
	 * tileset.tile.smallmap.d_3rd_red 
	 * tileset.tile.smallmap.d_3rd_green
	 * tileset.tile.smallmap.d_3rd_blue
	 */
        SDL_Color d_third_color;

};

#endif // TILE_H

// End of file
