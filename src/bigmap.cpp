// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006, 2007 Ulf Lorenz
// Copyright (C) 2004, 2005 Bryan Duff
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2014 Ben Asselstine
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

#include "bigmap.h"

#include "army.h"
#include "path.h"
#include "Item.h"
#include "stacklist.h"
#include "stack.h"
#include "citylist.h"
#include "city.h"
#include "ruinlist.h"
#include "ruin.h"
#include "signpostlist.h"
#include "signpost.h"
#include "templelist.h"
#include "temple.h"
#include "armysetlist.h"
#include "portlist.h"
#include "port.h"
#include "bridgelist.h"
#include "bridge.h"
#include "roadlist.h"
#include "road.h"
#include "playerlist.h"
#include "File.h"
#include "GameMap.h"
#include "Configuration.h"
#include "ImageCache.h"
#include "MapRenderer.h"
#include "FogMap.h"
#include "MapBackpack.h"
#include "GameScenarioOptions.h"
#include "stacktile.h"

#include <iostream>
//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

BigMap::BigMap()
    : d_renderer(0), buffer(0), magnified_buffer(0), magnification_factor(1.0)
{
    // note: we are not fully initialized before set_view is called
    view.x = view.y = 0;
    view.w = 0;
    view.h = 0;

    d_grid_toggled = false;

    blank_screen = false;
    image = Gtk::Allocation(0, 0, 320, 200);
}

BigMap::~BigMap()
{
    if (buffer == true)
      buffer.clear();

    if (magnified_buffer == true)
      magnified_buffer.clear();

    delete d_renderer;
}

void BigMap::set_view(Rectangle new_view)
{
    int tilesize = GameMap::getInstance()->getTileSize();
    
    int width = image.get_width();
    int height = image.get_height();
    if (view.dim == new_view.dim && buffer && image.get_width() == width && image.get_height() == height)
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
	    draw(Playerlist::getViewingplayer());
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

    Cairo::RefPtr<Cairo::Surface> empty = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, buffer_view.w *tilesize, buffer_view.h * tilesize);
    buffer = Cairo::Surface::create (empty, Cairo::CONTENT_COLOR_ALPHA, buffer_view.w * tilesize, buffer_view.h * tilesize);
    buffer_gc = Cairo::Context::create(buffer);

    //now create the part that will go out to the gtk::image
    if (outgoing == true)
      outgoing.clear();
    outgoing = Cairo::Surface::create(buffer, Cairo::CONTENT_COLOR_ALPHA, image.get_width(), image.get_height());


    if (d_renderer)
        delete d_renderer;
    // now set the MapRenderer so that it draws on the buffer
    d_renderer = new MapRenderer(buffer);
}

void BigMap::clip_viewable_buffer(Cairo::RefPtr<Cairo::Surface> pixmap, Cairo::RefPtr<Cairo::Context> gc, Vector<int> pos, Cairo::RefPtr<Cairo::Surface> out)
{
  Cairo::RefPtr<Cairo::Context> out_gc = Cairo::Context::create(out);
  out_gc->rectangle(0, 0, image.get_width(), image.get_height());
  out_gc->clip();
  out_gc->save();
  out_gc->set_source(pixmap, -pos.x, -pos.y);
  out_gc->rectangle (0, 0, image.get_width(), image.get_height());
  out_gc->clip();
  out_gc->paint();
  out_gc->restore();
  return;
}

void BigMap::draw(Player *player, bool redraw_buffer)
{
    // no size and buffer yet, return
    if (!buffer)
        return;
    Playerlist::getInstance()->setViewingplayer(player);

    int tilesize = GameMap::getInstance()->getTileSize();

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
    outgoing.clear();
    outgoing = Cairo::Surface::create(buffer, Cairo::CONTENT_COLOR_ALPHA, image.get_width(), image.get_height());
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

    if (blank_screen)
      {
	int width = image.get_width();
	int height = image.get_height();
	Cairo::RefPtr<Cairo::Context> outgoing_gc = Cairo::Context::create(outgoing);
	outgoing_gc->set_source_rgba(FOG_COLOUR.get_red(), FOG_COLOUR.get_green(), FOG_COLOUR.get_blue(), FOG_COLOUR.get_alpha());
	outgoing_gc->rectangle(0, 0, width, height);
	outgoing_gc->fill();
      }
    map_changed.emit(outgoing);
}

void BigMap::screen_size_changed(Gtk::Allocation box)
{
    int ts = GameMap::getInstance()->getTileSize();

    Rectangle new_view = view;
    
    new_view.w = box.get_width() / (ts * magnification_factor) + 0;
    new_view.h = box.get_height() / (ts * magnification_factor) + 0;

    if (new_view.w <= GameMap::getWidth() && new_view.h <= GameMap::getHeight()
	&& new_view.w >= 0 && new_view.h >= 0)
      {
	new_view.pos = clip(Vector<int>(0,0), new_view.pos,
			    GameMap::get_dim() - new_view.dim);

	if (image.get_width() != box.get_width() ||
	    image.get_height() != box.get_height() ||
	    new_view != view)
	  {
	    image = box;
	    set_view(new_view);
	    view_changed.emit(view);
	  }
      }
    image = box;
}

Vector<int> BigMap::get_view_pos_from_view()
{
    Vector<int> screen_dim(image.get_width(), image.get_height());
    int ts = GameMap::getInstance()->getTileSize();

    // clip to make sure we don't see a black border at the bottom and right
    return clip(Vector<int>(0, 0), view.pos * ts * magnification_factor,
		GameMap::get_dim() * ts * magnification_factor - screen_dim);
}

Vector<int> BigMap::tile_to_buffer_pos(Vector<int> tile)
{
    int ts = GameMap::getInstance()->getTileSize();
    return (tile - buffer_view.pos) * ts;
}

Vector<int> BigMap::mouse_pos_to_tile(Vector<int> pos)
{
    int ts = GameMap::getInstance()->getTileSize();
    return (view_pos + pos) / (ts * magnification_factor);
}

Vector<int> BigMap::mouse_pos_to_tile_offset(Vector<int> pos)
{
    int ts = GameMap::getInstance()->getTileSize();
    return (view_pos + pos) % (int)rint(ts * magnification_factor);
}

MapTipPosition BigMap::map_tip_position(Vector<int> tile)
{
  return map_tip_position (Rectangle(tile.x, tile.y, 1, 1)); 
}

MapTipPosition BigMap::map_tip_position(Rectangle tile_area)
{
    // convert area to pixels on the screen
    int tilesize = GameMap::getInstance()->getTileSize();

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

void BigMap::blit_object(const Location &obj, Vector<int> tile, PixMask *image, Cairo::RefPtr<Cairo::Surface> surface, Cairo::RefPtr<Cairo::Context> surface_gc)
{
  Vector<int> diff = tile - obj.getPos();
  int tilesize = GameMap::getInstance()->getTileSize();
  Vector<int> p = tile_to_buffer_pos(tile);
  image->blit(diff, tilesize, surface, p);
}

void BigMap::draw_stack(Stack *s, Cairo::RefPtr<Cairo::Surface> surface, Cairo::RefPtr<Cairo::Context> surface_gc)
{
  //this routine is for drawing the active stack.
  //for all other stacks see ImageCache::draw_tile_pic
  GameMap *gm = GameMap::getInstance();
  ImageCache *gc = ImageCache::getInstance();
  Vector<int> p = s->getPos();
  Player *player = s->getOwner();

  // check if the object lies in the viewed part of the map
  // otherwise we shouldn't draw it
  if (is_inside(buffer_view, p) && !s->getDeleting())
    {
      if (s->empty())
	{
	  std::cerr << "WARNING: empty stack found" << std::endl;
	  return;
	}

      p = tile_to_buffer_pos(p);

      // draw stack

      bool show_army = true;
      //we don't show the army or the flag if we're in fortified tent.
      if (s->hasShip())
	{
	  gc->getShipPic(player)->blit(surface, p);
	}
      else
	{
	  if (s->getFortified() == true)
	    {
	      //We don't show the active stack here.
	      if (player->getStacklist()->getActivestack() != s &&
		  player == Playerlist::getActiveplayer())
		show_army = false;
	      Maptile *tile = gm->getTile(s->getPos());
	      if (tile->getBuilding() != Maptile::CITY &&
		  tile->getBuilding() != Maptile::RUIN &&
		  tile->getBuilding() != Maptile::TEMPLE)
		gc->getTowerPic(player)->blit(surface, p);
	      else
		show_army = true;
	    }

	  if (show_army == true)
	    {
	      Army *a = *s->begin();
	      gc->getArmyPic(a)->blit(surface, p);
	    }
	}

      if (show_army)
        gc->getFlagPic(s)->blit(surface, p);
    }
}

void BigMap::draw_buffer()
{
  draw_buffer (buffer_view, buffer, buffer_gc);
    
  //the idea here is that we want to show what happens when an AI-owned
  //stack moves through our area.  in lieu of that, we just block everything
  //if we're a computer player.
  if (Playerlist::getViewingplayer()->getType() != Player::HUMAN &&
      GameScenarioOptions::s_hidden_map == true)
    {
      int width = image.get_width();
      int height = image.get_height();
      buffer_gc->set_source_rgba(FOG_COLOUR.get_red(), FOG_COLOUR.get_green(), FOG_COLOUR.get_blue(), FOG_COLOUR.get_alpha());
      buffer_gc->rectangle(0, 0, width, height);
      buffer_gc->fill();
    }
  else
    after_draw();

}

bool BigMap::saveViewAsBitmap(Glib::ustring filename)
{
  int width = image.get_width();
  int height = image.get_height();
  File::erase(filename);
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create(buffer, 0, 0, width, height);
  pixbuf->save (filename, "png");
  return true;
}

bool BigMap::saveUnderlyingMapAsBitmap(Glib::ustring filename)
{
  return d_renderer->saveAsBitmap(filename);
}

bool BigMap::saveAsBitmap(Glib::ustring filename)
{
  int tilesize = GameMap::getInstance()->getTileSize();
  int width = GameMap::getWidth() * tilesize;
  int height = GameMap::getHeight() * tilesize;
  Cairo::RefPtr<Cairo::Surface> empty;
  Cairo::RefPtr<Cairo::Surface> surf = Cairo::Surface::create(empty, Cairo::CONTENT_COLOR_ALPHA, width, height);
  
  bool orig_grid = d_grid_toggled;
  d_grid_toggled = false;
  draw_buffer(Rectangle (0, 0, GameMap::getWidth(), GameMap::getHeight()), surf,
	      Cairo::Context::create(surf));
  d_grid_toggled = orig_grid;
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create(surf, 0, 0, width, height);
  pixbuf->save (filename, "png");
  return true;
}

void BigMap::draw_buffer_tile(Vector<int> tile, Cairo::RefPtr<Cairo::Surface> surface, Cairo::RefPtr<Cairo::Context> context)
{
  guint32 tilesize = GameMap::getInstance()->getTileSize();
  Player *viewing = Playerlist::getViewingplayer();
  ImageCache *gc = ImageCache::getInstance();
  GameMap *gm = GameMap::getInstance();
  int tile_style_id = gm->getTile(tile)->getTileStyle()->getId();
  int fog_type_id = 0;
  fog_type_id = viewing->getFogMap()->getShadeTile(tile);

  bool has_bag = false;
  bool has_standard = false;
  guint32 player_standard_id = 0;
  int stack_size = -1;
  int stack_player_id = -1;
  int army_type_id = -1;
  bool has_ship = false;
  bool has_tower = false;
  Maptile::Building building_type = gm->getTile(tile)->getBuilding();
  Vector<int> building_tile = Vector<int>(-1,-1);
  int building_subtype = -1;
  int building_player_id = -1;

  if (fog_type_id == FogMap::ALL)
    {
      //short circuit.  the tile is completely fogged.
      PixMask *pixmask =
	gc->getTilePic(tile_style_id, fog_type_id, has_bag, has_standard, 
		       player_standard_id, stack_size, stack_player_id, 
		       army_type_id, has_tower, has_ship, building_type, 
		       building_subtype, building_tile, building_player_id, 
		       tilesize, d_grid_toggled);
      pixmask->blit(surface, tile_to_buffer_pos(tile));
      return;
    }
  MapBackpack *backpack = GameMap::getInstance()->getTile(tile)->getBackpack();
  if (backpack && backpack->empty() == false)
    {
      bool standard_planted = false;
      Item *flag = backpack->getFirstPlantedItem();
      if (flag)
	standard_planted = true;

      //only show one of the bag or the flag
      if (standard_planted && flag)
	{
	  has_standard = true;
	  player_standard_id = flag->getPlantableOwner()->getId();
	}
      else
	has_bag = true;
    }

  Stack *stack = GameMap::getStack(tile);
  if (stack)
    {
      if (viewing->getFogMap()->isCompletelyObscuredFogTile(tile) == false)
	{
	  //selected stack gets drawn in gamebigmap
	  if (Playerlist::getActiveplayer()->getActivestack() != stack)
	    {
	      stack_player_id = stack->getOwner()->getId();
	      Maptile *m = gm->getTile(tile);
	      if (stack->getFortified() == true &&
		  m->getBuilding() != Maptile::CITY &&
		  m->getBuilding() != Maptile::RUIN &&
		  m->getBuilding() != Maptile::TEMPLE)
		has_tower = true;
	      else if (stack->hasShip() == true)
                {
                  has_ship = true;
                  stack_size = GameMap::getStacks(stack->getPos())->countNumberOfArmies(Playerlist::getInstance()->getPlayer(stack_player_id));
                  //here we show the number of armies on the tile.
                  //instead of the number of armies in the stack.
                  //so that stacks appear whole before we click on them.
                  //and that a stack of 1 can't hide a stack of 7.
                }
	      else
		{
		  army_type_id = (*stack->begin())->getTypeId();
                  stack_size = GameMap::getStacks(stack->getPos())->countNumberOfArmies(Playerlist::getInstance()->getPlayer(stack_player_id));
		}
	    }
	}
    }

  if (building_type != Maptile::NONE)
    {
      switch (building_type)
	{
	case Maptile::CITY:
	    {
	      City *city = GameMap::getCity(tile);
	      building_player_id = city->getOwner()->getId();
	      building_tile = tile - city->getPos();
	      if (city->isBurnt())
		building_subtype = -1;
	      else
		building_subtype = 0;
	    }
	  break;
	case Maptile::RUIN:
	    {
	      Ruin *ruin = GameMap::getRuin(tile);
	      if (ruin->isHidden() == true && ruin->getOwner() == viewing)
		{
		  building_tile = tile - ruin->getPos();
		  building_subtype = ruin->getType();
		}
	      else if (ruin->isHidden() == false)
		{
		  building_tile = tile - ruin->getPos();
		  building_subtype = ruin->getType();
		}
	      else
		building_type = Maptile::NONE;
	    }
	  break;
	case Maptile::TEMPLE:
	    {
	      Temple *temple = GameMap::getTemple(tile);
	      building_tile = tile - temple->getPos();
	      building_subtype = temple->getType();
	    }
	  break;
	case Maptile::SIGNPOST:
	    {
	      Signpost *signpost = GameMap::getSignpost(tile);
	      building_tile = tile - signpost->getPos();
	    }
	  break;
	case Maptile::ROAD:
	    {
	      Road *road = GameMap::getRoad(tile);
	      building_tile = tile - road->getPos();
	      building_subtype = road->getType();
	    }
	  break;
	case Maptile::PORT:
	    {
	      Port *port = GameMap::getPort(tile);
	      building_tile = tile - port->getPos();
	    }
	  break;
	case Maptile::BRIDGE:
	    {
	      Bridge *bridge = GameMap::getBridge(tile);
	      building_tile = tile - bridge->getPos();
	      building_subtype = bridge->getType();
	    }
	  break;
	case Maptile::NONE: default:
	  break;
	}
    }
  PixMask *pixmask = 
    gc->getTilePic(tile_style_id, fog_type_id, has_bag, has_standard, 
		   player_standard_id, stack_size, stack_player_id, 
		   army_type_id, has_tower, has_ship, building_type, 
		   building_subtype, building_tile, building_player_id, 
		   tilesize, d_grid_toggled);
  pixmask->blit(surface, tile_to_buffer_pos(tile));
}

void BigMap::draw_buffer_tiles(Rectangle map_view, Cairo::RefPtr<Cairo::Surface> surface, Cairo::RefPtr<Cairo::Context> context)
{
  for (int i = map_view.x; i < map_view.x + map_view.w; i++)
    for (int j = map_view.y; j < map_view.y + map_view.h; j++)
      if (i < GameMap::getWidth() && j < GameMap::getHeight())
	draw_buffer_tile(Vector<int>(i,j), surface, context);
}

void BigMap::draw_buffer(Rectangle map_view, Cairo::RefPtr<Cairo::Surface> surface, Cairo::RefPtr<Cairo::Context> context)
{
  draw_buffer_tiles(map_view, surface, context);
}

//here we want to magnify the entire buffer, not a subset
Cairo::RefPtr<Cairo::Surface> BigMap::magnify(Cairo::RefPtr<Cairo::Surface> orig)
{
  //magnify the buffer into a buffer of the correct size

  int width = image.get_width();
  int height = image.get_height();
  if (width == 0 || height == 0)
    return orig;
  Cairo::RefPtr<Cairo::Surface> result = 
    Cairo::Surface::create(orig, Cairo::CONTENT_COLOR_ALPHA,
                           width * magnification_factor,
                           height * magnification_factor);
  Glib::RefPtr<Gdk::Pixbuf> unzoomed_buffer;
  unzoomed_buffer = Gdk::Pixbuf::create(orig, 0, 0, width, height);

  return result;
}

void BigMap::toggle_grid()
{
  d_grid_toggled = !d_grid_toggled;
  draw(Playerlist::getViewingplayer(), true);
}

void BigMap::blank(bool on)
{
  blank_screen = on;
  draw (Playerlist::getViewingplayer());
}
