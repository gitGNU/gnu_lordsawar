//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014 Ben Asselstine
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

#ifndef NEW_RANDOM_MAP_DIALOG_H
#define NEW_RANDOM_MAP_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <gtkmm.h>

#include "Tile.h"
#include "game-parameters.h"

//! A dialog to let the user create a new random map.
class NewRandomMapDialog: public sigc::trackable
{
 public:
    NewRandomMapDialog();
    ~NewRandomMapDialog();

    void set_parent_window(Gtk::Window &parent);

    int run();

    std::string getRandomMapFilename() const {return d_filename;};

    //std::string create_and_dump_scenario(const std::string &file, const GameParameters &g);
static std::string create_and_dump_scenario(const std::string &file,
                                                         const GameParameters &g, sigc::slot<void> *pulse);
    
 private:

    void pulse();
    GameParameters getParams();
    struct Map
    {
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
    Gtk::Dialog* dialog;

    Gtk::Box *dialog_vbox;
    Gtk::ButtonBox *dialog_action_area;
    Gtk::ComboBox *map_size_combobox;
    Gtk::ProgressBar *progressbar;
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
    Gtk::Button *cancel_button;
    Gtk::ToggleButton *grass_random_togglebutton;
    Gtk::ToggleButton *water_random_togglebutton;
    Gtk::ToggleButton *swamp_random_togglebutton;
    Gtk::ToggleButton *forest_random_togglebutton;
    Gtk::ToggleButton *hills_random_togglebutton;
    Gtk::ToggleButton *mountains_random_togglebutton;
    Gtk::ToggleButton *cities_random_togglebutton;

    Gtk::CheckButton *cities_can_produce_allies_checkbutton;

    enum { MAP_SIZE_NORMAL = 0, MAP_SIZE_SMALL, MAP_SIZE_TINY };

    void on_map_size_changed();

    void on_grass_random_toggled();
    void on_water_random_toggled();
    void on_swamp_random_toggled();
    void on_forest_random_toggled();
    void on_hills_random_toggled();
    void on_mountains_random_toggled();
    void on_cities_random_toggled();
    void on_accept_clicked();
    void on_cancel_clicked();

    guint32 get_active_tile_size();
    void on_tile_size_changed();
    void on_grass_changed();
    int dialog_response;
    std::string d_filename;
};

#endif
