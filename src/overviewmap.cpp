// Copyright (C) 2006, 2007 Ulf Lorenz
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#include <assert.h>
#include <math.h>
#include <stdlib.h>

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
#include "GraphicsCache.h"
#include "FogMap.h"

OverviewMap::OverviewMap()
{
    surface = 0;
    static_surface = 0;
    d_player = Playerlist::getActiveplayer();
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

static int 
prand(int i, int j)
{
  unsigned int x = i;
  unsigned int y = j;
  return (rand_r (&x) ^ rand_r (&y)) % 3;
}

static int 
crand(int i, int j)
{
  return (i + 1) ^ (j + 1);
  //return i + j + (i*j) + (i*100) + (j*43) / 43;
}

static int 
drand(int i, int j)
{
  float f = i / 43 * j / 43;
  f *= 10000000;
  return (int) roundf(f) | (i  + i + j);
}

void OverviewMap::draw_tile_pixel(SDL_Surface *surface, 
				  SmallTile::Pattern pattern,
				  SDL_Color first_color, SDL_Color second_color,
				  SDL_Color third_color,
				  int i, int j, bool shadowed)
{
  SDL_Color c = first_color;
  
  Uint32 first = SDL_MapRGB(surface->format, c.r, c.g, c.b);
  switch (pattern)
    {
      case SmallTile::SOLID:
        draw_pixel(surface, i, j, first);
        break;
      case SmallTile::STIPPLED:
        {
          SDL_Color s = second_color;
          Uint32 second = SDL_MapRGB(surface->format, s.r, s.g, s.b);
         
          if ((i+j) % 2 == 0)
            draw_pixel(surface, i, j, first);
          else
            draw_pixel(surface, i, j, second);
        }
        break;
      case SmallTile::RANDOMIZED:
        {
          SDL_Color s = second_color;
          Uint32 second = SDL_MapRGB(surface->format, s.r, s.g, s.b);
          SDL_Color th = third_color;
          Uint32 third = SDL_MapRGB(surface->format, th.r, th.g, th.b);

          int num = prand(i, j) % 3;
          if (num == 0)
            draw_pixel(surface, i, j, first);
          else if (num == 1)
            draw_pixel(surface, i, j, second);
          else
            draw_pixel(surface, i, j, third);
        }
        break;
      case SmallTile::DIAGONAL:
        {
          SDL_Color s = second_color;
          Uint32 second = SDL_MapRGB(surface->format, s.r, s.g, s.b);
          SDL_Color th = third_color;
          Uint32 third = SDL_MapRGB(surface->format, th.r, th.g, th.b);

          int num = drand(i, j) % 3;
          if (num == 0)
            draw_pixel(surface, i, j, first);
          else if (num == 1)
            draw_pixel(surface, i, j, second);
          else
            draw_pixel(surface, i, j, third);
        }
        break;
      case SmallTile::CROSSHATCH:
        {
          SDL_Color s = second_color;
          Uint32 second = SDL_MapRGB(surface->format, s.r, s.g, s.b);
          SDL_Color th = third_color;
          Uint32 third = SDL_MapRGB(surface->format, th.r, th.g, th.b);

          int num = crand(i, j) % 3;
          if (num == 0)
            draw_pixel(surface, i, j, first);
          else if (num == 1)
            draw_pixel(surface, i, j, second);
          else
            draw_pixel(surface, i, j, third);
        }
        break;
      case SmallTile::SUNKEN:
        if (shadowed == false)
          draw_pixel(surface, i, j, first);
        else
          {
            SDL_Color s = second_color;
            Uint32 shadow_color = SDL_MapRGB(surface->format, s.r, s.g, s.b);
            draw_pixel(surface, i, j, shadow_color);
          }
        break;
      case SmallTile::TABLECLOTH:
          {
            SDL_Color s = second_color;
            Uint32 second = SDL_MapRGB(surface->format, s.r, s.g, s.b);
            SDL_Color th = third_color;
            Uint32 third = SDL_MapRGB(surface->format, th.r, th.g, th.b);
            if (i % 4 == 0 && j % 4 == 0)
              draw_pixel(surface, i, j, first);
            else if (i % 4 == 0 && j % 4 == 1)
              draw_pixel(surface, i, j, second);
            else if (i % 4 == 0 && j % 4 == 2)
              draw_pixel(surface, i, j, first);
            else if (i % 4 == 0 && j % 4 == 3)
              draw_pixel(surface, i, j, second);
            else if (i % 4 == 1 && j % 4 == 0)
              draw_pixel(surface, i, j, second);
            else if (i % 4 == 1 && j % 4 == 1)
              draw_pixel(surface, i, j, third);
            else if (i % 4 == 1 && j % 4 == 2)
              draw_pixel(surface, i, j, second);
            else if (i % 4 == 1 && j % 4 == 3)
              draw_pixel(surface, i, j, third);
            else if (i % 4 == 2 && j % 4 == 0)
              draw_pixel(surface, i, j, first);
            else if (i % 4 == 2 && j % 4 == 1)
              draw_pixel(surface, i, j, second);
            else if (i % 4 == 2 && j % 4 == 2)
              draw_pixel(surface, i, j, first);
            else if (i % 4 == 2 && j % 4 == 3)
              draw_pixel(surface, i, j, second);
            else if (i % 4 == 3 && j % 4 == 0)
              draw_pixel(surface, i, j, second);
            else if (i % 4 == 3 && j % 4 == 1)
              draw_pixel(surface, i, j, third);
            else if (i % 4 == 3 && j % 4 == 2)
              draw_pixel(surface, i, j, second);
            else if (i % 4 == 3 && j % 4 == 3)
              draw_pixel(surface, i, j, third);
          }
        break;
    }
}

void OverviewMap::draw_tile_pixel(Maptile *t, int i, int j)
{
  bool shadowed = isShadowed(t->getType(), i, j);
  draw_tile_pixel(static_surface, t->getPattern(), 
		  t->getColor(), t->getSecondColor(), t->getThirdColor(),
		  i, j, shadowed);
}

int OverviewMap::calculateResizeFactor()
{
  if (GameMap::getWidth() <= (int)MAP_SIZE_TINY_WIDTH && 
      GameMap::getHeight() <= (int)MAP_SIZE_TINY_HEIGHT)
    return 4;
  else if (GameMap::getWidth() <= (int)MAP_SIZE_SMALL_WIDTH && 
	   GameMap::getHeight() <= (int)MAP_SIZE_SMALL_HEIGHT)
    return 3;
  else
    return 2;
}

void OverviewMap::resize()
{
  int factor = calculateResizeFactor();
  resize(GameMap::get_dim() * factor);
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
    draw(d_player);
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

void OverviewMap::draw(Player *player)
{
    d_player = player;
    Uint32 fog_color = SDL_MapRGB(surface->format, 0, 0, 0);
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
        Ruin *r = *it;
        if (r->isHidden() == true && r->getOwner() != d_player)
          continue;
        if (r->isFogged(d_player))
          continue;
        Vector<int> pos = r->getPos();
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
      Temple *t = *it;
        if (t->isFogged(d_player))
          continue;
        Vector<int> pos = t->getPos();
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
          if (FogMap::isFogged(pos, d_player) == true)
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
    if (pos.x < 0 || pos.y < 0
	   || pos.x >= GameMap::getWidth() || pos.y >= GameMap::getHeight())
      {
	printf ("pos.x is %d, pos.y is %d\n", pos.x, pos.y);
	printf ("width is %d, height is %d\n", GameMap::getWidth(),
		GameMap::getHeight());
      }
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
      City *c = *it;
      SDL_Surface *tmp;
      if (c->isFogged(d_player))
        continue;
      if (c->isBurnt() == true || all_razed == true)
        tmp = gc->getSmallRuinedCityPic();
      else
        tmp = gc->getShieldPic(0, c->getOwner());
  
      Vector<int> pos = c->getPos();
      pos = mapToSurface(pos);
      SDL_Rect r;
      r.x = pos.x - (tmp->w/2);
      r.y = pos.y - (tmp->h/2);
      r.w = tmp->w;
      r.h = tmp->h;
      SDL_BlitSurface(tmp, 0, surface, &r);
  }
}
