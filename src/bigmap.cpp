// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006, 2007 Ulf Lorenz
// Copyright (C) 2004, 2005 Bryan Duff
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
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

#include <config.h>

#include <SDL_image.h>
#include <assert.h>
#include <stdlib.h>

#include "bigmap.h"

#include "army.h"
#include "path.h"
#include "stacklist.h"
#include "stack.h"
#include "citylist.h"
#include "ruinlist.h"
#include "signpostlist.h"
#include "templelist.h"
#include "armysetlist.h"
#include "portlist.h"
#include "bridgelist.h"
#include "roadlist.h"
#include "ruin.h"
#include "signpost.h"
#include "temple.h"
#include "road.h"
#include "playerlist.h"
#include "File.h"
#include "GameMap.h"
#include "Configuration.h"
#include "GraphicsCache.h"
#include "GraphicsLoader.h"
#include "MapRenderer.h"
#include "FogMap.h"
#include "sdl-draw.h"

#include <iostream>
using namespace std;
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

BigMap::BigMap()
    : d_renderer(0), buffer(0), magnified_buffer(0), magnification_factor(1.0)
{
    // load all pictures
    d_itempic = GraphicsLoader::getMiscPicture("items.png");

    // note: we are not fully initialized before set_view is called
    view.x = view.y = 0;

    d_grid_toggled = false;
}

BigMap::~BigMap()
{
    SDL_FreeSurface(d_itempic);

    if (buffer)
        SDL_FreeSurface(buffer);

    if (magnified_buffer)
        SDL_FreeSurface(magnified_buffer);

    delete d_renderer;
}

void BigMap::set_view(Rectangle new_view)
{
    int tilesize = GameMap::getInstance()->getTileset()->getTileSize();
    
    if (view.dim == new_view.dim && buffer)
    {
	// someone wants us to move the view, not resize it, no need to
	// construct new surfaces and all that stuff

	view = new_view;
	Vector<int> new_view_pos = get_view_pos_from_view();

	if (view_pos != new_view_pos)
	{
	    view_pos = new_view_pos;
	    draw();
	}
	
	return;
    }

    view = new_view;
    view_pos = get_view_pos_from_view();
    
    if (buffer)
        SDL_FreeSurface(buffer);
    if (d_renderer)
        delete d_renderer;

    // now create a buffer surface which is two maptiles wider and
    // higher than the screen you actually see. That is how smooth scrolling
    // becomes comparatively easy. You just blit from the extended screen to
    // the screen with some offset.
    // this represents a 1 tile border around the outside of the picture.
    // it gets rid of the black border.
    buffer_view.dim = view.dim + Vector<int>(2, 2);

    SDL_PixelFormat* fmt = SDL_GetVideoSurface()->format;
    buffer = SDL_CreateRGBSurface
      (SDL_SWSURFACE, buffer_view.w * tilesize, buffer_view.h * tilesize,
       fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

    // now set the MapRenderer so that it draws directly on the new surface
    d_renderer = new MapRenderer(buffer);
}

void BigMap::draw(bool redraw_buffer)
{
    // no size and buffer yet, return
    if (!buffer)
        return;

    SDL_Surface* screen = SDL_GetVideoSurface();
    int tilesize = GameMap::getInstance()->getTileset()->getTileSize();

    // align the buffer view
    Vector<int> new_buffer_view = clip(
	Vector<int>(0, 0),
	view.pos - Vector<int>(1, 1),
	GameMap::get_dim() - buffer_view.dim + Vector<int>(1, 1));
    buffer_view.pos = new_buffer_view;

    // redraw the buffer
    if (redraw_buffer)
	draw_buffer();

    // blit the visible part of buffer to the screen
    Vector<int> p = view_pos - (buffer_view.pos * tilesize * magnification_factor);
    //assert(p.x >= 0 && p.x + screen->w <= buffer_view.w * tilesize * magnification_factor &&
	   //p.y >= 0 && p.y + screen->h <= buffer_view.h * tilesize * magnification_factor);
    SDL_Rect src, dest;
    src.x = p.x;
    src.y = p.y;
    src.w = screen->w;
    src.h = screen->h;
    dest.w = screen->w;
    dest.h = screen->h;
    dest.x = dest.y = 0;
    magnify();
    //now we want to take a portion of what we magnified
    SDL_BlitSurface(magnified_buffer, &src, screen, &dest);
    //SDL_BlitSurface(buffer, &src, screen, &dest);
    SDL_UpdateRects(screen, 1, &dest);
}

void BigMap::screen_size_changed()
{
    SDL_Surface *v = SDL_GetVideoSurface();
    int ts = GameMap::getInstance()->getTileset()->getTileSize();

    Rectangle new_view = view;
    
    new_view.w = v->w / (ts * magnification_factor) + 1;
    new_view.h = v->h / (ts * magnification_factor) + 1;

    if (new_view.w <= GameMap::getWidth() && new_view.h <= GameMap::getHeight()
	&& new_view.w >= 0 && new_view.h >= 0)
      {
	new_view.pos = clip(Vector<int>(0,0), new_view.pos,
			    GameMap::get_dim() - new_view.dim);

	if (new_view != view)
	  {
	    set_view(new_view);
	    view_changed.emit(view);
	  }
      }
}

Vector<int> BigMap::get_view_pos_from_view()
{
    SDL_Surface *screen = SDL_GetVideoSurface();
    Vector<int> screen_dim(screen->w, screen->h);
    int ts = GameMap::getInstance()->getTileset()->getTileSize();

    // clip to make sure we don't see a black border at the bottom and right
    return clip(Vector<int>(0, 0), view.pos * ts * magnification_factor,
		GameMap::get_dim() * ts * magnification_factor - screen_dim);
}

Vector<int> BigMap::tile_to_buffer_pos(Vector<int> tile)
{
    int ts = GameMap::getInstance()->getTileset()->getTileSize();
    return (tile - buffer_view.pos) * ts;
}

Vector<int> BigMap::mouse_pos_to_tile(Vector<int> pos)
{
    int ts = GameMap::getInstance()->getTileset()->getTileSize();

    return (view_pos + pos) / (ts * magnification_factor);
}

Vector<int> BigMap::mouse_pos_to_tile_offset(Vector<int> pos)
{
    int ts = GameMap::getInstance()->getTileset()->getTileSize();

    return (view_pos + pos) % (int)rint(ts * magnification_factor);
}

MapTipPosition BigMap::map_tip_position(Vector<int> tile)
{
  return map_tip_position (Rectangle(tile.x, tile.y, 1, 1)); 
}

MapTipPosition BigMap::map_tip_position(Rectangle tile_area)
{
    // convert area to pixels on the SDL screen
    int tilesize = GameMap::getInstance()->getTileset()->getTileSize();

    Rectangle area(tile_area.pos * tilesize * magnification_factor - view_pos,
		   tile_area.dim * tilesize * magnification_factor);

    // calculate screen edge distances
    SDL_Surface *screen = SDL_GetVideoSurface();
    int left, right, top, bottom;
    
    left = area.x;
    right = screen->w - (area.x + area.w);
    top = area.y;
    bottom = screen->h - (area.y + area.h);

    int const MARGIN = 2;
    
    // then set the position
    MapTipPosition m;
    if (right >= left && right >= top && right >= bottom)
    {
	m.pos.x = area.x + area.w + MARGIN;
	m.pos.y = area.y;
	m.justification = MapTipPosition::LEFT;
    }
    else if (left >= top && left >= bottom)
    {
	m.pos.x = area.x - MARGIN;
	m.pos.y = area.y;
	m.justification = MapTipPosition::RIGHT;
    }
    else if (bottom >= top)
    {
	m.pos.x = area.x;
	m.pos.y = area.y + area.h + MARGIN;
	m.justification = MapTipPosition::TOP;
    }
    else
    {
	m.pos.x = area.x;
	m.pos.y = area.y - MARGIN;
	m.justification = MapTipPosition::BOTTOM;
    }
    
    return m;
}

void BigMap::blit_object(const Location &obj, SDL_Surface *image, SDL_Surface *surface)
{
  Vector<int> p = tile_to_buffer_pos(obj.getPos());
  SDL_Rect rect;
  rect.x = p.x;
  rect.y = p.y;
  rect.w = obj.getSize();
  rect.h = obj.getSize();
  SDL_BlitSurface(image, 0, surface, &rect);
}

bool BigMap::blit_if_inside_buffer(const Location &obj, SDL_Surface *image,
				   Rectangle &map_view, SDL_Surface *surface)
{
  if (is_overlapping(map_view, obj.get_area()))
    {
      blit_object(obj, image, surface);
      return true;
    }
  return false;
}


/*
   fog display algorithm

   smallmap shows fog placement
   - it is a peek into the data model

   bigmap shows a rendering of that
   if a tile on the bigmap is partially fogged, then it is completely fogged on the small map.

   this means that partially fogged tiles depend on adjacent tiles being fogged
   completely fogged tiles depends on adjacent tiles being fogged

   when one tile is fogged and is not surrounded by adjacent fogged tiles it is shown as not fogged on the bigmap, while it is shown as fogged on the small map.  it is then marked as defogged at the start of the next turn.

   every tile has 4 faces:
   it can connect to an adjacent tile darkly, lightly, or not at all
   a dark face means the whole side is black
   a light face means the side is a gradient


   graphics cache
   fog types:
   1 = light corner se: connects lightly to south and east
   2 = light corner sw: connects lightly to south and west
   3 = light corner nw: connects lightly to north and west
   4 = light corner ne: connects lightly to north and east
   5 = dark corner nw: connects darkly to north and west, lightly to south and east
   6 = dark corner ne: connects darkly to north and east, lightly to south and west
   7 = dark corner se: connects darkly to east and south, lightly to north and west
   8 = dark corner sw: connects darkly to south and west,  lightly to north and east
   9 = bottom to top: connects darkly to south, connects lightly to east and west
   10 =  top to bottom: connects darkly to north, connects lightly to east and west
   11 = right to left: connects darkly to west, connects lightly to north and south
   12 = left to right: connects darkly to east, connects lightly to north and south
   13 = all black: connects darkly to north, south, east and west

   bigmap tile processing algorithm:
   for each tile currently being shown, examine each tile in normal order

   here are the cases that we can handle for fogging a tile:
   the sets are read as follows:

   876
   5x4 = (fog tile type)
   321
   (bit count)

   the most significant bit is in the 1st position, and the least sigificant bit
   is in the 8th position

   we check each position and if it's a fogged tile, then we add a 1 to that
   bit position.

   111
   1x1 = 13
   111
   (255)  (all 8 bits on is 255)

   111      111      011     110
   1x1 = 5  1x1 = 6  1x1 = 7 1x1 = 8
   110      011      111     111
   (127)    (223)    (254)   (251) (e.g. 251 == 11111011)

   101      111      111     111
   1x1 = 9  1x0 = 12 1x1 =10 0x1 = 11
   111      111      101     111
   (253)    (239)    (191)   (247) (e.g. 247 == 11110111)

   001      111      100     111
   1x1 = 9  1x1 = 10 1x1 = 9 1x1 = 10
   111      001      111     100
(252)    (159)    (249)   (63)

	011      111      110     111
	0x1 = 11 0x1 = 11 1x0 =12 1x0 = 12
111      011      111     110
(246)    (215)    (235)   (111)

	000      000      110      011
	0x1 = 1  1x0 = 2  1x0 = 3  0x1 = 4
011      110      000      000
(208)    (104)    (11)     (22)


	000      111      011      110
	1x1 = 9  1x1 = 10 0x1 = 11 1x0 = 12
111      000      011      110
(248)    (31)     (214)    (107)


	001      111      100     111
	0x1 = 1  0x1 = 4  1x0 = 2 1x0 = 3
111      001      111     100
(244)    (151)    (233)   (47)

	000      011      000     111
	0x1 = 1  0x1 = 4  1x0 = 2 1x0 = 3
111      001      111     000
(240)    (150)    (232)   (15)

	100      110      001     111
	1x0 = 2  1x0 = 3  0x1 = 1 0x1 = 4
110      100      011     000
(105)    (43)     (232)   (15)

	011      110
	1x1 = 14 1x1 = 15
110      011
(126)    (219)

	special note:
	none of these sets contain a so-called "lone" tile.
	a lone tile is a fogged tile surrounded by two unfogged tiles on either side.
*/
void BigMap::drawFogTile (int x, int y)
{
  int idx = 0;
  int count = 0;
  bool foggyTile;
  for (int i = x - 1; i <= x + 1; i++)
    for (int j = y - 1; j <= y + 1; j++)
      {
	foggyTile = false;
	if (i == x && j == y)
	  continue;
	if (i < 0 || j < 0 || 
	    i >= GameMap::getWidth() || j >= GameMap::getHeight())
	  foggyTile = true;
	else
	  {
	    Vector<int> pos;
	    pos.x = i;
	    pos.y = j;
	    foggyTile = FogMap::isFogged(pos);
	  }
	if (foggyTile)
	  {
	    switch (count)
	      {
	      case 0: idx += 1; break;
	      case 1: idx += 2; break;
	      case 2: idx += 4; break;
	      case 3: idx += 8; break;
	      case 4: idx += 16; break;
	      case 5: idx += 32; break;
	      case 6: idx += 64; break;
	      case 7: idx += 128; break;
	      }
	  }

	count++;
      }

  //now idx relates to a particular fog picture
  int type = 0;
  switch (idx)
    {
    case 208: case 212: case 240: case 244: type = 1; break;
    case 104: case 105: case 232: case 233: type = 2; break;
    case  11: case 15: case 43: case 47: type = 3; break;
    case  22: case 150: case 151: case 23: type = 4; break;
    case 127: type = 5; break;
    case 223: type = 6; break;
    case 254: type = 7; break;
    case 251: type = 8; break;
    case 248: case 249: case 252: case 253: type = 9; break;
    case  31: case 63: case 159: case 191: type = 10; break;
    case 214: case 215: case 246: case 247: type = 11; break;
    case 107: case 111: case 235: case 239: type = 12; break;
    case 126: type = 14; break;
    case 219: type = 15; break;
    case 255: type = 13; break;
    }
  if (type)
    {
      switch (type) //fixme: figure out why this flipping is necessary!
	{
	case 12: type = 10; break;
	case 10: type = 12; break;
	case 9: type = 11; break;
	case 11: type = 9; break;
	case 6: type = 8; break;
	case 8: type = 6; break;
	case 2: type = 4; break;
	case 4: type = 2; break;
	}
      Vector<int> p = tile_to_buffer_pos(Vector<int>(x, y));
      SDL_Rect r;
      r.x = p.x;
      r.y = p.y;
      r.w = GameMap::getInstance()->getTileset()->getTileSize();
      SDL_BlitSurface(GraphicsCache::getInstance()->getFogPic(type - 1), 0, 
		      buffer, &r);
    }
  return;
}

void BigMap::draw_stack(Stack *s)
{
  GameMap *gm = GameMap::getInstance();
  GraphicsCache *gc = GraphicsCache::getInstance();
  int tilesize = GameMap::getInstance()->getTileset()->getTileSize();
  Vector<int> p = s->getPos();
  Player *player = s->getOwner();
  int army_tilesize;

  // check if the object lies in the viewed part of the map
  // otherwise we shouldn't draw it
  if (is_inside(buffer_view, p) && !s->getDeleting())
    {
      Armysetlist *al = Armysetlist::getInstance();
      army_tilesize = al->getTileSize(player->getArmyset());
      if (s->empty())
	{
	  std::cerr << "WARNING: empty stack found" << std::endl;
	  return;
	}

      p = tile_to_buffer_pos(p);

      // draw stack

      SDL_Rect r;
      r.x = p.x;
      r.y = p.y;

      bool show_army = true;
      if (s->hasShip())
	{
	  r.w = r.h = army_tilesize;
	  SDL_BlitSurface(gc->getShipPic(player), 0, buffer, &r);
	}
      else
	{
	  if (s->getFortified() == true)
	    {
	      r.w = r.h = tilesize;
	      if (player->getStacklist()->getActivestack() != s &&
		  player == Playerlist::getActiveplayer())
		show_army = false;
	      Maptile *tile = gm->getTile(s->getPos());
	      if (tile->getBuilding() != Maptile::CITY &&
		  tile->getBuilding() != Maptile::RUIN &&
		  tile->getBuilding() != Maptile::TEMPLE)
		SDL_BlitSurface(gc->getTowerPic(player), 0, buffer, &r);
	      else
		show_army = true;
	    }

	  if (show_army == true)
	    {
	      r.w = r.h = army_tilesize;
	      Army *a = *s->begin();
	      SDL_BlitSurface(gc->getArmyPic(a), 0, buffer, &r);
	    }
	}


      if (show_army)
	{
	  // draw flag
	  r.x = p.x;
	  r.y = p.y;
	  r.w = r.h = tilesize;
	  SDL_BlitSurface(gc->getFlagPic(s), 0, buffer, &r);
	}
    }
}

void BigMap::draw_buffer()
{
  draw_buffer (buffer_view, buffer);
  after_draw();

}

bool BigMap::saveViewAsBitmap(std::string filename)
{
  remove (filename.c_str());
  SDL_SaveBMP(buffer, filename.c_str());
  return true;
}

bool BigMap::saveUnderlyingMapAsBitmap(std::string filename)
{
  return d_renderer->saveAsBitmap(filename);
}

bool BigMap::saveAsBitmap(std::string filename)
{
  int tilesize = GameMap::getInstance()->getTileset()->getTileSize();
  SDL_PixelFormat *fmt = buffer->format;
  SDL_Surface *surf = SDL_CreateRGBSurface 
    (SDL_SWSURFACE, 
     GameMap::getWidth() * tilesize, GameMap::getHeight() * tilesize,
     fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

  draw_buffer(Rectangle (0, 0, GameMap::getWidth(), GameMap::getHeight()), surf);
  remove (filename.c_str());
  SDL_SaveBMP(surf, filename.c_str());
  SDL_FreeSurface(surf);
  return true;
}

void BigMap::draw_buffer(Rectangle map_view, SDL_Surface *surface)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  int tilesize = GameMap::getInstance()->getTileset()->getTileSize();
  d_renderer->render(0, 0, map_view.x, map_view.y, map_view.w, map_view.h,
		     surface);

  for (Ruinlist::iterator i = Ruinlist::getInstance()->begin();
       i != Ruinlist::getInstance()->end(); ++i)
    {
      if (((*i).isHidden() == true && 
	   (*i).getOwner() == Playerlist::getActiveplayer()) ||
	  (*i).isHidden() == false)
	blit_if_inside_buffer(*i, gc->getRuinPic((*i).getType()), map_view,
			      surface);
    }

  for (Signpostlist::iterator i = Signpostlist::getInstance()->begin();
       i != Signpostlist::getInstance()->end(); ++i)
    blit_if_inside_buffer(*i, gc->getSignpostPic(), map_view, surface);

  for (Templelist::iterator i = Templelist::getInstance()->begin();
       i != Templelist::getInstance()->end(); ++i)
    blit_if_inside_buffer( *i, gc->getTemplePic(i->getType()), map_view, surface);

  for (Roadlist::iterator i = Roadlist::getInstance()->begin();
       i != Roadlist::getInstance()->end(); ++i)
    blit_if_inside_buffer( *i, gc->getRoadPic(i->getType()), map_view, surface);

  for (Bridgelist::iterator i = Bridgelist::getInstance()->begin();
       i != Bridgelist::getInstance()->end(); ++i)
    blit_if_inside_buffer( *i, gc->getBridgePic(i->getType()), map_view, surface);

  for (Portlist::iterator i = Portlist::getInstance()->begin();
       i != Portlist::getInstance()->end(); ++i)
    blit_if_inside_buffer( *i, gc->getPortPic(), map_view, surface);

  for (Citylist::iterator i = Citylist::getInstance()->begin();
       i != Citylist::getInstance()->end(); ++i)
    blit_if_inside_buffer(*i, gc->getCityPic(&*i), map_view, surface);

  GameMap *gm = GameMap::getInstance();
  // If there are any items lying around, blit the itempic as well
  for (int x = map_view.x; x < map_view.x + map_view.w; x++)
    for (int y = map_view.y; y < map_view.y + map_view.h; y++)
      if (x < GameMap::getWidth() && y < GameMap::getHeight()
	  && !gm->getTile(x,y)->getItems().empty())
	{
	  std::list<Item*> items = gm->getTile(x, y)->getItems();
	  bool standard_planted = false;
	  Item *flag = NULL;
	  for (std::list<Item*>::iterator it = items.begin(); 
	       it != items.end(); it++)
	    {
	      if ((*it)->getPlanted() == true)
		{
		  standard_planted = true;
		  flag = *it;
		  break;
		}
	    }

	  //only show one of the bag or the flag
	  Vector<int> p = tile_to_buffer_pos(Vector<int>(x, y));
	  if (standard_planted && flag)
	    {
	      Armysetlist *al = Armysetlist::getInstance();
	      Player *player = flag->getPlantableOwner();
	      int army_tilesize = al->getTileSize(player->getArmyset());
	      SDL_Rect r;
	      r.x = p.x+((tilesize/2)-(army_tilesize/2));
	      r.y = p.y+((tilesize/2)-(army_tilesize/2));
	      r.w = r.h = army_tilesize;
	      SDL_Surface *surf;
	      surf = gc->getPlantedStandardPic(player);
	      SDL_BlitSurface(surf, 0, surface,&r);
	    }
	  else
	    {
	      SDL_Rect r;
	      r.x = p.x+(tilesize-18);
	      r.y = p.y+(tilesize-18);
	      r.w = r.h = 16;
	      SDL_BlitSurface(d_itempic, 0, surface,&r);
	    }
	}

  // Draw stacks
  for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
       pit != Playerlist::getInstance()->end(); pit++)
    {
      Stacklist* mylist = (*pit)->getStacklist();
      for (Stacklist::iterator it= mylist->begin(); it != mylist->end(); it++)
	{
	  if (*pit == Playerlist::getInstance()->getActiveplayer() &&
	      *it == (*pit)->getStacklist()->getActivestack())
	    ; //skip it.  the selected stack gets drawn in gamebigmap.
	  else
	    draw_stack (*it);
	}
    }

  // draw the grid
  if (d_grid_toggled)
    {
      for (int x = map_view.x; x < map_view.x + map_view.w; x++)
	{
	  for (int y = map_view.y; y < map_view.y + map_view.h; y++)
	    {
	      if (x < GameMap::getWidth() && y < GameMap::getHeight())
		{
		  Vector<int> p = tile_to_buffer_pos(Vector<int>(x, y));
		  Uint32 raw = SDL_MapRGB(surface->format, 0, 0, 0);
		  draw_rect_clipped(surface, p.x, p.y, p.x + tilesize,
				    p.y + tilesize, raw);
		}
	    }
	}
    }

  // fog it up
  for (int x = map_view.x; x < map_view.x + map_view.w; x++)
    {
      for (int y = map_view.y; y < map_view.y + map_view.h; y++)
	{
	  if (x < GameMap::getWidth() && y < GameMap::getHeight())
	    {
	      Vector<int> pos;
	      pos.x = x;
	      pos.y = y;
	      if (FogMap::isFogged(pos))
		drawFogTile (x, y);
	    }
	}
    }

}

void BigMap::blank ()
{
  // fog it up
  for (int x = buffer_view.x; x < buffer_view.x + buffer_view.w; x++)
    {
      for (int y = buffer_view.y; y < buffer_view.y + buffer_view.h; y++)
	{
	  if (x < GameMap::getWidth() && y < GameMap::getHeight())
	    {
	      Vector<int> pos;
	      pos.x = x;
	      pos.y = y;
	      drawFogTile (x, y);
	    }
	}
    }
}

//here we want to magnify the entire buffer, not a subset
void BigMap::magnify()
{
  //magnify the buffer into a buffer of the correct size
  if (magnified_buffer)
    SDL_FreeSurface(magnified_buffer);
  SDL_PixelFormat* fmt = SDL_GetVideoSurface()->format;
  magnified_buffer = SDL_CreateRGBSurface
    (SDL_SWSURFACE, buffer->w * magnification_factor, 
     buffer->h * magnification_factor,
     fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
  SDL_Rect s;
  s.x = s.y = 0;
  s.w = buffer->w;
  s.h = buffer->h;
  SDL_Rect r;
  r.x = r.y = 0;
  r.w = magnified_buffer->w;
  r.h = magnified_buffer->h;
  SDL_SoftStretch (buffer, &s, magnified_buffer, &r);
}

void BigMap::toggle_grid()
{
  d_grid_toggled = !d_grid_toggled;
  draw(true);
}
