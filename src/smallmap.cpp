// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2004 Thomas Plonka
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
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

#include <algorithm>
#include <assert.h>

#include "vector.h"

#include "smallmap.h"
#include "sdl-draw.h"
#include "timing.h"
#include "GameMap.h"
#include "Configuration.h"

SmallMap::SmallMap()
{
    input_locked = false;
}

void SmallMap::set_view(Rectangle new_view)
{
    if (view != new_view)
    {
	view = new_view;
	draw();
    }
}

void SmallMap::draw_selection()
{
    // draw the selection rectangle that shows the viewed part of the map
    Vector<int> pos = mapToSurface(view.pos);

    int w = int(view.w * pixels_per_tile);
    int h = int(view.h * pixels_per_tile);

    // this is a bit unfortunate.  we require this catch-all
    // so that our selector box isn't too big for the smallmap
    if (pos.x + w >= surface->w)
      pos.x = surface->w - w - 1;
    if (pos.y + h >= surface->h)
      pos.y = surface->h - h - 1;

    assert(pos.x >= 0 && pos.x + w < surface->w &&
	   pos.y >= 0 && pos.y + h < surface->h);
    
    Uint32 raw = SDL_MapRGB(surface->format, 255, 255, 255);
    draw_rect(surface, pos.x, pos.y, pos.x + w, pos.y + h, raw);
    draw_rect(surface, pos.x+1, pos.y+1, pos.x + w-1, pos.y + h-1, raw);
}

void SmallMap::center_view_on_tile(Vector<int> pos, bool slide)
{
  pos = clip(Vector<int>(0,0), pos - view.dim / calculateResizeFactor(), 
	     GameMap::get_dim() - view.dim);

  if (slide)
    slide_view(Rectangle(pos.x, pos.y, view.w, view.h));
  else
    set_view(Rectangle(pos.x, pos.y, view.w, view.h));
	  
  view_changed.emit(view);
}

void SmallMap::center_view_on_pixel(Vector<int> pos, bool slide)
{
  pos.x = int(round(pos.x / pixels_per_tile));
  pos.y = int(round(pos.y / pixels_per_tile));

  pos -= view.dim / calculateResizeFactor();

  pos = clip(Vector<int>(0, 0), pos, GameMap::get_dim() - view.dim);

  if (slide)
    slide_view(Rectangle(pos.x, pos.y, view.w, view.h));
  else
    set_view(Rectangle(pos.x, pos.y, view.w, view.h));
	  
  view_changed.emit(view);
}

void SmallMap::after_draw()
{
  OverviewMap::after_draw();
  if (Playerlist::getActiveplayer()->getType() == Player::HUMAN ||
      Configuration::s_hidden_map == false)
    {
      draw_cities(false);
      draw_selection();
    }
  map_changed.emit(get_surface());
}

void SmallMap::mouse_button_event(MouseButtonEvent e)
{
  if (input_locked)
    return;

  if ((e.button == MouseButtonEvent::LEFT_BUTTON ||
       e.button == MouseButtonEvent::RIGHT_BUTTON)
      && e.state == MouseButtonEvent::PRESSED)
    center_view_on_pixel(e.pos, true);
}

void SmallMap::mouse_motion_event(MouseMotionEvent e)
{
  if (input_locked)
    return;

  if (e.pressed[MouseMotionEvent::LEFT_BUTTON] ||
      e.pressed[MouseMotionEvent::RIGHT_BUTTON])
    center_view_on_pixel(e.pos, false);
}

int slide (int x, int y)
{
  int skip = 2;
  if (x < y)
    {
      if (x + skip < y)
	x += skip;
      else
	x++;
    }
  else if (x > y)
    {
      if (x - skip > y)
	x -= skip;
      else
	x--;
    }
  return x;
}

void SmallMap::slide_view(Rectangle new_view)
{
  if (view != new_view)
    {
      while (1)
	{
	  if (Playerlist::isFinished()) //window closed while ai player moves
	      return;
	  Rectangle tmp_view(view);
	  tmp_view.x = slide(tmp_view.x, new_view.x);
	  tmp_view.y = slide(tmp_view.y, new_view.y);

	  view = tmp_view;
	  draw();
	  view_slid.emit(view);

	  if (tmp_view.x == new_view.x && tmp_view.y == new_view.y)
	    break;
	}
    }
}

void SmallMap::blank()
{
  Uint32 fog_color = SDL_MapRGB(surface->format, 0, 0, 0);
  int size = int(pixels_per_tile) > 1 ? int(pixels_per_tile) : 1;
  //fog it up
  for (int i = 0; i < GameMap::getWidth(); i++)
    for (int j = 0; j < GameMap::getHeight(); j++)
      {
	Vector <int> pos;
	pos.x = i;
	pos.y = j;
	pos = mapToSurface(pos);
	draw_filled_rect(surface, pos.x, pos.y,
			 pos.x + size, pos.y + size, fog_color);
      }
  map_changed.emit(get_surface());
}

