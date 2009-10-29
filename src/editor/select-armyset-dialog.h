//  Copyright (C) 2009 Ben Asselstine
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

#ifndef SELECT_ARMYSET_DIALOG_H
#define SELECT_ARMYSET_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm.h>

class Armyset;

//! Scenario editor.  Select an Armyset object from the Armysetlist.
class SelectArmysetDialog: public sigc::trackable
{
 public:
    SelectArmysetDialog();
    ~SelectArmysetDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();

    const Armyset *get_selected_armyset() { return selected_armyset; }
    
 private:
    Gtk::Dialog* dialog;
    Gtk::Button *select_button;

    const Armyset *selected_armyset;

    Gtk::TreeView *armysets_treeview;
    class ArmysetsColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmysetsColumns() 
        { add(name); add(armyset);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Armyset*> armyset;
    };
    const ArmysetsColumns armysets_columns;
    Glib::RefPtr<Gtk::ListStore> armysets_list;


    void addArmyset(Armyset *armyset);
    
    void set_select_button_state();
};

#endif
