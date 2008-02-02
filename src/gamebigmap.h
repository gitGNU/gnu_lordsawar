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

#ifndef GAMEBIGMAP_H
#define GAMEBIGMAP_H

#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>

#include "vector.h"
#include "input-events.h"
#include "GraphicsCache.h"
#include "bigmap.h"

class Stack;
class City;
class Ruin;
class Signpost;
class Temple;

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
    bool is_shift_key_down () const {return shift_key_is_down;}

    // whether the map accepts input events
    void set_input_locked(bool locked) { input_locked = locked; }

    // signals for mouse clicks, deselect is signified with a null pointer
    sigc::signal<void, Stack*> stack_selected;
    sigc::signal<void, City*, bool> city_queried; // true => show brief info
    sigc::signal<void, Ruin*, bool> ruin_queried; //true => show brief info
    sigc::signal<void, Signpost*> signpost_queried;
    sigc::signal<void, Stack*> stack_queried;
    sigc::signal<void, Temple*, bool> temple_queried; //true=>show brief info

    // emitted when a path for a stack is set
    sigc::signal<void> path_set;
    // emitted when the cursor changes
    sigc::signal<void, GraphicsCache::CursorType> cursor_changed;

 private:
    SDL_Surface* d_waypoints;
    Vector<int> current_tile, prev_mouse_pos;
    
    bool input_locked;
	
    enum {
	NONE, DRAGGING, SHOWING_CITY, SHOWING_RUIN,
	SHOWING_TEMPLE, SHOWING_SIGNPOST, SHOWING_STACK
    } mouse_state;
    bool shift_key_is_down;
	
    GraphicsCache::CursorType d_cursor;
    void determine_mouse_cursor(Stack *stack, Vector<int> tile);

    // for the marching ants around selected stack
    sigc::connection selection_timeout_handler;
    bool on_selection_timeout();

    virtual void after_draw();
    bool d_intense_combat; 
    bool d_see_opponents_production;
    bool d_see_opponents_stacks;
    bool d_military_advisor;
};

#endif
