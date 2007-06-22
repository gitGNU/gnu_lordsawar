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

#include <config.h>

#include <SDL_image.h>
#include <assert.h>
#include <stdlib.h>

#include "editorbigmap.h"

#include "../army.h"
#include "../stacklist.h"
#include "../stack.h"
#include "../citylist.h"
#include "../ruinlist.h"
#include "../signpostlist.h"
#include "../templelist.h"
#include "../stonelist.h"
#include "../roadlist.h"
#include "../ruin.h"
#include "../signpost.h"
#include "../temple.h"
#include "../stone.h"
#include "../road.h"
#include "../playerlist.h"
#include "../defs.h"
#include "../File.h"
#include "../GameMap.h"
#include "../Configuration.h"
#include "../GraphicsCache.h"
#include "../armysetlist.h"
#include "../sdl-draw.h"
#include "../MapRenderer.h"


EditorBigMap::EditorBigMap()
{
    mouse_pos = prev_mouse_pos = Vector<int>(0, 0);

    mouse_state = NONE;
    input_locked = false;
    pointer = EditorBigMap::POINTER;
    pointer_size = 1;
    pointer_terrain = Tile::GRASS;
}

EditorBigMap::~EditorBigMap()
{
}

void EditorBigMap::set_pointer(Pointer p, int size, Tile::Type t)
{
    bool redraw = false;
    if (pointer != p || pointer_size != size)
	redraw = true;
    pointer = p;
    pointer_terrain = t;
    pointer_size = size;

    if (redraw)
	draw();
}

void EditorBigMap::mouse_button_event(MouseButtonEvent e)
{
    if (input_locked)
	return;
    
    Vector<int> tile = mouse_pos_to_tile(e.pos);
    
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

    if (mouse_pos != prev_mouse_pos && pointer == STONE &&
	mouse_pos_to_stone_type(mouse_pos) !=
	mouse_pos_to_stone_type(prev_mouse_pos))
	redraw = true;

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
	
	int ts = GameMap::getInstance()->getTileSet()->getTileSize();
	SDL_Surface *screen = SDL_GetVideoSurface();
	Vector<int> screen_dim(screen->w, screen->h);
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

    if (redraw)
	draw();
    
    prev_mouse_pos = mouse_pos;
}

void EditorBigMap::mouse_leave_event()
{
    mouse_pos.x = mouse_pos.y = -10000;
    mouse_on_tile.emit(Vector<int>(-100, -100));
    draw();
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


void EditorBigMap::after_draw()
{
    if (pointer == POINTER)
	return;
    
    // we need to draw a drawing cursor on the map
    
    std::vector<Vector<int> > tiles = get_cursor_tiles();

    // draw each tile
    int tilesize = GameMap::getInstance()->getTileSet()->getTileSize();
    for (std::vector<Vector<int> >::iterator i = tiles.begin(),
	     end = tiles.end(); i != end; ++i)
    {
	Vector<int> pos = tile_to_buffer_pos(*i);

	const Uint32 outline = SDL_MapRGB(buffer->format, 200, 200, 200);
	const Uint32 red_outline = SDL_MapRGB(buffer->format, 200, 50, 50);

	SDL_Rect r;
	r.x = pos.x;
	r.y = pos.y;
	r.w = r.h = tilesize;
	SDL_Surface *pic = 0;
	
	switch (pointer)
	{
	case POINTER:
	    break;
	    
	case TERRAIN:
	    draw_rect_clipped(buffer, pos.x + 1, pos.y + 1,
			      pos.x + tilesize - 2, pos.y + tilesize - 2,
			      outline);
	    break;
	    
	case ERASE:
	    draw_rect_clipped(buffer, pos.x + 1, pos.y + 1,
			      pos.x + tilesize - 2, pos.y + tilesize - 2,
			      red_outline);
	    break;

	case STACK:
	    pic = GraphicsCache::getInstance()->getArmyPic(
		Armysetlist::getInstance()->getStandardId(), 0,
		Playerlist::getInstance()->getNeutral(), 1, NULL);
	    SDL_BlitSurface(pic, 0, buffer, &r);
	    break;
	    
	case CITY:
	    pic = GraphicsCache::getInstance()->getCityPic(
		0, Playerlist::getInstance()->getNeutral());
	    r.w = r.h = tilesize * 2;
	    SDL_BlitSurface(pic, 0, buffer, &r);
	    break;
	    
	case RUIN:
	    pic = d_ruinpic;
	    SDL_BlitSurface(pic, 0, buffer, &r);
	    break;
	    
	case TEMPLE:
	    pic = GraphicsCache::getInstance()->getTemplePic(0);
	    SDL_BlitSurface(pic, 0, buffer, &r);
	    break;
	    
	case SIGNPOST:
	    pic = d_signpostpic;
	    SDL_BlitSurface(pic, 0, buffer, &r);
	    break;
	    
	case STONE:
	    pic = GraphicsCache::getInstance()->getStonePic(
		mouse_pos_to_stone_type(mouse_pos));
	    SDL_BlitSurface(pic, 0, buffer, &r);
	    draw_rect_clipped(buffer, pos.x + 1, pos.y + 1,
			      pos.x + tilesize - 2, pos.y + tilesize - 2,
			      outline);
	    break;
	    
	case ROAD:
	    pic = GraphicsCache::getInstance()->getRoadPic(tile_to_road_type(*i));
	    SDL_BlitSurface(pic, 0, buffer, &r);
	    break;
	}
    }
}

int EditorBigMap::mouse_pos_to_stone_type(Vector<int> mpos)
{
    Vector<int> p = mouse_pos_to_tile_offset(mpos);
    int type = 0;
    if (p.x > 43)
    {
	if (p.y > 43)
	    type = 5;
	else if (p.y > 21)
	    type = 3;
	else
	    type = 2;
    }
    else if (p.x > 21)
    {
	if (p.y > 43)
	    type = 6;
	else if (p.y > 21)
	    type = 4;
	else
	    type = 1;
    }
    else
    {
	if (p.y > 43)
	    type = 7; 
	else if (p.y > 21)
	    type = 8;
	else
	    type = 0;
    }
    
    return type;
}

int EditorBigMap::tile_to_road_type(Vector<int> t)
{
    // examine neighbour tiles to discover whether there's a road on them
    bool u = Roadlist::getInstance()->getObjectAt(t + Vector<int>(0, -1));
    bool b = Roadlist::getInstance()->getObjectAt(t + Vector<int>(0, 1));
    bool l = Roadlist::getInstance()->getObjectAt(t + Vector<int>(-1, 0));
    bool r = Roadlist::getInstance()->getObjectAt(t + Vector<int>(1, 0));

    // then translate this to the type
    int type = 7;
    if (!u && !b && !l && !r)
	type = 7;
    else if (u && b && l && r)
	type = 2;
    else if (!u && b && l && r)
	type = 10;
    else if (u && !b && l && r)
	type = 9;
    else if (u && b && !l && r)
	type = 8;
    else if (u && b && l && !r)
	type = 11;
    else if (u && b && !l && !r)
	type = 1;
    else if (!u && !b && l && r)
	type = 0;
    else if (u && !b && l && !r)
	type = 3;
    else if (u && !b && !l && r)
	type = 4;
    else if (!u && b && l && !r)
	type = 6;
    else if (!u && b && !l && r)
	type = 5;
    else if (u && !b && !l && !r)
	type = 1;
    else if (!u && b && !l && !r)
	type = 1;
    else if (!u && !b && l && !r)
	type = 0;
    else if (!u && !b && !l && r)
	type = 0;
    return type;
}


namespace
{
    template <typename T>
    void remove_from_map(T *l, Vector<int> tile)
    {
	for (typename T::iterator i = l->begin(), end = l->end(); i != end; ++i)
	    if (i->contains(tile))
	    {
		// erase from map
		GameMap *gamemap = GameMap::getInstance();
		Rectangle r = i->get_area();
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
    TileSet* ts = GameMap::getInstance()->getTileSet();
    
    for (std::vector<Vector<int> >::iterator i = tiles.begin(),
	     end = tiles.end(); i != end; ++i)
    {
	Vector<int> tile = *i;
	Maptile* maptile = GameMap::getInstance()->getTile(tile);
	Tile::Type tiletype = (*ts)[pointer_terrain]->getType();
	switch (pointer)
	{
	case POINTER:
	    break;
	    
	case TERRAIN:
        
	    // don't change terrain to water if there is a building underneath
	    if (maptile->getBuilding() != Maptile::NONE
		&& tiletype == Tile::WATER)
		break;
	    // don't change the terrain to anything else than grass if there is
	    // a city
	    if (maptile->getBuilding() == Maptile::CITY
		&& tiletype != Tile::GRASS)
		break;

	    maptile->setType(pointer_terrain);

	    // we expect the renderer to catch out of bound errors
	    for (int x = tile.x - 1; x <= tile.x + 1; ++x)
		for (int y = tile.y - 1; y <= tile.y + 1; ++y)
		    d_renderer->smooth(x, y);
	    break;
	    
	case ERASE:
	    // check if there is a building or a stack there and remove it

	    // first stack, it's above everything else
	    if (Stack* s = Stacklist::getObjectAt(tile))
	    {
		s->getPlayer()->deleteStack(s);
		break;
	    }
	    
	    maptile->setBuilding(Maptile::NONE);
	    
	    // ... or a temple ...
	    remove_from_map(Templelist::getInstance(), tile);
	    // ... or a stone ...
	    remove_from_map(Stonelist::getInstance(), tile);
	    // ... or a ruin ...
	    remove_from_map(Ruinlist::getInstance(), tile);
	    // ... or a road ...
	    remove_from_map(Roadlist::getInstance(), tile);
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
		Army* a = new Army(*al->getArmy(al->getStandardId(), 0), p);
		s->push_back(a);
		p->addStack(s);
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
			goto after_for;
		    }
		}
	after_for:
	    if (!city_placeable)
		break;
	    
	    // create the city
	    City c(tile);
	    c.setPlayer(Playerlist::getInstance()->getNeutral());
	    Citylist::getInstance()->push_back(c);

	    // find the index of the "grass" tile
	    unsigned int index;
	    for (index = 0; index < ts->size(); ++index)
		if ((*ts)[index]->getType() == Tile::GRASS)
		    break;
        
	    // notify the maptiles that a city has been placed here
	    Rectangle r = c.get_area();
	    for (int x = r.x; x < r.x + r.w; ++x)
		for (int y = r.y; y < r.y + r.h; ++y)
		{
		    Maptile* t = GameMap::getInstance()->getTile(Vector<int>(x, y));
		    t->setBuilding(Maptile::CITY);
		    t->setType(index);
		}

	    // finally, smooth the surrounding map
	    for (int x = r.x - 1; x < r.x + r.w + 1; ++x)
		for (int y = r.y - 1; y < r.y + r.h + 1; ++y)
		    d_renderer->smooth(x, y);
	}
	break;
	    
	case RUIN:
	    if (maptile->getBuilding() == Maptile::NONE 
		&& maptile->getMaptileType() != Tile::WATER)
	    {
		maptile->setBuilding(Maptile::RUIN);
		Ruinlist::getInstance()->push_back(Ruin(tile));
	    }
	    break;
	    
	case TEMPLE:
	    if (maptile->getBuilding() == Maptile::NONE 
		&& maptile->getMaptileType() != Tile::WATER)
	    {
		maptile->setBuilding(Maptile::TEMPLE);
		Templelist::getInstance()->push_back(Temple(tile));
	    }
	    break;
	    
	case SIGNPOST:
	    if (maptile->getBuilding() == Maptile::NONE 
		&& maptile->getMaptileType() != Tile::WATER)
	    {
		maptile->setBuilding(Maptile::SIGNPOST);
		Signpostlist::getInstance()->push_back(Signpost(tile));
	    }
	    break;
	    
	case STONE:
	    if ((maptile->getBuilding() == Maptile::NONE
		 || maptile->getBuilding() == Maptile::STONE)
		&& maptile->getMaptileType() == Tile::GRASS)
	    {
		int type = mouse_pos_to_stone_type(mouse_pos);
		if (maptile->getBuilding() == Maptile::STONE)
		    Stonelist::getInstance()->getObjectAt(tile)->setType(type);
		else
		{
		    maptile->setBuilding(Maptile::STONE);
		    Stonelist::getInstance()->push_back(Stone(tile, "", type));
		}
	    }
	    break;
	    
	case ROAD:
	    if ((maptile->getBuilding() == Maptile::NONE
		 || maptile->getBuilding() == Maptile::ROAD)
		&& maptile->getMaptileType() == Tile::GRASS)
	    {
		int type = tile_to_road_type(tile);
		if (maptile->getBuilding() == Maptile::NONE)
		{
		    maptile->setBuilding(Maptile::ROAD);
		    Roadlist::getInstance()->push_back(Road(tile, "", type));
		}

		// now reconfigure all roads in the surroundings
		for (int x = tile.x - 1; x <= tile.x + 1; ++x)
		    for (int y = tile.y - 1; y <= tile.y + 1; ++y)
		    {
			if (x < 0 || x >= GameMap::getWidth() &&
			    y < 0 || y >= GameMap::getHeight())
			    continue;

			Vector<int> pos(x, y);
			if (Road *r = Roadlist::getInstance()->getObjectAt(pos))
			{
			    int newtype = tile_to_road_type(pos);
			    r->setType(newtype);
			}
		    }
		
	    }
	    break;
	}
    }

    draw();
    map_changed.emit();
}

