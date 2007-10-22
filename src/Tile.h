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

#ifndef TILE_H
#define TILE_H

#include <string>
#include <SDL.h>

#include "xmlhelper.h"
#include "defs.h"

#include "tilestyleset.h"

/** Class which describes the terrain
  * 
  * Many tiles are put together to form a tileset. Thus, a tile describes a
  * single terrain type. It keeps the name, movement points, type (grass, water
  * etc.) as well as the images together.
  */

class Tile : public std::vector<TileStyleSet*>
{
    public:
        //! Describe terrain types
        enum Type { NONE = 0, GRASS = NONE, WATER = 1, FOREST = 2, HILLS = 4,
                    MOUNTAIN = 8, SWAMP = 16 };
	enum Pattern { SOLID = 0, STIPPLED = 1, RANDOMIZED = 2, SUNKEN = 3,
	               TABLECLOTH = 4};
                    
        //! Loading constructor
        Tile(XML_Helper* helper);

        ~Tile();

        //! Get the number of movement points needed to cross this tile
        Uint32 getMoves() const {return d_moves;}

        //! Get the color associated with this tile for the smallmap
        SDL_Color getColor() const {return d_color;}
        //! Set the color associated with this tile for the smallmap
	void setColor(SDL_Color clr) {d_color = clr;}

        //! Get the type (grass, hill,...) of this tile type
        Type getType() const {return d_type;}
                
        //! Get the pattern (solid, stippled, random) of this type
        Pattern getPattern() const {return d_pattern;}
        //! set the pattern (solid, stippled, random) of this type
	void setPattern(Pattern pattern) {d_pattern = pattern;}

	std::string getName() const {return d_name;}

        //! Get the alternate color associated with this tile's pattern 
	//!This "second" colour gets stippled, or randomized, or sunken
        SDL_Color getSecondColor() const {return d_second_color;}
        //! set the alternate color associated with this tile's pattern 
        void setSecondColor(SDL_Color color) {d_second_color = color;}

        //! Get another alternate color associated with this tile's pattern 
	//!This "third" colour gets randomized, or sunken
        SDL_Color getThirdColor() const {return d_third_color;}
        //! set another alternate color associated with this tile's pattern 
        void setThirdColor(SDL_Color color) {d_third_color = color;}

	void instantiatePixmaps(std::string tileset, Uint32 tilesize);

    private:
        // DATA
	std::string d_name;
        Uint32 d_moves;                    // moves needed to walk over maptile
        SDL_Color d_color;                // color shown in the smallmap
        Type d_type;
	Pattern d_pattern;
        SDL_Color d_second_color;         // the extra pattern-related color
        SDL_Color d_third_color;         // another pattern-related color

};

#endif // TILE_H

// End of file
