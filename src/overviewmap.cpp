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
#include "playerlist.h"
#include "city.h"
#include "ruin.h"
#include "temple.h"
#include "playerlist.h"
#include "player.h"
#include "GameMap.h"
#include "sdl-draw.h"
#include "GraphicsCache.h"
#include "FogMap.h"
#include "GameScenario.h"

OverviewMap::OverviewMap()
{
    surface = 0;
}

OverviewMap::~OverviewMap()
{
    if (surface)
	SDL_FreeSurface(surface);
    if (static_surface)
	SDL_FreeSurface(static_surface);
}

bool OverviewMap::isShadowed(Uint32 type, int i, int j)
{
  GameMap *gm = GameMap::getInstance();
  int x = int(i / pixels_per_tile);
  int y = int(j / pixels_per_tile);
  int x2;
  int y2;
  //what's this tile?
  //is it water?
  //no?  then false;
  //if yes, then maybe
  //if the tile above us or beside us is land then this might be a shadow pixel

  if (gm->getTile(x,y)->getType() != type)
    return false;
  if (x > 0 && gm->getTile(x-1,y)->getType() != type)
    {
      x2 = int((i-1) / pixels_per_tile);
      y2 = int(j / pixels_per_tile);
      if (gm->getTile(x-1,y) == gm->getTile(x2,y2))
        return true;
    }
  if (y > 0 && gm->getTile(x,y-1)->getType() != type)
    {
      x2 = int(i / pixels_per_tile);
      y2 = int((j-1) / pixels_per_tile);
      if (gm->getTile(x,y-1) == gm->getTile(x2,y2))
        return true;
    }
  if (y > 0 && x > 0 && gm->getTile(x-1,y-1)->getType() != type)
    {
      x2 = int((i-1) / pixels_per_tile);
      y2 = int((j-1) / pixels_per_tile);
      if (gm->getTile(x-1,y-1) == gm->getTile(x2,y2))
        return true;
    }

  return false;
}

void OverviewMap::draw_tile_pixel(Maptile *t, int i, int j)
{
  SDL_Color c = t->getColor();
  Tile::Pattern p = t->getPattern();
  
  Uint32 first = SDL_MapRGB(static_surface->format, c.r, c.g, c.b);
  switch (p)
    {
      case Tile::SOLID:
        draw_pixel(static_surface, i, j, first);
        break;
      case Tile::STIPPLED:
        {
          SDL_Color s = t->getSecondColor();
          Uint32 second = SDL_MapRGB(static_surface->format, s.r, s.g, s.b);
         
          if ((i+j) % 2 == 0)
            draw_pixel(static_surface, i, j, first);
          else
            draw_pixel(static_surface, i, j, second);
        }
        break;
      case Tile::RANDOMIZED:
        {
          SDL_Color s = t->getSecondColor();
          Uint32 second = SDL_MapRGB(static_surface->format, s.r, s.g, s.b);
          SDL_Color th = t->getThirdColor();
          Uint32 third = SDL_MapRGB(static_surface->format, th.r, th.g, th.b);
          int num = rand() % 3;
          if (num == 0)
            draw_pixel(static_surface, i, j, first);
          else if (num == 1)
            draw_pixel(static_surface, i, j, second);
          else
            draw_pixel(static_surface, i, j, third);
        }
        break;
      case Tile::SUNKEN:
        if (isShadowed(t->getType(), i, j) == false)
          draw_pixel(static_surface, i, j, first);
        else
          {
            SDL_Color s = t->getSecondColor();
            Uint32 shadow_color = SDL_MapRGB(static_surface->format, s.r, s.g, 
                                             s.b);
            draw_pixel(static_surface, i, j, shadow_color);
          }
        break;
      case Tile::TABLECLOTH:
          {
            SDL_Color s = t->getSecondColor();
            Uint32 second = SDL_MapRGB(static_surface->format, s.r, s.g, s.b);
            SDL_Color th = t->getThirdColor();
            Uint32 third = SDL_MapRGB(static_surface->format, th.r, th.g, th.b);
            if (i % 4 == 0 && j % 4 == 0)
              draw_pixel(static_surface, i, j, first);
            else if (i % 4 == 0 && j % 4 == 1)
              draw_pixel(static_surface, i, j, second);
            else if (i % 4 == 0 && j % 4 == 2)
              draw_pixel(static_surface, i, j, first);
            else if (i % 4 == 0 && j % 4 == 3)
              draw_pixel(static_surface, i, j, second);
            else if (i % 4 == 1 && j % 4 == 0)
              draw_pixel(static_surface, i, j, second);
            else if (i % 4 == 1 && j % 4 == 1)
              draw_pixel(static_surface, i, j, third);
            else if (i % 4 == 1 && j % 4 == 2)
              draw_pixel(static_surface, i, j, second);
            else if (i % 4 == 1 && j % 4 == 3)
              draw_pixel(static_surface, i, j, third);
            else if (i % 4 == 2 && j % 4 == 0)
              draw_pixel(static_surface, i, j, first);
            else if (i % 4 == 2 && j % 4 == 1)
              draw_pixel(static_surface, i, j, second);
            else if (i % 4 == 2 && j % 4 == 2)
              draw_pixel(static_surface, i, j, first);
            else if (i % 4 == 2 && j % 4 == 3)
              draw_pixel(static_surface, i, j, second);
            else if (i % 4 == 3 && j % 4 == 0)
              draw_pixel(static_surface, i, j, second);
            else if (i % 4 == 3 && j % 4 == 1)
              draw_pixel(static_surface, i, j, third);
            else if (i % 4 == 3 && j % 4 == 2)
              draw_pixel(static_surface, i, j, second);
            else if (i % 4 == 3 && j % 4 == 3)
              draw_pixel(static_surface, i, j, third);
          }
        break;
    }
}

void OverviewMap::resize()
{
  if (GameMap::getWidth() <= MAP_SIZE_TINY_WIDTH && 
      GameMap::getHeight() <= MAP_SIZE_TINY_HEIGHT)
    resize(GameMap::get_dim() * 4);
  else if (GameMap::getWidth() <= MAP_SIZE_SMALL_WIDTH && 
	   GameMap::getHeight() <= MAP_SIZE_SMALL_HEIGHT)
    resize(GameMap::get_dim() * 3);
  else
    resize(GameMap::get_dim() * 2);
}

void OverviewMap::resize(Vector<int> max_dimensions)
{
    if (surface)
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
	// if too big, scale to vertical
	pixels_per_tile = max_dimensions.y / double(bigmap_dim.y);
	d.x = int(round(bigmap_dim.x * pixels_per_tile));
	d.y = max_dimensions.y;
    }

    
    static_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, d.x, d.y, 24,
					  0xFFu, 0xFFu << 8, 0xFFu << 16, 0);
    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, d.x, d.y, 24,
				   0xFFu, 0xFFu << 8, 0xFFu << 16, 0);

    draw_terrain_pixels(Rectangle(0, 0, static_surface->w, static_surface->h));
}

void OverviewMap::redraw_tiles(Rectangle tiles)
{
    if (tiles.w > 0 && tiles.h > 0)
    {
	tiles.pos -= Vector<int>(1, 1);
	tiles.dim += Vector<int>(2, 2);
	
	// translate to pixel coordinates 
	Vector<int> pos(int(round(tiles.x * pixels_per_tile)),
			int(round(tiles.y * pixels_per_tile)));
	Vector<int> dim(int(round(tiles.w * pixels_per_tile)),
			int(round(tiles.h * pixels_per_tile)));

	// ensure we're within bounds
	pos = clip(Vector<int>(0, 0), pos,
		   Vector<int>(static_surface->w, static_surface->h) - Vector<int>(1, 1));

	if (pos.x + dim.x >= int(GameMap::getWidth() * pixels_per_tile))
	    dim.x = int(GameMap::getWidth() * pixels_per_tile) - pos.x;
	
	if (pos.y + dim.y >= int(GameMap::getHeight() * pixels_per_tile))
	    dim.y = int(GameMap::getHeight() * pixels_per_tile) - pos.y;

	draw_terrain_pixels(Rectangle(pos, dim));
    }
    draw();
}

void OverviewMap::draw_terrain_pixels(Rectangle r)
{
    GameMap *gm = GameMap::getInstance();
    // draw static map
    Uint32 road_color = SDL_MapRGB(static_surface->format, 164, 84, 0);
    
    for (int i = r.x; i < r.x + r.w; ++i)
        for (int j = r.y; j < r.y + r.h; ++j)
        {
            int x = int(i / pixels_per_tile);
            int y = int(j / pixels_per_tile);

	    if (gm->getTile(x,y)->getBuilding() == Maptile::ROAD ||
                     gm->getTile(x,y)->getBuilding() == Maptile::BRIDGE)
		         draw_pixel(static_surface, i, j, road_color);
	    else
	    {
                draw_tile_pixel (GameMap::getInstance()->getTile(x,y), i, j);
	    }
        }
    
}

void OverviewMap::after_draw()
{
}

void OverviewMap::draw()
{
    Uint32 fog_color = SDL_MapRGB(surface->format, 0, 0, 0);
    Playerlist *pl = Playerlist::getInstance();
    int size = int(pixels_per_tile) > 1 ? int(pixels_per_tile) : 1;
    assert(surface);


    // During the whole drawing stuff, ALWAYS consider that 
    // there is an offset of 1 between map coordinates and coordinates
    // of the surface when drawing. I will implcitely assume this during this
    // function.
    
    SDL_BlitSurface(static_surface, 0, surface, 0);

    // Draw ruins as a white dot
    for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
        it != Ruinlist::getInstance()->end(); it++)
    {
        if (it->isHidden() == true && it->getOwner() != pl->getActiveplayer())
          continue;
        if (it->isFogged())
          continue;
        Vector<int> pos = it->getPos();
        pos = mapToSurface(pos);

	Uint32 raw;
	raw = SDL_MapRGB(surface->format, 255, 255, 255);

	draw_filled_rect(surface, pos.x, pos.y,
			 pos.x + size, pos.y + size, raw);
    }

    // Draw temples as a white dot
    for (Templelist::iterator it = Templelist::getInstance()->begin();
        it != Templelist::getInstance()->end(); it++)
    {
        if (it->isFogged())
          continue;
        Vector<int> pos = it->getPos();
        pos = mapToSurface(pos);
	Uint32 raw;
	raw = SDL_MapRGB(surface->format, 255, 255, 255);

	draw_filled_rect(surface, pos.x, pos.y,
			 pos.x + size, pos.y + size, raw);
    }

    //fog it up
    for (int i = 0; i < GameMap::getWidth(); i++)
        for (int j = 0; j < GameMap::getHeight(); j++)
        {
          Vector <int> pos;
          pos.x = i;
          pos.y = j;
          if (FogMap::isFogged(pos) == true)
            {
              pos = mapToSurface(pos);
              draw_filled_rect(surface, pos.x, pos.y,
                               pos.x + size, pos.y + size, fog_color);
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

void OverviewMap::draw_cities (bool all_razed)
{
  GraphicsCache *gc = GraphicsCache::getInstance();

  // Draw all cities as shields over the city location, in the colors of
  // the players.
  for (Citylist::iterator it = Citylist::getInstance()->begin();
      it != Citylist::getInstance()->end(); it++)
  {
      SDL_Surface *tmp;
      if (it->isFogged())
        continue;
      if (it->isBurnt() == true || all_razed == true)
        tmp = gc->getSmallRuinedCityPic();
      else
        tmp = gc->getShieldPic(0, it->getOwner());
  
      Vector<int> pos = it->getPos();
      pos = mapToSurface(pos);
      SDL_Rect r;
      r.x = pos.x - (tmp->w/2);
      r.y = pos.y - (tmp->h/2);
      r.w = tmp->w;
      r.h = tmp->h;
      SDL_BlitSurface(tmp, 0, surface, &r);
  }
}
