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

/** Class which describes the terrain
  * 
  * Many tiles are put together to form a tileset. Thus, a tile describes a
  * single terrain type. It keeps the name, movement points, type (grass, water
  * etc.) as well as the images together.
  */

class Tile
{
    public:
        //! Describe terrain types
        enum Type { NONE = 0, GRASS = NONE, WATER = 1, FOREST = 2, HILLS = 4,
                    MOUNTAIN = 8, SWAMP = 16 };
                    
        //! Loading constructor
        Tile(XML_Helper* helper);

        ~Tile();

        //! Set the images of the terrain. See MapRenderer how they are used.
        void setSurface(int t, SDL_Surface* surface){d_surface[t] = surface;}

        //! Prints some debug information
        void printDebugInfo() const;

        
        //! Get a surface . See MapRenderer how they are used.
        SDL_Surface* getSurface(int t) const {return d_surface[t];}

        //! Get the name of the terrain
        std::string getName() const {return __(d_name);}

        //! Get the number of movement points needed to cross this tile
        Uint32 getMoves() const {return d_moves;}

        //! Get the color associated with this tile for the smallmap
        SDL_Color getColor() const {return d_color;}

        //! Get the type (grass, hill,...) of this tile type
        Type getType() const {return d_type;}
                
    private:
        // DATA
        SDL_Surface* d_surface[8*4];      // base tiles * 4 = corners
        std::string d_name;                // name    
        Uint32 d_moves;                    // moves needed to walk over maptile
        SDL_Color d_color;                // color shown in the smallmap
        Type d_type;
};

#endif // TILE_H

// End of file
