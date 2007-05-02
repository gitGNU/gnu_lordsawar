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

#ifndef SMALLMAP_H
#define SMALLMAP_H

#include <sigc++/signal.h>
#include <sigc++/connection.h>

#include "overviewmap.h"

#include "rectangle.h"
#include "input-events.h"

/** Miniature display of the whole game map.
  * 
  * SmallMap is the image at the top right of the game screen which provides an
  * overview of the whole game map. It handles mouse clicks, moving of the
  * currently visible portion and changes of the underlying map. It draws an
  * animation of the selection rectangle.
  */
class SmallMap: public OverviewMap
{
 public:
    SmallMap();
    ~SmallMap();

    void set_view(Rectangle new_view);
        
    void mouse_button_event(MouseButtonEvent e);
    void mouse_motion_event(MouseMotionEvent e);
    
    // emitted when the view changes because of user interactions
    sigc::signal<void, Rectangle> view_changed;

    // emitted when the map surface has changed
    sigc::signal<void, SDL_Surface *> map_changed;
        
 private:
    Rectangle view;
    SDL_Color selection_color;
    bool selection_value_increasing;
    sigc::connection selection_timeout_handler;
    
    void center_view(Vector<int> p);
    void draw_selection();
    bool on_selection_timeout();

    // hook from base class
    virtual void after_draw();
	
    // EVENT HANDLERS
    bool eventMouseButtonDown(const SDL_MouseButtonEvent* event);
    bool eventMouseMotion(const SDL_MouseMotionEvent* event);
        
    bool d_pressed;
    bool d_add;
};

#endif
