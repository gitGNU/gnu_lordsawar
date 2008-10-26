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

#include <config.h>

#include <assert.h>
#include <sigc++/functors/mem_fun.h>
#include <libglademm/xml.h>
#include <gtkmm/box.h>
#include <gtkmm/filefilter.h>

#include "game-preferences-dialog.h"
#include "cycle-button.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "xmlhelper.h"
#include "armysetlist.h"
#include "shieldsetlist.h"
#include "GameScenario.h"
#include "GraphicsCache.h"
#include "GraphicsLoader.h"
#include "tilesetlist.h"
#include "citysetlist.h"
#include "player.h"

static bool inhibit_difficulty_combobox = false;

Uint32 GamePreferencesDialog::get_active_tile_size()
{
  return (Uint32) atoi(tile_size_combobox->get_active_text().c_str());
}

void GamePreferencesDialog::init()
{
    Uint32 counter = 0;
    Uint32 default_id = 0;
    Gtk::Box *box;
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/game-preferences-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    decorate(dialog.get());
    window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));

    xml->get_widget("start_game_button", start_game_button);
    xml->get_widget("difficulty_label", difficulty_label);
    xml->get_widget("random_map_radio", random_map_radio);
    xml->get_widget("load_map_radio", load_map_radio);
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
    xml->get_widget("game_name_label", game_name_label);
    xml->get_widget("game_name_entry", game_name_entry);

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

    //fill in tile sizes combobox
    tile_size_combobox = manage(new Gtk::ComboBoxText);
    std::list<Uint32> sizes;
    Tilesetlist::getInstance()->getSizes(sizes);
    Citysetlist::getInstance()->getSizes(sizes);
    Armysetlist::getInstance()->getSizes(sizes);
    for (std::list<Uint32>::iterator it = sizes.begin(); it != sizes.end();
	 it++)
      {
	Glib::ustring s = String::ucompose("%1x%1", *it);
	tile_size_combobox->append_text(s);
	if ((*it) == Tileset::getDefaultTileSize())
	  default_id = counter;
	counter++;
      }
    tile_size_combobox->set_active(default_id);
    xml->get_widget("tile_size_box", box);
    box->pack_start(*tile_size_combobox, Gtk::PACK_SHRINK);
    tile_size_combobox->signal_changed().connect(
	sigc::mem_fun(*this, &GamePreferencesDialog::on_tile_size_changed));

    // make new tile themes combobox
    tile_theme_combobox = manage(new Gtk::ComboBoxText);
    xml->get_widget("tile_theme_box", box);
    box->pack_start(*tile_theme_combobox, Gtk::PACK_SHRINK);

    // make new army themes combobox
    army_theme_combobox = manage(new Gtk::ComboBoxText);
    xml->get_widget("army_theme_box", box);
    box->pack_start(*army_theme_combobox, Gtk::PACK_SHRINK);

    // make new city themes combobox
    city_theme_combobox = manage(new Gtk::ComboBoxText);
    xml->get_widget("city_theme_box", box);
    box->pack_start(*city_theme_combobox, Gtk::PACK_SHRINK);

    on_tile_size_changed();

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
    shield_theme_combobox->signal_changed().connect
      (sigc::mem_fun(this, &GamePreferencesDialog::update_shields));

    xml->get_widget("shield_theme_box", box);
    box->pack_start(*shield_theme_combobox, Gtk::PACK_SHRINK);

    start_game_button->signal_clicked().connect
      (sigc::mem_fun(*this, &GamePreferencesDialog::on_start_game_clicked));

    xml->connect_clicked(
	"edit_options_button",
	sigc::mem_fun(*this, &GamePreferencesDialog::on_edit_options_clicked));

    xml->get_widget("cities_can_produce_allies_checkbutton", 
		    cities_can_produce_allies_checkbutton);
  game_options_dialog = new GameOptionsDialog(false);
  game_options_dialog->difficulty_option_changed.connect(
	sigc::mem_fun(*this, 
		      &GamePreferencesDialog::update_difficulty_rating));
      
  return;
}

GamePreferencesDialog::GamePreferencesDialog(GameScenario::PlayMode play_mode)
{
  d_campaign = false;
  mode = play_mode;
  init();
  if (mode != GameScenario::NETWORKED)
    {
      delete game_name_label;
      delete game_name_entry;
    }

}

GamePreferencesDialog::GamePreferencesDialog(std::string filename, bool campaign)
{
  d_campaign = campaign;
  mode = GameScenario::HOTSEAT;
  if (campaign)
    mode = GameScenario::CAMPAIGN;
  init ();
  delete game_name_label;
  delete game_name_entry;
  load_map_radio->set_active(true);
  load_map_filechooser->set_filename(filename);
  load_map_radio->set_sensitive(false);
  random_map_radio->set_sensitive(false);
  //load_map_filechooser->set_sensitive(false);
  load_map_filechooser->set_child_visible(false);
  if (campaign)
    dialog->set_title(_("New Campaign"));
  else
    dialog->set_title(_("New Scenario"));
}

GamePreferencesDialog::~GamePreferencesDialog()
{
  delete game_options_dialog;
}

void GamePreferencesDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
}

void GamePreferencesDialog::hide()
{
  dialog->hide();
}

bool GamePreferencesDialog::run(std::string nickname)
{
    dialog->show_all();
    if (mode == GameScenario::NETWORKED)
      {
	std::string text = nickname;
	text += "'s game";
	game_name_entry->set_text(text);
      }
    update_shields();
    on_player_type_changed();
    int response = dialog->run();
    if (response == 0)
      return true;
    return false;
}

SDL_Surface *GamePreferencesDialog::getShieldPic(Uint32 type, Uint32 owner)
{
  Shieldsetlist *sl = Shieldsetlist::getInstance();
  std::string shieldset = "default";
  if (shield_theme_combobox->is_sensitive() == true)
    shieldset = sl->getShieldsetDir
      (Glib::filename_from_utf8(shield_theme_combobox->get_active_text()));
  else if (load_map_parameters.shield_theme != "")
    shieldset = load_map_parameters.shield_theme;

  ShieldStyle *sh= sl->getShield(shieldset, type, owner);
  SDL_Color color = sl->getMaskColor(shieldset, owner);
  return GraphicsCache::applyMask(sh->getPixmap(), sh->getMask(), color, false);
}

void GamePreferencesDialog::add_player(const Glib::ustring &type,
				       const Glib::ustring &name)
{
  //okay, add a new hbox, with a combo and an entry in it
  //add it to players_vbox
  Gtk::HBox *player_hbox = new Gtk::HBox();
  std::list<Gtk::Button*> states;
  states.push_back(manage(new Gtk::Button(HUMAN_PLAYER_TYPE)));
  states.push_back(manage(new Gtk::Button(EASY_PLAYER_TYPE)));
  states.push_back(manage(new Gtk::Button(HARD_PLAYER_TYPE)));
  states.push_back(manage(new Gtk::Button(NO_PLAYER_TYPE)));
  CycleButton *player_type = new CycleButton(states);
  player_type->signal_changed.connect
      (sigc::mem_fun(this, &GamePreferencesDialog::on_player_type_changed));
  Gtk::Entry *player_name = new Gtk::Entry();
  player_name->set_text(name);

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
  player_hbox->pack_start(*manage(player_name), Gtk::PACK_SHRINK, 10);
  player_hbox->add(*manage(player_type->get_widget()));
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
	std::list<CycleButton *>::iterator c = player_types.begin();
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
	    std::list<CycleButton *>::iterator c = player_types.begin();
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
		(*e)->set_text((*i).name);
	      }
	  }
	else
	  {
	    start_game_button->set_sensitive(false);
	  }
      }

    load_map_filechooser->set_sensitive(!random_map);
    random_map_container->set_sensitive(random_map);
    update_shields();
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

void GamePreferencesDialog::update_shields()
{
  if (dialog->is_realized() == false)
    return;
  GraphicsLoader::instantiatePixmaps(Shieldsetlist::getInstance());
  //get rid of the old shields
  player_shields.clear();

  std::vector<Gtk::Widget*> list;
  list = players_vbox->get_children();

  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      Gtk::HBox *player_hbox = static_cast<Gtk::HBox*>(list[i + 1]);
      std::vector<Gtk::Widget*> sublist;
      sublist = player_hbox->get_children();
      if (sublist.size() > 2)
	  player_hbox->remove(*sublist[0]);
    }

  //make the shields and display them
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      Gtk::Image *player_shield = new Gtk::Image
	(to_pixbuf(getShieldPic(2, i)));
      player_shields.push_back(player_shield);
      Gtk::HBox *player_hbox = static_cast<Gtk::HBox*>(list[i + 1]);
      player_hbox->pack_start(*manage(player_shield), Gtk::PACK_SHRINK, 10);
      player_hbox->reorder_child(*player_shield, 0);
      player_hbox->show_all();
    }
  players_vbox->show_all();

}

void GamePreferencesDialog::on_player_type_changed()
{
  Uint32 offcount = 0;
    std::list<CycleButton *>::iterator c = player_types.begin();
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
    std::list<CycleButton *>::iterator c = player_types.begin();
    for (; c != player_types.end(); c++)
      {
	GameParameters::Player p;
	p.type = player_type_to_enum((*c)->get_active_text());
	g.players.push_back(p);
      }

    g.see_opponents_stacks = GameScenarioOptions::s_see_opponents_stacks;
    g.see_opponents_production = GameScenarioOptions::s_see_opponents_production;
    g.play_with_quests = GameScenarioOptions::s_play_with_quests;
    g.hidden_map = GameScenarioOptions::s_hidden_map;
    g.neutral_cities = GameScenarioOptions::s_neutral_cities;
    g.razing_cities = GameScenarioOptions::s_razing_cities;
    g.diplomacy = GameScenarioOptions::s_diplomacy;
    g.cusp_of_war = GameScenarioOptions::s_cusp_of_war;
    g.random_turns = GameScenarioOptions::s_random_turns;
    g.quick_start = Configuration::s_quick_start;
    g.intense_combat = GameScenarioOptions::s_intense_combat;
    g.military_advisor = GameScenarioOptions::s_military_advisor;

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
	    g.map.width = MAP_SIZE_SMALL_WIDTH;
	    g.map.height = MAP_SIZE_SMALL_HEIGHT;
	    g.map.ruins = 20;
	    break;
	
	case MAP_SIZE_TINY:
	    g.map.width = MAP_SIZE_TINY_WIDTH;
	    g.map.height = MAP_SIZE_TINY_HEIGHT;
	    g.map.ruins = 15;
	    break;
	
	case MAP_SIZE_NORMAL:
	default:
	    g.map.width = MAP_SIZE_NORMAL_WIDTH;
	    g.map.height = MAP_SIZE_NORMAL_HEIGHT;
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

    int id = 0;
    std::list<CycleButton *>::iterator c = player_types.begin();
    std::list<Gtk::Entry *>::iterator e = player_names.begin();
    for (; c != player_types.end(); c++, e++, id++)
      {
	GameParameters::Player p;
	p.type = player_type_to_enum((*c)->get_active_text());
	Glib::ustring name = (*e)->get_text();
	p.name = name;
	p.id = id;
	g.players.push_back(p);
      }

    g.tile_theme = tl->getTilesetDir 
      (Glib::filename_from_utf8(tile_theme_combobox->get_active_text()),
       get_active_tile_size());

    g.army_theme = al->getArmysetDir
      (Glib::filename_from_utf8(army_theme_combobox->get_active_text()),
       get_active_tile_size());

    g.shield_theme = sl->getShieldsetDir 
      (Glib::filename_from_utf8(shield_theme_combobox->get_active_text()));

    g.city_theme = cl->getCitysetDir 
      (Glib::filename_from_utf8(city_theme_combobox->get_active_text()),
       get_active_tile_size());

    g.process_armies = GameParameters::PROCESS_ARMIES_AT_PLAYERS_TURN;

    g.see_opponents_stacks = GameScenarioOptions::s_see_opponents_stacks;
    g.see_opponents_production = GameScenarioOptions::s_see_opponents_production;
    g.play_with_quests = GameScenarioOptions::s_play_with_quests;
    g.hidden_map = GameScenarioOptions::s_hidden_map;
    g.neutral_cities = GameScenarioOptions::s_neutral_cities;
    g.razing_cities = GameScenarioOptions::s_razing_cities;
    g.diplomacy = GameScenarioOptions::s_diplomacy;
    g.random_turns = GameScenarioOptions::s_random_turns;
    g.quick_start = Configuration::s_quick_start;
    g.intense_combat = GameScenarioOptions::s_intense_combat;
    g.military_advisor = GameScenarioOptions::s_military_advisor;
    g.cities_can_produce_allies = 
      cities_can_produce_allies_checkbutton->get_active();

    g.difficulty = GameScenario::calculate_difficulty_rating(g);

    if (mode == GameScenario::NETWORKED)
      g.name = game_name_entry->get_text();
    else
      g.name = _("Autogenerated");

    //don't start if the armyset size differs from the terrain set size.
    Uint32 army_tilesize = al->getArmyset(g.army_theme)->getTileSize();
      //al->getTileSize(al->getArmysetId(g.army_theme, get_active_tile_size()));
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
	dialog->hide();
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
      GameScenarioOptions::s_see_opponents_stacks = true;
      GameScenarioOptions::s_see_opponents_production = true;
      GameScenarioOptions::s_play_with_quests = false;
      GameScenarioOptions::s_hidden_map = false;
      GameScenarioOptions::s_neutral_cities = GameParameters::AVERAGE;
      GameScenarioOptions::s_razing_cities = GameParameters::ALWAYS;
      GameScenarioOptions::s_diplomacy = false;
      GameScenarioOptions::s_cusp_of_war = false;
      type_num = 1; break;

    case INTERMEDIATE:
      GameScenarioOptions::s_see_opponents_stacks = false;
      GameScenarioOptions::s_see_opponents_production = true;
      GameScenarioOptions::s_play_with_quests = true;
      GameScenarioOptions::s_hidden_map = false;
      GameScenarioOptions::s_neutral_cities = GameParameters::STRONG;
      GameScenarioOptions::s_razing_cities = GameParameters::ALWAYS;
      GameScenarioOptions::s_diplomacy = true;
      GameScenarioOptions::s_cusp_of_war = false;
      type_num = 1; break;

    case ADVANCED:
      GameScenarioOptions::s_see_opponents_stacks = false;
      GameScenarioOptions::s_see_opponents_production = false;
      GameScenarioOptions::s_play_with_quests = true;
      GameScenarioOptions::s_hidden_map = true;
      GameScenarioOptions::s_neutral_cities = GameParameters::ACTIVE;
      GameScenarioOptions::s_razing_cities = GameParameters::ON_CAPTURE;
      GameScenarioOptions::s_diplomacy = true;
      GameScenarioOptions::s_cusp_of_war = false;
      type_num = 2; break;

    case I_AM_THE_GREATEST:
      GameScenarioOptions::s_see_opponents_stacks = false;
      GameScenarioOptions::s_see_opponents_production = false;
      GameScenarioOptions::s_play_with_quests = true;
      GameScenarioOptions::s_hidden_map = true;
      GameScenarioOptions::s_neutral_cities = GameParameters::ACTIVE;
      GameScenarioOptions::s_razing_cities = GameParameters::NEVER;
      GameScenarioOptions::s_diplomacy = true;
      GameScenarioOptions::s_cusp_of_war = true;
      type_num = 2; break;

    case CUSTOM:
      break;
    }

  if (inhibit_difficulty_combobox == false)
    {
      if (type_num)
	{
	  std::list<CycleButton *>::iterator c = player_types.begin();
	  for (; c != player_types.end(); c++)
	    {
	      if ((*c)->get_active_row_number() != 3)
		(*c)->set_active (type_num);
	    }
	}
      update_difficulty_rating();
    }
}

bool GamePreferencesDialog::is_beginner()
{
  return (GameScenarioOptions::s_see_opponents_stacks == true &&
	  GameScenarioOptions::s_see_opponents_production == true &&
	  GameScenarioOptions::s_play_with_quests == false &&
	  GameScenarioOptions::s_hidden_map == false &&
	  GameScenarioOptions::s_neutral_cities == GameParameters::AVERAGE &&
	  GameScenarioOptions::s_razing_cities == GameParameters::ALWAYS &&
	  GameScenarioOptions::s_diplomacy == false &&
	  GameScenarioOptions::s_cusp_of_war == false);
}

bool GamePreferencesDialog::is_intermediate()
{
  return (GameScenarioOptions::s_see_opponents_stacks == false &&
	  GameScenarioOptions::s_see_opponents_production == true &&
	  GameScenarioOptions::s_play_with_quests == true &&
	  GameScenarioOptions::s_hidden_map == false &&
	  GameScenarioOptions::s_neutral_cities == GameParameters::STRONG &&
	  GameScenarioOptions::s_razing_cities == GameParameters::ALWAYS &&
	  GameScenarioOptions::s_diplomacy == true &&
	  GameScenarioOptions::s_cusp_of_war == false);
}

bool GamePreferencesDialog::is_advanced()
{
  return (GameScenarioOptions::s_see_opponents_stacks == false &&
	  GameScenarioOptions::s_see_opponents_production == false &&
	  GameScenarioOptions::s_play_with_quests == true &&
	  GameScenarioOptions::s_hidden_map == true &&
	  GameScenarioOptions::s_neutral_cities == GameParameters::ACTIVE &&
	  GameScenarioOptions::s_razing_cities == GameParameters::ON_CAPTURE &&
	  GameScenarioOptions::s_diplomacy == true &&
	  GameScenarioOptions::s_cusp_of_war == false);
}

bool GamePreferencesDialog::is_greatest()
{
  return (GameScenarioOptions::s_see_opponents_stacks == false &&
	  GameScenarioOptions::s_see_opponents_production == false &&
	  GameScenarioOptions::s_play_with_quests == true &&
	  GameScenarioOptions::s_hidden_map == true &&
	  GameScenarioOptions::s_neutral_cities == GameParameters::ACTIVE &&
	  GameScenarioOptions::s_razing_cities == GameParameters::NEVER &&
	  GameScenarioOptions::s_diplomacy == true &&
	  GameScenarioOptions::s_cusp_of_war == true);
}

void GamePreferencesDialog::on_map_chosen()
{
  std::string selected_filename = load_map_filechooser->get_filename();
  if (selected_filename.empty())
    return;
  bool broken;
  load_map_parameters = GameScenario::loadGameParameters(selected_filename,
							 broken);
  if (broken)
    return;
  on_random_map_toggled();
  update_shields();
  on_player_type_changed();

  //loop through all players, making the non-humans unsensitive
  if (d_campaign)
    {
      bool force = false;
      std::list<CycleButton *>::iterator c = player_types.begin();
      std::list<Gtk::Entry *>::iterator e = player_names.begin();
      for (; c != player_types.end(); c++, e++)
	{
	  if ((*c)->get_active_text() != HUMAN_PLAYER_TYPE)
	    {
	      (*c)->set_sensitive(false);
	      (*e)->set_sensitive(false);
	    }
	  else if ((*c)->get_active_text() == HUMAN_PLAYER_TYPE && force)
	    {
	      (*c)->set_sensitive(true);
	      (*c)->set_active(1);
	      (*c)->set_sensitive(false);
	      (*e)->set_sensitive(false);
	    }
	  else if ((*c)->get_active_text() == HUMAN_PLAYER_TYPE)
	    {
	      force = true;
	      (*c)->set_sensitive(false);
	    }
	}
    }
  return;
}

void GamePreferencesDialog::set_title(std::string text)
{
  Decorated::set_title(text);
}

void GamePreferencesDialog::on_tile_size_changed()
{
  Uint32 default_id = 0;
  Uint32 counter = 0;

  tile_theme_combobox->clear_items();
  Tilesetlist *tl = Tilesetlist::getInstance();
  std::list<std::string> tile_themes = tl->getNames(get_active_tile_size());
  for (std::list<std::string>::iterator i = tile_themes.begin(),
       end = tile_themes.end(); i != end; ++i)
    {
      if (*i == "Default")
	default_id = counter;
      tile_theme_combobox->append_text(Glib::filename_to_utf8(*i));
      counter++;
    }

  tile_theme_combobox->set_active(default_id);
  if (tile_theme_combobox->get_children().size() == 0)
    start_game_button->set_sensitive(false);

  army_theme_combobox->clear_items();
  Armysetlist *al = Armysetlist::getInstance();
  std::list<std::string> army_themes = al->getNames(get_active_tile_size());
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
  if (army_theme_combobox->get_children().size() == 0)
    start_game_button->set_sensitive(false);

  city_theme_combobox->clear_items();
  Citysetlist *cl = Citysetlist::getInstance();
  std::list<std::string> city_themes = cl->getNames(get_active_tile_size());
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
  if (city_theme_combobox->get_children().size() == 0)
    start_game_button->set_sensitive(false);
}
