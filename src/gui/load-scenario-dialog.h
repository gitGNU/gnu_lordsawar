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

#ifndef LOAD_SCENARIOS_DIALOG_H
#define LOAD_SCENARIOS_DIALOG_H

#include <memory>
#include <string>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm.h>

#include "decorated.h"
// dialog for choosing a scenario
class LoadScenarioDialog: public Decorated
{
 public:
    LoadScenarioDialog();
    ~LoadScenarioDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();
    void hide();

    std::string get_scenario_filename();

 private:
    Gtk::Dialog* dialog;
    Gtk::Button *load_button;
    Gtk::TextView *description_textview;
    Gtk::Label *num_players_label;
    Gtk::Label *num_cities_label;
    Gtk::TreeView *scenarios_treeview;
    Gtk::Button *add_scenario_button;
    Gtk::Button *remove_scenario_button;

    class ScenariosColumns: public Gtk::TreeModelColumnRecord {
    public:
	ScenariosColumns() 
        { add(name); add(filename); }
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<std::string> filename;
    };
    const ScenariosColumns scenarios_columns;
    Glib::RefPtr<Gtk::ListStore> scenarios_list;
    
    std::string selected_filename;
    
    void on_selection_changed();
    void add_scenario(std::string filename);
    void on_add_scenario_clicked();
    void on_remove_scenario_clicked();
    int copy_file (Glib::ustring from, Glib::ustring to);
};

#endif
