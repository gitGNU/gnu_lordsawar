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

#ifndef RUIN_DIALOG_H
#define RUIN_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/spinbutton.h>

class Ruin;
class Stack;
class CreateScenarioRandomize;

class RuinDialog: public sigc::trackable
{
 public:
    RuinDialog(Ruin *ruin, CreateScenarioRandomize *randomize);

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::Entry *name_entry;
    Gtk::SpinButton *type_entry;
    Gtk::Button *keeper_button;
    Gtk::Button *randomize_name_button;
    Gtk::CheckButton *sage_button;
    Gtk::CheckButton *hidden_button;
    Gtk::ComboBoxText *player_combobox;
    Ruin *ruin;
    Stack *keeper;
    CreateScenarioRandomize *d_randomizer;

    void set_keeper_name();

    void on_keeper_clicked();
    void on_hidden_toggled();
    void on_randomize_name_clicked();
};

#endif
