//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011 Ben Asselstine
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

#ifndef GAMEBIGMAP_H
#define GAMEBIGMAP_H

#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>

#include "vector.h"
#include "input-events.h"
#include "GraphicsCache.h"
#include "bigmap.h"
#include "LocationBox.h"
#include "PixMask.h"

class Stack;
class City;
class Ruin;
class Signpost;
class Temple;
class PathCalculator;

/** Specialization of BigMap for the game (as opposed to the editor)
  */
class GameBigMap: public BigMap
{
 public:
    GameBigMap(bool intense_combat, bool see_opponents_production, bool see_opponents_stacks, bool military_advisor);
    virtual ~GameBigMap();

    // will center the bigmap on the stack
    void select_active_stack();
    void unselect_active_stack();

    void mouse_button_event(MouseButtonEvent e);
    void mouse_motion_event(MouseMotionEvent e);
    void set_shift_key_down (bool down);
    bool is_shift_key_down () const {return control_key_is_down;}
    void set_control_key_down (bool down);
    bool is_control_key_down () const {return control_key_is_down;}

    // whether the map accepts input events
    void set_input_locked(bool locked) { input_locked = locked; }

    // signals for mouse clicks, deselect is signified with a null pointer
    sigc::signal<void, Stack*> stack_selected;
    sigc::signal<void, Stack*> stack_grouped_or_ungrouped;
    sigc::signal<void, City*> city_visited;  //for citywindow
    sigc::signal<void, Vector<int>, City*> city_queried;  //for city-info-tip
    sigc::signal<void> city_unqueried;
    sigc::signal<void, Ruin*, bool> ruin_queried; //true => show brief info
    sigc::signal<void, Signpost*> signpost_queried;
    sigc::signal<void, Vector<int> > stack_queried;
    sigc::signal<void> stack_unqueried;
    sigc::signal<void, Temple*, bool> temple_queried; //true=>show brief info

    // emitted when a path for a stack is set
    sigc::signal<void> path_set;
    // emitted when the cursor changes
    sigc::signal<void, GraphicsCache::CursorType> cursor_changed;

    //the game object sets this when the active stack is fighting so we can 
    //draw a fight graphic, or not
    void setFighting(LocationBox ruckus) {d_fighting = ruckus;};

    void reset_zoom();
    void zoom_in();
    void zoom_out();

    static const double zoom_step = 10.0;
    static const double max_magnification_factor = 200.0;
    static const double min_magnification_factor = 50.0;

    void reset_path_calculator(Stack *s);

    bool d_intense_combat; 
    bool d_see_opponents_production;
    bool d_see_opponents_stacks;
    bool d_military_advisor;
 private:
    Vector<int> current_tile, prev_mouse_pos;
    
    bool input_locked;
	
    enum mouse_state_enum {
	NONE, DRAGGING_MAP, SHOWING_CITY, SHOWING_RUIN,
	SHOWING_TEMPLE, SHOWING_SIGNPOST, SHOWING_STACK,
	DRAGGING_STACK, DRAGGING_ENDPOINT
    } mouse_state;
    bool shift_key_is_down;
    bool control_key_is_down;
	
    GraphicsCache::CursorType d_cursor;
    void determine_mouse_cursor(Stack *stack, Vector<int> tile);

    // for the marching ants around selected stack
    sigc::connection selection_timeout_handler;
    bool on_selection_timeout();
    void zoom_view(double percent);

    virtual void after_draw();
    LocationBox d_fighting;
    PathCalculator *path_calculator;
};

#endif
