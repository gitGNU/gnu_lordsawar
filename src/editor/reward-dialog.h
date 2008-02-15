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
    Gtk::RadioButton *gold_radiobutton;
    Gtk::RadioButton *item_radiobutton;
    Gtk::RadioButton *allies_radiobutton;
    Gtk::RadioButton *map_radiobutton;
    Gtk::HBox *gold_hbox;
    Gtk::HBox *item_hbox;
    Gtk::HBox *allies_hbox;
    Gtk::HBox *map_hbox;
    void on_gold_toggled();
    void on_item_toggled();
    void on_allies_toggled();
    void on_map_toggled();
    void on_random_toggled();

};

#endif
