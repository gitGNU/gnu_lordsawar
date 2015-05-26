// Copyright (C) 2006, 2007 Ulf Lorenz
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2014 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#include <config.h>
#include <assert.h>
#include <cairomm/context.h>

#include "overviewmap.h"
#include "citylist.h"
#include "ruinlist.h"
#include "templelist.h"
#include "city.h"
#include "ruin.h"
#include "temple.h"
#include "playerlist.h"
#include "player.h"
#include "GameMap.h"
#include "ImageCache.h"
#include "FogMap.h"
#include "GameScenarioOptions.h"
#include "Configuration.h"
#include "shieldset.h"
#include "tileset.h"

OverviewMap::OverviewMap()
{
  blank_screen = false;
  map_tiles_per_tile = GameMap::calculateTilesPerOverviewMapTile();
  pixels_per_tile = 2.0;
}

bool OverviewMap::isShadowed(Tile::Type type, int i, int j)
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
  return (rand () % 3);
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


void OverviewMap::choose_surface(bool front, Cairo::RefPtr<Cairo::Surface> &surf,
				 Cairo::RefPtr<Cairo::Context> &gc)
{
  if (front)
    {
      gc = surface_gc;
      surf = surface;
    }
  else
    {
      gc = static_surface_gc;
      surf = static_surface;
    }
}
void
OverviewMap::draw_pixel(Cairo::RefPtr<Cairo::Surface> surf, Cairo::RefPtr<Cairo::Context> gc, int x, int y, const Gdk::RGBA color)
{
  gc->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
  gc->rectangle(x, y, 1, 1);
  gc->fill();
  return;
}

void
OverviewMap::draw_filled_rect(int x, int y, int width, int height, const Gdk::RGBA color)
{
  draw_filled_rect(true, x, y, width, height, color);
}

void
OverviewMap::draw_filled_rect(bool front, int x, int y, int width, int height, const Gdk::RGBA color)
{
  Cairo::RefPtr<Cairo::Surface> surf;
  Cairo::RefPtr<Cairo::Context> gc;
  choose_surface (front, surf, gc);
  gc->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
  gc->rectangle(x, y, width, height);
  gc->fill();
}

void
OverviewMap::draw_line(int src_x, int src_y, int dst_x, int dst_y, const Gdk::RGBA color)
{
  draw_line(true, src_x, src_y, dst_x, dst_y, color);
}

void
OverviewMap::draw_line(bool front, int src_x, int src_y, int dst_x, int dst_y, Gdk::RGBA color)
{
  Cairo::RefPtr<Cairo::Surface> surf;
  Cairo::RefPtr<Cairo::Context> gc;
  choose_surface (front, surf, gc);
  gc->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
  gc->move_to(src_x, src_y);
  gc->line_to(dst_x, dst_y);
}

void
OverviewMap::draw_rect(int x, int y, int width, int height, const Gdk::RGBA color)
{
  draw_rect (true, x, y, width, height, color);
}

void
OverviewMap::draw_rect(bool front, int x, int y, int width, int height, const Gdk::RGBA color)
{
  Cairo::RefPtr<Cairo::Surface> surf;
  Cairo::RefPtr<Cairo::Context> gc;
  choose_surface (front, surf, gc);
  gc->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), color.get_alpha());
  //gc->rectangle(x, y, width, height);
   gc->move_to(x, y);
    gc->rel_line_to(width, 0);
     gc->rel_line_to(0, height);
      gc->rel_line_to(-width, 0);
      gc->rel_line_to(0, -height);
      gc->set_line_width(1.0);
      gc->stroke();
}

void OverviewMap::draw_terrain_tile(Cairo::RefPtr<Cairo::Surface> surf,
				    Cairo::RefPtr<Cairo::Context> gc,
				    SmallTile::Pattern pattern,
				    Gdk::RGBA first, 
				    Gdk::RGBA second,
				    Gdk::RGBA third,
				    int i, int j, bool shadowed)
{

  switch (pattern)
    {
      case SmallTile::SOLID:
        draw_pixel(surf, gc, i, j, first);
        break;
      case SmallTile::STIPPLED:
        {
          if ((i+j) % 2 == 0)
            draw_pixel(surf, gc, i, j, first);
          else
            draw_pixel(surf, gc, i, j, second);
        }
        break;
      case SmallTile::RANDOMIZED:
        {
          int num = prand(i, j) % 3;
          if (num == 0)
            draw_pixel(surf, gc, i, j, first);
          else if (num == 1)
            draw_pixel(surf, gc, i, j, second);
          else
            draw_pixel(surf, gc, i, j, third);
        }
        break;
      case SmallTile::DIAGONAL:
        {
          int num = drand(i, j) % 3;
          if (num == 0)
            draw_pixel(surf, gc, i, j, first);
          else if (num == 1)
            draw_pixel(surf, gc, i, j, second);
          else
            draw_pixel(surf, gc, i, j, third);
        }
        break;
      case SmallTile::CROSSHATCH:
        {
          int num = crand(i, j) % 3;
          if (num == 0)
            draw_pixel(surf, gc, i, j, first);
          else if (num == 1)
            draw_pixel(surf, gc , i, j, second);
          else
            draw_pixel(surf, gc, i, j, third);
        }
        break;
      case SmallTile::SUNKEN:
        if (shadowed == false)
          draw_pixel(surf, gc, i, j, first);
        else
          {
            draw_pixel(surf, gc, i, j, second);
          }
        break;
      case SmallTile::SUNKEN_STRIPED:
        if (shadowed == false)
	  {
	    if (j % 2 == 0)
	      draw_pixel(surf, gc, i, j, first);
	    else
	      draw_pixel(surf, gc, i, j, third);
	  }
        else
          {
            draw_pixel(surf, gc, i, j, second);
          }
        break;
      case SmallTile::TABLECLOTH:
          {
            if (i % 4 == 0 && j % 4 == 0)
              draw_pixel(surf, gc, i, j, first);
            else if (i % 4 == 0 && j % 4 == 1)
              draw_pixel(surf, gc, i, j, second);
            else if (i % 4 == 0 && j % 4 == 2)
              draw_pixel(surf, gc, i, j, first);
            else if (i % 4 == 0 && j % 4 == 3)
              draw_pixel(surf, gc, i, j, second);
            else if (i % 4 == 1 && j % 4 == 0)
              draw_pixel(surf, gc, i, j, second);
            else if (i % 4 == 1 && j % 4 == 1)
              draw_pixel(surf, gc, i, j, third);
            else if (i % 4 == 1 && j % 4 == 2)
              draw_pixel(surf, gc, i, j, second);
            else if (i % 4 == 1 && j % 4 == 3)
              draw_pixel(surf, gc, i, j, third);
            else if (i % 4 == 2 && j % 4 == 0)
              draw_pixel(surf, gc, i, j, first);
            else if (i % 4 == 2 && j % 4 == 1)
              draw_pixel(surf, gc, i, j, second);
            else if (i % 4 == 2 && j % 4 == 2)
              draw_pixel(surf, gc, i, j, first);
            else if (i % 4 == 2 && j % 4 == 3)
              draw_pixel(surf, gc, i, j, second);
            else if (i % 4 == 3 && j % 4 == 0)
              draw_pixel(surf, gc, i, j, second);
            else if (i % 4 == 3 && j % 4 == 1)
              draw_pixel(surf, gc, i, j, third);
            else if (i % 4 == 3 && j % 4 == 2)
              draw_pixel(surf, gc, i, j, second);
            else if (i % 4 == 3 && j % 4 == 3)
              draw_pixel(surf, gc, i, j, third);
          }
        break;
      case SmallTile::SUNKEN_RADIAL:
        if (shadowed == true)
          draw_pixel(surf, gc, i, j, third);
        break;
    }
}

void OverviewMap::draw_terrain_tile(Maptile *t, int i, int j)
{
  bool shadowed = isShadowed(t->getType(), i, j);
  draw_terrain_tile (static_surface, static_surface_gc, t->getPattern(), 
		     t->getColor(), 
		     t->getSecondColor(), 
		     t->getThirdColor(),
		     i, j, shadowed);
}

int OverviewMap::calculatePixelsPerTile(int width, int height)
{
  int pixels = 2;
  if (width <= (int)MAP_SIZE_TINY_WIDTH && 
      height <= (int)MAP_SIZE_TINY_HEIGHT)
    pixels = 4;
  else if (width <= (int)MAP_SIZE_SMALL_WIDTH && 
	   height <= (int)MAP_SIZE_SMALL_HEIGHT)
    pixels = 3;
  else
    pixels = 2;
  switch (Configuration::UiFormFactor(Configuration::s_ui_form_factor))
    {
    case Configuration::UI_FORM_FACTOR_DESKTOP:
    case Configuration::UI_FORM_FACTOR_NETBOOK:
      break;
    case Configuration::UI_FORM_FACTOR_LARGE_SCREEN:
      pixels++;
      break;
    }
  return pixels;
}

int OverviewMap::calculatePixelsPerTile()
{
  return calculatePixelsPerTile(GameMap::getWidth(), GameMap::getHeight());
}

void OverviewMap::resize()
{
  int factor = calculatePixelsPerTile();
  resize(GameMap::get_dim() * factor, 
         GameMap::calculateTilesPerOverviewMapTile());
}

Vector<int> OverviewMap::calculate_smallmap_size()
{
  Vector<int> max_dimensions = GameMap::get_dim() * calculatePixelsPerTile();
  Vector<int> d;
  int p = d.x / double(GameMap::get_dim().x);
  d.x = max_dimensions.x;
  d.y = int(round(GameMap::get_dim().y * p));
  if (d.y > max_dimensions.y)
    {
	p = max_dimensions.y / double(GameMap::get_dim().y);
	d.x = int(round(GameMap::get_dim().x * p));
	d.y = max_dimensions.y;
    }
  return d;
}

void OverviewMap::resize(Vector<int> max_dimensions, float scale)
{
  surface.clear();

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
    map_tiles_per_tile = scale;

    Cairo::RefPtr<Cairo::Surface> empty = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, d.x, d.y);
    static_surface = Cairo::Surface::create(empty, Cairo::CONTENT_COLOR_ALPHA, d.x, d.y);
    static_surface_gc = Cairo::Context::create(static_surface);

    Tileset *ts = GameMap::getTileset();
    Tile *tile = ts->getFirstTile(SmallTile::SUNKEN_RADIAL);
    if (tile)
      draw_radial_gradient(tile->getSmallTile()->getColor(),
                           tile->getSmallTile()->getSecondColor(), d.x, d.y);
    draw_terrain_tiles(Rectangle(0, 0, d.x, d.y));
    surface = Cairo::Surface::create(empty, Cairo::CONTENT_COLOR_ALPHA, d.x, d.y);
    surface_gc = Cairo::Context::create(surface);

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

	int width = get_width();
	int height = get_height();
	// ensure we're within bounds
	pos = clip(Vector<int>(0, 0), pos,
		   Vector<int>(width, height) - Vector<int>(1, 1));

	if (pos.x + dim.x >= int(GameMap::getWidth() * pixels_per_tile))
	    dim.x = int(GameMap::getWidth() * pixels_per_tile) - pos.x;
	
	if (pos.y + dim.y >= int(GameMap::getHeight() * pixels_per_tile))
	    dim.y = int(GameMap::getHeight() * pixels_per_tile) - pos.y;

	draw_terrain_tiles(Rectangle(pos, dim));
    }
    draw(Playerlist::getViewingplayer());
}

Maptile* OverviewMap::getTile(int x, int y)
{
  //look for something interesting so we don't skip over important tiles.
  GameMap *gm = GameMap::getInstance();
  Maptile *favoured_tile = gm->getTile(x,y);
  int xmax = x + map_tiles_per_tile - 1;
  if (xmax >= GameMap::getWidth())
    xmax = GameMap::getWidth() - 1;
  int ymax = y + map_tiles_per_tile - 1;
  if (ymax >= GameMap::getHeight())
    ymax = GameMap::getHeight() - 1;
  for (int i = x; i < xmax; i++)
    for (int j = y; j < ymax; j++)
      {
        Vector<int> pos(i, j);
        if (gm->getBuilding(pos) == Maptile::TEMPLE)
          favoured_tile = gm->getTile(pos);
        else if (gm->getBuilding(pos) == Maptile::RUIN)
          favoured_tile = gm->getTile(pos);
        else if (gm->getTerrainType(pos) == Tile::WATER)
          favoured_tile = gm->getTile(pos);
        else if (gm->getTerrainType(pos) == Tile::MOUNTAIN)
          favoured_tile = gm->getTile(pos);
      }
  return favoured_tile;
}

void OverviewMap::draw_terrain_tiles(Rectangle r)
{
    unsigned int oldrand = rand();
    srand(0);
    Tileset *ts = GameMap::getTileset();
    Gdk::RGBA rd = ts->getRoadColor();
    for (int i = r.x; i < r.x + r.w; i+=int(map_tiles_per_tile))
      for (int j = r.y; j < r.y + r.h; j+=int(map_tiles_per_tile))
        {
          int x = int(i / pixels_per_tile);
          int y = int(j / pixels_per_tile);
          Maptile *mtile = getTile(x,y);

          if (mtile->isRoadTerrain())
            draw_pixel(static_surface, static_surface_gc, i, j, rd);
          else
            draw_terrain_tile (mtile, i, j);
        }
    srand (oldrand);
}

void OverviewMap::after_draw()
{
}

void OverviewMap::draw(Player *player)
{
    Tileset *ts = GameMap::getTileset();
    Playerlist::getInstance()->setViewingplayer(player);
    int size = int(pixels_per_tile) > 1 ? int(pixels_per_tile) : 1;
    assert(surface);


    // During the whole drawing stuff, ALWAYS consider that 
    // there is an offset of 1 between map coordinates and coordinates
    // of the surface when drawing. I will implcitely assume this during this
    // function.
    
    //put the static surface on the surface
    surface_gc->set_source(static_surface, 0, 0);
    surface_gc->paint();

    // Draw ruins as a white dot
	
    Gdk::RGBA ruindotcolor = ts->getRuinColor();
    for (Ruinlist::iterator it = Ruinlist::getInstance()->begin();
        it != Ruinlist::getInstance()->end(); it++)
    {
        Ruin *r = *it;
        if (r->isHidden() == true && 
	    r->getOwner() != Playerlist::getViewingplayer())
          continue;
        if (r->isVisible(Playerlist::getViewingplayer()) == false)
          continue;
        Vector<int> pos = r->getPos();
        pos = mapToSurface(pos);

	draw_filled_rect(true, pos.x, pos.y, size, size, ruindotcolor);
    }

    // Draw temples as a white dot
    Gdk::RGBA templedotcolor = ts->getTempleColor();
    for (Templelist::iterator it = Templelist::getInstance()->begin();
        it != Templelist::getInstance()->end(); it++)
    {
      Temple *t = *it;
        if (t->isVisible(Playerlist::getViewingplayer()) == false)
          continue;
        Vector<int> pos = t->getPos();
        pos = mapToSurface(pos);

	draw_filled_rect(true, pos.x, pos.y, size, size, templedotcolor);
    }

    //fog it up
    for (int i = 0; i < GameMap::getWidth(); i++)
        for (int j = 0; j < GameMap::getHeight(); j++)
        {
          Vector <int> pos;
          pos.x = i;
          pos.y = j;
          if (Playerlist::getViewingplayer()->getFogMap()->isFogged(pos) == true)
            {
              pos = mapToSurface(pos);
              draw_filled_rect(true, pos.x, pos.y, size, size, FOG_COLOUR);
            }
        }

    //the idea here is that we want to show what happens when an AI-owned
    //stack moves through our area.  so we block the view of computer
    //players after the fact.
    if (Playerlist::getViewingplayer()->getType() != Player::HUMAN &&
       	GameScenarioOptions::s_hidden_map == true)
      {
	int width = get_width();
	int height = get_height();
	draw_filled_rect(true, 0, 0, width, height, FOG_COLOUR);
      }

  if (blank_screen)
    {
      int width = get_width();
      int height = get_height();
      surface_gc->set_source_rgba(FOG_COLOUR.get_red(), FOG_COLOUR.get_green(), FOG_COLOUR.get_blue(), FOG_COLOUR.get_alpha());
      surface_gc->rectangle(0,0,width, height);
      surface_gc->fill();
    }
    // let derived classes do their job
    after_draw();
}

Cairo::RefPtr<Cairo::Surface> OverviewMap::get_surface()
{
    return surface;
}

Vector<int> OverviewMap::mapFromScreen(Vector<int> pos)
{
    int x = int(pos.x / pixels_per_tile / map_tiles_per_tile);
    int y = int(pos.y / pixels_per_tile / map_tiles_per_tile);
    
    if (x >= GameMap::getWidth())
        x = GameMap::getWidth() - 1;

    if (y >= GameMap::getHeight())
        y = GameMap::getHeight() - 1;

    if (x < 0)
      x = 0;
    
    if (y < 0)
      y = 0;
    
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

    int x = int(round(pos.x * pixels_per_tile / map_tiles_per_tile));
    int y = int(round(pos.y * pixels_per_tile / map_tiles_per_tile));

    if (pixels_per_tile > 2)
        // try to take the center position of the pixel
        x += int(0.5 * pixels_per_tile);

    if (pixels_per_tile > 2)
        y += int(0.5 * pixels_per_tile);
    
    return Vector<int>(x, y);
}

void OverviewMap::draw_cities (bool all_razed)
{
  int csize = 0;
  switch (Configuration::UiFormFactor(Configuration::s_ui_form_factor))
    {
    case Configuration::UI_FORM_FACTOR_DESKTOP:
    case Configuration::UI_FORM_FACTOR_NETBOOK:
      break;
    case Configuration::UI_FORM_FACTOR_LARGE_SCREEN:
      csize = 1;
      break;
    }
  ImageCache *gc = ImageCache::getInstance();

  // Draw all cities as shields over the city location, in the colors of
  // the players.
  for (Citylist::iterator it = Citylist::getInstance()->begin();
      it != Citylist::getInstance()->end(); it++)
  {
      City *c = *it;
      PixMask *tmp;
      if (c->isVisible(Playerlist::getViewingplayer()) == false)
        continue;
      if (c->isBurnt() == true || all_razed == true)
        tmp = gc->getSmallRuinedCityImage();
      else
        tmp = gc->getShieldPic(csize, c->getOwner());
  
      Vector<int> pos = c->getPos();
      pos = mapToSurface(pos);
      tmp->blit_centered(surface, pos);
  }
}

void OverviewMap::blank(bool on)
{
  blank_screen = on;
  draw(Playerlist::getViewingplayer());
}

void OverviewMap::draw_hero(Vector<int> pos, bool white)
{
    ImageCache *gc = ImageCache::getInstance();
    // draw the hero picture over top of the host city

    Vector<int> start = mapToSurface(pos);

    start += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));

    PixMask *heropic = gc->getSmallHeroImage(white);
    heropic->blit_centered(surface, start);
}

void OverviewMap::draw_target_box(Vector<int> pos, const Gdk::RGBA c)
{
  Vector<int> start = mapToSurface(pos);
  start += Vector<int>(int(pixels_per_tile/2), int(pixels_per_tile/2));
  int xsize = 8;
  int ysize = 8;
  //draw an 8 by 8 box, with a smaller box inside of it
  draw_rect(start.x - (xsize / 2), start.y - (ysize / 2), 
	    xsize, ysize, c);
  xsize = 5;
  ysize = 5;
  draw_filled_rect(start.x - (xsize / 2), start.y - (ysize / 2), 
		   xsize, ysize, c);
}

void OverviewMap::draw_square_around_city(City *c, Gdk::RGBA colour)
{
  Vector<int> start = c->getPos();
  start = mapToSurface(start);
  Shieldset *ss = GameMap::getShieldset();
  guint32 width = ss->getSmallWidth();
  guint32 height = ss->getSmallHeight();
  start -= Vector<int>(width,height)/2;
  Vector<int> end = start + Vector<int>(width,height);
  draw_rect (start.x-3, start.y-3, end.x-start.x+4, end.y-start.y+4, colour);
}

void OverviewMap::draw_radial_gradient(Cairo::RefPtr<Cairo::Surface> surface, Gdk::RGBA inner, Gdk::RGBA outer, int width, int height)
{
  double ired = inner.get_red();
  double igreen = inner.get_green();
  double iblue = inner.get_blue();
  double ored = outer.get_red();
  double ogreen = outer.get_green();
  double oblue = outer.get_blue();
  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);
  double max = (double) width;
  if ((double)height > max)
    max = (double)width;
  double xcenter = (double)width / 2.0;
  double ycenter = (double)height / 2.0;
  Cairo::RefPtr<Cairo::RadialGradient> gradient = 
    Cairo::RadialGradient::create(xcenter, ycenter, 1, xcenter, ycenter, max);
  gradient->add_color_stop_rgb(0, ired, igreen, iblue);
  gradient->add_color_stop_rgb(1.0, ored, ogreen, oblue);
  cr->set_source(gradient);
  cr->paint();
}

void OverviewMap::draw_radial_gradient(Gdk::RGBA inner, Gdk::RGBA outer, int width, int height)
{
  return draw_radial_gradient(static_surface, inner, outer, width, height);
}

int OverviewMap::get_width()
{
  return GameMap::get_dim().x / map_tiles_per_tile * pixels_per_tile;
}

int OverviewMap::get_height()
{
  return GameMap::get_dim().y / map_tiles_per_tile * pixels_per_tile;
}
