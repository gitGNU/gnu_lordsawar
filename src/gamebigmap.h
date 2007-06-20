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
    GameBigMap();
    virtual ~GameBigMap();

    // will center the bigmap on the stack
    void select_active_stack();
    void unselect_active_stack();

    void mouse_button_event(MouseButtonEvent e);
    void mouse_motion_event(MouseMotionEvent e);

    // whether the map accepts input events
    void set_input_locked(bool locked) { input_locked = locked; }

    // signals for mouse clicks, deselect is signified with a null pointer
    sigc::signal<void, Stack*> stack_selected;
    sigc::signal<void, City*, bool> city_selected; // true => show brief info
    sigc::signal<void, Ruin*> ruin_selected;
    sigc::signal<void, Signpost*> signpost_selected;
    sigc::signal<void, Temple*> temple_selected;

    // emitted when a path for a stack is set
    sigc::signal<void> path_set;

 private:
    SDL_Surface* d_arrows;
    Vector<int> current_tile, prev_mouse_pos;
    
    bool input_locked;
	
    enum {
	NONE, DRAGGING, SHOWING_CITY, SHOWING_RUIN,
	SHOWING_TEMPLE, SHOWING_SIGNPOST
    } mouse_state;
	

    // for the marching ants around selected stack
    sigc::connection selection_timeout_handler;
    bool on_selection_timeout();

    virtual void after_draw();
};

#endif
