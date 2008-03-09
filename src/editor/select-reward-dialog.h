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

#ifndef SELECT_REWARD_DIALOG_H
#define SELECT_REWARD_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/container.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <gtkmm/textview.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>


class Reward;

//! Scenario editor.  Select a Reward from the Rewardlist.
class SelectRewardDialog: public sigc::trackable
{
 public:
    SelectRewardDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();

    const Reward *get_selected_reward() { return selected_reward; }
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::Button *select_button;

    const Reward *selected_reward;

    Gtk::TreeView *rewards_treeview;
    class RewardsColumns: public Gtk::TreeModelColumnRecord {
    public:
	RewardsColumns() 
        { add(name); add(reward);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Reward *> reward;
    };
    const RewardsColumns rewards_columns;
    Glib::RefPtr<Gtk::ListStore> rewards_list;

    void addReward(Reward *reward);
    
    void set_select_button_state();
};

#endif
