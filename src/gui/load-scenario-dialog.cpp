//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2012, 2014 Ben Asselstine
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
#include <list>
#include <iostream>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm.h>

#include "load-scenario-dialog.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "GameScenario.h"

LoadScenarioDialog::LoadScenarioDialog(Gtk::Window &parent)
 : LwDialog(parent, "load-scenario-dialog.ui")
{
  xml->get_widget("description_textview", description_textview);
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
  scenarios_treeview->signal_row_activated().connect(sigc::hide(sigc::hide(sigc::mem_fun(this, &LoadScenarioDialog::on_scenario_activated))));
  // add the scenarios
  add_scenario("random.map");
  std::list<Glib::ustring> lm = File::scanMaps();
  for (std::list<Glib::ustring>::iterator i = lm.begin(), end = lm.end();
       i != end; ++i)
    add_scenario(File::getMapFile(*i));
  lm.clear();
  lm = File::scanUserMaps();
  for (std::list<Glib::ustring>::iterator i = lm.begin(), end = lm.end();
       i != end; ++i)
    add_scenario(File::getUserMapFile(*i));


  Gtk::TreeModel::Row row;
  row = scenarios_treeview->get_model()->children()[0];
  if(row)
    scenarios_treeview->get_selection()->select(row);
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
  if (response != Gtk::RESPONSE_ACCEPT)
    selected_filename = "";

  dialog->get_size(width, height);
}

Glib::ustring LoadScenarioDialog::get_scenario_filename() 
{
  return selected_filename;
}

void LoadScenarioDialog::add_scenario(Glib::ustring filename)
{
  Gtk::TreeIter i = scenarios_list->append();
  (*i)[scenarios_columns.filename] = filename;
  if (filename == "random.map") 
    {
      (*i)[scenarios_columns.name] = _("Random Scenario");
      return;
    }
  bool broken = false;
  Glib::ustring name = "", comment = "", id = "";
  guint32 player_count = 0, city_count = 0;
  selected_filename = Glib::ustring((*i)[scenarios_columns.filename]);
  GameScenario::loadDetails(selected_filename, broken, player_count, city_count, name, comment, id);
  if (broken == false)
    (*i)[scenarios_columns.name] = name;
}

void LoadScenarioDialog::on_selection_changed()
{
  Gtk::TreeIter i = scenarios_treeview->get_selection()->get_selected();

  if (i)
    {
      Glib::ustring filename = (*i)[scenarios_columns.filename];
      if (filename == "random.map")
	{
	  load_button->set_sensitive(true);
	  description_textview->get_buffer()->set_text(_("Play a new scenario with a random map.  You get to decide the number of players, and number of cities on the map.  You can also control the amount of the map that is covered in forest, water, swamps and mountains."));
	  num_players_label->set_markup (String::ucompose("<b>--</b>", ""));
	  num_cities_label->set_markup(String::ucompose("<b>--</b>", ""));
	  remove_scenario_button->set_sensitive(false);
	  load_button->set_sensitive(true);
	  selected_filename = filename;
	  return;
	}

      selected_filename = filename;
      bool broken = false;
      Glib::ustring name, comment, id;
      guint32 player_count = 0, city_count = 0;
      GameScenario::loadDetails(filename, broken, player_count, city_count, name, comment, id);

      if (broken == true)
	{
	  std::cerr << "Error: Could not parse " << selected_filename << std::endl;
	  load_button->set_sensitive(false);
	  return;
	}
	  
      remove_scenario_button->set_sensitive(true);
      load_button->set_sensitive(true);
      num_players_label->set_markup 
	("<b>" + String::ucompose("%1", player_count - 1) + "</b>");
      num_cities_label->set_markup 
	("<b>" + String::ucompose("%1", city_count) + "</b>");
      description_textview->get_buffer()->set_text(comment);
    }
  else
    load_button->set_sensitive(false);
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
  
  Glib::RefPtr<Gtk::FileFilter> map_filter = Gtk::FileFilter::create();
  map_filter->add_pattern("*.map");
  map_filter->set_name(_("LordsAWar Maps (*.map)"));
  load_map_filechooser->set_current_folder(Glib::get_home_dir ());
  load_map_filechooser->add_filter(map_filter);
  int res = load_map_filechooser->run();
  load_map_filechooser->hide();
  if (res == Gtk::RESPONSE_ACCEPT) 
    {
      Glib::ustring filename = load_map_filechooser->get_filename();
      Glib::ustring mapname = Glib::path_get_basename (filename);
      // copy it into our ~/.lordsawar/ dir.
      File::copy (filename, File::getUserMapFile(mapname));
      // add it to the list
      add_scenario(File::getUserMapFile(mapname));
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
      Glib::ustring filename = (*i)[scenarios_columns.filename];
      if (filename == "random.map")
        return;
      File::erase(filename);
      scenarios_list->erase(i);
      description_textview->get_buffer()->set_text("");
      num_players_label->set_text ("");
      num_cities_label->set_text ("");
    }
  return;
}
  
void LoadScenarioDialog::on_scenario_activated()
{
  load_button->activate();
}
