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
class Object;

/** Specialization of BigMap for the editor
  */
class EditorBigMap: public BigMap
{
 public:
    EditorBigMap();
    ~EditorBigMap();

    enum Pointer {
	POINTER, TERRAIN, STACK, CITY, RUIN, TEMPLE, SIGNPOST,
	STONE, ROAD, ERASE
    };

    void set_pointer(Pointer pointer, int size, Tile::Type terrain);

    void mouse_button_event(MouseButtonEvent e);
    void mouse_motion_event(MouseMotionEvent e);
    void mouse_leave_event();

    // signals for mouse clicks, deselect is signified with a null pointer
    sigc::signal<void, Stack*> stack_selected;
    sigc::signal<void, City*> city_selected;
    sigc::signal<void, Ruin*> ruin_selected;
    sigc::signal<void, Signpost*> signpost_selected;
    sigc::signal<void, Temple*> temple_selected;

    // emitted whenever the user moves the mouse to a new tile
    sigc::signal<void, Vector<int> > mouse_on_tile;

    // emitted when the map is changed by the user
    sigc::signal<void> map_changed;

 private:
    Vector<int> prev_mouse_pos, mouse_pos;

    Pointer pointer;
    Tile::Type pointer_terrain;
    int pointer_size;

    enum {
	NONE, DRAGGING, SHOWING_CITY, SHOWING_RUIN,
	SHOWING_TEMPLE, SHOWING_SIGNPOST
    } mouse_state;

    virtual void after_draw();
    int mouse_pos_to_stone_type(Vector<int> mpos);
    int tile_to_road_type(Vector<int> tile);
    void change_map();
    std::vector<Vector<int> > get_cursor_tiles();
};

#endif
