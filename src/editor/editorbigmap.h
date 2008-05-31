//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef EDITORBIGMAP_H
#define EDITORBIGMAP_H

#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>

#include "../vector.h"
#include "../input-events.h"
#include "../bigmap.h"
#include "../Tile.h"

class Stack;
class MapRenderer;

class City;
class Ruin;
class Signpost;
class Temple;
class UniquelyIdentified;


//! Scenario editor.  Specializatoin of the BigMap class for the editor.
class EditorBigMap: public BigMap
{
 public:
    EditorBigMap();
    ~EditorBigMap();

    enum Pointer {
	POINTER, TERRAIN, STACK, CITY, RUIN, TEMPLE, SIGNPOST,
	ROAD, ERASE, PORT, BRIDGE
    };

    void set_pointer(Pointer pointer, int size, Tile::Type terrain, 
		     int tile_style_id);

    void mouse_button_event(MouseButtonEvent e);
    void mouse_motion_event(MouseMotionEvent e);
    void mouse_leave_event();

    // something was selected
    typedef std::vector<UniquelyIdentified *> map_selection_seq;
    sigc::signal<void, map_selection_seq> objects_selected;

    // emitted whenever the user moves the mouse to a new tile
    sigc::signal<void, Vector<int> > mouse_on_tile;

    // emitted when the map is changed by the user
    sigc::signal<void, Rectangle> map_changed;

    void smooth_view();

 private:
    Vector<int> prev_mouse_pos, mouse_pos;

    Pointer pointer;
    Tile::Type pointer_terrain;
    int pointer_size;
    int pointer_tile_style_id;

    enum {
	NONE, DRAGGING
    } mouse_state;

    virtual void after_draw();
    int tile_to_road_type(Vector<int> tile);
    int tile_to_bridge_type(Vector<int> tile);
    void change_map_under_cursor();
    std::vector<Vector<int> > get_cursor_tiles();
    int calculateRoadType (Vector<int> t);
};

#endif
