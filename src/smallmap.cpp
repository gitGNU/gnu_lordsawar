// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2004 Thomas Plonka
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

#include "smallmap.h"
#include "vector.h"
#include "GameScenarioOptions.h"
#include "GameMap.h"
#include "playerlist.h"

SmallMap::SmallMap()
{
    input_locked = false;
    sliding = false;
    view.pos = Vector<int>(0, 0);
    view.dim = Vector<int>(3, 3);
    sleep_interval = TIMER_SMALLMAP_REFRESH;
}

void SmallMap::set_view(Rectangle new_view)
{
    if (view != new_view)
    {
	view = new_view;
	draw(Playerlist::getViewingplayer());
    }
}

void SmallMap::draw_selection()
{
    // draw the selection rectangle that shows the viewed part of the map
    Vector<int> pos = mapToSurface(view.pos);

    int w = int(view.w * pixels_per_tile);
    int h = int(view.h * pixels_per_tile);

    int width = get_width();
    int height = get_height();
    
    // this is a bit unfortunate.  we require this catch-all
    // so that our selector box isn't too big for the smallmap
    if (pos.x + w >= width)
      pos.x = width - w - 1;
    if (pos.y + h >= height)
      pos.y = height - h - 1;

    assert(pos.x >= 0 && pos.x + w < width &&
	   pos.y >= 0 && pos.y + h < height);
    
    draw_rect(pos.x, pos.y, w, h, SELECTOR_BOX_COLOUR);
    draw_rect(pos.x-1, pos.y-1, w+2,  h+2, SELECTOR_BOX_COLOUR);
}

void SmallMap::center_view_on_tile(Vector<int> pos, bool slide)
{
  pos = clip(Vector<int>(0,0), pos - view.dim / 2, 
             GameMap::get_dim() - view.dim);

  sliding = false;
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

  pos -= view.dim / pixels_per_tile;

  pos = clip(Vector<int>(0, 0), pos, GameMap::get_dim() - view.dim);

  if (slide)
    slide_view(Rectangle(pos.x, pos.y, view.w, view.h));
  else
    set_view(Rectangle(pos.x, pos.y, view.w, view.h));

  view_changed.emit(view);
}

void SmallMap::after_draw()
{
  int width = get_width(), height = get_height();
  if (blank_screen == true)
    {
      map_changed.emit(surface, Gdk::Rectangle(0, 0, width, height));
      return;
    }
  Player *p = Playerlist::getViewingplayer();
  OverviewMap::after_draw();
  if (p->getType() == Player::HUMAN ||
      GameScenarioOptions::s_hidden_map == false)
    {
      draw_cities(false);
      draw_selection();
    }
  //for the editor...
  if (GameScenarioOptions::s_round == 0)
    {
      draw_cities(false);
      draw_selection();
    }
    map_changed.emit(surface, Gdk::Rectangle(0, 0, width, height));
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

int SmallMap::slide (int x, int y)
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
      sliding = true;
      sliding_to = new_view;
      while (1)
	{
	  Rectangle tmp_view(view);
	  tmp_view.x = slide(tmp_view.x, new_view.x);
	  tmp_view.y = slide(tmp_view.y, new_view.y);

	  view = tmp_view;
	  draw(Playerlist::getViewingplayer());
          view_slid.emit(view);
          if (sliding_to != new_view)
            break;
          Glib::usleep(sleep_interval);

	  if (tmp_view.x == new_view.x && tmp_view.y == new_view.y)
	    break;
	}
      sliding = false;
    }
}

void SmallMap::move_map_in_dir(Vector<int> dir)
{
  Rectangle new_view = view;
  new_view.pos += dir;
  if (new_view.pos.x + new_view.w >= GameMap::getWidth() ||
      new_view.pos.y + new_view.h >= GameMap::getHeight() ||
      new_view.pos.x < 0 || new_view.pos.y < 0)
    new_view.pos -= dir;

  set_view(new_view);
  view_changed.emit(view);
}
