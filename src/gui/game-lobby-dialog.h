//  Copyright (C) 2008 Ben Asselstine
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

#ifndef GAME_LOBBY_DIALOG_H
#define GAME_LOBBY_DIALOG_H

#include <string>
#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include "../citymap.h"
#include "../GameScenario.h"

struct SDL_Surface;

//! dialog for accepting/rejecting a hero
class GameLobbyDialog: public sigc::trackable
{
 public:
    GameLobbyDialog(std::string filename, bool has_ops);
    GameLobbyDialog(GameScenario *game_scenario, bool has_ops);

    ~GameLobbyDialog();

    void set_parent_window(Gtk::Window &parent);

    bool run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    //! The mini map that shows the scenario map
    std::auto_ptr<CityMap> citymap;
    Gtk::Image *map_image;

    void initDialog(GameScenario *gamescenario);
    void on_map_changed(SDL_Surface *map);
    bool d_destroy_gamescenario;
    GameScenario *d_game_scenario;
    Gtk::Label *turn_label;
    Gtk::Label *scenario_name_label;
    Gtk::Label *cities_label;
    void fill_in_scenario_details();
    void on_show_options_clicked();
};

#endif
