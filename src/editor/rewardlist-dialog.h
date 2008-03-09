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

#ifndef GUI_REWARDLIST_DIALOG_H
#define GUI_REWARDLIST_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <libglademm/xml.h>
#include <gtkmm/window.h>
#include <gtkmm/dialog.h>
#include <gtkmm/container.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <gtkmm/textview.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/table.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/tooltips.h>

#include "../rewardlist.h"

//! Scenario editor.  Manages Reward objects in the Rewardlist.
class RewardlistDialog: public sigc::trackable
{
 public:
    RewardlistDialog();
    ~RewardlistDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();

    Gtk::Dialog &get_dialog() { return *dialog.get(); }

 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Reward *d_reward; //current reward
    Gtk::TreeView *rewards_treeview;
    Gtk::Button *add_button;
    Gtk::Button *remove_button;
    Gtk::Button *edit_button;

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
    void update_rewardlist_buttons();

    //callbacks
    void on_add_clicked();
    void on_remove_clicked();
    void on_edit_clicked();
    void on_reward_selected();


};

#endif
