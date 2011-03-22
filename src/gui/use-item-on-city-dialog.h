//  Copyright (C) 2011 Ben Asselstine
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

#ifndef USE_ITEM_ON_CITY_DIALOG_H
#define USE_ITEM_ON_CITY_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "select-city-map.h"

class City;

#include "decorated.h"

// dialog for targetting a player when using an item.
class UseItemOnCityDialog: public Decorated
{
 public:
    UseItemOnCityDialog(SelectCityMap::Type type);
    ~UseItemOnCityDialog();

    void set_parent_window(Gtk::Window &parent);

    void hide();
    City *run();
    
 private:
    Gtk::Dialog* dialog;
    SelectCityMap* citymap;

    Gtk::Image *map_image;
    Gtk::Button *continue_button;
    
    void on_map_changed(Glib::RefPtr<Gdk::Pixmap> map);

    void on_city_selected(City *city);

    bool on_map_mouse_button_event(GdkEventButton *e);
};

#endif
