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

#ifndef CITY_DIALOG_H
#define CITY_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>


class CreateScenarioRandomize;
class City;
class ArmyProdBase;

//! Scenario editor.  Edits a City object.
class CityDialog: public sigc::trackable
{
 public:
    CityDialog(City *city, CreateScenarioRandomize *randomizer);
    ~CityDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    Gtk::Dialog* dialog;
    Gtk::ComboBoxText *player_combobox;
    Gtk::CheckButton *capital_checkbutton;
    Gtk::Entry *name_entry;
    Gtk::SpinButton *income_spinbutton;
    Gtk::CheckButton *burned_checkbutton;

    Gtk::TreeView *army_treeview;
    
    class ArmyColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmyColumns()
	    { add(army); add(name);
	      add(strength); add(moves); add(upkeep); add(duration); }

	Gtk::TreeModelColumn<const ArmyProdBase *> army;
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<int> strength, moves, upkeep, duration;
    };
    const ArmyColumns army_columns;
    Glib::RefPtr<Gtk::ListStore> army_list;
    Gtk::Button *add_button;
    Gtk::Button *remove_button;
    Gtk::Button *randomize_armies_button;
    Gtk::Button *randomize_name_button;
    Gtk::Button *randomize_income_button;

    City *city;

    void on_add_clicked();
    void on_remove_clicked();
    void on_randomize_armies_clicked();
    void on_randomize_name_clicked();
    void on_randomize_income_clicked();
    void on_selection_changed();
    void on_player_changed();

    void add_army(const ArmyProdBase *a);
    void set_button_sensitivity();
    CreateScenarioRandomize *d_randomizer;
};

#endif
