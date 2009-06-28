//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include <fstream>
#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm/filechooserdialog.h>

#include "load-scenario-dialog.h"

#include "glade-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "Configuration.h"
#include "File.h"
#include "xmlhelper.h"
#include <gtkmm/stock.h>


LoadScenarioDialog::LoadScenarioDialog()
{
  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path()
				+ "/load-scenario-dialog.glade");

  Gtk::Dialog *d = 0;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  decorate(dialog.get());
  window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));

  xml->get_widget("name_label", name_label);
  xml->get_widget("description_label", description_label);
  xml->get_widget("load_button", load_button);
  xml->get_widget("num_players_label", num_players_label);
  xml->get_widget("num_cities_label", num_cities_label);

  scenarios_list = Gtk::ListStore::create(scenarios_columns);
  xml->get_widget("treeview", scenarios_treeview);
  scenarios_treeview->set_model(scenarios_list);
  scenarios_treeview->append_column("", scenarios_columns.name);

  xml->get_widget("add_scenario_button", add_scenario_button);
  add_scenario_button->signal_clicked().connect
    (sigc::mem_fun(*this, &LoadScenarioDialog::on_add_scenario_clicked));
  xml->get_widget("remove_scenario_button", remove_scenario_button);
  remove_scenario_button->signal_clicked().connect
    (sigc::mem_fun(*this, &LoadScenarioDialog::on_remove_scenario_clicked));

  scenarios_treeview->get_selection()->signal_changed()
    .connect(sigc::mem_fun(this, &LoadScenarioDialog::on_selection_changed));
  // add the scenarios
  add_scenario("random.map");
  std::list<std::string> lm = File::scanMaps();
  for (std::list<std::string>::iterator i = lm.begin(), end = lm.end();
       i != end; ++i)
    add_scenario(Configuration::s_dataPath + "/" + MAPDIR + "/" + *i);
  lm.clear();
  lm = File::scanUserMaps();
  for (std::list<std::string>::iterator i = lm.begin(), end = lm.end();
       i != end; ++i)
    add_scenario(Configuration::s_savePath + "/" + *i);


  Gtk::TreeModel::Row row;
  row = scenarios_treeview->get_model()->children()[0];
  if(row)
    scenarios_treeview->get_selection()->select(row);
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
  if (filename == "random.map") 
    {
      (*i)[scenarios_columns.name] = _("Random Scenario");
      return;
    }
  selected_filename = std::string((*i)[scenarios_columns.filename]);

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
      std::string filename = (*i)[scenarios_columns.filename];
      if (filename == "random.map")
	{
	  load_button->set_sensitive(true);
	  name_label->set_text(_("Random Scenario"));
	  description_label->set_text(_("Play a new scenario with a random map."));
	  num_players_label->set_text (_("?"));
	  num_cities_label->set_text (_("?"));
	  remove_scenario_button->set_sensitive(true);
	  load_button->set_sensitive(true);
	  selected_filename = filename;
	  return;
	}

      selected_filename = filename;

      XML_Helper helper(selected_filename, std::ios::in, 
			Configuration::s_zipfiles);

      loaded_scenario_player_count = 0;
      loaded_scenario_city_count = 0;
      helper.registerTag ("scenario", sigc::mem_fun
			  (this, &LoadScenarioDialog::scan_scenario_details));
      helper.registerTag ("player", sigc::mem_fun
			  (this, &LoadScenarioDialog::scan_scenario_details));
      helper.registerTag ("city", sigc::mem_fun
			  (this, &LoadScenarioDialog::scan_scenario_details));

      if (!helper.parse())
	{
	  std::cerr << "Error: Could not parse " << selected_filename << std::endl;
	  load_button->set_sensitive(false);
	  return;
	}

      helper.close();
	{
	  remove_scenario_button->set_sensitive(true);
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

void LoadScenarioDialog::on_add_scenario_clicked() 
{
  // go get a .map file from somewhere.
  Gtk::FileChooserDialog *load_map_filechooser = new 
    Gtk::FileChooserDialog(_("Select a scenario file to add to the library"), 
			   Gtk::FILE_CHOOSER_ACTION_OPEN);
  load_map_filechooser->add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  load_map_filechooser->add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  load_map_filechooser->set_default_response(Gtk::RESPONSE_ACCEPT);
  
  Gtk::FileFilter map_filter;
  map_filter.add_pattern("*.map");
  map_filter.set_name(_("LordsAWar map files (*.map)"));
  load_map_filechooser->set_current_folder(Glib::get_home_dir ());
  load_map_filechooser->set_filter(map_filter);
  int res = load_map_filechooser->run();
  load_map_filechooser->hide();
  if (res == Gtk::RESPONSE_ACCEPT) 
    {
      std::string filename = load_map_filechooser->get_filename();
      std::string mapname = Glib::path_get_basename (filename);
      // copy it into our ~/.lordsawar/ dir.
      copy_file(filename, Configuration::s_savePath + "/" + mapname);
      // add it to the list
      add_scenario(Configuration::s_savePath + "/" + mapname);
    }
  delete load_map_filechooser;
}

void LoadScenarioDialog::on_remove_scenario_clicked() 
{
  //remove THIS scenario.
  //only highlight this button when we have something selected.

  //erase the selected row from the treeview
  //remove the scenario from the list of scenarios 
  //delete the file, if we can.
  Gtk::TreeIter i = scenarios_treeview->get_selection()->get_selected();
  if (i)
    {
      std::string filename = (*i)[scenarios_columns.filename];
      if (filename == "random.map")
	return;
      if (remove (filename.c_str()) == 0)
	{
	  scenarios_list->erase(i);
	  name_label->set_text("");
	  description_label->set_text("");
	  num_players_label->set_text ("");
	  num_cities_label->set_text ("");
	}
    }
  return;
}

//copy_file taken from ardour-2.0rc2, gplv2.
int LoadScenarioDialog::copy_file (Glib::ustring from, Glib::ustring to)
{
  std::ifstream in (from.c_str());
  std::ofstream out (to.c_str());

  if (!in)
    return -1;

  if (!out)
    return -1;

  out << in.rdbuf();

  if (!in || !out) 
    {
      unlink (to.c_str());
      return -1;
    }

  return 0;
}

