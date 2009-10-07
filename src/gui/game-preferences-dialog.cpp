//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include "game-preferences-dialog.h"

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
void GamePreferencesDialog::init(std::string filename)
{
  d_filename = filename;
  bool broken;
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/game-preferences-dialog.ui");

    xml->get_widget("dialog", dialog);
    decorate(dialog);
    window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));

    xml->get_widget("start_game_button", start_game_button);
    xml->get_widget("difficulty_label", difficulty_label);
    xml->get_widget("difficulty_combobox", difficulty_combobox);

    xml->get_widget("players_vbox", players_vbox);
    xml->get_widget("game_name_label", game_name_label);
    xml->get_widget("game_name_entry", game_name_entry);

    difficulty_combobox->set_active(CUSTOM);
    difficulty_combobox->signal_changed().connect(
	sigc::mem_fun(*this, &GamePreferencesDialog::on_difficulty_changed));

    start_game_button->signal_clicked().connect
      (sigc::mem_fun(*this, &GamePreferencesDialog::on_start_game_clicked));

    xml->get_widget("edit_options_button", edit_options_button);
    edit_options_button->signal_clicked().connect(
	sigc::mem_fun(*this, &GamePreferencesDialog::on_edit_options_clicked));

  game_options_dialog = new GameOptionsDialog(false);
  game_options_dialog->difficulty_option_changed.connect(
	sigc::mem_fun(*this, 
		      &GamePreferencesDialog::update_difficulty_rating));
  GameParameters load_map_parameters;
  load_map_parameters = GameScenario::loadGameParameters(d_filename,
							 broken);
  if (broken)
    start_game_button->set_sensitive(false);

  d_shieldset = load_map_parameters.shield_theme;
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    add_player(GameParameters::Player::EASY, "");

  //disable all names, and types
  std::list<Gtk::ComboBoxText*>::iterator c = player_types.begin();
  std::list<Gtk::Entry *>::iterator e = player_names.begin();
  for (; c != player_types.end(); c++, e++)
    {
      (*c)->set_sensitive(true);
      (*c)->set_active(GameParameters::Player::OFF);
      (*c)->set_sensitive(false);
      (*e)->set_sensitive(false);
    }
  //parse load map parameters.

  int f = 0;
  int b;
  for (std::vector<GameParameters::Player>::const_iterator
       i = load_map_parameters.players.begin(), 
       end = load_map_parameters.players.end(); i != end; ++i, ++f) 
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
  return;
}

GamePreferencesDialog::GamePreferencesDialog(std::string filename, GameScenario::PlayMode play_mode)
{
  mode = play_mode;
  init(filename);
  if (mode != GameScenario::NETWORKED)
    {
      delete game_name_label;
      delete game_name_entry;
    }

}

GamePreferencesDialog::~GamePreferencesDialog()
{
  delete game_options_dialog;
  delete dialog;
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
  if (response == Gtk::RESPONSE_ACCEPT)
    return true;
  return false;
}

Glib::RefPtr<Gdk::Pixbuf> GamePreferencesDialog::getShieldPic(guint32 type, guint32 owner)
{
  Shieldsetlist *sl = Shieldsetlist::getInstance();

  ShieldStyle *sh= sl->getShield(d_shieldset, type, owner);
  return GraphicsCache::applyMask(sh->getImage(), sh->getMask(), 
				  sl->getMaskColorShifts(d_shieldset, owner), false)->to_pixbuf();
}

void GamePreferencesDialog::add_player(GameParameters::Player::Type type,
				       const Glib::ustring &name)
{
  //okay, add a new hbox, with a combo and an entry in it
  //add it to players_vbox
  Gtk::HBox *player_hbox = new Gtk::HBox();
  Gtk::ComboBoxText *player_type = new Gtk::ComboBoxText();
  player_type->append_text(HUMAN_PLAYER_TYPE);
  player_type->append_text(EASY_PLAYER_TYPE);
  player_type->append_text(HARD_PLAYER_TYPE);
  player_type->append_text(NO_PLAYER_TYPE);
  player_type->signal_changed().connect
    (sigc::mem_fun(this, &GamePreferencesDialog::on_player_type_changed));
  Gtk::Entry *player_name = new Gtk::Entry();
  player_name->set_text(name);

  if (type == GameParameters::Player::HUMAN)
    player_type->set_active(0);
  else if (type == GameParameters::Player::EASY)
    player_type->set_active(1);
  else if (type == GameParameters::Player::HARD)
    player_type->set_active(2);
  else if (type== GameParameters::Player::OFF)
    player_type->set_active(3);

  player_types.push_back(player_type);
  player_names.push_back(player_name);
  player_hbox->pack_start(*manage(player_name), Gtk::PACK_SHRINK, 10);
  player_hbox->add(*manage(player_type));
  players_vbox->add(*manage(player_hbox));
}

void GamePreferencesDialog::on_edit_options_clicked()
{
  inhibit_difficulty_combobox = true;
  game_options_dialog->set_parent_window(*dialog);
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
  GraphicsLoader::instantiateImages(Shieldsetlist::getInstance());

  std::vector<Gtk::Widget*> list;
  list = players_vbox->get_children();
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      Gtk::Image *player_shield = new Gtk::Image ();
      player_shield->property_pixbuf() = getShieldPic(2, i);
      player_shields.push_back(player_shield);
      Gtk::HBox *player_hbox = static_cast<Gtk::HBox*>(list[i+1]);
      player_hbox->pack_start(*manage(player_shield), Gtk::PACK_SHRINK, 10);
      player_hbox->reorder_child(*player_shield, 0);
      player_hbox->show_all();
    }
  players_vbox->show_all();

}

void GamePreferencesDialog::on_player_type_changed()
{
  guint32 offcount = 0;
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
  // read out the values in the widgets
  GameParameters g;

  g.map_path = d_filename;

  int id = 0;
  std::list<Gtk::ComboBoxText*>::iterator c = player_types.begin();
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

  g.difficulty = GameScenario::calculate_difficulty_rating(g);

  if (mode == GameScenario::NETWORKED)
    g.name = game_name_entry->get_text();

  // and call callback
  dialog->hide();
  game_started(g);
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
      GameScenarioOptions::s_neutral_cities = GameParameters::DEFENSIVE;
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
	  std::list<Gtk::ComboBoxText*>::iterator c = player_types.begin();
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

void GamePreferencesDialog::set_title(std::string text)
{
  Decorated::set_title(text);
}

