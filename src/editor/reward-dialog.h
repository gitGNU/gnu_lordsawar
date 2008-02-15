//  Copyright (C) 2008, Ben Asselstine
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

#ifndef REWARD_DIALOG_H
#define REWARD_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/radiobutton.h>

class Reward;

class RewardDialog: public sigc::trackable
{
 public:
    RewardDialog(Reward *reward);

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Reward *reward;
    Gtk::RadioButton *reward_gold_radiobutton;
    Gtk::RadioButton *reward_item_radiobutton;
    Gtk::RadioButton *reward_allies_radiobutton;
    Gtk::RadioButton *reward_map_radiobutton;
    Gtk::RadioButton *reward_random_radiobutton;
    Gtk::HBox *reward_gold_hbox;
    Gtk::HBox *reward_item_hbox;
    Gtk::HBox *reward_allies_hbox;
    Gtk::HBox *reward_map_hbox;
    void on_reward_gold_toggled();
    void on_reward_item_toggled();
    void on_reward_allies_toggled();
    void on_reward_map_toggled();
    void on_reward_random_toggled();
};

#endif
