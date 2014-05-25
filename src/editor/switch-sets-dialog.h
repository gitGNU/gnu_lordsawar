//  Copyright (C) 2009, 2014 Ben Asselstine
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

#ifndef SWITCH_SETS_DIALOG_H
#define SWITCH_SETS_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <gtkmm.h>

#include "Tile.h"
#include "game-parameters.h"

class Tileset;
class Armyset;
class Cityset;
class Shieldset;

//! Scenario editor.  Change the army/tile/city/shieldsets of the map.
class SwitchSetsDialog
{
 public:
    SwitchSetsDialog();
    ~SwitchSetsDialog();

    void set_parent_window(Gtk::Window &parent);

    int run();

    Tileset* get_selected_tileset() {return selected_tileset;};
    Armyset* get_selected_armyset() {return selected_armyset;};
    Cityset* get_selected_cityset() {return selected_cityset;};
    Shieldset* get_selected_shieldset() {return selected_shieldset;};

    bool get_armyset_changed() const {return armyset_changed;};
    bool get_tileset_changed() const {return tileset_changed;};
    bool get_cityset_changed() const {return cityset_changed;};
    bool get_shieldset_changed() const {return shieldset_changed;};
    
 private:
    Gtk::Dialog* dialog;

    Gtk::ComboBoxText *tile_size_combobox;
    Gtk::ComboBoxText *tile_theme_combobox;
    Gtk::ComboBoxText *city_theme_combobox;
    Gtk::ComboBoxText *army_theme_combobox;
    Gtk::ComboBoxText *shield_theme_combobox;
    Gtk::Button *accept_button;

    guint32 get_active_tile_size();
    void on_tile_size_changed();
    Tileset* selected_tileset;
    Shieldset* selected_shieldset;
    Cityset* selected_cityset;
    Armyset* selected_armyset;
    bool armyset_changed;
    bool tileset_changed;
    bool cityset_changed;
    bool shieldset_changed;

    void switchArmyset(Armyset *armyset);
};

#endif
