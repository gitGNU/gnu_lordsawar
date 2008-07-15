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

#include <list>

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>

#include "load-scenario-dialog.h"

#include "glade-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../Configuration.h"
#include "../File.h"
#include "../xmlhelper.h"


LoadScenarioDialog::LoadScenarioDialog()
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/load-scenario-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("name_label", name_label);
    xml->get_widget("description_label", description_label);
    xml->get_widget("load_button", load_button);
    xml->get_widget("num_players_label", num_players_label);
    xml->get_widget("num_cities_label", num_cities_label);

    scenarios_list = Gtk::ListStore::create(scenarios_columns);
    xml->get_widget("treeview", scenarios_treeview);
    scenarios_treeview->set_model(scenarios_list);
    scenarios_treeview->append_column("", scenarios_columns.name);

    scenarios_treeview->get_selection()->signal_changed()
	.connect(sigc::mem_fun(this, &LoadScenarioDialog::on_selection_changed));
    // add the scenarios
    std::list<std::string> lm = File::scanMaps();
    for (std::list<std::string>::iterator i = lm.begin(), end = lm.end();
	i != end; ++i)
	add_scenario(*i);

    load_button->set_sensitive(false);
}

void LoadScenarioDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void LoadScenarioDialog::hide()
{
  dialog->hide();
}

void LoadScenarioDialog::run()
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    int response = dialog->run();
    if (response != 1)
	selected_filename = "";
    
    dialog->get_size(width, height);
}

std::string LoadScenarioDialog::get_scenario_filename() 
{
    return selected_filename;
}

void LoadScenarioDialog::add_scenario(std::string filename)
{
    Gtk::TreeIter i = scenarios_list->append();
    (*i)[scenarios_columns.filename] = filename;
    selected_filename = Configuration::s_dataPath + "/map/"
      + std::string((*i)[scenarios_columns.filename]);

    XML_Helper helper(selected_filename, std::ios::in, 
		      Configuration::s_zipfiles);

    helper.registerTag ("scenario", sigc::mem_fun
			(this, &LoadScenarioDialog::scan_scenario_name));

    if (!helper.parse())
      {
	std::cerr << "Error: Could not parse " << selected_filename << std::endl;
	(*i)[scenarios_columns.name] = filename;
	    
	return;
      }
    else
	(*i)[scenarios_columns.name] = loaded_scenario_name;

	
    helper.close();
}


void LoadScenarioDialog::on_selection_changed()
{
    Gtk::TreeIter i = scenarios_treeview->get_selection()->get_selected();

    if (i)
    {
	selected_filename = Configuration::s_dataPath + "/map/"
	    + std::string((*i)[scenarios_columns.filename]);

	XML_Helper helper(selected_filename, std::ios::in, Configuration::s_zipfiles);

	loaded_scenario_player_count = 0;
	loaded_scenario_city_count = 0;
	helper.registerTag
	  ("scenario", sigc::mem_fun
	   (this, &LoadScenarioDialog::scan_scenario_details));
	helper.registerTag
	  ("player", sigc::mem_fun
	   (this, &LoadScenarioDialog::scan_scenario_details));
	helper.registerTag
	  ("city", sigc::mem_fun
	   (this, &LoadScenarioDialog::scan_scenario_details));

	if (!helper.parse())
	{
	    std::cerr << "Error: Could not parse " << selected_filename << std::endl;
	    load_button->set_sensitive(false);
	    return;
	}
	
	helper.close();
	{
	    load_button->set_sensitive(true);
	  num_players_label->set_text
	    (String::ucompose("%1", loaded_scenario_player_count - 1));
	  num_cities_label->set_text
	    (String::ucompose("%1", loaded_scenario_city_count));
	}
    }
    else
	load_button->set_sensitive(false);
}

bool LoadScenarioDialog::scan_scenario_details(std::string tag, 
					       XML_Helper* helper)
{
    if (tag == "scenario")
    {
        if (helper->getVersion() != LORDSAWAR_SAVEGAME_VERSION)
        {
            std::cerr << "scenario has wrong version, we want "
		      << LORDSAWAR_SAVEGAME_VERSION <<",\n"
		      << "scenario offers " << helper->getVersion() <<".\n";
            return false;
        }

	std::string name, comment;
        helper->getData(name, "name");
        helper->getData(comment, "comment");

	name_label->set_text(name);
	description_label->set_text(comment);
    }
    else if (tag == "player")
    {
      loaded_scenario_player_count++;
    }
    else if (tag == "city")
    {
      loaded_scenario_city_count++;
    }

    return true;
}

bool LoadScenarioDialog::scan_scenario_name(std::string tag, XML_Helper* helper)
{
    if (tag == "scenario")
    {
        if (helper->getVersion() != LORDSAWAR_SAVEGAME_VERSION)
        {
            std::cerr << "scenario has wrong version, we want "
		      << LORDSAWAR_SAVEGAME_VERSION <<",\n"
		      << "scenario offers " << helper->getVersion() <<".\n";
            return false;
        }

	std::string name;
        helper->getData(name, "name");

	loaded_scenario_name = name;
    }

    return true;
}

