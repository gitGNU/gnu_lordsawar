//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2014 Ben Asselstine
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
#include <stdlib.h>

#include "editorbigmap.h"

#include "army.h"
#include "stacklist.h"
#include "stack.h"
#include "stacktile.h"
#include "citylist.h"
#include "city.h"
#include "ruinlist.h"
#include "ruin.h"
#include "signpostlist.h"
#include "signpost.h"
#include "templelist.h"
#include "temple.h"
#include "bridgelist.h"
#include "bridge.h"
#include "portlist.h"
#include "port.h"
#include "roadlist.h"
#include "road.h"
#include "playerlist.h"
#include "defs.h"
#include "File.h"
#include "GameMap.h"
#include "Configuration.h"
#include "rewardlist.h"
#include "GraphicsCache.h"
#include "armysetlist.h"
#include "CreateScenario.h"
#include "Backpack.h"
#include "MapBackpack.h"
#include "backpack-editor-dialog.h"
#include "citysetlist.h"
#include "cityset.h"


EditorBigMap::EditorBigMap()
{
    mouse_pos = prev_mouse_pos = Vector<int>(0, 0);

    moving_objects_from = Vector<int>(-1,-1);
    mouse_state = NONE;
    input_locked = false;
    pointer = POINTER;
    pointer_size = 1;
    pointer_terrain = Tile::GRASS;
    pointer_tile_style_id = -1;
    show_tile_types_instead_of_tile_styles = false;
}

EditorBigMap::~EditorBigMap()
{
}

void EditorBigMap::set_pointer(Pointer p, int size, Tile::Type t, 
			       int tile_style_id)
{
    bool redraw = false;
    if (pointer != p || pointer_size != size || 
	pointer_tile_style_id != tile_style_id)
      redraw = true;
    pointer = p;
    pointer_terrain = t;
    pointer_size = size;
    pointer_tile_style_id = tile_style_id;

    moving_objects_from = Vector<int>(-1,-1);
    if (redraw)
      draw(Playerlist::getViewingplayer());
    
}

void EditorBigMap::mouse_button_event(MouseButtonEvent e)
{
    if (input_locked)
	return;
    
    mouse_pos = e.pos;
    Vector<int> tile = mouse_pos_to_tile(mouse_pos);
    
    if (e.button == MouseButtonEvent::LEFT_BUTTON
	&& e.state == MouseButtonEvent::PRESSED)
    {
	change_map_under_cursor();
    }

    else if (e.button == MouseButtonEvent::RIGHT_BUTTON
	     && e.state == MouseButtonEvent::PRESSED)
    {
	map_selection_seq seq;
	
	if (Stack* s = GameMap::getStack(tile))
	    seq.push_back(s);
	if (City* c = GameMap::getCity(tile))
	    seq.push_back(c);
	if (Ruin* r = GameMap::getRuin(tile))
	    seq.push_back(r);
	if (Signpost* s = GameMap::getSignpost(tile))
	    seq.push_back(s);
	if (Temple* t = GameMap::getTemple(tile))
	    seq.push_back(t);
	MapBackpack *b = GameMap::getInstance()->getTile(tile)->getBackpack();
	if (b->empty() == false)
	  seq.push_back(b);


	if (!seq.empty())
	    objects_selected.emit(seq);
    }
}

void EditorBigMap::mouse_motion_event(MouseMotionEvent e)
{
    if (input_locked)
	return;

    bool redraw = false;

    mouse_pos = e.pos;
    Vector<int> new_tile = mouse_pos_to_tile(mouse_pos);
    if (new_tile != mouse_pos_to_tile(prev_mouse_pos))
    {
	mouse_on_tile.emit(new_tile);
	redraw = true;
    }

    // draw with left mouse button
    if (e.pressed[MouseMotionEvent::LEFT_BUTTON])
    {
	change_map_under_cursor();
    }
    
    // drag with right mouse button
    if (e.pressed[MouseMotionEvent::RIGHT_BUTTON]
	&& (mouse_state == NONE || mouse_state == DRAGGING))
    {
	Vector<int> delta = -(mouse_pos - prev_mouse_pos);

	// ignore very small drags to ensure that a shaking mouse does not
	// prevent the user from making right clicks
	if (mouse_state == NONE && length(delta) <= 2)
	    return;
	
	// FIXME: show a drag cursor
	
	int ts = GameMap::getInstance()->getTileSize();
	Vector<int> screen_dim(image.get_width(), image.get_height());
	view_pos = clip(Vector<int>(0, 0),
			view_pos + delta,
			GameMap::get_dim() * ts - screen_dim);

	// calculate new view position in tiles, rounding up
	Vector<int> new_view = (view_pos + Vector<int>(ts - 1, ts - 1)) / ts;

	bool redraw_buffer = false;
	
	if (new_view != view.pos)
	{
	    view.x = new_view.x;
	    view.y = new_view.y;
	    view_changed.emit(view);
	    redraw_buffer = true;
	}

	draw(Playerlist::getViewingplayer(),redraw_buffer);
	redraw = false;
	mouse_state = DRAGGING;
    }

    if (redraw && pointer != POINTER)
	draw(Playerlist::getViewingplayer());
    
    prev_mouse_pos = mouse_pos;
}

void EditorBigMap::mouse_leave_event()
{
    mouse_pos.x = mouse_pos.y = -10000;
    mouse_on_tile.emit(Vector<int>(-100, -100));
    draw(Playerlist::getViewingplayer());
}

std::vector<Vector<int> > EditorBigMap::get_screen_tiles()
{
    // find out which tiles are within bounds
    std::vector<Vector<int> > tiles;

    for (int y = buffer_view.y; y < buffer_view.y + buffer_view.h; y++)
      for (int x = buffer_view.x; x < buffer_view.x + buffer_view.w; x++)
	{
	    Vector<int> tile(x, y);
	    if (tile.x >= 0 && tile.x < GameMap::getWidth() &&
		tile.y >= 0 && tile.y < GameMap::getHeight())
		tiles.push_back(tile);
	}
    
    return tiles;
}

std::vector<Vector<int> > EditorBigMap::get_cursor_tiles()
{
    // find out which cursor tiles are within bounds
    std::vector<Vector<int> > tiles;

    Vector<int> current_tile = mouse_pos_to_tile(mouse_pos);
	
    for (int y = 0; y < pointer_size; ++y)
	for (int x = 0; x < pointer_size; ++x)
	{
	    int offset = - (pointer_size - 1) / 2;
	    Vector<int> tile(x + offset, y + offset);
	    tile += current_tile;

	    if (tile.x >= 0 && tile.x < GameMap::getWidth() &&
		tile.y >= 0 && tile.y < GameMap::getHeight())
		tiles.push_back(tile);
	}
    
    return tiles;
}

Rectangle EditorBigMap::get_cursor_rectangle()
{
    // find out which cursor tiles are within bounds
    std::vector<Vector<int> > tiles;

    Vector<int> current_tile = mouse_pos_to_tile(mouse_pos);
    int offset = (pointer_size - 1) / 2;
    Vector<int> tile = current_tile - Vector<int>(offset, offset);

    return Rectangle (tile.x, tile.y, pointer_size, pointer_size);
}

int EditorBigMap::tile_to_bridge_type(Vector<int> t)
{
    // examine neighbour tiles to discover whether there's a road on them
    bool u = Roadlist::getInstance()->getObjectAt(t + Vector<int>(0, -1));
    bool b = Roadlist::getInstance()->getObjectAt(t + Vector<int>(0, 1));
    bool l = Roadlist::getInstance()->getObjectAt(t + Vector<int>(-1, 0));
    bool r = Roadlist::getInstance()->getObjectAt(t + Vector<int>(1, 0));

    // then translate this to the type
    int type = 0;
    if (!u && !b && !l && !r)
	type = 0;
    else if (u && b && l && r)
	type = 0;
    else if (!u && b && l && r)
	type = 0;
    else if (u && !b && l && r)
	type = 0;
    else if (u && b && !l && r)
	type = 1;
    else if (u && b && l && !r)
	type = 1;
    else if (u && b && !l && !r)
	type = 1;
    else if (!u && !b && l && r)
	type = 0;
    else if (u && !b && l && !r)
	type = 0;
    else if (u && !b && !l && r)
	type = 2;
    else if (!u && b && l && !r)
	type = 0;
    else if (!u && b && !l && r)
	type = 2;
    else if (u && !b && !l && !r)
	type = 3;
    else if (!u && b && !l && !r)
	type = 1;
    else if (!u && !b && l && !r)
	type = 0;
    else if (!u && !b && !l && r)
	type = 2;
    return type;
}

void EditorBigMap::change_map_under_cursor()
{

  Player* active = Playerlist::getInstance()->getActiveplayer();
  std::vector<Vector<int> > tiles = get_cursor_tiles();

  if (tiles.size() == 0)
    return;
  Vector<int> tile = tiles.front();
  Rectangle changed_tiles(tile, Vector<int>(-1, -1));
  Maptile* maptile = GameMap::getInstance()->getTile(tile);
  switch (pointer)
    {
    case POINTER:
      break;

    case TERRAIN:

      // don't change terrain to water if there is a building underneath
      if (maptile->getBuilding() != Maptile::NONE && 
          pointer_terrain == Tile::WATER)
        break;
      // don't change the terrain to anything else than grass if there is
      // a city
      if (maptile->getBuilding() == Maptile::CITY && 
          pointer_terrain != Tile::GRASS)
        break;

      changed_tiles = GameMap::getInstance()->putTerrain(get_cursor_rectangle(), pointer_terrain, pointer_tile_style_id, true);
      if (pointer_terrain == Tile::WATER)
        map_water_changed.emit();
      break;

    case MOVE:
      if (moving_objects_from == Vector<int>(-1,-1))
        {
          if (GameMap::getInstance()->getBuilding(tile) != 
              Maptile::NONE || 
              GameMap::getStack(tile) != NULL ||
              GameMap::getBackpack(tile)->empty() == false)
            moving_objects_from = tile;
        }
      else
        {
          Vector<int> from = moving_objects_from;
          //here we go with the move!
          GameMap *gm = GameMap::getInstance();
          if (gm->getStack(from) != NULL)
            {
              Stack *s = gm->getFriendlyStack(from);
              if (!s)
                s = gm->getStack(from);
              if (gm->canPutStack(s->size(), s->getOwner(), tile) == true)
                {
                  gm->moveStack(s, tile);
                  moving_objects_from = Vector<int>(-1,-1);
                }
            }
          else if (gm->getBackpack(from)->empty() == false)
            {
              gm->moveBackpack(from, tile);
              moving_objects_from = Vector<int>(-1,-1);
            }
          else if (gm->getBuilding(from) != Maptile::NONE)
            {
              guint32 s = gm->getBuildingSize(from);
              if (gm->canPutBuilding
                  (gm->getBuilding(from), s, tile, false) == true)
                {
                  gm->moveBuilding(from, tile);
                  moving_objects_from = Vector<int>(-1,-1);
                }
              else
                {
                  if (gm->getLocation(from)->contains(tile) ||
                      LocationBox(tile, s).contains(from))
                    {
                      gm->moveBuilding(from, tile);
                      moving_objects_from = Vector<int>(-1,-1);
                    }
                }
            }
        }
      break;
    case ERASE:
      // check if there is a building or a stack there and remove it

      GameMap::getInstance()->eraseTile(tile);
      break;

    case STACK:
      if (GameMap::getInstance()->countArmyUnits(tile))
        {
          map_selection_seq seq;
          Stack *s = GameMap::getStack(tile);
          if (s)
            seq.push_back(s);

          if (!seq.empty())
            objects_selected.emit(seq);
        }
      else if (GameMap::getInstance()->canPutStack(1, active, tile) == true)
        {
          // Create a new dummy stack. As we don't want to have empty
          // stacks hanging around, it's assumed that the default armyset
          // has at least one entry.
          Stack* s = new Stack(active, tile);
          const Armysetlist* al = Armysetlist::getInstance();
          Army* a = new Army(*al->getArmy(active->getArmyset(), 0), active);
          s->add(a);
          GameMap::getInstance()->putStack(s);
          //if we're on a city, change the allegiance of the stack
          //and it's armies to that of the city
          if (GameMap::getInstance()->getBuilding(s->getPos()) == Maptile::CITY)
            {
              City *c = GameMap::getCity(s->getPos());
              if (c->getOwner() != active)
                {
                  GameMap::getStacks(s->getPos())->leaving(s);
                  s = Stacklist::changeOwnership(s, c->getOwner());
                  GameMap::getStacks(s->getPos())->arriving(s);
                }
            }
        }

      break;

    case CITY:
      if (GameMap::getInstance()->getBuilding(tile) == Maptile::CITY)
        {
          map_selection_seq seq;
          City *c = GameMap::getCity(tile);
          if (c)
            seq.push_back(c);

          if (!seq.empty())
            objects_selected.emit(seq);
        }
      else
        GameMap::getInstance()->putNewCity(tile);
      break;

    case RUIN:
      if (GameMap::getInstance()->getBuilding(tile) == Maptile::RUIN)
        {
          map_selection_seq seq;
          Ruin *r = GameMap::getRuin(tile);
          if (r)
            seq.push_back(r);

          if (!seq.empty())
            objects_selected.emit(seq);
        }
      else
        GameMap::getInstance()->putNewRuin(tile);
      break;

    case TEMPLE:
      if (GameMap::getInstance()->getBuilding(tile) == Maptile::TEMPLE)
        {
          map_selection_seq seq;
          Temple *t = GameMap::getTemple(tile);
          if (t)
            seq.push_back(t);

          if (!seq.empty())
            objects_selected.emit(seq);
        }
      else
        GameMap::getInstance()->putNewTemple(tile);
      break;

    case SIGNPOST:
        {
          if (GameMap::getInstance()->getBuilding(tile) == Maptile::SIGNPOST)
            {
              map_selection_seq seq;
              Signpost *s = GameMap::getSignpost(tile);
              if (s)
                seq.push_back(s);

              if (!seq.empty())
                objects_selected.emit(seq);
            }
          else
            {
              bool signpost_placeable = GameMap::getInstance()->canPutBuilding
                (Maptile::SIGNPOST, 1, tile);
              if (!signpost_placeable)
                break;
              Signpost *s = new Signpost(tile);
              GameMap::getInstance()->putSignpost(s);
            }
          break;
        }

    case PORT:
        {
          bool port_placeable = GameMap::getInstance()->canPutBuilding
            (Maptile::PORT, 1, tile);
          if (!port_placeable)
            break;
          Port *p = new Port(tile);
          GameMap::getInstance()->putPort(p);
          break;
        }

    case BRIDGE:
        {
          bool bridge_placeable = GameMap::getInstance()->canPutBuilding
            (Maptile::BRIDGE, 1, tile);
          if (!bridge_placeable)
            break;
          Bridge *b = new Bridge(tile, tile_to_bridge_type (tile));
          GameMap::getInstance()->putBridge(b);
          break;
        }

    case ROAD:
        {
          bool road_placeable = GameMap::getInstance()->canPutBuilding
            (Maptile::ROAD, 1, tile);
          if (!road_placeable)
            {
              //overwrite existing roads
              if (GameMap::getRoad(tile) != NULL)
                GameMap::getInstance()->removeRoad(tile);
              else
                break;
            }

          int type = CreateScenario::calculateRoadType(tile);
          Road *r = new Road(tile, type);
          GameMap::getInstance()->putRoad(r);

          changed_tiles.pos -= Vector<int>(1, 1);
          changed_tiles.dim = Vector<int>(3, 3);
          break;
        }
    case BAG:
      if (maptile->getType() != Tile::WATER)
        {
          //open the dialog
          MapBackpack *bag = 
            GameMap::getInstance()->getTile(tile)->getBackpack();
          BackpackEditorDialog d(dynamic_cast<Backpack*>(bag));
          d.run();
        }
      break;

    }

  if (changed_tiles.w > 0 && changed_tiles.h > 0)
    map_tiles_changed.emit(changed_tiles);

  draw(Playerlist::getViewingplayer());
}

void EditorBigMap::smooth_view()
{
  GameMap::getInstance()->applyTileStyles(view.y, view.x, view.y+view.h, 
					  view.x+view.w, true);
  draw(Playerlist::getViewingplayer());
}

void EditorBigMap::after_draw()
{
    int tilesize = GameMap::getInstance()->getTileSize();
    std::vector<Vector<int> > tiles;

    if (show_tile_types_instead_of_tile_styles)
      {
	tiles = get_screen_tiles();
	for (std::vector<Vector<int> >::iterator i = tiles.begin(),
	     end = tiles.end(); i != end; ++i)
	  {
	    Vector<int> pos = tile_to_buffer_pos(*i);
	    buffer_gc->set_source_rgba(GameMap::getInstance()->getTile(*i)->getColor().get_red(),
                                       GameMap::getInstance()->getTile(*i)->getColor().get_green(),
                                       GameMap::getInstance()->getTile(*i)->getColor().get_blue(),
                                       GameMap::getInstance()->getTile(*i)->getColor().get_alpha());
	    int x = pos.x;
	    int y = pos.y;
	    int ts = tilesize;
	    if (d_grid_toggled)
	      {
		x++;
		y++;
		ts--;
	      }
            buffer_gc->rectangle(x, y, ts, ts);
	  }
        buffer_gc->paint();
      }

    // we need to draw a drawing cursor on the map
    tiles = get_cursor_tiles();
    // draw each tile
	
    Gdk::RGBA terrain_box_color = Gdk::RGBA();
    terrain_box_color.set_rgba(200.0/255.0, 200.0/255.0, 200.0/255.0);
    Gdk::RGBA erase_box_color = Gdk::RGBA();
    erase_box_color.set_rgba(200.0/255.0, 50.0/255.0, 50.0/255.0);
    Gdk::RGBA move_box_color = Gdk::RGBA();
    move_box_color.set_rgba(50.0/255.0, 200.0/255.0, 50.0/255.0);
    Gdk::RGBA moving_box_color = Gdk::RGBA();
    moving_box_color.set_rgba(50.0/255.0, 50.0/255.0, 200.0/255.0);
    for (std::vector<Vector<int> >::iterator i = tiles.begin(),
	     end = tiles.end(); i != end; ++i)
      {
	Vector<int> pos = tile_to_buffer_pos(*i);

	PixMask *pic;


	switch (pointer)
	  {
	  case POINTER:
	    break;

	  case TERRAIN:
	    buffer_gc->set_source_rgb(terrain_box_color.get_red(),
                                       terrain_box_color.get_green(),
                                       terrain_box_color.get_blue());
            buffer_gc->move_to(pos.x+1, pos.y+1);
            buffer_gc->rel_line_to(tilesize-2, 0);
            buffer_gc->rel_line_to(0, tilesize-2);
            buffer_gc->rel_line_to(-tilesize +2, 0);
            buffer_gc->rel_line_to(0, -tilesize+2);
            buffer_gc->set_line_width(1.0);
            buffer_gc->stroke();
	    break;

	  case ERASE:
	    buffer_gc->set_source_rgb(erase_box_color.get_red(),
                                       erase_box_color.get_green(),
                                       erase_box_color.get_blue());
            buffer_gc->move_to(pos.x+1, pos.y+1);
            buffer_gc->rel_line_to(tilesize-2, 0);
            buffer_gc->rel_line_to(0, tilesize-2);
            buffer_gc->rel_line_to(-tilesize +2, 0);
            buffer_gc->rel_line_to(0, -tilesize+2);
            buffer_gc->set_line_width(1.0);
            buffer_gc->stroke();
	    break;

	  case MOVE:
	    if (moving_objects_from != Vector<int>(-1,-1))
              buffer_gc->set_source_rgb(moving_box_color.get_red(),
                                        moving_box_color.get_green(),
                                        moving_box_color.get_blue());
	    else
              buffer_gc->set_source_rgb(move_box_color.get_red(),
                                        move_box_color.get_green(),
                                        move_box_color.get_blue());
            buffer_gc->move_to(pos.x+1, pos.y+1);
            buffer_gc->rel_line_to(tilesize-2, 0);
            buffer_gc->rel_line_to(0, tilesize-2);
            buffer_gc->rel_line_to(-tilesize +2, 0);
            buffer_gc->rel_line_to(0, -tilesize+2);
            buffer_gc->set_line_width(1.0);
            buffer_gc->stroke();
	    break;

	  case STACK:
	    pic = GraphicsCache::getInstance()->getArmyPic
	      (Playerlist::getInstance()->getActiveplayer()->getArmyset(), 0,
	       Playerlist::getInstance()->getActiveplayer(), NULL);
	    pic->blit(buffer, pos);
	    break;

	  case CITY:
	    pic = GraphicsCache::getInstance()->getCityPic(0, Playerlist::getInstance()->getActiveplayer(), GameMap::getInstance()->getCitysetId());
	    pic->blit(buffer, pos);
	    break;

	  case RUIN:
	    pic = GraphicsCache::getInstance()->getRuinPic(0, GameMap::getInstance()->getCitysetId());
	    pic->blit(buffer, pos);
	    break;

	  case TEMPLE:
	    pic = GraphicsCache::getInstance()->getTemplePic(0, GameMap::getInstance()->getCitysetId());
	    pic->blit(buffer, pos);
	    break;

	  case SIGNPOST:
	    pic = GraphicsCache::getInstance()->getSignpostPic();
	    pic->blit(buffer, pos);
	    break;

	  case ROAD:
	    pic = GraphicsCache::getInstance()->getRoadPic(CreateScenario::calculateRoadType(*i));
	    pic->blit(buffer, pos);
	    break;
	  case PORT:
	    pic = GraphicsCache::getInstance()->getPortPic();
	    pic->blit(buffer, pos);
	    break;
	  case BRIDGE:
	    pic = GraphicsCache::getInstance()->getBridgePic(tile_to_bridge_type(*i));
	    pic->blit(buffer, pos);
	    break;
	  case BAG:
	    pic = GraphicsCache::getInstance()->getBagPic();
	    pic->blit(buffer, pos);
	    break;
	  }
      }
}
