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

#ifndef OVERVIEWMAP_H
#define OVERVIEWMAP_H

#include <SDL.h>
#include "vector.h"
#include "Tile.h"

class Maptile;

/** A smaller version of the map
  * 
  * This class actually cares for drawing a small map with colors representing
  * terrain and symbols representing cities, ruins etc.
  *
  * It provides the basis for actual implementations, namely SmallMap and
  * VectorMap.
  */
class OverviewMap
{
 public:
    OverviewMap();
    virtual ~OverviewMap();

    // the map will keep its aspect ratio and resize itself to take up at most
    // max_dimensions space
    void resize(Vector<int> max_dimensions);
    
    void draw();

    // returns the drawn map
    SDL_Surface *get_surface();

 private:
     // the background, we keep it cached so it doesn't have to be drawn all the time
    SDL_Surface* static_surface;
    bool isShadowed(Uint32 type, int i, int j);
    void draw_tile_pixel(Maptile *, int, int);

 protected:
    double pixels_per_tile;
	
    //! Maps the given point in absolute screen coordinates to a map coordinate
    Vector<int> mapFromScreen(Vector<int> pos);
        
    //! And (almost) the other way round. Map a map coordinate to a surface pixel.
    Vector<int> mapToSurface(Vector<int> pos);

    // hook for derived classes
    virtual void after_draw();

    SDL_Surface* surface;
};

#endif // OVERVIEWMAP_H
