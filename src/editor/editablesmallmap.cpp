// Copyright (C) 2010 Ben Asselstine
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

#include <algorithm>
#include <assert.h>

#include "vector.h"
#include "GraphicsCache.h"
#include "playerlist.h"
#include "tilesetlist.h"
#include "GameMap.h"
#include "citysetlist.h"
#include "cityset.h"

#include "editablesmallmap.h"

EditableSmallMap::EditableSmallMap()
{
}

EditableSmallMap::~EditableSmallMap()
{
}

void EditableSmallMap::after_draw()
{
  int width = 0, height = 0;
  surface->get_size(width, height);
  OverviewMap::after_draw();
  draw_cities(false);
  map_changed.emit(surface, Gdk::Rectangle(0, 0, width, height));
}

Rectangle EditableSmallMap::get_cursor_rectangle(Vector<int> current_tile)
{
    int offset = (pointer_size - 1) / 2;
    Vector<int> tile = current_tile - Vector<int>(offset, offset);

    return Rectangle (tile.x, tile.y, pointer_size, pointer_size);
}

void EditableSmallMap::change_map(Vector<int> tile)
{
  switch (pointer)
    {
    case POINTER:
      //fixme: say what's on a given tile, and what kind of tile this is.
      //then don't redraw
      break;
    case ERASE: 
      GameMap::getInstance()->eraseTile(tile);
      break;
    case TERRAIN:
        {
          Maptile *maptile = GameMap::getInstance()->getTile(tile);
          // don't change terrain to water if there is a building underneath
          if (maptile->getBuilding() != Maptile::NONE && 
              pointer_terrain == Tile::WATER)
            break;
          // don't change the terrain to anything else than grass if there is
          // a city
          if (maptile->getBuilding() == Maptile::CITY && 
              pointer_terrain != Tile::GRASS)
            break;

          GameMap::getInstance()->putTerrain(get_cursor_rectangle(tile), 
                                             pointer_terrain, -1, true);
        }
      break;
    case CITY:
      GameMap::getInstance()->putNewCity(tile);
      break;
    case RUIN:
      GameMap::getInstance()->putNewRuin(tile);
      break;
    case TEMPLE: 
      GameMap::getInstance()->putNewTemple(tile);
      break;
    }
  draw(Playerlist::getViewingplayer());
  return;
}

void EditableSmallMap::mouse_button_event(MouseButtonEvent e)
{

  if (e.button == MouseButtonEvent::LEFT_BUTTON
      && e.state == MouseButtonEvent::PRESSED)
    change_map(mapFromScreen(e.pos));
}

void EditableSmallMap::mouse_motion_event(MouseMotionEvent e)
{
  if (e.pressed[MouseMotionEvent::LEFT_BUTTON])
    change_map(mapFromScreen(e.pos));
}

    
void EditableSmallMap::set_pointer(Pointer p, int size, Tile::Type t)
{
  bool redraw = false;
  if (pointer != p || pointer_size != size)
    redraw = true;
  pointer = p;
  pointer_terrain = t;
  pointer_size = size;

  if (redraw)
    draw(Playerlist::getViewingplayer());
}

Glib::RefPtr<Gdk::Pixbuf> EditableSmallMap::get_cursor(Vector<int> & hotspot) const
{
  Glib::RefPtr<Gdk::Pixbuf> cursor;
  switch (pointer)
    {
    case POINTER:
      cursor = GraphicsCache::getInstance()->getCursorPic(GraphicsCache::POINTER)->to_pixbuf();
      hotspot = Vector<int>(cursor->get_width() / 2, cursor->get_height() / 2);
      break;
    case ERASE: 
      cursor = GraphicsCache::getInstance()->getCursorPic(GraphicsCache::TARGET)->to_pixbuf();
      hotspot = Vector<int>(cursor->get_width() / 2, cursor->get_height() / 2);
      break;
    case TERRAIN:
        {
          Gdk::Color terraincolor = 
            Tilesetlist::getInstance()->getColor
            (GameMap::getInstance()->getTileset(), pointer_terrain);
          cursor = getDotPic(pointer_size, pointer_size, terraincolor);
          hotspot = 
            Vector<int>(cursor->get_width() / 2, cursor->get_height() / 2);
        }
      break;
    case CITY:
      cursor = GraphicsCache::getInstance()->getShieldPic(0, Playerlist::getInstance()->getNeutral())->to_pixbuf();
      hotspot = Vector<int>(cursor->get_width() / 2, cursor->get_height() / 2);
      break;
    case RUIN:
        {
          Gdk::Color ruindotcolor = Gdk::Color();
          ruindotcolor.set_rgb_p(100,100,100);
          int size = int(pixels_per_tile) > 1 ? int(pixels_per_tile) : 1;
          cursor = getDotPic(size, size, ruindotcolor);
          hotspot = 
            Vector<int>(cursor->get_width() / 2, cursor->get_height() / 2);
        }
      break;
    case TEMPLE: 
        {
          Gdk::Color templedotcolor = Gdk::Color();
          templedotcolor.set_rgb_p(100,100,100);
          int size = int(pixels_per_tile) > 1 ? int(pixels_per_tile) : 1;
          cursor = getDotPic(size, size, templedotcolor);
          hotspot = 
            Vector<int>(cursor->get_width() / 2, cursor->get_height() / 2);
        }
      break;
    }
  return cursor;
}
          
Glib::RefPtr<Gdk::Pixbuf> EditableSmallMap::getDotPic(guint32 width, 
                                                      guint32 height, 
                                                      Gdk::Color color) const
{
  Glib::RefPtr<Gdk::Pixbuf> dot = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, 
                                                      true, 8, width, height);
  dot->fill(color.get_pixel());
  return dot;

}
