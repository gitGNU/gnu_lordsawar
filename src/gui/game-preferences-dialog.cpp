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

#include "glade-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../File.h"
#include "../xmlhelper.h"
#include "../armysetlist.h"
#include "../shieldsetlist.h"
#include "../GameScenario.h"
#include "../tilesetlist.h"
#include "../citysetlist.h"
#include "../player.h"

#define HUMAN_PLAYER_TYPE _("Human")
#define EASY_PLAYER_TYPE _("Easy")
#define HARD_PLAYER_TYPE _("Hard")
#define NO_PLAYER_TYPE _("Off")

static bool inhibit_difficulty_combobox = false;

GamePreferencesDialog::GamePreferencesDialog()
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/game-preferences-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("start_game_button", start_game_button);
    xml->get_widget("difficulty_label", difficulty_label);
    xml->get_widget("random_map_radio", random_map_radio);
    xml->get_widget("load_map_filechooser", load_map_filechooser);
    load_map_filechooser->signal_selection_changed().connect
       (sigc::mem_fun(*this, &GamePreferencesDialog::on_map_chosen));

    xml->get_widget("random_map_container", random_map_container);
    xml->get_widget("grass_scale", grass_scale);
    xml->get_widget("grass_random_togglebutton", grass_random_togglebutton);
    grass_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, &GamePreferencesDialog::on_grass_random_toggled));
    xml->get_widget("water_scale", water_scale);
    xml->get_widget("water_random_togglebutton", water_random_togglebutton);
    water_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, &GamePreferencesDialog::on_water_random_toggled));
    xml->get_widget("swamp_scale", swamp_scale);
    xml->get_widget("swamp_random_togglebutton", swamp_random_togglebutton);
    swamp_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, &GamePreferencesDialog::on_swamp_random_toggled));
    xml->get_widget("forest_scale", forest_scale);
    xml->get_widget("forest_random_togglebutton", forest_random_togglebutton);
    forest_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, &GamePreferencesDialog::on_forest_random_toggled));
    xml->get_widget("hills_scale", hills_scale);
    xml->get_widget("hills_random_togglebutton", hills_random_togglebutton);
    hills_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, &GamePreferencesDialog::on_hills_random_toggled));
    xml->get_widget("mountains_scale", mountains_scale);
    xml->get_widget("mountains_random_togglebutton", 
		    mountains_random_togglebutton);
    mountains_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, 
		     &GamePreferencesDialog::on_mountains_random_toggled));
    xml->get_widget("cities_scale", cities_scale);
    xml->get_widget("cities_random_togglebutton", cities_random_togglebutton);
    cities_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, &GamePreferencesDialog::on_cities_random_toggled));
    xml->get_widget("map_size_combobox", map_size_combobox);
    xml->get_widget("difficulty_combobox", difficulty_combobox);

    xml->get_widget("players_vbox", players_vbox);

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
    difficulty_combobox->set_active(CUSTOM);
    difficulty_combobox->signal_changed().connect(
	sigc::mem_fun(*this, &GamePreferencesDialog::on_difficulty_changed));
    on_map_size_changed();

    Gtk::FileFilter map_filter;
    map_filter.add_pattern("*.map");
    map_filter.set_name(_("LordsAWar map files (*.map)"));
    load_map_filechooser->set_current_folder(Configuration::s_savePath);
    load_map_filechooser->set_filter(map_filter);


    // fill in tile themes combobox
    tile_theme_combobox = manage(new Gtk::ComboBoxText);
    
    Uint32 default_id = 0;
    Uint32 counter = 0;
    Tilesetlist *tl = Tilesetlist::getInstance();
    std::list<std::string> tile_themes = tl->getNames();
    for (std::list<std::string>::iterator i = tile_themes.begin(),
	     end = tile_themes.end(); i != end; ++i)
      {
	if (*i == "Default")
	  default_id = counter;
	tile_theme_combobox->append_text(Glib::filename_to_utf8(*i));
	counter++;
      }

    tile_theme_combobox->set_active(default_id);

    Gtk::Box *box;
    xml->get_widget("tile_theme_box", box);
    box->pack_start(*tile_theme_combobox, Gtk::PACK_SHRINK);

    // fill in army themes combobox
    army_theme_combobox = manage(new Gtk::ComboBoxText);
    
    Armysetlist *al = Armysetlist::getInstance();
    std::list<std::string> army_themes = al->getNames();
    counter = 0;
    default_id = 0;
    for (std::list<std::string>::iterator i = army_themes.begin(),
	     end = army_themes.end(); i != end; ++i)
      {
	if (*i == "Default")
	  default_id = counter;
	army_theme_combobox->append_text(Glib::filename_to_utf8(*i));
	counter++;
      }

    army_theme_combobox->set_active(default_id);

    xml->get_widget("army_theme_box", box);
    box->pack_start(*army_theme_combobox, Gtk::PACK_SHRINK);

    // fill in shield themes combobox
    shield_theme_combobox = manage(new Gtk::ComboBoxText);
    
    Shieldsetlist *sl = Shieldsetlist::getInstance();
    std::list<std::string> shield_themes = sl->getNames();
    counter = 0;
    default_id = 0;
    for (std::list<std::string>::iterator i = shield_themes.begin(),
	     end = shield_themes.end(); i != end; ++i)
      {
	if (*i == "Default")
	  default_id = counter;
	shield_theme_combobox->append_text(Glib::filename_to_utf8(*i));
	counter++;
      }

    shield_theme_combobox->set_active(default_id);

    xml->get_widget("shield_theme_box", box);
    box->pack_start(*shield_theme_combobox, Gtk::PACK_SHRINK);

    // fill in city themes combobox
    city_theme_combobox = manage(new Gtk::ComboBoxText);
    
    Citysetlist *cl = Citysetlist::getInstance();
    std::list<std::string> city_themes = cl->getNames();
    counter = 0;
    default_id = 0;
    for (std::list<std::string>::iterator i = city_themes.begin(),
	     end = city_themes.end(); i != end; ++i)
      {
	if (*i == "Default")
	  default_id = counter;
	city_theme_combobox->append_text(Glib::filename_to_utf8(*i));
	counter++;
      }

    city_theme_combobox->set_active(default_id);

    xml->get_widget("city_theme_box", box);
    box->pack_start(*city_theme_combobox, Gtk::PACK_SHRINK);

    start_game_button->signal_clicked().connect
      (sigc::mem_fun(*this, &GamePreferencesDialog::on_start_game_clicked));

    xml->connect_clicked(
	"edit_options_button",
	sigc::mem_fun(*this, &GamePreferencesDialog::on_edit_options_clicked));

    xml->get_widget("cities_can_produce_allies_checkbutton", 
		    cities_can_produce_allies_checkbutton);
  game_options_dialog = new GameOptionsDialog();
  game_options_dialog->difficulty_option_changed.connect(
	sigc::mem_fun(*this, 
		      &GamePreferencesDialog::update_difficulty_rating));
}

GamePreferencesDialog::~GamePreferencesDialog()
{
  delete game_options_dialog;
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
  add_player(HUMAN_PLAYER_TYPE, *current_player_name);

    ++current_player_name;

    if (current_player_name == default_player_names.end())
	current_player_name = default_player_names.begin();
}

void GamePreferencesDialog::on_random_map_toggled()
{
    bool random_map = random_map_radio->get_active();
    
    if (random_map)
      {
	std::list<Gtk::ComboBoxText *>::iterator c = player_types.begin();
	std::list<Gtk::Entry *>::iterator e = player_names.begin();
	for (; c != player_types.end(); c++, e++)
	  {
	    (*c)->set_sensitive(true);
	    (*e)->set_sensitive(true);
	  }
	start_game_button->set_sensitive(true);
      }
    else
      {
	if (load_map_filechooser->get_filename().empty() == false)
	  {
	    //disable all names, and types
	    std::list<Gtk::ComboBoxText *>::iterator c = player_types.begin();
	    std::list<Gtk::Entry *>::iterator e = player_names.begin();
	    for (; c != player_types.end(); c++, e++)
	      {
		(*c)->set_sensitive(true);
		(*c)->set_active(GameParameters::Player::OFF);
		(*c)->set_sensitive(false);
		(*e)->set_sensitive(false);
	      }
	    //parse load map parameters.
  
	    int d = 0;
	    int b;
	    for (std::vector<GameParameters::Player>::const_iterator
		 i = load_map_parameters.players.begin(), 
		 end = load_map_parameters.players.end(); i != end; ++i, ++d) 
	      {
		c = player_types.begin();
		e = player_names.begin();
		//zip to correct combobox, entry
		for (b = 0; b < (*i).id; b++, c++, e++) ;
		(*c)->set_sensitive(true);
		(*c)->set_active((*i).type);
		(*e)->set_sensitive(true);
		(*e)->set_name((*i).name);
	      }
	  }
	else
	  {
	    start_game_button->set_sensitive(false);
	  }
      }

    load_map_filechooser->set_sensitive(!random_map);
    random_map_container->set_sensitive(random_map);
}

void GamePreferencesDialog::on_map_size_changed()
{
    switch (map_size_combobox->get_active_row_number()) {
    case MAP_SIZE_SMALL:
	cities_scale->set_value(15);
	break;
	
    case MAP_SIZE_TINY:
	cities_scale->set_value(10);
	break;

    case MAP_SIZE_NORMAL:
    default:
	cities_scale->set_value(40);
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
  inhibit_difficulty_combobox = true;
  game_options_dialog->set_parent_window(*dialog.get());
  game_options_dialog->run();
  
  update_difficulty_rating();
  update_difficulty_combobox();
  inhibit_difficulty_combobox = false;
}

void GamePreferencesDialog::update_difficulty_combobox()
{
  if (is_greatest())
    difficulty_combobox->set_active(I_AM_THE_GREATEST);
  else if (is_advanced())
    difficulty_combobox->set_active(ADVANCED);
  else if (is_intermediate())
    difficulty_combobox->set_active(INTERMEDIATE);
  else if (is_beginner())
    difficulty_combobox->set_active(BEGINNER);
  else
    difficulty_combobox->set_active(CUSTOM);
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
    update_difficulty_rating();
}

void GamePreferencesDialog::update_difficulty_rating()
{
    GameParameters g;
    std::list<Gtk::ComboBoxText *>::iterator c = player_types.begin();
    for (; c != player_types.end(); c++)
      {
	GameParameters::Player p;
	p.type = player_type_to_enum((*c)->get_active_text());
	g.players.push_back(p);
      }

    g.see_opponents_stacks = Configuration::s_see_opponents_stacks;
    g.see_opponents_production = Configuration::s_see_opponents_production;
    g.play_with_quests = Configuration::s_play_with_quests;
    g.hidden_map = Configuration::s_hidden_map;
    g.neutral_cities = Configuration::s_neutral_cities;
    g.razing_cities = Configuration::s_razing_cities;
    g.diplomacy = Configuration::s_diplomacy;
    g.cusp_of_war = Configuration::s_cusp_of_war;
    g.random_turns = Configuration::s_random_turns;
    g.quick_start = Configuration::s_quick_start;
    g.intense_combat = Configuration::s_intense_combat;
    g.military_advisor = Configuration::s_military_advisor;

    int difficulty = GameScenario::calculate_difficulty_rating(g);
    g.players.clear();

    difficulty_label->set_markup(String::ucompose("<b>%1%%</b>", difficulty));
}
void GamePreferencesDialog::on_start_game_clicked()
{
  Armysetlist *al = Armysetlist::getInstance();
  Tilesetlist *tl = Tilesetlist::getInstance();
  Shieldsetlist *sl = Shieldsetlist::getInstance();
  Citysetlist *cl = Citysetlist::getInstance();
    // read out the values in the widgets
    GameParameters g;

    if (random_map_radio->get_active()) {
	g.map_path = "";
	switch (map_size_combobox->get_active_row_number()) {
	case MAP_SIZE_SMALL:
	    g.map.width = 70;
	    g.map.height = 105;
	    g.map.ruins = 20;
	    break;
	
	case MAP_SIZE_TINY:
	    g.map.width = 50;
	    g.map.height = 75;
	    g.map.ruins = 15;
	    break;
	
	case MAP_SIZE_NORMAL:
	default:
	    g.map.width = 112;
	    g.map.height = 156;
	    g.map.ruins = 35;
	    break;
	}

	if (grass_random_togglebutton->get_active())
	  g.map.grass =  
	    int(grass_scale->get_adjustment()->get_lower()) + 
	    (rand() % (int(grass_scale->get_adjustment()->get_upper()) -
		       int(grass_scale->get_adjustment()->get_lower()) + 1));
	else
	  g.map.grass = int(grass_scale->get_value());

	if (water_random_togglebutton->get_active())
	  g.map.water =  
	    int(water_scale->get_adjustment()->get_lower()) + 
	    (rand() % (int(water_scale->get_adjustment()->get_upper()) -
		       int(water_scale->get_adjustment()->get_lower()) + 1));
	else
	  g.map.water = int(water_scale->get_value());

	if (swamp_random_togglebutton->get_active())
	  g.map.swamp =  
	    int(swamp_scale->get_adjustment()->get_lower()) + 
	    (rand() % (int(swamp_scale->get_adjustment()->get_upper()) -
		       int(swamp_scale->get_adjustment()->get_lower()) + 1));
	else
	  g.map.swamp = int(swamp_scale->get_value());

	if (forest_random_togglebutton->get_active())
	  g.map.forest =  
	    int(forest_scale->get_adjustment()->get_lower()) + 
	    (rand() % (int(forest_scale->get_adjustment()->get_upper()) -
		       int(forest_scale->get_adjustment()->get_lower()) + 1));
	else
	  g.map.forest = int(forest_scale->get_value());

	if (hills_random_togglebutton->get_active())
	  g.map.hills =  
	    int(hills_scale->get_adjustment()->get_lower()) + 
	    (rand() % (int(hills_scale->get_adjustment()->get_upper()) -
		       int(hills_scale->get_adjustment()->get_lower()) + 1));
	else
	  g.map.hills = int(hills_scale->get_value());

	if (mountains_random_togglebutton->get_active())
	  g.map.mountains =  
	    int(mountains_scale->get_adjustment()->get_lower()) + 
	    (rand() % (int(mountains_scale->get_adjustment()->get_upper()) -
		       int(mountains_scale->get_adjustment()->get_lower()) 
		       + 1));
	else
	  g.map.mountains = int(mountains_scale->get_value());

	if (cities_random_togglebutton->get_active())
	  g.map.cities =  
	    int(cities_scale->get_adjustment()->get_lower()) + 
	    (rand() % (int(cities_scale->get_adjustment()->get_upper()) -
		       int(cities_scale->get_adjustment()->get_lower()) + 1));
	else
	  g.map.cities = int(cities_scale->get_value());

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

    g.tile_theme = tl->getTilesetDir 
      (Glib::filename_from_utf8(tile_theme_combobox->get_active_text()));

    g.army_theme = Glib::filename_from_utf8(army_theme_combobox->get_active_text());
    g.shield_theme = sl->getShieldsetDir 
      (Glib::filename_from_utf8(shield_theme_combobox->get_active_text()));

    g.city_theme = cl->getCitysetDir 
      (Glib::filename_from_utf8(city_theme_combobox->get_active_text()));

    g.process_armies = GameParameters::PROCESS_ARMIES_AT_PLAYERS_TURN;

    g.see_opponents_stacks = Configuration::s_see_opponents_stacks;
    g.see_opponents_production = Configuration::s_see_opponents_production;
    g.play_with_quests = Configuration::s_play_with_quests;
    g.hidden_map = Configuration::s_hidden_map;
    g.neutral_cities = Configuration::s_neutral_cities;
    g.razing_cities = Configuration::s_razing_cities;
    g.diplomacy = Configuration::s_diplomacy;
    g.random_turns = Configuration::s_random_turns;
    g.quick_start = Configuration::s_quick_start;
    g.intense_combat = Configuration::s_intense_combat;
    g.military_advisor = Configuration::s_military_advisor;
    g.cities_can_produce_allies = 
      cities_can_produce_allies_checkbutton->get_active();

    g.difficulty = GameScenario::calculate_difficulty_rating(g);

    //don't start if the armyset size differs from the terrain set size.
    Uint32 army_tilesize = al->getTileSize(al->getArmysetId(g.army_theme));
    Uint32 terrain_size = tl->getTileset(g.tile_theme)->getTileSize();
    Uint32 city_size = cl->getCityset(g.city_theme)->getTileSize();
    if (army_tilesize != (Uint32) terrain_size)
      fprintf (stderr, "army tile size doesn't match terrain tile size!\n");
    else if (army_tilesize != (Uint32) city_size)
      fprintf (stderr, "army tile size doesn't match city tile size!\n");
    else if ((Uint32)terrain_size != (Uint32) city_size)
      fprintf (stderr, "terrain tile size doesn't match city tile size!\n");
    else
      {
	// and call callback
	game_started(g);
      }
}
    
void GamePreferencesDialog::on_grass_random_toggled()
{
  grass_scale->set_sensitive(!grass_random_togglebutton->get_active());
}
void GamePreferencesDialog::on_water_random_toggled()
{
  water_scale->set_sensitive(!water_random_togglebutton->get_active());
}
void GamePreferencesDialog::on_swamp_random_toggled()
{
  swamp_scale->set_sensitive(!swamp_random_togglebutton->get_active());
}
void GamePreferencesDialog::on_forest_random_toggled()
{
  forest_scale->set_sensitive(!forest_random_togglebutton->get_active());
}
void GamePreferencesDialog::on_hills_random_toggled()
{
  hills_scale->set_sensitive(!hills_random_togglebutton->get_active());
}
void GamePreferencesDialog::on_mountains_random_toggled()
{
  mountains_scale->set_sensitive(!mountains_random_togglebutton->get_active());
}
void GamePreferencesDialog::on_cities_random_toggled()
{
  cities_scale->set_sensitive(!cities_random_togglebutton->get_active());
}

void GamePreferencesDialog::on_difficulty_changed()
{
  int type_num = 0;
  switch (difficulty_combobox->get_active_row_number()) 
    {
    case BEGINNER:
      Configuration::s_see_opponents_stacks = true;
      Configuration::s_see_opponents_production = true;
      Configuration::s_play_with_quests = false;
      Configuration::s_hidden_map = false;
      Configuration::s_neutral_cities = GameParameters::AVERAGE;
      Configuration::s_razing_cities = GameParameters::ALWAYS;
      Configuration::s_diplomacy = false;
      Configuration::s_cusp_of_war = false;
      type_num = 1; break;

    case INTERMEDIATE:
      Configuration::s_see_opponents_stacks = false;
      Configuration::s_see_opponents_production = true;
      Configuration::s_play_with_quests = true;
      Configuration::s_hidden_map = false;
      Configuration::s_neutral_cities = GameParameters::STRONG;
      Configuration::s_razing_cities = GameParameters::ALWAYS;
      Configuration::s_diplomacy = true;
      Configuration::s_cusp_of_war = false;
      type_num = 1; break;

    case ADVANCED:
      Configuration::s_see_opponents_stacks = false;
      Configuration::s_see_opponents_production = false;
      Configuration::s_play_with_quests = true;
      Configuration::s_hidden_map = true;
      Configuration::s_neutral_cities = GameParameters::ACTIVE;
      Configuration::s_razing_cities = GameParameters::ON_CAPTURE;
      Configuration::s_diplomacy = true;
      Configuration::s_cusp_of_war = false;
      type_num = 2; break;

    case I_AM_THE_GREATEST:
      Configuration::s_see_opponents_stacks = false;
      Configuration::s_see_opponents_production = false;
      Configuration::s_play_with_quests = true;
      Configuration::s_hidden_map = true;
      Configuration::s_neutral_cities = GameParameters::ACTIVE;
      Configuration::s_razing_cities = GameParameters::NEVER;
      Configuration::s_diplomacy = true;
      Configuration::s_cusp_of_war = true;
      type_num = 2; break;

    case CUSTOM:
      break;
    }

  if (inhibit_difficulty_combobox == false)
    {
      if (type_num)
	{
	  std::list<Gtk::ComboBoxText *>::iterator c = player_types.begin();
	  for (; c != player_types.end(); c++)
	    (*c)->set_active (type_num);
	}
      update_difficulty_rating();
    }
}

bool GamePreferencesDialog::is_beginner()
{
  return (Configuration::s_see_opponents_stacks == true &&
	  Configuration::s_see_opponents_production == true &&
	  Configuration::s_play_with_quests == false &&
	  Configuration::s_hidden_map == false &&
	  Configuration::s_neutral_cities == GameParameters::AVERAGE &&
	  Configuration::s_razing_cities == GameParameters::ALWAYS &&
	  Configuration::s_diplomacy == false &&
	  Configuration::s_cusp_of_war == false);
}
bool GamePreferencesDialog::is_intermediate()
{
  return (
	  Configuration::s_see_opponents_stacks == false &&
	  Configuration::s_see_opponents_production == true &&
	  Configuration::s_play_with_quests == true &&
	  Configuration::s_hidden_map == false &&
	  Configuration::s_neutral_cities == GameParameters::STRONG &&
	  Configuration::s_razing_cities == GameParameters::ALWAYS &&
	  Configuration::s_diplomacy == true &&
	  Configuration::s_cusp_of_war == false);
}
bool GamePreferencesDialog::is_advanced()
{
  return (Configuration::s_see_opponents_stacks == false &&
	  Configuration::s_see_opponents_production == false &&
	  Configuration::s_play_with_quests == true &&
	  Configuration::s_hidden_map == true &&
	  Configuration::s_neutral_cities == GameParameters::ACTIVE &&
	  Configuration::s_razing_cities == GameParameters::ON_CAPTURE &&
	  Configuration::s_diplomacy == true &&
	  Configuration::s_cusp_of_war == false);
}
bool GamePreferencesDialog::is_greatest()
{
  return (Configuration::s_see_opponents_stacks == false &&
	  Configuration::s_see_opponents_production == false &&
	  Configuration::s_play_with_quests == true &&
	  Configuration::s_hidden_map == true &&
	  Configuration::s_neutral_cities == GameParameters::ACTIVE &&
	  Configuration::s_razing_cities == GameParameters::NEVER &&
	  Configuration::s_diplomacy == true &&
	  Configuration::s_cusp_of_war == true);
}
void GamePreferencesDialog::on_map_chosen()
{
  std::string selected_filename = load_map_filechooser->get_filename();
  if (selected_filename.empty())
    return;
  //try to read the file, grab the players
  printf ("got it : %s\n", selected_filename.c_str());
  XML_Helper helper(selected_filename, std::ios::in, Configuration::s_zipfiles);

  load_map_parameters.players.clear();
  helper.registerTag("player",
		     sigc::mem_fun(this, &GamePreferencesDialog::scan_players));

  if (!helper.parse())
    {
      std::cerr << "Error: Could not parse " << selected_filename << std::endl;
      load_map_filechooser->set_filename("");
      return;
    }
  helper.close();
  return;
}
bool GamePreferencesDialog::scan_players(std::string tag, XML_Helper* helper)
{
    if (tag == "player")
    {
        int type;
	int id;
	std::string name;
        if (helper->getVersion() != LORDSAWAR_SAVEGAME_VERSION)
        {
            std::cerr << "scenario has wrong version, we want "
		      << LORDSAWAR_SAVEGAME_VERSION <<",\n"
		      << "scenario offers " << helper->getVersion() <<".\n";
            return false;
        }

	GameParameters::Player p;
        helper->getData(id, "id");
	p.id = id;
        helper->getData(type, "type");
	switch (Player::Type(type))
	  {
	  case Player::HUMAN: p.type = GameParameters::Player::HUMAN;
	    break;
	  case Player::AI_FAST: p.type = GameParameters::Player::EASY;
	    break;
	  case Player::AI_DUMMY: p.type = GameParameters::Player::EASY;
	    break;
	  case Player::AI_SMART: p.type = GameParameters::Player::HARD;
	    break;
	  }
        helper->getData(name, "name");
	p.name = name;
	if (p.id != 8) //is not neutral
	  load_map_parameters.players.push_back(p);
	on_random_map_toggled();
    }

    return true;
}

