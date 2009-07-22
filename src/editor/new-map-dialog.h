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

#ifndef NEW_MAP_DIALOG_H
#define NEW_MAP_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <gtkmm.h>

#include "Tile.h"
#include "game-parameters.h"

//! Scenario editor.  Edit parameters to make a new map.
class NewMapDialog
{
 public:
    NewMapDialog();
    ~NewMapDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();

    struct Map
    {
	int fill_style;
	int width, height;
	int grass, water, swamp, forest, hills, mountains;
	int cities, ruins, temples;
	int signposts;
	std::string tileset;
	std::string shieldset;
	std::string cityset;
	std::string armyset;
    };

    Map map;
    
    bool map_set;
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;

    Gtk::ComboBox *map_size_combobox;
    Gtk::ComboBoxText *fill_style_combobox;
    Gtk::Widget *random_map_container;
    Gtk::ComboBoxText *tile_size_combobox;
    Gtk::ComboBoxText *tile_theme_combobox;
    Gtk::ComboBoxText *city_theme_combobox;
    Gtk::ComboBoxText *army_theme_combobox;
    Gtk::ComboBoxText *shield_theme_combobox;
    Gtk::Scale *grass_scale;
    Gtk::Scale *water_scale;
    Gtk::Scale *swamp_scale;
    Gtk::Scale *forest_scale;
    Gtk::Scale *hills_scale;
    Gtk::Scale *mountains_scale;
    Gtk::Scale *cities_scale;
    Gtk::Scale *ruins_scale;
    Gtk::Scale *temples_scale;
    Gtk::Scale *signposts_scale;
    Gtk::Button *accept_button;

    enum { MAP_SIZE_NORMAL = 0, MAP_SIZE_SMALL, MAP_SIZE_TINY };

    void on_fill_style_changed();
    void on_map_size_changed();

    void add_fill_style(Tile::Type tile_type);

    Uint32 get_active_tile_size();
    void on_tile_size_changed();
    std::vector<int> fill_style;
};

#endif
