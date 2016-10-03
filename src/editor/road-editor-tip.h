//  Copyright (C) 2015 Ben Asselstine
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

#ifndef ROAD_EDITOR_TIP_H
#define ROAD_EDITOR_TIP_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include "defs.h"
#include "map-tip-position.h"

class Road;
// shows a tooltip like window with information about an army
class RoadEditorTip: public sigc::trackable
{
 public:
    // the tip is shown above target, simply delete the object to hide it again
    RoadEditorTip(Gtk::Widget *target, MapTipPosition mpos, Road *r);
    ~RoadEditorTip();
    sigc::signal<void,Vector<int>,int> road_picked;

 private:
    Gtk::Window* window;
    Gtk::Box *button_box;
    Gtk::RadioButton* buttons[ROAD_TYPES];
    Gtk::RadioButton::Group group;
    Road *road;

    void fill_road_buttons();
    void connect_signals();
    void on_road_selected(int type);
};

#endif
