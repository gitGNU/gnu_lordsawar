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

#include <algorithm>
#include <assert.h>

#include "vector.h"

#include "smallmap.h"
#include "sdl-draw.h"
#include "timing.h"
#include "GameMap.h"

namespace 
{
    int selection_timeout = 1000;	// 1 second
    int upper_selection_value = 255;
    int lower_selection_value = 255;
    int selection_change = 0;
}

SmallMap::SmallMap()
{
    input_locked = false;
    
    selection_value_increasing = true;
    selection_color.r = lower_selection_value;
    selection_color.g = lower_selection_value;
    selection_color.b = lower_selection_value;
}

void SmallMap::set_view(Rectangle new_view)
{
    if (view != new_view)
    {
	view = new_view;
	draw();
    }

    if (!selection_timeout_handler)
	selection_timeout_handler = 
	    Timing::instance().register_timer(
		sigc::mem_fun(*this, &SmallMap::on_selection_timeout),
		selection_timeout);
}

bool SmallMap::on_selection_timeout()
{
    // change selection color
    if (selection_color.r >= upper_selection_value)
	selection_value_increasing = false;

    if (selection_color.r <= lower_selection_value)
	selection_value_increasing = true;

    draw_selection();
    map_changed.emit(get_surface());
    
    if (selection_value_increasing)
    {
	selection_color.r = std::min(selection_color.r + selection_change, 255);
	selection_color.g = std::min(selection_color.g + selection_change, 255);
	selection_color.b = std::min(selection_color.b + selection_change, 255);
    }
    else
    {
	selection_color.r = std::max(selection_color.r - selection_change, 0);
	selection_color.g = std::max(selection_color.g - selection_change, 0);
	selection_color.b = std::max(selection_color.b - selection_change, 0);
    }
    
    return Timing::CONTINUE;
}

void SmallMap::draw_selection()
{
    // FIXME: this should be translucent
    
    // draw the selection rectangle that shows the viewed part of the map
    Vector<int> pos = mapToSurface(view.pos);

    // subtract 2 to account for the border on both sides
    int w = int(view.w * pixels_per_tile) - 2;
    int h = int(view.h * pixels_per_tile) - 2;

    assert(pos.x >= 0 && pos.x + w < surface->w &&
	   pos.y >= 0 && pos.y + h < surface->h);
    
    Uint32 raw = SDL_MapRGB(surface->format,
			    selection_color.r,
			    selection_color.g,
			    selection_color.b);
    draw_rect(surface, pos.x, pos.y, pos.x + w, pos.y + h, raw);
    draw_rect(surface, pos.x+1, pos.y+1, pos.x + w-1, pos.y + h-1, raw);
}

void SmallMap::center_view(Vector<int> p)
{
    p.x = int(round(p.x / pixels_per_tile));
    p.y = int(round(p.y / pixels_per_tile));
    
    p -= view.dim / 2;

    p = clip(Vector<int>(0, 0), p, GameMap::get_dim() - view.dim);
    
    set_view(Rectangle(p.x, p.y, view.w, view.h));
    view_changed.emit(view);
}

void SmallMap::after_draw()
{
    OverviewMap::after_draw();
    draw_cities(false);
    draw_selection();
    map_changed.emit(get_surface());
}

void SmallMap::mouse_button_event(MouseButtonEvent e)
{
    if (input_locked)
	return;
    
    if ((e.button == MouseButtonEvent::LEFT_BUTTON ||
	 e.button == MouseButtonEvent::RIGHT_BUTTON)
	&& e.state == MouseButtonEvent::PRESSED)
	center_view(e.pos);
}

void SmallMap::mouse_motion_event(MouseMotionEvent e)
{
    if (input_locked)
	return;
    
    if (e.pressed[MouseMotionEvent::LEFT_BUTTON] ||
	e.pressed[MouseMotionEvent::RIGHT_BUTTON])
	center_view(e.pos);
}
