// Copyright (C) 2008 Ben Asselstine
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

#ifndef SMALLTILE_H
#define SMALLTILE_H

#include <string>
#include <SDL.h>

#include "xmlhelper.h"

//! Describes the appearance of a tile on the miniature map.
/** 
 */
class SmallTile 
{
    public:
	//! The xml tag of this object in a tileset configuration file.
	static std::string d_tag; 

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
	  TABLECLOTH = 4,
	  DIAGONAL = 5, 
	  CROSSHATCH = 6, 
	};
                    

	//! Default constructor.
	SmallTile();

        //! Loading constructor.
	/**
	 * Loads the tileset.tile XML entities in the tileset configuration 
	 * files.
	 * */
        SmallTile(XML_Helper* helper);

	//! Destructor.
        ~SmallTile();

        //! Get the colour associated with this tile for the smallmap.
        SDL_Color getColor() const {return d_color;}

        //! Set the colour associated with this tile for the smallmap.
	void setColor(SDL_Color clr) {d_color = clr;}

        //! Get the pattern (solid, stippled, random) of this type.
        Pattern getPattern() const {return d_pattern;}

        //! set the pattern (solid, stippled, random) of this type.
	void setPattern(Pattern pattern) {d_pattern = pattern;}

        //! Get the alternate colour associated with this tile's pattern.
	/**
	 * This "second" colour gets used when SmallTile::Pattern is
	 * STIPPLED, RANDOMIZED, SUNKEN, or TABLECLOTH.
	 */
        SDL_Color getSecondColor() const {return d_second_color;}

        //! Set the alternate colour associated with this tile's pattern.
        void setSecondColor(SDL_Color color) {d_second_color = color;}

        //! Get another alternate colour associated with this tile's pattern.
	/**
	 * This "third" colour gets used when SmallTile::Pattern is
	 * RANDOMIZED, DIAGONAL, CROSSHATCH, or TABLECLOTH.
	 */
        SDL_Color getThirdColor() const {return d_third_color;}

        //! Set another alternate colour associated with this tile's pattern.
        void setThirdColor(SDL_Color color) {d_third_color = color;}

	//! Save a SmallTile to an opened tile configuration file.
	/**
	 * @param  The opened XML tile configuration file.
	 */
	bool save(XML_Helper *helper);

    private:

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
	 * Only used when SmallTile::Pattern is one of: STIPPLED, 
	 * RANDOMIZED, SUNKEN, TABLECLOTH, DIAGONAL, or CROSSHATCH.
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

#endif // SMALLTILE_H

// End of file
