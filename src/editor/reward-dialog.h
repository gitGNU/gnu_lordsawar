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
class Item;
class Army;
class Ruin;
class Player;

class RewardDialog: public sigc::trackable
{
 public:
    RewardDialog(Player *player, bool hidden_ruins, Reward *r);

    void set_parent_window(Gtk::Window &parent);

    void run();

    Reward *get_reward() {return reward;}
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Player *d_player;
    Reward *reward;
    Item *item;
    Army *ally;
    Ruin *hidden_ruin;
    bool d_hidden_ruins;
    Gtk::RadioButton *gold_radiobutton;
    Gtk::RadioButton *item_radiobutton;
    Gtk::RadioButton *allies_radiobutton;
    Gtk::RadioButton *map_radiobutton;
    Gtk::RadioButton *hidden_ruin_radiobutton;
    Gtk::SpinButton *gold_spinbutton;
    Gtk::Button *randomize_gold_button;
    Gtk::Button *item_button;
    Gtk::Button *clear_item_button;
    Gtk::Button *randomize_item_button;
    Gtk::Button *ally_button;
    Gtk::Button *clear_ally_button;
    Gtk::Button *randomize_allies_button;
    Gtk::SpinButton *num_allies_spinbutton;
    Gtk::SpinButton *map_x_spinbutton;
    Gtk::SpinButton *map_y_spinbutton;
    Gtk::SpinButton *map_width_spinbutton;
    Gtk::SpinButton *map_height_spinbutton;
    Gtk::Button *randomize_map_button;
    Gtk::Button *randomize_hidden_ruin_button;
    Gtk::Button *clear_hidden_ruin_button;
    Gtk::Button *hidden_ruin_button;

    Gtk::HBox *gold_hbox;
    Gtk::HBox *item_hbox;
    Gtk::HBox *allies_hbox;
    Gtk::HBox *map_hbox;
    Gtk::HBox *hidden_ruin_hbox;
    void on_gold_toggled();
    void on_item_toggled();
    void on_allies_toggled();
    void on_map_toggled();
    void on_hidden_ruin_toggled();
    void on_randomize_gold_clicked();
    void on_item_clicked();
    void on_clear_item_clicked();
    void on_randomize_item_clicked();
    void set_item_name();
    void on_ally_clicked();
    void on_randomize_allies_clicked();
    void on_clear_ally_clicked();
    void set_ally_name();
    void on_randomize_map_clicked();
    void on_hidden_ruin_clicked();
    void on_randomize_hidden_ruin_clicked();
    void on_clear_hidden_ruin_clicked();
    void set_hidden_ruin_name();

    void fill_in_reward_info();

};

#endif
