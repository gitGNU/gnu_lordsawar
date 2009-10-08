//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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
#include "MapRenderer.h"
#include "CreateScenario.h"
#include "Backpack.h"
#include "MapBackpack.h"
#include "backpack-editor-dialog.h"


EditorBigMap::EditorBigMap()
{
    mouse_pos = prev_mouse_pos = Vector<int>(0, 0);

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

    if (redraw)
      draw();
    
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
	
	if (Stack* s = Stacklist::getObjectAt(tile))
	    seq.push_back(s);
	if (City* c = Citylist::getInstance()->getObjectAt(tile))
	    seq.push_back(c);
	if (Ruin* r = Ruinlist::getInstance()->getObjectAt(tile))
	    seq.push_back(r);
	if (Signpost* s = Signpostlist::getInstance()->getObjectAt(tile))
	    seq.push_back(s);
	if (Temple* t = Templelist::getInstance()->getObjectAt(tile))
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
	
	int ts = GameMap::getInstance()->getTileset()->getTileSize();
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

	draw(redraw_buffer);
	redraw = false;
	mouse_state = DRAGGING;
    }

    if (redraw && pointer != POINTER)
	draw();
    
    prev_mouse_pos = mouse_pos;
}

void EditorBigMap::mouse_leave_event()
{
    mouse_pos.x = mouse_pos.y = -10000;
    mouse_on_tile.emit(Vector<int>(-100, -100));
    draw();
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



namespace
{
    template <typename T>
    void remove_from_map(T *l, Vector<int> tile)
    {
	for (typename T::iterator i = l->begin(), end = l->end(); i != end; ++i)
	    if ((*i)->contains(tile))
	    {
		// erase from map
		GameMap *gamemap = GameMap::getInstance();
		Rectangle r = (*i)->get_area();
		for (int x = r.x; x < r.x + r.w; ++x)
		    for (int y = r.y; y < r.y + r.h; ++y)
			gamemap->getTile(Vector<int>(x, y))->setBuilding(Maptile::NONE);
		
		// erase from list
		l->erase(i);
		break;
	    }
    }
}

void EditorBigMap::change_map_under_cursor()
{
    std::vector<Vector<int> > tiles = get_cursor_tiles();
    Tileset* ts = GameMap::getInstance()->getTileset();
    
    // find the index of the "grass" tile
    unsigned int grass_index;
    for (grass_index = 0; grass_index < ts->size(); ++grass_index)
      if ((*ts)[grass_index]->getType() == Tile::GRASS)
	break;
        
    for (std::vector<Vector<int> >::iterator i = tiles.begin(),
	     end = tiles.end(); i != end; ++i)
    {
	Vector<int> tile = *i;
	Rectangle changed_tiles(tile, Vector<int>(-1, -1));
	Maptile* maptile = GameMap::getInstance()->getTile(tile);
	switch (pointer)
	{
	case POINTER:
	    break;
	    
	case TERRAIN:
        
	    // don't change terrain to water if there is a building underneath
	    if (maptile->getBuilding() != Maptile::NONE
		&& pointer_terrain == Tile::WATER)
		break;
	    // don't change the terrain to anything else than grass if there is
	    // a city
	    if (maptile->getBuilding() == Maptile::CITY
		&& pointer_terrain != Tile::GRASS)
		break;

	    if (pointer_terrain == maptile->getMaptileType() &&
		pointer_tile_style_id == -1)
	      break;

	    maptile->setType(ts->getIndex(pointer_terrain));
	    if (pointer_tile_style_id == -1)
	      {
		if (pointer_terrain == Tile::MOUNTAIN)
		  {
		    GameMap *gm = GameMap::getInstance();
		    gm->surroundMountains (tile.y-1, tile.x-1, 
					   tile.y+2, tile.x+2);
		    gm->applyTileStyles(tile.y-2, tile.x-2, 
					tile.y+3, tile.x+3,
					false);
		  }
		else
		  GameMap::getInstance()->applyTileStyles(tile.y-1, tile.x-1, 
							  tile.y+2, tile.x+2,
							  false);
	      }
	    else
	      {
		Tileset *tileset = GameMap::getInstance()->getTileset();
		TileStyle *tile_style;
		tile_style = tileset->getTileStyle(pointer_tile_style_id);
		maptile->setTileStyle (tile_style);
	      }
	    changed_tiles.dim = Vector<int>(1, 1);
	    //change the boatedness of stacks that might be here.
	    if (Stacklist::getObjectAt(tile))
	      {
		Stack* s = Stacklist::getObjectAt(tile);
		Port *port = Portlist::getInstance()->getObjectAt(tile);
		Bridge *bridge = Bridgelist::getInstance()->getObjectAt(tile);
		if (pointer_terrain == Tile::WATER && s->hasShip() == false && 
		    !port && !bridge)
		  {
		    for (Stack::iterator it = s->begin(); it != s->end(); it++)
		      (*it)->setInShip(true);
		  }
		else if (pointer_terrain != Tile::WATER && s->hasShip() == true)
		  {
		    for (Stack::iterator it = s->begin(); it != s->end(); it++)
		      (*it)->setInShip(false);
		  }
	      }
	    break;
	    
	case ERASE:
	    // check if there is a building or a stack there and remove it

	    // first stack, it's above everything else
	    if (Stack* s = Stacklist::getObjectAt(tile))
	    {
		s->getOwner()->deleteStack(s);
		break;
	    }
	    
	    maptile->setBuilding(Maptile::NONE);
	    
	    // ... or a temple ...
	    remove_from_map(Templelist::getInstance(), tile);
	    // ... or a port ...
	    remove_from_map(Portlist::getInstance(), tile);
	    // ... or a ruin ...
	    {
	      if (Ruinlist::getInstance()->getObjectAt(tile))
		{
		  Rewardlist *rl = Rewardlist::getInstance();
		  for (Rewardlist::iterator it = rl->begin(); 
		       it != rl->end(); it++)
		    {
		      if ((*it)->getType() == Reward::RUIN)
			{
			  Reward_Ruin *rr = static_cast<Reward_Ruin*>(*it);
			  if (rr->getRuin()->getPos() == tile)
			    {
			      rl->remove(*it);
			    }
			}
		    }
		}
	    }
	    remove_from_map(Ruinlist::getInstance(), tile);
	    // ... or a road ...
	    remove_from_map(Roadlist::getInstance(), tile);
	    // ... or a bridge...
	    remove_from_map(Bridgelist::getInstance(), tile);
	    // ... or a signpost ...
	    remove_from_map(Signpostlist::getInstance(), tile);
	    // ... or a city
	    remove_from_map(Citylist::getInstance(), tile);
	    break;

	case STACK:
	    if (!Stacklist::getObjectAt(tile))
	    {
		// Create a new dummy stack. As we don't want to have empty
		// stacks hanging around, it's assumed that the default armyset
		// has at least one entry.
		Player* p = Playerlist::getInstance()->getNeutral();
		Stack* s = new Stack(p, tile);
		const Armysetlist* al = Armysetlist::getInstance();
		Army* a = new Army(*al->getArmy(p->getArmyset(), 0), p);
		GameMap *gm = GameMap::getInstance();
		if (gm->getTile(s->getPos())->getBuilding() == Maptile::PORT ||
		    gm->getTile(s->getPos())->getBuilding() == Maptile::BRIDGE)
		  a->setInShip(false);
		else if (gm->getTile(s->getPos())->getMaptileType() == 
			 Tile::WATER)
		  a->setInShip(true);
		else
		  a->setInShip(false);
		s->push_back(a);
		s->group();
		p->addStack(s);
		//if we're on a city, change the allegiance of the stack
		//and it's armies to that of the city
		if (gm->getTile(s->getPos())->getBuilding() == Maptile::CITY)
		  {
		    Citylist *clist = Citylist::getInstance();
		    City *c = clist->getNearestCity(s->getPos());
			if (s->getOwner() != c->getOwner())
			  {
			    //remove it from the old player's list of stacks
			    s->getOwner()->getStacklist()->remove(s);
			    //and give it to the new player list of stacks
			    c->getOwner()->getStacklist()->push_back(s);
			    //change the ownership of the stack
			    s->setOwner(c->getOwner());
			    //and all of it's armies
			    for (Stack::iterator it = s->begin(); 
				 it != s->end(); it++)
			      (*it)->setOwner(c->getOwner());
			  }
	          }
	    }

	    break;
	    
	case CITY:
	{
	    // check if we can place the city
	    bool city_placeable = true;
	    
	    for (int x = tile.x; x <= tile.x + 1; ++x)
		for (int y = tile.y; y <= tile.y + 1; ++y)
		{
		    if (x >= GameMap::getWidth() || y >= GameMap::getHeight()
			|| GameMap::getInstance()->getTile(Vector<int>(x, y))->getBuilding() != Maptile::NONE)
		    {
			city_placeable = false;
			break;
		    }
		    if (city_placeable == false)
		      break;
		}
	    if (!city_placeable)
		break;
	    
	    // create the city
	    City *c = new City(tile);
	    c->setOwner(Playerlist::getInstance()->getNeutral());
	    Citylist::getInstance()->add(c);

	    bool replaced_grass = false;
	    // notify the maptiles that a city has been placed here
	    Rectangle r = c->get_area();
	    for (int x = r.x; x < r.x + r.w; ++x)
		for (int y = r.y; y < r.y + r.h; ++y)
		{
		    Maptile* t = GameMap::getInstance()->getTile(Vector<int>(x, y));
		    if (t->getMaptileType() != Tile::GRASS)
		      replaced_grass = true;
		    t->setBuilding(Maptile::CITY);
		    t->setType(grass_index);
		}

	    //change allegiance of stacks under this city
	    for (unsigned int x = 0; x < c->getSize(); x++)
	      {
		for (unsigned int y = 0; y < c->getSize(); y++)
		  {
		    Stack *s = Stacklist::getObjectAt(c->getPos().x + x, 
						      c->getPos().y + y);
		    if (s)
		      {
			if (s->getOwner() != c->getOwner())
			  {
			    //remove it from the old player's list of stacks
			    s->getOwner()->getStacklist()->remove(s);
			    //and give it to the new player list of stacks
			    c->getOwner()->getStacklist()->push_back(s);
			    //change the ownership of the stack
			    s->setOwner(c->getOwner());
			    //and all of it's armies
			    for (Stack::iterator it = s->begin(); 
				 it != s->end(); it++)
			      (*it)->setOwner(c->getOwner());
			  }
		      }
		  }
    
	      }

	    // finally, smooth the surrounding map
	    if (replaced_grass)
	      GameMap::getInstance()->applyTileStyles
		(0, 0, GameMap::getHeight(), GameMap::getWidth(), true);
	}
	break;
	    
	case RUIN:
	    if (maptile->getBuilding() == Maptile::NONE 
		&& maptile->getMaptileType() != Tile::WATER)
	    {
		maptile->setBuilding(Maptile::RUIN);
		bool replaced_grass = false;
		if (maptile->getMaptileType() != Tile::GRASS)
		  replaced_grass = true;
		maptile->setType(grass_index);
		Ruin *ruin = new Ruin(tile);
		Ruinlist::getInstance()->add(ruin);
		if (replaced_grass)
		  GameMap::getInstance()->applyTileStyles
		    (0, 0, GameMap::getHeight(), GameMap::getWidth(), true);
	    }
	    break;
	    
	case TEMPLE:
	    if (maptile->getBuilding() == Maptile::NONE 
		&& maptile->getMaptileType() != Tile::WATER)
	    {
	        bool replaced_grass = false;
		maptile->setBuilding(Maptile::TEMPLE);
		if (maptile->getMaptileType() != Tile::GRASS)
		  replaced_grass = true;
		maptile->setType(grass_index);
		Templelist::getInstance()->add(new Temple(tile));
		if (replaced_grass)
		  GameMap::getInstance()->applyTileStyles
		    (0, 0, GameMap::getHeight(), GameMap::getWidth(), true);
	    }
	    break;
	    
	case SIGNPOST:
	    if (maptile->getBuilding() == Maptile::NONE 
		&& maptile->getMaptileType() == Tile::GRASS)
	    {
		maptile->setBuilding(Maptile::SIGNPOST);
		Signpostlist::getInstance()->add(new Signpost(tile));
	    }
	    break;
	    
	case PORT:
	    if (maptile->getBuilding() == Maptile::NONE 
		&& maptile->getMaptileType() == Tile::WATER &&
		maptile->getTileStyle()->getType() != TileStyle::INNERMIDDLECENTER)
	    {
		maptile->setBuilding(Maptile::PORT);
		Portlist::getInstance()->add(new Port(tile));
		if (Stacklist::getObjectAt(tile))
		  {
		    Stack* s = Stacklist::getObjectAt(tile);
		    if (s->hasShip() == true)
		      {
			for (Stack::iterator it = s->begin(); it != s->end(); 
			     it++)
			  (*it)->setInShip(false);
		      }
		  }
	    }
	    break;
	    
	case BRIDGE:
	    if ((maptile->getBuilding() == Maptile::NONE
		 || maptile->getBuilding() == Maptile::BRIDGE)
		&& maptile->getMaptileType() == Tile::WATER)
	    {
		int type = tile_to_bridge_type (tile);
		if (maptile->getBuilding() == Maptile::BRIDGE)
		    Bridgelist::getInstance()->getObjectAt(tile)->setType(type);
		else
		{
		    maptile->setBuilding(Maptile::BRIDGE);
		    Bridgelist::getInstance()->add(new Bridge(tile, type));
		    if (Stacklist::getObjectAt(tile))
		      {
			Stack* s = Stacklist::getObjectAt(tile);
			if (s->hasShip() == true)
			  {
			    for (Stack::iterator it = s->begin(); it != s->end(); 
				 it++)
			      (*it)->setInShip(false);
			  }
		      }
		}
	        changed_tiles.dim = Vector<int>(1, 1);
	    }
	    break;
	    
	case ROAD:
	    if ((maptile->getBuilding() == Maptile::NONE
		 || maptile->getBuilding() == Maptile::ROAD)
		&& maptile->getMaptileType() != Tile::WATER)
	    {
		int type = CreateScenario::calculateRoadType(tile);
		if (maptile->getBuilding() == Maptile::NONE)
		{
		    maptile->setBuilding(Maptile::ROAD);
		    Roadlist::getInstance()->add(new Road(tile, type));
		}

		// now reconfigure all roads in the surroundings
		for (int x = tile.x - 1; x <= tile.x + 1; ++x)
		    for (int y = tile.y - 1; y <= tile.y + 1; ++y)
		    {
			if ((x < 0 || x >= GameMap::getWidth()) &&
			    (y < 0 || y >= GameMap::getHeight()))
			    continue;

			Vector<int> pos(x, y);
			if (Road *r = Roadlist::getInstance()->getObjectAt(pos))
			{
			    int newtype = CreateScenario::calculateRoadType(pos);
			    r->setType(newtype);
			}
		    }
		
		changed_tiles.pos -= Vector<int>(1, 1);
		changed_tiles.dim = Vector<int>(3, 3);
	    }
	    break;
	case BAG:
	    if (maptile->getMaptileType() != Tile::WATER)
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
    }

    draw();
}

void EditorBigMap::smooth_view()
{
  GameMap::getInstance()->applyTileStyles(view.y, view.x, view.y+view.h, 
					  view.x+view.w, true);
  draw();
}

void EditorBigMap::after_draw()
{
    int tilesize = GameMap::getInstance()->getTileset()->getTileSize();
    std::vector<Vector<int> > tiles;

    if (show_tile_types_instead_of_tile_styles)
      {
	tiles = get_screen_tiles();
	for (std::vector<Vector<int> >::iterator i = tiles.begin(),
	     end = tiles.end(); i != end; ++i)
	  {
	    Vector<int> pos = tile_to_buffer_pos(*i);
	    buffer_gc->set_rgb_fg_color(GameMap::getInstance()->getTile(*i)->getColor());
	    int x = pos.x;
	    int y = pos.y;
	    int ts = tilesize;
	    if (d_grid_toggled)
	      {
		x++;
		y++;
		ts--;
	      }
	    buffer->draw_rectangle (buffer_gc, true, x, y, ts, ts);
	  }
      }

    // we need to draw a drawing cursor on the map
    tiles = get_cursor_tiles();
    // draw each tile
	
    Gdk::Color terrain_box_color = Gdk::Color();
    terrain_box_color.set_rgb_p(200.0/255.0, 200.0/255.0, 200.0/255.0);
    Gdk::Color erase_box_color = Gdk::Color();
    erase_box_color.set_rgb_p(200.0/255.0, 50.0/255.0, 50.0/255.0);
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
	    buffer_gc->set_rgb_fg_color (terrain_box_color);
	    buffer->draw_rectangle (buffer_gc, false, pos.x + 1, pos.y + 1, 
				    tilesize - 1, tilesize -1);
	    break;

	  case ERASE:
	    buffer_gc->set_rgb_fg_color (erase_box_color);
	    buffer->draw_rectangle (buffer_gc, false, pos.x + 1, pos.y + 1, 
				    tilesize - 1, tilesize -1);
	    break;

	  case STACK:
	    pic = GraphicsCache::getInstance()->getArmyPic
	      (Playerlist::getInstance()->getNeutral()->getArmyset(), 0,
	       Playerlist::getInstance()->getNeutral(), NULL);
	    pic->blit(buffer, pos);
	    break;

	  case CITY:
	    pic = GraphicsCache::getInstance()->getCityPic(0, Playerlist::getInstance()->getNeutral());
	    pic->blit(buffer, pos);
	    break;

	  case RUIN:
	    pic = GraphicsCache::getInstance()->getRuinPic(0);
	    pic->blit(buffer, pos);
	    break;

	  case TEMPLE:
	    pic = GraphicsCache::getInstance()->getTemplePic(0);
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
