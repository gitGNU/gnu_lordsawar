// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006, 2007 Ulf Lorenz
// Copyright (C) 2004, 2005 Bryan Duff
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009 Ben Asselstine
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
#include "MapBackpack.h"

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

    image = Gtk::Allocation(0, 0, 320, 200);
}

BigMap::~BigMap()
{
  d_itempic.clear();

    if (buffer == true)
      buffer.clear();

    if (magnified_buffer == true)
      magnified_buffer.clear();

    delete d_renderer;
}

void BigMap::set_view(Rectangle new_view)
{
    int tilesize = GameMap::getInstance()->getTileset()->getTileSize();
    
    if (view.dim == new_view.dim && buffer)
    {
	// someone wants us to move the view, not resize it, no need to
	// construct new surfaces and all that stuff
	//
	// fixme: if we're moving the view, maybe there's some pixmap in common
	// between this view and the new view.  why render?

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
    
    // now create a buffer surface which is two maptiles wider and
    // higher than the screen you actually see. That is how smooth scrolling
    // becomes comparatively easy. You just blit from the extended screen to
    // the screen with some offset.
    // this represents a 1 tile border around the outside of the picture.
    // it gets rid of the black border.

    if (buffer == true)
      buffer.clear();
    
    buffer_view.dim = view.dim + Vector<int>(2, 2);

    buffer = Gdk::Pixmap::create (Glib::RefPtr<Gdk::Drawable>(0), buffer_view.w * tilesize, buffer_view.h * tilesize, 24);
    buffer_gc = Gdk::GC::create(buffer);

    //now create the part that will go out to the gtk::image
    if (outgoing == true)
      outgoing.clear();
    outgoing = Gdk::Pixmap::create(Glib::RefPtr<Gdk::Drawable>(buffer), image.get_width(), image.get_height(), 24);


    if (d_renderer)
        delete d_renderer;
    // now set the MapRenderer so that it draws on the buffer
    d_renderer = new MapRenderer(buffer);
}

void BigMap::clip_viewable_buffer(Glib::RefPtr<Gdk::Pixmap> pixmap, Glib::RefPtr<Gdk::GC> gc, Vector<int> pos, Glib::RefPtr<Gdk::Pixmap> out)
{
    //Glib::RefPtr<Gdk::Pixmap> outgoing = Gdk::Pixmap::create(Glib::RefPtr<Gdk::Drawable>(pixmap), image.get_width(), image.get_height(), 24) ;
    int width = 0;
    int height = 0;
    pixmap->get_size(width,height);
    out->draw_drawable(gc, pixmap, pos.x, pos.y, 0, 0, image.get_width(), image.get_height());
    return;
}
void BigMap::draw(bool redraw_buffer)
{
    // no size and buffer yet, return
    if (!buffer)
        return;

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
    //Glib::RefPtr<Gdk::Pixmap> outgoing;
    if (magnification_factor != 1.0)
      {
	if (magnified_buffer == true)
	  magnified_buffer.clear();
	magnified_buffer = magnify(buffer);
	clip_viewable_buffer(magnified_buffer, buffer_gc, p, outgoing);
      }
    else
      {
	clip_viewable_buffer(buffer, buffer_gc, p, outgoing);
      }

    map_changed.emit(outgoing);
}

void BigMap::screen_size_changed(Gtk::Allocation box)
{
    int ts = GameMap::getInstance()->getTileset()->getTileSize();

    Rectangle new_view = view;
    
    image = box;
    new_view.w = image.get_width() / (ts * magnification_factor) + 1;
    new_view.h = image.get_height() / (ts * magnification_factor) + 1;

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
    Vector<int> screen_dim(image.get_width(), image.get_height());
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
    // convert area to pixels on the screen
    int tilesize = GameMap::getInstance()->getTileset()->getTileSize();

    Rectangle area(tile_area.pos * tilesize * magnification_factor - view_pos,
		   tile_area.dim * tilesize * magnification_factor);

    // calculate screen edge distances
    int left, right, top, bottom;
    
    left = area.x;
    right = image.get_width() - (area.x + area.w);
    top = area.y;
    bottom = image.get_height() - (area.y + area.h);

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

void BigMap::blit_object(const Location &obj, Glib::RefPtr<Gdk::Pixbuf> image, Glib::RefPtr<Gdk::Pixmap> surface)
{
  int tilesize = GameMap::getInstance()->getTileset()->getTileSize();
  Vector<int> p = tile_to_buffer_pos(obj.getPos());
  surface->draw_pixbuf(image, 0, 0, p.x, p.y, obj.getSize() * tilesize, obj.getSize() * tilesize, Gdk::RGB_DITHER_NONE, 0, 0);
}

bool BigMap::blit_if_inside_buffer(const Location &obj, 
				   Glib::RefPtr<Gdk::Pixbuf> image,
				   Rectangle &map_view, 
				   Glib::RefPtr<Gdk::Pixmap> surface)
{
  if (is_overlapping(map_view, obj.get_area()))
    {
      blit_object(obj, image, surface);
      return true;
    }
  return false;
}


/*
 * fog display algorithm
 *
 * smallmap shows fog placement
 * - it is a peek into the data model
 *
 * bigmap shows a rendering of that
 * if a tile on the bigmap is partially fogged, then it is completely fogged on the small map.
 *
 * this means that partially fogged tiles depend on adjacent tiles being fogged
 * completely fogged tiles depends on adjacent tiles being fogged
 *
 * when one tile is fogged and is not surrounded by adjacent fogged tiles it is shown as not fogged on the bigmap, while it is shown as fogged on the small map.  it is then marked as defogged at the start of the next turn.
 *
 * every tile has 4 faces:
 * it can connect to an adjacent tile darkly, lightly, or not at all
 * a dark face means the whole side is black
 * a light face means the side is a gradient *
 *
 *
 * graphics cache
 * fog types:
 * 1 = light corner se: connects lightly to south and east
 * 2 = light corner sw: connects lightly to south and west
 * 3 = light corner nw: connects lightly to north and west
 * 4 = light corner ne: connects lightly to north and east
 * 5 = dark corner nw: connects darkly to north and west, lightly to south and east
 * 6 = dark corner ne: connects darkly to north and east, lightly to south and west
 * 7 = dark corner se: connects darkly to east and south, lightly to north and west
 * 8 = dark corner sw: connects darkly to south and west,  lightly to north and east
 * 9 = bottom to top: connects darkly to south, connects lightly to east and west
 * 10 =  top to bottom: connects darkly to north, connects lightly to east and west
 * 11 = right to left: connects darkly to west, connects lightly to north and south
 * 12 = left to right: connects darkly to east, connects lightly to north and south
 * 13 = all black: connects darkly to north, south, east and west
 *
 * bigmap tile processing algorithm:
 * for each tile currently being shown, examine each tile in normal order
 *
 * here are the cases that we can handle for fogging a tile:
 * the sets are read as follows:
 *
 * 876
 * 5x4 = (fog tile type)
 * 321
 * (bit count)
 *
 * the most significant bit is in the 1st position, and the least sigificant bit
 * is in the 8th position
 *
 * we check each position and if it's a fogged tile, then we add a 1 to that
 * bit position.
 *
 * 111
 * 1x1 = 13
 * 111
 * (255)  (all 8 bits on is 255)
 *
 * 111      111      011     110
 * 1x1 = 5  1x1 = 6  1x1 = 7 1x1 = 8
 * 110      011      111     111
 * (127)    (223)    (254)   (251) (e.g. 251 == 11111011)
 *
 * 101      111      111     111
 * 1x1 = 9  1x0 = 12 1x1 =10 0x1 = 11
 * 111      111      101     111
 * (253)    (239)    (191)   (247) (e.g. 247 == 11110111)
 *
 * 001      111      100     111
 * 1x1 = 9  1x1 = 10 1x1 = 9 1x1 = 10
 * 111      001      111     100
 * (252)    (159)    (249)   (63)
 *
 * 011      111      110     111
 * 0x1 = 11 0x1 = 11 1x0 =12 1x0 = 12
 * 111      011      111     110
 * (246)    (215)    (235)   (111)
 *
 * 000      000      110      011
 * 0x1 = 1  1x0 = 2  1x0 = 3  0x1 = 4
 * 011      110      000      000
 * (208)    (104)    (11)     (22)
 *
 *
 * 000      111      011      110
 * 1x1 = 9  1x1 = 10 0x1 = 11 1x0 = 12
 * 111      000      011      110
 * (248)    (31)     (214)    (107)
 *
 *
 * 001      111      100     111
 * 0x1 = 1  0x1 = 4  1x0 = 2 1x0 = 3
 * 111      001      111     100
 * (244)    (151)    (233)   (47)
 *
 * 000      011      000     111
 * 0x1 = 1  0x1 = 4  1x0 = 2 1x0 = 3
 * 111      001      111     000
 * (240)    (150)    (232)   (15)
 *
 * 100      110      001     111
 * 1x0 = 2  1x0 = 3  0x1 = 1 0x1 = 4
 * 110      100      011     000
 * (105)    (43)     (232)   (15)
 *
 * 011      110
 * 1x1 = 14 1x1 = 15
 * 110      011
 * (126)    (219)
 *
 *special note:
 *none of these sets contain a so-called "lone" tile.
 *a lone tile is a fogged tile surrounded by two unfogged tiles on either side.
**/
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
	    foggyTile = FogMap::isFogged(pos, Playerlist::getActiveplayer());
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
    case 208: case 212: case 240: case 244: case 242: case 216: case 220: case 210: case 217: case 211: case 218: case 209: type = 1; break;
    case 104: case 105: case 232: case 233: case 121: case 120: case 110: case 106: case 122: case 124: case 234: case 108: type = 2; break;
    case  11: case 15: case 43: case 47: case 59: case 27: case 79: case 75: case 155: case 203: case 139: case 91: type = 3; break;
    case  22: case 150: case 151: case 23: case 87: case 86: case 158: case 118:case 94: case 30: case 62: case 54: type = 4; break;
    case 127: type = 5; break;
    case 223: type = 6; break;
    case 254: type = 7; break;
    case 251: type = 8; break;
    case 248: case 249: case 252: case 253: case 250: type = 9; break;
    case  31: case 63: case 159: case 191: case 95: type = 10; break;
    case 214: case 215: case 246: case 247: case 222: type = 11; break;
    case 107: case 111: case 235: case 239: case 123: type = 12; break;
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
      int ts= GameMap::getInstance()->getTileset()->getTileSize();
      buffer->draw_pixbuf (GraphicsCache::getInstance()->getFogPic(type - 1), 
			   0, 0, p.x, p.y, ts, ts, Gdk::RGB_DITHER_NONE, 0, 0);
    }
  return;
}

void BigMap::debugFogTile (int x, int y)
{
  int idx = 0;
  int count = 0;
  bool foggyTile;
  printf ("isFogged() == %d ", FogMap::isFogged(Vector<int>(x,y), Playerlist::getActiveplayer()));
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
	    Vector<int> pos = Vector<int>(i, j);
	    foggyTile = FogMap::isFogged(pos, Playerlist::getActiveplayer());
	  }
	if (foggyTile)
	  {
	    printf ("1");
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
	else
	    printf ("0");

	count++;
      }
  printf (" = %d\n", idx);
}

void BigMap::draw_stack(Stack *s, Glib::RefPtr<Gdk::Pixmap> surface)
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

      bool show_army = true;
      if (s->hasShip())
	{
	  surface->draw_pixbuf(gc->getShipPic(player), 0, 0, p.x, p.y, army_tilesize, army_tilesize, Gdk::RGB_DITHER_NONE, 0, 0);
	}
      else
	{
	  if (s->getFortified() == true)
	    {
	      if (player->getStacklist()->getActivestack() != s &&
		  player == Playerlist::getActiveplayer())
		show_army = false;
	      Maptile *tile = gm->getTile(s->getPos());
	      if (tile->getBuilding() != Maptile::CITY &&
		  tile->getBuilding() != Maptile::RUIN &&
		  tile->getBuilding() != Maptile::TEMPLE)
	      surface->draw_pixbuf(gc->getTowerPic(player), 0, 0, p.x, p.y, army_tilesize, army_tilesize, Gdk::RGB_DITHER_NONE, 0, 0);
	      else
		show_army = true;
	    }

	  if (show_army == true)
	    {
	      Army *a = *s->begin();
	      surface->draw_pixbuf(gc->getArmyPic(a), 0, 0, p.x, p.y, army_tilesize, army_tilesize, Gdk::RGB_DITHER_NONE, 0, 0);
	    }
	}


      if (show_army)
	{
	  // draw flag
	  surface->draw_pixbuf(gc->getFlagPic(s), 0, 0, p.x, p.y, tilesize, tilesize, Gdk::RGB_DITHER_NONE, 0, 0);
	}
    }
}

void BigMap::draw_buffer()
{
  draw_buffer (buffer_view, buffer, buffer_gc);
  after_draw();

}

bool BigMap::saveViewAsBitmap(std::string filename)
{
  int width;
  int height;
  buffer->get_size(width, height);
  remove (filename.c_str());
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create(Glib::RefPtr<Gdk::Drawable>(buffer), 0, 0, width, height);
  pixbuf->save (filename, "png");
  return true;
}

bool BigMap::saveUnderlyingMapAsBitmap(std::string filename)
{
  return d_renderer->saveAsBitmap(filename);
}

bool BigMap::saveAsBitmap(std::string filename)
{
  int tilesize = GameMap::getInstance()->getTileset()->getTileSize();
  int width = GameMap::getWidth() * tilesize;
  int height = GameMap::getHeight() * tilesize;
  Glib::RefPtr<Gdk::Pixmap> surf = Gdk::Pixmap::create(Glib::RefPtr<Gdk::Drawable>(0), width, height, 24);
  
  bool orig_grid = d_grid_toggled;
  d_grid_toggled = false;
  draw_buffer(Rectangle (0, 0, GameMap::getWidth(), GameMap::getHeight()), surf,
	      Gdk::GC::create(surf));
  d_grid_toggled = orig_grid;
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create(Glib::RefPtr<Gdk::Drawable>(surf), 0, 0, width, height);
  pixbuf->save (filename, "png");
  return true;
}

void BigMap::draw_buffer(Rectangle map_view, Glib::RefPtr<Gdk::Pixmap> surface, Glib::RefPtr<Gdk::GC> context)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  int tilesize = GameMap::getInstance()->getTileset()->getTileSize();
  d_renderer->render(0, 0, map_view.x, map_view.y, map_view.w, map_view.h,
		     surface, context);

  for (Ruinlist::iterator i = Ruinlist::getInstance()->begin();
       i != Ruinlist::getInstance()->end(); ++i)
    {
      Ruin *r = *i;
      if ((r->isHidden() == true && 
	   r->getOwner() == Playerlist::getActiveplayer()) ||
	  r->isHidden() == false)
	blit_if_inside_buffer(**i, gc->getRuinPic(r->getType()), map_view,
			      surface);
    }

  for (Signpostlist::iterator i = Signpostlist::getInstance()->begin();
       i != Signpostlist::getInstance()->end(); ++i)
    blit_if_inside_buffer(**i, gc->getSignpostPic(), map_view, surface);

  for (Templelist::iterator i = Templelist::getInstance()->begin();
       i != Templelist::getInstance()->end(); ++i)
    blit_if_inside_buffer(**i, gc->getTemplePic((*i)->getType()), map_view, 
			  surface);

  for (Roadlist::iterator i = Roadlist::getInstance()->begin();
       i != Roadlist::getInstance()->end(); ++i)
    blit_if_inside_buffer(**i, gc->getRoadPic((*i)->getType()), map_view, 
			  surface);

  for (Bridgelist::iterator i = Bridgelist::getInstance()->begin();
       i != Bridgelist::getInstance()->end(); ++i)
    blit_if_inside_buffer(**i, gc->getBridgePic((*i)->getType()), map_view, 
			  surface);

  for (Portlist::iterator i = Portlist::getInstance()->begin();
       i != Portlist::getInstance()->end(); ++i)
    blit_if_inside_buffer(**i, gc->getPortPic(), map_view, surface);

  for (Citylist::iterator i = Citylist::getInstance()->begin();
       i != Citylist::getInstance()->end(); ++i)
    blit_if_inside_buffer(**i, gc->getCityPic(*i), map_view, surface);

  GameMap *gm = GameMap::getInstance();
  // If there are any items lying around, blit the itempic as well
  for (int x = map_view.x; x < map_view.x + map_view.w; x++)
    for (int y = map_view.y; y < map_view.y + map_view.h; y++)
      if (x < GameMap::getWidth() && y < GameMap::getHeight()
	  && !gm->getTile(x,y)->getBackpack()->empty())
	{
	  MapBackpack *backpack = gm->getTile(x, y)->getBackpack();
	  bool standard_planted = false;
	  Item *flag = backpack->getFirstPlantedItem();
	  if (flag)
	    standard_planted = true;

	  //only show one of the bag or the flag
	  Vector<int> p = tile_to_buffer_pos(Vector<int>(x, y));
	  if (standard_planted && flag)
	    draw_standard(flag, p, surface);
	  else
	    draw_dropped_backpack(backpack, p, surface);
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
	    draw_stack (*it, surface);
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
		  Gdk::Color line_color = Gdk::Color();
		  line_color.set_rgb_p(0,0,0);

		  context->set_rgb_fg_color(line_color);
		  surface->draw_rectangle(context, false, p.x, p.y, 
					  tilesize, tilesize);
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
	      if (FogMap::isFogged(pos, Playerlist::getActiveplayer()))
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
Glib::RefPtr<Gdk::Pixmap> BigMap::magnify(Glib::RefPtr<Gdk::Pixmap> orig)
{
  //magnify the buffer into a buffer of the correct size

  int width = 0;
  int height = 0;
  orig->get_size(width, height);
  if (width == 0 || height == 0)
    return orig;
  Glib::RefPtr<Gdk::Pixmap> result = 
    Gdk::Pixmap::create(Glib::RefPtr<Gdk::Drawable>(orig), 
			width * magnification_factor,
			height * magnification_factor);
  Glib::RefPtr<Gdk::Pixbuf> unzoomed_buffer;
  unzoomed_buffer = Gdk::Pixbuf::create(Glib::RefPtr<Gdk::Drawable>(orig), 0, 0, width, height);

  Glib::RefPtr<Gdk::Pixbuf> zoomed_buffer;
  zoomed_buffer = unzoomed_buffer->scale_simple(width * magnification_factor, height * magnification_factor, Gdk::INTERP_BILINEAR);
  result->draw_pixbuf(zoomed_buffer, 0, 0, 0, 0, zoomed_buffer->get_width(), zoomed_buffer->get_height(), Gdk::RGB_DITHER_NONE, 0, 0);

  return result;
}

void BigMap::toggle_grid()
{
  d_grid_toggled = !d_grid_toggled;
  draw(true);
}

void BigMap::draw_standard(Item *flag, Vector<int> p, Glib::RefPtr<Gdk::Pixmap> surface)
{
  Armysetlist *al = Armysetlist::getInstance();
  int tilesize = GameMap::getInstance()->getTileset()->getTileSize();
  Player *player = flag->getPlantableOwner();
  int army_tilesize = al->getTileSize(player->getArmyset());
  int x= p.x+((tilesize/2)-(army_tilesize/2));
  int y = p.y+((tilesize/2)-(army_tilesize/2));
  Glib::RefPtr<Gdk::Pixbuf> standard_image = 
    GraphicsCache::getInstance()->getPlantedStandardPic(player);
  surface->draw_pixbuf(standard_image, 0, 0, x, y, 
		       army_tilesize, army_tilesize, 
		       Gdk::RGB_DITHER_NONE, 0, 0);
  return;
}

void BigMap::draw_dropped_backpack(MapBackpack *backpack, Vector<int> p, Glib::RefPtr<Gdk::Pixmap> surface)
{
  int tilesize = GameMap::getInstance()->getTileset()->getTileSize();
  int x = p.x+(tilesize-18);
  int y = p.y+(tilesize-18);
  int ts = 16;
  surface->draw_pixbuf (d_itempic, 0, 0, x, y, ts, ts, Gdk::RGB_DITHER_NONE, 0, 0);
  return;
}
