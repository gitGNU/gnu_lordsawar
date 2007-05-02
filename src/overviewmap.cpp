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

#include <assert.h>
#include <math.h>

#include "overviewmap.h"
#include "stacklist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "templelist.h"
#include "city.h"
#include "ruin.h"
#include "temple.h"
#include "playerlist.h"
#include "player.h"
#include "GameMap.h"
#include "sdl-draw.h"

OverviewMap::OverviewMap()
{
    surface = 0;
}

OverviewMap::~OverviewMap()
{
    SDL_FreeSurface(surface);
}

void OverviewMap::resize(Vector<int> max_dimensions)
{
    if (!surface)
        SDL_FreeSurface(surface);

    // calculate the width and height relations between pixels and maptiles
    Vector<int> bigmap_dim = GameMap::get_dim();
    Vector<int> d;

    // first try scaling to horizontal size
    pixels_per_tile = max_dimensions.x / double(bigmap_dim.x);
    d.x = max_dimensions.x;
    d.y = int(round(bigmap_dim.y * pixels_per_tile));
    
    if (d.y > max_dimensions.y)
    {
	// if to big, scale to vertical
	pixels_per_tile = max_dimensions.y / double(bigmap_dim.y);
	d.x = int(round(bigmap_dim.x * pixels_per_tile));
	d.y = max_dimensions.y;
    }

    
    static_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, d.x, d.y, 24,
					  0xFFu, 0xFFu << 8, 0xFFu << 16, 0);
    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, d.x, d.y, 24,
				   0xFFu, 0xFFu << 8, 0xFFu << 16, 0);

    Uint32 road_color = SDL_MapRGB(static_surface->format, 255, 255, 255);
    
    // draw static map
    for (int i = 0; i < static_surface->w; i++)
        for (int j = 0; j < static_surface->h; j++)
        {
            int x = int(i / pixels_per_tile);
            int y = int(j / pixels_per_tile);
	    if (GameMap::getInstance()->getTile(x,y)->getBuilding() == Maptile::ROAD)
		draw_pixel(static_surface, i, j, road_color);
	    else
	    {
		SDL_Color c = GameMap::getInstance()->getTile(x,y)->getColor();
		Uint32 raw = SDL_MapRGB(static_surface->format, c.r, c.g, c.b);
		draw_pixel(static_surface, i, j, raw);
	    }
        }
}

void OverviewMap::draw()
{
    // During the whole drawing stuff, ALWAYS consider that 
    // there is an offset of 1 between map coordinates and coordinates
    // of the surface when drawing. I will implcitely assume this during this
    // function.
    
    SDL_BlitSurface(static_surface, 0, surface, 0);

    // minimum size for typical features is 1
    int size = int(pixels_per_tile) > 1 ? int(pixels_per_tile) : 1;

    // Draw all cities as rectangles around the city location, in the colors of
    // the players.
    for (Citylist::iterator it = Citylist::getInstance()->begin();
        it != Citylist::getInstance()->end(); it++)
    {    
        SDL_Color c = it->getPlayer()->getColor();
	Uint32 fill = SDL_MapRGB(surface->format, c.r, c.g, c.b);
	Uint32 outline = SDL_MapRGB(surface->format, c.r / 2, c.g / 2, c.b / 2);
    
        Vector<int> pos = it->getPos();
        pos = mapToSurface(pos);

        // fill a rect with the player colors
	draw_filled_rect(surface, pos.x, pos.y,
			 pos.x + 2 * size, pos.y + 2 * size, fill);

        // if we have space, draw a black outline around the city
        if (size > 1)
            draw_rect_clipped(surface, pos.x, pos.y,
			      pos.x + 2 * size, pos.y + 2 * size, outline);
    }

    // Draw ruins as yellow boxes
    for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
        it != Ruinlist::getInstance()->end(); it++)
    {
        Vector<int> pos = it->getPos();
        pos = mapToSurface(pos);

	Uint32 raw;
	// FIXME: this should probably be defined in the tileset
	if (!it->isSearched())
	   raw = SDL_MapRGB(surface->format, 255, 230, 45);
	else
	   raw = SDL_MapRGB(surface->format, 161, 74, 17);

	draw_filled_rect(surface, pos.x, pos.y,
			 pos.x + size, pos.y + size, raw);
    }

    // Draw temples as two poles with a crossbar
    for (Templelist::iterator it = Templelist::getInstance()->begin();
        it != Templelist::getInstance()->end(); it++)
    {
        Vector<int> pos = it->getPos();
        pos = mapToSurface(pos);
	Uint32 outline = SDL_MapRGB(surface->format, 200, 200, 200);

	draw_hline_clipped(surface, pos.x - size, pos.x + size,
			   pos.y - size, outline);
	draw_vline_clipped(surface, pos.x - size,
			   pos.y - size, pos.y + size, outline);
	draw_vline_clipped(surface, pos.x + size,
			   pos.y - size, pos.y + size, outline);
    }

    // Draw stacks as crosses using the player color
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        SDL_Color c = (*pit)->getColor();
	Uint32 outline = SDL_MapRGB(surface->format, c.r, c.g, c.b);
        
        for (Stacklist::iterator it= mylist->begin(); it != mylist->end(); it++)
        {
            Vector<int> pos = (*it)->getPos();

            // don't draw stacks in cities, they could hardly be identified
            Maptile* mytile = GameMap::getInstance()->getTile(pos.x, pos.y);
            if (mytile->getBuilding() == Maptile::CITY)
                continue;

            pos = mapToSurface(pos);
            draw_hline(surface, pos.x - size, pos.x + size, pos.y, outline);
            draw_vline(surface, pos.x, pos.y - size, pos.y + size, outline);
        }
    }

    // let derived classes do their job
    after_draw();
}

SDL_Surface *OverviewMap::get_surface()
{
    return surface;
}

Vector<int> OverviewMap::mapFromScreen(Vector<int> pos)
{
    // subtract the widget's location and 1 for the border
    int x = int(pos.x / pixels_per_tile);
    int y = int(pos.y / pixels_per_tile);
    
    if (x >= GameMap::getWidth())
        x = GameMap::getWidth() - 1;

    if (y >= GameMap::getHeight())
        y = GameMap::getHeight() - 1;
    
    return Vector<int>(x,y);
}

Vector<int> OverviewMap::mapToSurface(Vector<int> pos)
{
    assert(pos.x >= 0 && pos.y >= 0
	   && pos.x < GameMap::getWidth() && pos.y < GameMap::getHeight());

    int x = int(round(pos.x * pixels_per_tile));
    int y = int(round(pos.y * pixels_per_tile));

    if (pixels_per_tile > 2)
        // try to take the center position of the pixel
        x += int(0.5 * pixels_per_tile);

    if (pixels_per_tile > 2)
        y += int(0.5 * pixels_per_tile);
    
    return Vector<int>(x, y);
}
