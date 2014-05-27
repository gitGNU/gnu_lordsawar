//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014 Ben Asselstine
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

#ifndef RUIN_EDITOR_DIALOG_H
#define RUIN_EDITOR_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>

class Ruin;
class Stack;
class CreateScenarioRandomize;
class Reward;

//! Scenario editor.  Edits Ruin objects.
class RuinEditorDialog: public sigc::trackable
{
 public:
    RuinEditorDialog(Gtk::Window &parent, Ruin *ruin, 
                     CreateScenarioRandomize *randomize);
    ~RuinEditorDialog();

    int run();
    
 private:
    Gtk::Dialog* dialog;
    Gtk::Entry *name_entry;
    Gtk::Entry *description_entry;
    Gtk::SpinButton *type_entry;
    Gtk::Button *keeper_button;
    Gtk::Button *randomize_name_button;
    Gtk::Button *clear_keeper_button;
    Gtk::Button *randomize_keeper_button;
    Gtk::CheckButton *hidden_button;
    Gtk::ComboBoxText *player_combobox;
    Gtk::HBox *new_reward_hbox;
    Gtk::RadioButton *new_reward_radiobutton;
    Gtk::RadioButton *random_reward_radiobutton;
    Gtk::Button *reward_button;
    Gtk::Button *clear_reward_button;
    Gtk::Button *randomize_reward_button;
    Gtk::Button *reward_list_button;
    Ruin *ruin;
    Stack *keeper;
    Reward *reward;
    CreateScenarioRandomize *d_randomizer;

    void set_keeper_name();
    void set_reward_name();

    void on_keeper_clicked();
    void on_clear_keeper_clicked();
    void on_hidden_toggled();
    void on_randomize_name_clicked();
    void on_randomize_keeper_clicked();
    void on_new_reward_toggled();
    void on_random_reward_toggled();
    void on_clear_reward_clicked();
    void on_randomize_reward_clicked();
    void on_reward_list_clicked();
    void on_reward_clicked();
};

#endif
