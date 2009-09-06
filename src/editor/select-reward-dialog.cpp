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

#include <config.h>

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>
#include <assert.h>

#include "select-reward-dialog.h"

#include "glade-helpers.h"
#include "gui/input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "reward.h"
#include "rewardlist.h"

SelectRewardDialog::SelectRewardDialog()
{
    selected_reward = 0;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/select-reward-dialog.ui");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    
    xml->get_widget("select_button", select_button);

    xml->get_widget("rewards_treeview", rewards_treeview);
    rewards_list = Gtk::ListStore::create(rewards_columns);
    rewards_treeview->set_model(rewards_list);
    rewards_treeview->append_column("", rewards_columns.name);
    rewards_treeview->set_headers_visible(false);

    Rewardlist *rewardlist = Rewardlist::getInstance();
    Rewardlist::iterator iter = rewardlist->begin();
    for (;iter != rewardlist->end(); iter++)
      addReward(*iter);
      
    guint32 max = rewardlist->size();
    if (max)
      {
	Gtk::TreeModel::Row row;
	row = rewards_treeview->get_model()->children()[0];
	if(row)
	  rewards_treeview->get_selection()->select(row);
      }
    else
      select_button->set_sensitive(false);
}

void SelectRewardDialog::addReward(Reward *reward)
{
  Gtk::TreeIter i = rewards_list->append();
  (*i)[rewards_columns.name] = reward->getName();
  (*i)[rewards_columns.reward] = reward;
}

void SelectRewardDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void SelectRewardDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response != Gtk::RESPONSE_ACCEPT)
	selected_reward = 0;
    else
      {
	Glib::RefPtr<Gtk::TreeSelection> selection = 
	  rewards_treeview->get_selection();
	Gtk::TreeModel::iterator iterrow = selection->get_selected();

	if (iterrow) 
	  {
	    Gtk::TreeModel::Row row = *iterrow;
	    selected_reward = row[rewards_columns.reward];
	  }
      }
}

