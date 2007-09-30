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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.*

#include <config.h>

#include <assert.h>
#include <sigc++/functors/mem_fun.h>
#include <libglademm/xml.h>
#include <gtkmm/box.h>
#include <gtkmm/filefilter.h>

#include "game-preferences-dialog.h"
#include "game-options-dialog.h"

#include "glade-helpers.h"
#include "../defs.h"
#include "../File.h"

#define HUMAN_PLAYER_TYPE _("Human")
#define EASY_PLAYER_TYPE _("Easy")
#define HARD_PLAYER_TYPE _("Hard")
#define NO_PLAYER_TYPE _("Off")

GamePreferencesDialog::GamePreferencesDialog()
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/game-preferences-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("start_game_button", start_game_button);
    xml->get_widget("process_armies_combobox", process_armies_combobox);
    xml->get_widget("random_map_radio", random_map_radio);
    xml->get_widget("load_map_filechooser", load_map_filechooser);
    xml->get_widget("random_map_container", random_map_container);
    xml->get_widget("grass_scale", grass_scale);
    xml->get_widget("water_scale", water_scale);
    xml->get_widget("swamp_scale", swamp_scale);
    xml->get_widget("forest_scale", forest_scale);
    xml->get_widget("hills_scale", hills_scale);
    xml->get_widget("mountains_scale", mountains_scale);
    xml->get_widget("cities_scale", cities_scale);
    xml->get_widget("ruins_scale", ruins_scale);
    xml->get_widget("temples_scale", temples_scale);
    xml->get_widget("map_size_combobox", map_size_combobox);

    xml->get_widget("players_vbox", players_vbox);

    process_armies_combobox->set_active(
	GameParameters::PROCESS_ARMIES_AT_PLAYERS_TURN);

    // add default players
    default_player_names.push_back("The Sirians");
    default_player_names.push_back("Elvallie");
    default_player_names.push_back("Storm Giants");
    default_player_names.push_back("The Selentines");
    default_player_names.push_back("Grey Dwarves");
    default_player_names.push_back("Horse Lords");
    default_player_names.push_back("Orcs of Kor");
    default_player_names.push_back("Lord Bane");

    current_player_name = default_player_names.begin();

    for (unsigned int j = 0; j < default_player_names.size(); ++j)
	on_add_player_clicked();

    // setup map settings
    random_map_radio->signal_toggled().connect(
	sigc::mem_fun(*this, &GamePreferencesDialog::on_random_map_toggled));
    on_random_map_toggled();
    
    map_size_combobox->set_active(MAP_SIZE_NORMAL);
    map_size_combobox->signal_changed().connect(
	sigc::mem_fun(*this, &GamePreferencesDialog::on_map_size_changed));
    on_map_size_changed();

    Gtk::FileFilter map_filter;
    map_filter.add_pattern("*.map");
    map_filter.set_name(_("LordsAWar map files (*.map)"));
    load_map_filechooser->set_filter(map_filter);


    // fill in tile themes combobox
    tile_theme_combobox = manage(new Gtk::ComboBoxText);
    
    std::list<std::string> tile_themes = File::scanTilesets();
    for (std::list<std::string>::iterator i = tile_themes.begin(),
	     end = tile_themes.end(); i != end; ++i)
	tile_theme_combobox->append_text(Glib::filename_to_utf8(*i));

    tile_theme_combobox->set_active(0);

    Gtk::Box *box;
    xml->get_widget("tile_theme_box", box);
    box->pack_start(*tile_theme_combobox, Gtk::PACK_SHRINK);

    // fill in army themes combobox
    army_theme_combobox = manage(new Gtk::ComboBoxText);
    
    std::list<std::string> army_themes = File::scanArmysets();
    for (std::list<std::string>::iterator i = army_themes.begin(),
	     end = army_themes.end(); i != end; ++i)
      {
	army_theme_combobox->append_text(Glib::filename_to_utf8(*i));
      }

    army_theme_combobox->set_active(0);

    xml->get_widget("army_theme_box", box);
    box->pack_start(*army_theme_combobox, Gtk::PACK_SHRINK);
    start_game_button->signal_clicked().connect
      (sigc::mem_fun(*this, &GamePreferencesDialog::on_start_game_clicked));

    xml->connect_clicked(
	"edit_options_button",
	sigc::mem_fun(*this, &GamePreferencesDialog::on_edit_options_clicked));

}

GamePreferencesDialog::~GamePreferencesDialog()
{
}

void GamePreferencesDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
}

void GamePreferencesDialog::run()
{
    dialog->show_all();
    dialog->run();
}

void GamePreferencesDialog::add_player(const Glib::ustring &type,
				       const Glib::ustring &name)
{
  //okay, add a new hbox, with a combo and an entry in it
  //add it to players_vbox
  Gtk::HBox *player_hbox = new Gtk::HBox();
  Gtk::ComboBoxText *player_type = new Gtk::ComboBoxText();
  player_type->signal_changed().connect
      (sigc::mem_fun(this, &GamePreferencesDialog::on_player_type_changed));
  Gtk::Entry *player_name = new Gtk::Entry();
  player_name->set_text(name);
  player_type->append_text(HUMAN_PLAYER_TYPE);
  player_type->append_text(EASY_PLAYER_TYPE);
  player_type->append_text(HARD_PLAYER_TYPE);
  player_type->append_text(NO_PLAYER_TYPE);

  if (type == HUMAN_PLAYER_TYPE)
    player_type->set_active(0);
  else if (type == EASY_PLAYER_TYPE)
    player_type->set_active(1);
  else if (type == HARD_PLAYER_TYPE)
    player_type->set_active(2);
  else if (type== NO_PLAYER_TYPE)
    player_type->set_active(3);

  player_types.push_back(player_type);
  player_names.push_back(player_name);
  player_hbox->add(*manage(player_name));
  player_hbox->add(*manage(player_type));
  players_vbox->add(*manage(player_hbox));
}

void GamePreferencesDialog::on_add_player_clicked()
{
    if (player_names.empty())
	add_player(HUMAN_PLAYER_TYPE, *current_player_name);

    else
	add_player(EASY_PLAYER_TYPE, *current_player_name);

    ++current_player_name;

    if (current_player_name == default_player_names.end())
	current_player_name = default_player_names.begin();
}

void GamePreferencesDialog::on_random_map_toggled()
{
    bool random_map = random_map_radio->get_active();
    
    load_map_filechooser->set_sensitive(!random_map);
    random_map_container->set_sensitive(random_map);
}

void GamePreferencesDialog::on_map_size_changed()
{
    switch (map_size_combobox->get_active_row_number()) {
    case MAP_SIZE_SMALL:
	cities_scale->set_value(15);
	ruins_scale->set_value(20);
	temples_scale->set_value(20);
	break;
	
    case MAP_SIZE_TINY:
	cities_scale->set_value(10);
	ruins_scale->set_value(15);
	temples_scale->set_value(15);
	break;

    case MAP_SIZE_NORMAL:
    default:
	cities_scale->set_value(40);
	ruins_scale->set_value(25);
	temples_scale->set_value(10);
	break;
    }
}

namespace 
{
    GameParameters::Player::Type player_type_to_enum(const Glib::ustring &s)
    {
	if (s == HUMAN_PLAYER_TYPE)
	    return GameParameters::Player::HUMAN;
	else if (s == EASY_PLAYER_TYPE)
	    return GameParameters::Player::EASY;
	else if (s == NO_PLAYER_TYPE)
	    return GameParameters::Player::OFF;
	else
	    return GameParameters::Player::HARD;
    }
}

void GamePreferencesDialog::on_edit_options_clicked()
{
  GameOptionsDialog d;
  d.set_parent_window(*dialog.get());
  d.run();
}

void GamePreferencesDialog::on_player_type_changed()
{
  Uint32 offcount = 0;
    std::list<Gtk::ComboBoxText *>::iterator c = player_types.begin();
    for (; c != player_types.end(); c++)
      {
	if (player_type_to_enum((*c)->get_active_text()) ==
	    GameParameters::Player::OFF)
	  offcount++;
      }
    if (offcount > player_types.size() - 2)
      start_game_button->set_sensitive(false);
    else
      start_game_button->set_sensitive(true);
}
void GamePreferencesDialog::on_start_game_clicked()
{
    // read out the values in the widgets
    GameParameters g;

    if (random_map_radio->get_active()) {
	g.map_path = "";
	switch (map_size_combobox->get_active_row_number()) {
	case MAP_SIZE_SMALL:
	    g.map.width = 70;
	    g.map.height = 105;
	    break;
	
	case MAP_SIZE_TINY:
	    g.map.width = 50;
	    g.map.height = 75;
	    break;
	
	case MAP_SIZE_NORMAL:
	default:
	    g.map.width = 112;
	    g.map.height = 156;
	    break;
	}
	g.map.grass = int(grass_scale->get_value());
	g.map.water = int(water_scale->get_value());
	g.map.swamp = int(swamp_scale->get_value());
	g.map.forest = int(forest_scale->get_value());
	g.map.hills = int(hills_scale->get_value());
	g.map.mountains = int(mountains_scale->get_value());
	g.map.cities = int(cities_scale->get_value());
	g.map.ruins = int(ruins_scale->get_value());
	g.map.temples = int(temples_scale->get_value());
    }
    else
	g.map_path = load_map_filechooser->get_filename();

    std::list<Gtk::ComboBoxText *>::iterator c = player_types.begin();
    std::list<Gtk::Entry *>::iterator e = player_names.begin();
    for (; c != player_types.end(); c++, e++)
      {
	GameParameters::Player p;
	p.type = player_type_to_enum((*c)->get_active_text());
	Glib::ustring name = (*e)->get_text();
	p.name = name;
	g.players.push_back(p);
      }

    g.tile_theme
	= Glib::filename_from_utf8(tile_theme_combobox->get_active_text());

    g.army_theme = Glib::filename_from_utf8(army_theme_combobox->get_active_text());

    g.process_armies = GameParameters::ProcessArmies(
	process_armies_combobox->get_active_row_number());

    g.see_opponents_stacks = Configuration::s_see_opponents_stacks;
    g.see_opponents_production = Configuration::s_see_opponents_production;
    g.play_with_quests = Configuration::s_play_with_quests;
    g.hidden_map = Configuration::s_hidden_map;
    g.neutral_cities = Configuration::s_neutral_cities;
    g.diplomacy = Configuration::s_diplomacy;
    g.random_turns = Configuration::s_random_turns;
    g.quick_start = Configuration::s_quick_start;
    g.intense_combat = Configuration::s_intense_combat;
    g.military_advisor = Configuration::s_military_advisor;

    // and call callback
    game_started(g);
}
