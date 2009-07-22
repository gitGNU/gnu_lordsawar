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

#ifndef STACK_DIALOG_H
#define STACK_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>


class Stack;
class Army;

//! Scenario editor.  Change the contents of a Stack.
class StackDialog: public sigc::trackable
{
 public:
    StackDialog(Stack *stack, int min_size = 1);

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::ComboBoxText *player_combobox;

    Gtk::TreeView *army_treeview;
    
    class ArmyColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmyColumns()
	    { add(army); add(name);
	      add(strength); add(moves); add(hitpoints); add(upkeep); }

	Gtk::TreeModelColumn<Army *> army;
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<int> strength, moves, hitpoints, upkeep;
    };
    const ArmyColumns army_columns;
    Glib::RefPtr<Gtk::ListStore> army_list;
    Gtk::Button *add_button;
    Gtk::Button *remove_button;

    Stack *stack;
    int min_size;

    void on_add_clicked();
    void on_remove_clicked();
    void on_selection_changed();

    void add_army(Army *a);
    void set_button_sensitivity();
};

#endif
