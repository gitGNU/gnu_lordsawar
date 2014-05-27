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

#include <config.h>

#include <assert.h>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm.h>

#include "new-random-map-dialog.h"

#include "glade-helpers.h"
#include "defs.h"
#include "File.h"
#include "tileset.h"
#include "tilesetlist.h"
#include "armysetlist.h"
#include "citysetlist.h"
#include "shieldsetlist.h"
#include "ucompose.hpp"
#include "GameMap.h"
#include "GameScenarioOptions.h"
#include "CreateScenarioRandomize.h"
#include "CreateScenario.h"
#include "counter.h"


NewRandomMapDialog::NewRandomMapDialog(Gtk::Window &parent)
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + 
					 "/new-random-map-dialog.ui");

    xml->get_widget("dialog", dialog);
    dialog->set_transient_for(parent);
    xml->get_widget("dialog-vbox1", dialog_vbox);
    xml->get_widget("dialog-action_area1", dialog_action_area);
    xml->get_widget("map_size_combobox", map_size_combobox);
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
    xml->get_widget("signposts_scale", signposts_scale);
    xml->get_widget("progressbar", progressbar);
    xml->get_widget("accept2_button", accept_button);
    accept_button->signal_clicked().connect
      (sigc::mem_fun(*this, &NewRandomMapDialog::on_accept_clicked));
    xml->get_widget("cancel2_button", cancel_button);
    cancel_button->signal_clicked().connect
      (sigc::mem_fun(*this, &NewRandomMapDialog::on_cancel_clicked));
    xml->get_widget("grass_random_togglebutton", grass_random_togglebutton);
    grass_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, &NewRandomMapDialog::on_grass_random_toggled));
    xml->get_widget("water_random_togglebutton", water_random_togglebutton);
    water_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, &NewRandomMapDialog::on_water_random_toggled));
    xml->get_widget("swamp_random_togglebutton", swamp_random_togglebutton);
    swamp_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, &NewRandomMapDialog::on_swamp_random_toggled));
    xml->get_widget("forest_random_togglebutton", forest_random_togglebutton);
    forest_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, &NewRandomMapDialog::on_forest_random_toggled));
    xml->get_widget("hills_random_togglebutton", hills_random_togglebutton);
    hills_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, &NewRandomMapDialog::on_hills_random_toggled));
    xml->get_widget("mountains_random_togglebutton", mountains_random_togglebutton);
    mountains_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, &NewRandomMapDialog::on_mountains_random_toggled));
    xml->get_widget("cities_random_togglebutton", cities_random_togglebutton);
    cities_random_togglebutton->signal_toggled().connect
      (sigc::mem_fun(*this, &NewRandomMapDialog::on_cities_random_toggled));

    // fill in tile themes combobox
    
    guint32 counter = 0;
    guint32 default_id = 0;
    Gtk::HBox *box;

    //fill in tile sizes combobox
    tile_size_combobox = manage(new Gtk::ComboBoxText);
    std::list<guint32> sizes;
    Tilesetlist::getInstance()->getSizes(sizes);
    Citysetlist::getInstance()->getSizes(sizes);
    Armysetlist::getInstance()->getSizes(sizes);
    for (std::list<guint32>::iterator it = sizes.begin(); it != sizes.end();
	 it++)
      {
	Glib::ustring s = String::ucompose("%1x%1", *it);
	tile_size_combobox->append(s);
	if ((*it) == Tileset::getDefaultTileSize())
	  default_id = counter;
	counter++;
      }
    tile_size_combobox->set_active(default_id);
    xml->get_widget("tile_size_box", box);
    box->pack_start(*tile_size_combobox, Gtk::PACK_SHRINK);
    tile_size_combobox->signal_changed().connect
      (sigc::mem_fun(*this, &NewRandomMapDialog::on_tile_size_changed));

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

    counter = 0;
    default_id = 0;
    shield_theme_combobox = manage(new Gtk::ComboBoxText);
    Shieldsetlist *sl = Shieldsetlist::getInstance();
    std::list<std::string> shield_themes = sl->getValidNames();
    for (std::list<std::string>::iterator i = shield_themes.begin(),
	 end = shield_themes.end(); i != end; ++i)
      {
	if (*i == _("Default"))
	  default_id = counter;
	shield_theme_combobox->append(Glib::filename_to_utf8(*i));
	counter++;
      }

    shield_theme_combobox->set_active(default_id);

    xml->get_widget("shield_theme_box", box);
    box->pack_start(*shield_theme_combobox, Gtk::PACK_SHRINK);

    on_tile_size_changed();

    // map size
    map_size_combobox->set_active(MAP_SIZE_NORMAL);
    map_size_combobox->signal_changed().connect(
						sigc::mem_fun(*this, &NewRandomMapDialog::on_map_size_changed));

    Gtk::Label *temples_label;
    xml->get_widget("temples_label", temples_label);
    temples_label->set_sensitive(false);
    temples_scale->set_sensitive(false);

    Gtk::Label *signposts_label;
    xml->get_widget("signposts_label", signposts_label);
    signposts_label->set_sensitive(false);
    signposts_scale->set_sensitive(false);

    Gtk::Label *ruins_label;
    xml->get_widget("ruins_label", ruins_label);
    ruins_label->set_sensitive(false);
    ruins_scale->set_sensitive(false);

    xml->get_widget("cities_can_produce_allies_checkbutton", 
		    cities_can_produce_allies_checkbutton);
    grass_scale->set_value(78);
    water_scale->set_value(7);
    swamp_scale->set_value(2);
    forest_scale->set_value(3);
    hills_scale->set_value(5);
    mountains_scale->set_value(5);
    on_map_size_changed();
    grass_scale->signal_value_changed().connect(sigc::mem_fun(*this, &NewRandomMapDialog::on_grass_changed));
    dialog_response = Gtk::RESPONSE_CANCEL;
}

NewRandomMapDialog::~NewRandomMapDialog()
{
  delete dialog;
}

int NewRandomMapDialog::run()
{
  dialog->show_all();
  dialog_action_area->hide();
  progressbar->hide();
  //we're not using the buttons from the action area.
  //we have our own buttons so that we can show a progress bar after the
  //button is clicked.
  dialog->run();
  return dialog_response;
}

void NewRandomMapDialog::on_map_size_changed()
{
  switch (map_size_combobox->get_active_row_number()) {
  case MAP_SIZE_SMALL:
    map.width = MAP_SIZE_SMALL_WIDTH;
    map.height = MAP_SIZE_SMALL_HEIGHT;
    cities_scale->set_value(15);
    ruins_scale->set_value(20);
    temples_scale->set_value(4);
    break;

  case MAP_SIZE_TINY:
    map.width = MAP_SIZE_TINY_WIDTH;
    map.height = MAP_SIZE_TINY_HEIGHT;
    cities_scale->set_value(10);
    ruins_scale->set_value(15);
    temples_scale->set_value(4);
    break;

  case MAP_SIZE_NORMAL:
  default:
    map.width = MAP_SIZE_NORMAL_WIDTH;
    map.height = MAP_SIZE_NORMAL_HEIGHT;
    cities_scale->set_value(20);
    ruins_scale->set_value(25);
    temples_scale->set_value(4);
    break;
  }
  int num_signposts = 
      CreateScenario::calculateNumberOfSignposts(map.width, map.height,
                                                 int(grass_scale->get_value()));
  signposts_scale->set_value(num_signposts);
}

guint32 NewRandomMapDialog::get_active_tile_size()
{
  return (guint32) atoi(tile_size_combobox->get_active_text().c_str());
}

void NewRandomMapDialog::on_tile_size_changed()
{
  guint32 default_id = 0;
  guint32 counter = 0;

  accept_button->set_sensitive(true);
  tile_theme_combobox->remove_all();

  Tilesetlist *tl = Tilesetlist::getInstance();
  std::list<std::string> tile_themes = tl->getValidNames(get_active_tile_size());
  for (std::list<std::string>::iterator i = tile_themes.begin(),
       end = tile_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
	default_id = counter;
      tile_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  if (counter > 0)
    tile_theme_combobox->set_active(default_id);
  else
    accept_button->set_sensitive(false);
    

  army_theme_combobox->remove_all();

  Armysetlist *al = Armysetlist::getInstance();
  std::list<std::string> army_themes = al->getValidNames(get_active_tile_size());
  counter = 0;
  default_id = 0;
  for (std::list<std::string>::iterator i = army_themes.begin(),
       end = army_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
	default_id = counter;
      army_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  if (counter > 0)
    army_theme_combobox->set_active(default_id);
  else
    accept_button->set_sensitive(false);

  city_theme_combobox->remove_all();

  Citysetlist *cl = Citysetlist::getInstance();
  std::list<std::string> city_themes = cl->getValidNames(get_active_tile_size());
  counter = 0;
  default_id = 0;
  for (std::list<std::string>::iterator i = city_themes.begin(),
       end = city_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
	default_id = counter;
      city_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  if (counter > 0)
    city_theme_combobox->set_active(default_id);
  else
    accept_button->set_sensitive(false);
}

GameParameters NewRandomMapDialog::getParams()
{
  CreateScenarioRandomize random;
  GameParameters g;
  GameParameters::Player p;
  p.type = GameParameters::Player::HUMAN;
  g.players.clear();
  p.name = random.getPlayerName(Shield::WHITE);
  p.id = int(Shield::WHITE);
  g.players.push_back(p);
  p.id = int(Shield::GREEN);
  p.name = random.getPlayerName(Shield::Colour(p.id));
  g.players.push_back(p);
  p.id = int(Shield::YELLOW);
  p.name = random.getPlayerName(Shield::Colour(p.id));
  g.players.push_back(p);
  p.id = int(Shield::DARK_BLUE);
  p.name = random.getPlayerName(Shield::Colour(p.id));
  g.players.push_back(p);
  p.id = int(Shield::ORANGE);
  p.name = random.getPlayerName(Shield::Colour(p.id));
  g.players.push_back(p);
  p.id = int(Shield::LIGHT_BLUE);
  p.name = random.getPlayerName(Shield::Colour(p.id));
  g.players.push_back(p);
  p.id = int(Shield::RED);
  p.name = random.getPlayerName(Shield::Colour(p.id));
  g.players.push_back(p);
  p.id = int(Shield::BLACK);
  p.name = random.getPlayerName(Shield::Colour(p.id));
  g.players.push_back(p);

  g.map_path = "";
  switch (map_size_combobox->get_active_row_number()) {
  case MAP_SIZE_SMALL:
    g.map.width = MAP_SIZE_SMALL_WIDTH;
    g.map.height = MAP_SIZE_SMALL_HEIGHT;
    g.map.ruins = int(ruins_scale->get_value());
    g.map.temples = int(temples_scale->get_value());
    g.map.signposts = int(signposts_scale->get_value());
    break;

  case MAP_SIZE_TINY:
    g.map.width = MAP_SIZE_TINY_WIDTH;
    g.map.height = MAP_SIZE_TINY_HEIGHT;
    g.map.ruins = int(ruins_scale->get_value());
    g.map.temples = int(temples_scale->get_value());
    g.map.signposts = int(signposts_scale->get_value());
    break;

  case MAP_SIZE_NORMAL:
  default:
    g.map.width = MAP_SIZE_NORMAL_WIDTH;
    g.map.height = MAP_SIZE_NORMAL_HEIGHT;
    g.map.ruins = int(ruins_scale->get_value());
    g.map.temples = int(temples_scale->get_value());
    g.map.signposts = int(signposts_scale->get_value());
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

  Tilesetlist *tl = Tilesetlist::getInstance();
  Armysetlist *al = Armysetlist::getInstance();
  Shieldsetlist *sl = Shieldsetlist::getInstance();
  Citysetlist *cl = Citysetlist::getInstance();
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

  g.name = _("Autogenerated");
  return g;
}

void NewRandomMapDialog::on_grass_random_toggled()
{
  grass_scale->set_sensitive(!grass_random_togglebutton->get_active());
}

void NewRandomMapDialog::on_water_random_toggled()
{
  water_scale->set_sensitive(!water_random_togglebutton->get_active());
}

void NewRandomMapDialog::on_swamp_random_toggled()
{
  swamp_scale->set_sensitive(!swamp_random_togglebutton->get_active());
}

void NewRandomMapDialog::on_forest_random_toggled()
{
  forest_scale->set_sensitive(!forest_random_togglebutton->get_active());
}

void NewRandomMapDialog::on_hills_random_toggled()
{
  hills_scale->set_sensitive(!hills_random_togglebutton->get_active());
}

void NewRandomMapDialog::on_mountains_random_toggled()
{
  mountains_scale->set_sensitive(!mountains_random_togglebutton->get_active());
}

void NewRandomMapDialog::on_cities_random_toggled()
{
  cities_scale->set_sensitive(!cities_random_togglebutton->get_active());
}

std::string NewRandomMapDialog::create_and_dump_scenario(const std::string &file,
                                                         const GameParameters &g, sigc::slot<void> *pulse)
{
  CreateScenario creator (g.map.width, g.map.height);

  // then fill the other players
  int army_id = Armysetlist::getInstance()->getArmyset(g.army_theme)->getId();
  Shieldsetlist *ssl = Shieldsetlist::getInstance();
  guint32 id = ssl->getShieldset(g.shield_theme)->getId();
  for (std::vector<GameParameters::Player>::const_iterator
       i = g.players.begin(), end = g.players.end();
       i != end; ++i) {

    if (i->type == GameParameters::Player::OFF)
      {
        fl_counter->getNextId();
        continue;
      }

    Player::Type type;
    if (i->type == GameParameters::Player::EASY)
      type = Player::AI_FAST;
    else if (i->type == GameParameters::Player::HARD)
      type = Player::AI_SMART;
    else
      type = Player::HUMAN;

    creator.addPlayer(i->name, army_id, ssl->getColor(id, i->id), type);
  }


  CreateScenarioRandomize random;
  // the neutral player must come last so it has the highest id among players
  creator.addNeutral(random.getPlayerName(Shield::NEUTRAL), army_id, 
                     ssl->getColor(id, MAX_PLAYERS), Player::AI_DUMMY);

  // now fill in some map information
  creator.setMapTiles(g.tile_theme);
  creator.setShieldset(g.shield_theme);
  creator.setCityset(g.city_theme);
  creator.setNoCities(g.map.cities);
  creator.setNoRuins(g.map.ruins);
  creator.setNoTemples(g.map.temples);
  int num_signposts = g.map.signposts;
  if (num_signposts == -1)
    num_signposts = CreateScenario::calculateNumberOfSignposts(g.map.width,
                                                               g.map.height,
                                                               g.map.grass);
  creator.setNoSignposts(num_signposts);

  // terrain: the scenario generator also accepts input with a sum of
  // more than 100%, so the thing is rather easy here
  creator.setPercentages(g.map.grass, g.map.water, g.map.forest, g.map.swamp,
                         g.map.hills, g.map.mountains);

  // and tell it the turn mode
  if (g.process_armies == GameParameters::PROCESS_ARMIES_AT_PLAYERS_TURN)
    creator.setTurnmode(true);
  else
    creator.setTurnmode(false);

  // now create the map and dump the created map
  std::string path = File::getSavePath();
  path += file;

  if (pulse)
    //creator.progress.connect(sigc::mem_fun(this, &NewRandomMapDialog::pulse));
    creator.progress.connect(*pulse);
  
  creator.create(g);
  creator.dump(path);
  return path;
}

void NewRandomMapDialog::on_accept_clicked()
{
  dialog_vbox->set_sensitive(false);
  progressbar->show_all();
  progressbar->pulse();
  while (g_main_context_iteration(NULL, FALSE)); //doEvents

  switch (map_size_combobox->get_active_row_number()) {
  case MAP_SIZE_SMALL:
    map.width = MAP_SIZE_SMALL_WIDTH;
    map.height = MAP_SIZE_SMALL_HEIGHT;
    break;

  case MAP_SIZE_TINY:
    map.width = MAP_SIZE_TINY_WIDTH;
    map.height = MAP_SIZE_TINY_HEIGHT;
    break;

  case MAP_SIZE_NORMAL:
  default:
    map.width = MAP_SIZE_NORMAL_WIDTH;
    map.height = MAP_SIZE_NORMAL_HEIGHT;
    break;
  }

  map.tileset = Tilesetlist::getInstance()->getTilesetDir
    (Glib::filename_from_utf8(tile_theme_combobox->get_active_text()),
     get_active_tile_size());

  map.shieldset = Shieldsetlist::getInstance()->getShieldsetDir
    (Glib::filename_from_utf8(shield_theme_combobox->get_active_text()));

  map.cityset = Citysetlist::getInstance()->getCitysetDir
    (Glib::filename_from_utf8(city_theme_combobox->get_active_text()),
     get_active_tile_size());

  map.armyset = Armysetlist::getInstance()->getArmysetDir
    (Glib::filename_from_utf8(army_theme_combobox->get_active_text()),
     get_active_tile_size());

  map.grass = int(grass_scale->get_value());
  map.water = int(water_scale->get_value());
  map.swamp = int(swamp_scale->get_value());
  map.forest = int(forest_scale->get_value());
  map.hills = int(hills_scale->get_value());
  map.mountains = int(mountains_scale->get_value());
  map.cities = int(cities_scale->get_value());
  map.ruins = int(ruins_scale->get_value());
  map.temples = int(temples_scale->get_value());
  map.signposts = int(signposts_scale->get_value());

  GameParameters g = getParams();
  progressbar->pulse();
  while (g_main_context_iteration(NULL, FALSE)); //doEvents

    //creator.progress.connect(sigc::mem_fun(this, &NewRandomMapDialog::pulse));
  sigc::slot<void> progress = sigc::mem_fun(this, &NewRandomMapDialog::pulse);
  d_filename = create_and_dump_scenario("random.map", g, &progress);

  dialog_response = Gtk::RESPONSE_ACCEPT;
  dialog->hide();
}

void NewRandomMapDialog::on_cancel_clicked()
{
  dialog_response = Gtk::RESPONSE_CANCEL;
  dialog->hide();
}
void NewRandomMapDialog::pulse()
{
  progressbar->pulse();
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
}
    
void NewRandomMapDialog::on_grass_changed()
{
  int num_signposts = 
      CreateScenario::calculateNumberOfSignposts(map.width, map.height,
                                                 int(grass_scale->get_value()));
  signposts_scale->set_value(num_signposts);
}
