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

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>

#include "game-options-dialog.h"

#include "glade-helpers.h"
#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "Configuration.h"
#include "GameScenarioOptions.h"

GameOptionsDialog::GameOptionsDialog(bool readonly)
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/game-options-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    decorate(dialog.get());
    window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
    d->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));

    d_readonly = readonly;
    xml->get_widget("difficultoptionstable", difficultoptionstable);
    xml->get_widget("notdifficultoptionstable", notdifficultoptionstable);
    xml->get_widget("view_enemies_checkbutton", view_enemies_checkbutton);
    xml->get_widget("view_production_checkbutton", view_production_checkbutton);
    xml->get_widget("quests_checkbutton", quests_checkbutton);
    xml->get_widget("hidden_map_checkbutton", hidden_map_checkbutton);
    xml->get_widget("neutral_combobox", neutral_cities_combobox);
    xml->get_widget("razing_combobox", razing_cities_combobox);
    xml->get_widget("diplomacy_checkbutton", diplomacy_checkbutton);
    xml->get_widget("military_advisor_checkbutton", 
                    military_advisor_checkbutton);
    xml->get_widget("quick_start_checkbutton", quick_start_checkbutton);
    xml->get_widget("cusp_of_war_checkbutton", cusp_of_war_checkbutton);
    xml->get_widget("intense_combat_checkbutton", intense_combat_checkbutton);
    xml->get_widget("random_turns_checkbutton", random_turns_checkbutton);
	
}

void GameOptionsDialog::fill_in_options()
{
    neutral_cities_combobox->set_active(GameParameters::AVERAGE);
    razing_cities_combobox->set_active(GameParameters::ALWAYS);

    view_enemies_checkbutton->set_active(GameScenarioOptions::s_see_opponents_stacks);
    view_production_checkbutton->set_active(GameScenarioOptions::s_see_opponents_production);
    quests_checkbutton->set_active(GameScenarioOptions::s_play_with_quests);
    hidden_map_checkbutton->set_active(GameScenarioOptions::s_hidden_map);
    neutral_cities_combobox->set_active(int(GameScenarioOptions::s_neutral_cities));
    razing_cities_combobox->set_active(int(GameScenarioOptions::s_razing_cities));
    diplomacy_checkbutton->set_active(GameScenarioOptions::s_diplomacy);
    military_advisor_checkbutton->set_active(GameScenarioOptions::s_military_advisor);
    quick_start_checkbutton->set_active(Configuration::s_quick_start);
    cusp_of_war_checkbutton->set_active(GameScenarioOptions::s_cusp_of_war);
    cusp_of_war_checkbutton->set_sensitive(diplomacy_checkbutton->get_active());
    intense_combat_checkbutton->set_active(GameScenarioOptions::s_intense_combat);
    random_turns_checkbutton->set_active(GameScenarioOptions::s_random_turns);
    if (d_readonly)
      {
	difficultoptionstable->set_sensitive(false);
	notdifficultoptionstable->set_sensitive(false);
      }
}
void GameOptionsDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void GameOptionsDialog::hide()
{
  dialog->hide();
}

bool GameOptionsDialog::run()
{
  std::list<sigc::connection> connections;
    GameParameters g;
    fill_in_options();
    connections.push_back
      (view_enemies_checkbutton->signal_clicked().connect
       (sigc::mem_fun
	(this, &GameOptionsDialog::on_view_enemies_checkbutton_clicked)));
    connections.push_back
      (view_production_checkbutton->signal_clicked().connect
       (sigc::mem_fun
	(this, &GameOptionsDialog::on_view_production_checkbutton_clicked)));
    connections.push_back
      (quests_checkbutton->signal_clicked().connect
       (sigc::mem_fun
	(this, &GameOptionsDialog::on_quests_checkbutton_clicked)));
    connections.push_back
      (hidden_map_checkbutton->signal_clicked().connect
       (sigc::mem_fun
	(this, &GameOptionsDialog::on_hidden_map_checkbutton_clicked)));
    connections.push_back
      (neutral_cities_combobox->signal_changed().connect
       (sigc::mem_fun
	(this, &GameOptionsDialog::on_neutral_cities_combobox_changed)));
    connections.push_back
      (razing_cities_combobox->signal_changed().connect
       (sigc::mem_fun
	(this, &GameOptionsDialog::on_razing_cities_combobox_changed)));
    connections.push_back
      (diplomacy_checkbutton->signal_clicked().connect
       (sigc::mem_fun
	(this, &GameOptionsDialog::on_diplomacy_checkbutton_clicked)));
    connections.push_back
      (cusp_of_war_checkbutton->signal_clicked().connect
       (sigc::mem_fun
	(this, &GameOptionsDialog::on_cusp_of_war_checkbutton_clicked)));
    connections.push_back
      (random_turns_checkbutton->signal_clicked().connect
       (sigc::mem_fun
	(this, &GameOptionsDialog::on_random_turns_checkbutton_clicked)));
    connections.push_back
      (quick_start_checkbutton->signal_clicked().connect
       (sigc::mem_fun
	(this, &GameOptionsDialog::on_quick_start_checkbutton_clicked)));
    connections.push_back
      (intense_combat_checkbutton->signal_clicked().connect
       (sigc::mem_fun
	(this, &GameOptionsDialog::on_intense_combat_checkbutton_clicked)));
    connections.push_back
      (military_advisor_checkbutton->signal_clicked().connect
       (sigc::mem_fun
	(this, &GameOptionsDialog::on_military_advisor_checkbutton_clicked)));

    dialog->run();
  
    std::list<sigc::connection>::iterator it = connections.begin();
    for (; it != connections.end(); it++) 
      (*it).disconnect();
    connections.clear();
    
    g.see_opponents_stacks = view_enemies_checkbutton->get_active();
    GameScenarioOptions::s_see_opponents_stacks = g.see_opponents_stacks;
    g.see_opponents_production = view_production_checkbutton->get_active();
    GameScenarioOptions::s_see_opponents_production = g.see_opponents_production;
    g.play_with_quests = quests_checkbutton->get_active();
    GameScenarioOptions::s_play_with_quests = g.play_with_quests;
    g.hidden_map = hidden_map_checkbutton->get_active();
    GameScenarioOptions::s_hidden_map = g.hidden_map;

    g.neutral_cities = GameParameters::NeutralCities (
	neutral_cities_combobox->get_active_row_number());
    GameScenarioOptions::s_neutral_cities = g.neutral_cities;
    g.razing_cities = GameParameters::RazingCities (
	razing_cities_combobox->get_active_row_number());
    GameScenarioOptions::s_razing_cities = g.razing_cities;

    g.diplomacy = diplomacy_checkbutton->get_active();
    GameScenarioOptions::s_diplomacy = g.diplomacy;
    g.random_turns = random_turns_checkbutton->get_active();
    GameScenarioOptions::s_random_turns = g.random_turns;
    g.quick_start = quick_start_checkbutton->get_active();
    Configuration::s_quick_start = g.quick_start;
    g.cusp_of_war = cusp_of_war_checkbutton->get_active();
    GameScenarioOptions::s_cusp_of_war = g.cusp_of_war;
    g.intense_combat = intense_combat_checkbutton->get_active();
    GameScenarioOptions::s_intense_combat = g.intense_combat;
    g.military_advisor = military_advisor_checkbutton->get_active();
    GameScenarioOptions::s_military_advisor = g.military_advisor;
    //save it all to Configuration too
    Configuration::s_see_opponents_stacks = 
      GameScenarioOptions::s_see_opponents_stacks;
    Configuration::s_see_opponents_production = 
      GameScenarioOptions::s_see_opponents_production;
    Configuration::s_play_with_quests = GameScenarioOptions::s_play_with_quests;
    Configuration::s_hidden_map = GameScenarioOptions::s_hidden_map;
    Configuration::s_neutral_cities = GameScenarioOptions::s_neutral_cities;
    Configuration::s_razing_cities = GameScenarioOptions::s_razing_cities;
    Configuration::s_diplomacy = GameScenarioOptions::s_diplomacy;
    Configuration::s_random_turns = GameScenarioOptions::s_random_turns;
    Configuration::s_cusp_of_war = GameScenarioOptions::s_cusp_of_war;
    Configuration::s_intense_combat = GameScenarioOptions::s_intense_combat;
    Configuration::s_military_advisor = GameScenarioOptions::s_military_advisor;
    Configuration::saveConfigurationFile(Configuration::configuration_file_path);
    dialog->hide();
    return true;
}

void GameOptionsDialog::on_view_enemies_checkbutton_clicked()
{
  GameScenarioOptions::s_see_opponents_stacks = 
    view_enemies_checkbutton->get_active();
  difficulty_option_changed.emit();
}
void GameOptionsDialog::on_view_production_checkbutton_clicked()
{
  GameScenarioOptions::s_see_opponents_production = 
    view_production_checkbutton->get_active();
  difficulty_option_changed.emit();
}
void GameOptionsDialog::on_quests_checkbutton_clicked()
{
  GameScenarioOptions::s_play_with_quests = quests_checkbutton->get_active();
  difficulty_option_changed.emit();
}
void GameOptionsDialog::on_hidden_map_checkbutton_clicked()
{
  GameScenarioOptions::s_hidden_map = hidden_map_checkbutton->get_active();
  difficulty_option_changed.emit();
}
void GameOptionsDialog::on_neutral_cities_combobox_changed()
{
  GameScenarioOptions::s_neutral_cities = GameParameters::NeutralCities 
    (neutral_cities_combobox->get_active_row_number());
  difficulty_option_changed.emit();
}
void GameOptionsDialog::on_razing_cities_combobox_changed()
{
  GameScenarioOptions::s_razing_cities = GameParameters::RazingCities 
    (razing_cities_combobox->get_active_row_number());
  difficulty_option_changed.emit();
}
void GameOptionsDialog::on_diplomacy_checkbutton_clicked()
{
  if (diplomacy_checkbutton->get_active() == true)
    cusp_of_war_checkbutton->set_sensitive(true);
  else
    cusp_of_war_checkbutton->set_sensitive(false);
  GameScenarioOptions::s_diplomacy = diplomacy_checkbutton->get_active();
  difficulty_option_changed.emit();
}
void GameOptionsDialog::on_cusp_of_war_checkbutton_clicked()
{
  GameScenarioOptions::s_cusp_of_war = cusp_of_war_checkbutton->get_active();
  difficulty_option_changed.emit();
}
void GameOptionsDialog::on_random_turns_checkbutton_clicked()
{
  GameScenarioOptions::s_random_turns = random_turns_checkbutton->get_active();
}
void GameOptionsDialog::on_quick_start_checkbutton_clicked()
{
  Configuration::s_quick_start = quick_start_checkbutton->get_active();
}
void GameOptionsDialog::on_intense_combat_checkbutton_clicked()
{
  GameScenarioOptions::s_intense_combat = intense_combat_checkbutton->get_active();
}
void GameOptionsDialog::on_military_advisor_checkbutton_clicked()
{
  GameScenarioOptions::s_military_advisor = 
    military_advisor_checkbutton->get_active();
}
