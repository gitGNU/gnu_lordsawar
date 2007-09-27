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

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>

#include "game-options-dialog.h"

#include "glade-helpers.h"
#include "input-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../File.h"
#include "../Configuration.h"

GameOptionsDialog::GameOptionsDialog()
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/game-options-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("view_enemies_checkbutton", view_enemies_checkbutton);
    xml->get_widget("view_production_checkbutton", view_production_checkbutton);
    xml->get_widget("quests_checkbutton", quests_checkbutton);
    xml->get_widget("hidden_map_checkbutton", hidden_map_checkbutton);
    xml->get_widget("neutral_combobox", neutral_cities_combobox);
    xml->get_widget("diplomacy_checkbutton", diplomacy_checkbutton);
    xml->get_widget("military_advisor_checkbutton", 
                    military_advisor_checkbutton);
    xml->get_widget("quick_start_checkbutton", quick_start_checkbutton);
    xml->get_widget("intense_combat_checkbutton", intense_combat_checkbutton);
    xml->get_widget("random_turns_checkbutton", random_turns_checkbutton);

    neutral_cities_combobox->set_active(GameParameters::AVERAGE);

    view_enemies_checkbutton->set_active(Configuration::s_see_opponents_stacks);
    view_production_checkbutton->set_active(Configuration::s_see_opponents_production);
    quests_checkbutton->set_active(Configuration::s_play_with_quests);
    hidden_map_checkbutton->set_active(Configuration::s_hidden_map);
    neutral_cities_combobox->set_active(int(Configuration::s_neutral_cities));
    diplomacy_checkbutton->set_active(Configuration::s_diplomacy);
    military_advisor_checkbutton->set_active(Configuration::s_military_advisor);
    quick_start_checkbutton->set_active(Configuration::s_quick_start);
    intense_combat_checkbutton->set_active(Configuration::s_intense_combat);
    random_turns_checkbutton->set_active(Configuration::s_random_turns);
}

void GameOptionsDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

bool GameOptionsDialog::run()
{
    GameParameters g;
    dialog->run();
    
    g.see_opponents_stacks = view_enemies_checkbutton->get_active();
    Configuration::s_see_opponents_stacks = g.see_opponents_stacks;
    g.see_opponents_production = view_production_checkbutton->get_active();
    Configuration::s_see_opponents_production = g.see_opponents_production;
    g.play_with_quests = quests_checkbutton->get_active();
    Configuration::s_play_with_quests = g.play_with_quests;
    g.hidden_map = hidden_map_checkbutton->get_active();
    Configuration::s_hidden_map = g.hidden_map;

    g.neutral_cities = GameParameters::NeutralCities (
	neutral_cities_combobox->get_active_row_number());
    Configuration::s_neutral_cities = g.neutral_cities;

    g.diplomacy = diplomacy_checkbutton->get_active();
    Configuration::s_diplomacy = g.diplomacy;
    g.random_turns = random_turns_checkbutton->get_active();
    Configuration::s_random_turns = g.random_turns;
    g.quick_start = quick_start_checkbutton->get_active();
    Configuration::s_quick_start = g.quick_start;
    g.intense_combat = intense_combat_checkbutton->get_active();
    Configuration::s_intense_combat = g.intense_combat;
    g.military_advisor = military_advisor_checkbutton->get_active();
    Configuration::s_military_advisor = g.military_advisor;
    //save it all to Configuration
    Configuration::saveConfigurationFile(Configuration::configuration_file_path);
    return true;
}
