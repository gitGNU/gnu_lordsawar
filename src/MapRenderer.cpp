// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2005 Andrea Patton
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
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

#include <SDL.h>
#include <string>

#include "MapRenderer.h"
#include "GameMap.h"
#include "defs.h"

using namespace std;

MapRenderer::MapRenderer(SDL_Surface* surface)
{
    d_surface = surface;
}
 

MapRenderer::~MapRenderer()
{
}

void MapRenderer::render(int x, int y, int tileStartX, int tileStartY,
			 int columns, int rows)
{
    SDL_Rect r;
    GameMap* map = GameMap::getInstance();
    int width = GameMap::getWidth();
    int height = GameMap::getHeight();
    int tilesize = map->getTileset()->getTileSize();
    r.w = r.h = tilesize;
    int drawY = y;

    Uint32 background_color = SDL_MapRGB(d_surface->format, 0, 0, 0);
    
    for (int tileY = tileStartY; tileY < (tileStartY + rows); tileY++)
    {
        int drawX = x;
        for (int tileX = tileStartX; tileX < (tileStartX + columns); tileX++)
        {
	    // first check if we're out of the map bounds
	    if (tileX >= width || tileY >= height) {
		r.x = drawX;
		r.y = drawY;
		
		SDL_FillRect(d_surface, &r, background_color);
	    }
	    else {
		// get correct tile
		Maptile *mtile = map->getTile(tileX,tileY);

		r.x = drawX;
		r.y = drawY;
		TileStyle *style = mtile->getTileStyle();
		SDL_BlitSurface(style->getPixmap(), 0, d_surface, &r);

	    }
	    
            drawX += tilesize;
        }
        drawY += tilesize;
    }

}

// End of file
