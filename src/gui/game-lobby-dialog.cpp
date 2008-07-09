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

#include <config.h>

#include <libglademm/xml.h>
#include <gtkmm/eventbox.h>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm/label.h>

#include "game-lobby-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../File.h"
#include "../citylist.h"
#include "game-options-dialog.h"

void GameLobbyDialog::initDialog(GameScenario *gamescenario)
{
  d_game_scenario = gamescenario;
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/game-lobby-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("map_image", map_image);
    xml->get_widget("turn_label", turn_label);
    xml->get_widget("scenario_name_label", scenario_name_label);
    xml->get_widget("cities_label", cities_label);

    citymap.reset(new CityMap());
    citymap->map_changed.connect(
	sigc::mem_fun(this, &GameLobbyDialog::on_map_changed));

    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);
    xml->connect_clicked(
	"show_options_button",
	sigc::mem_fun(*this, &GameLobbyDialog::on_show_options_clicked));
}
GameLobbyDialog::GameLobbyDialog(std::string filename, bool has_ops)
{
  bool broken = false;
  d_destroy_gamescenario = true;
  GameScenario *game_scenario = new GameScenario(filename, broken);
  initDialog(game_scenario);
  fill_in_scenario_details();
}

GameLobbyDialog::GameLobbyDialog(GameScenario *game_scenario, bool has_ops)
{
  initDialog(game_scenario);
  fill_in_scenario_details();
}

GameLobbyDialog::~GameLobbyDialog()
{
  if (d_destroy_gamescenario)
    delete d_game_scenario;
  else
    {
      std::string filename = File::getSavePath() + "network.sav";
      d_game_scenario->saveGame(filename);
    }
}

void GameLobbyDialog::fill_in_scenario_details()
{
    
  Glib::ustring s;
  s = String::ucompose("%1", d_game_scenario->getRound());
  turn_label->set_text(s);
  scenario_name_label->set_text(d_game_scenario->getName());
  s = String::ucompose("%1", Citylist::getInstance()->size());
  cities_label->set_text(s);
}
void GameLobbyDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

bool GameLobbyDialog::run()
{
    citymap->resize();
    citymap->draw();

    dialog->show_all();
    int response = dialog->run();

    if (response == 0)
	return true;
    else
	return false;
}

void GameLobbyDialog::on_map_changed(SDL_Surface *map)
{
    map_image->property_pixbuf() = to_pixbuf(map);
}

void GameLobbyDialog::on_show_options_clicked()
{
  GameOptionsDialog gd(true);
  gd.run();
}
