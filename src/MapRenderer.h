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

#ifndef MAPRENDERER_H
#define MAPRENDERER_H

#include <SDL.h>

/** Class which cares about rendering of the map.
  * 
  * This class is initalized with the drawing surface of the BigMap class. It
  * cares for the actual terrain drawing. 
  */

class MapRenderer
{
    public:

        /** Constructor, also does the smoothing of the GameMap.
          * 
          * @param surface      the surface which is rendered with render()
          */
        MapRenderer(SDL_Surface* surface);
        ~MapRenderer();

        /** Render a portion of the map.
          * 
          * The part of the map which is drawn starts at the tile (tileX,tileY)
          * and goes on till (tileX+columns, tileY+rows). The drawing is done on
          * the surface handed over in the constructor and starts at pixel
          * position (x,y).
          */
        void render(int x, int y, int tileX, int tileY, int columns, int rows);

    private:
        //Data
        SDL_Surface* d_surface;
};

#endif // MAPRENDERER_H

// End of file
