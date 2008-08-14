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

#include <iostream>
#include <iomanip>
#include <assert.h>
#include <libgen.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>

#include <gtkmm/widget.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/image.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>

#include "rewardlist-dialog.h"

#include "gui/input-helpers.h"
#include "gui/error-utils.h"

#include "defs.h"
#include "Configuration.h"
#include "rewardlist.h"

#include "ucompose.hpp"
#include "playerlist.h"

#include "glade-helpers.h"
#include "reward-dialog.h"


RewardlistDialog::RewardlistDialog()
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + 
				    "/reward-list-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("rewards_treeview", rewards_treeview);
    xml->get_widget("add_button", add_button);
    add_button->signal_clicked().connect
      (sigc::mem_fun(this, &RewardlistDialog::on_add_clicked));
    xml->get_widget("remove_button", remove_button);
    remove_button->signal_clicked().connect
      (sigc::mem_fun(this, &RewardlistDialog::on_remove_clicked));
    xml->get_widget("edit_button", edit_button);
    edit_button->signal_clicked().connect
      (sigc::mem_fun(this, &RewardlistDialog::on_edit_clicked));

    rewards_list = Gtk::ListStore::create(rewards_columns);
    rewards_treeview->set_model(rewards_list);
    rewards_treeview->append_column("", rewards_columns.name);
    rewards_treeview->set_headers_visible(false);

    Rewardlist *rewardlist = Rewardlist::getInstance();
    Rewardlist::iterator iter = rewardlist->begin();
    for (;iter != rewardlist->end(); iter++)
      addReward(*iter);
      
    Uint32 max = rewardlist->size();
    if (max)
      {
	Gtk::TreeModel::Row row;
	row = rewards_treeview->get_model()->children()[0];
	if(row)
	  rewards_treeview->get_selection()->select(row);
      }


    update_rewardlist_buttons();
    rewards_treeview->get_selection()->signal_changed().connect
      (sigc::mem_fun(*this, &RewardlistDialog::on_reward_selected));
}

void
RewardlistDialog::update_rewardlist_buttons()
{
  if (!rewards_treeview->get_selection()->get_selected())
    {
      remove_button->set_sensitive(false);
      edit_button->set_sensitive(false);
    }
  else
    {
      remove_button->set_sensitive(true);
      edit_button->set_sensitive(true);
    }
}

RewardlistDialog::~RewardlistDialog()
{
}

void RewardlistDialog::addReward(Reward *reward)
{
  Gtk::TreeIter i = rewards_list->append();
  (*i)[rewards_columns.name] = reward->getName();
  (*i)[rewards_columns.reward] = reward;
}

void RewardlistDialog::on_reward_selected()
{
  update_rewardlist_buttons();
}

void RewardlistDialog::on_add_clicked()
{
  Player *neutral = Playerlist::getInstance()->getNeutral();
  RewardDialog d(neutral, true, NULL);
  d.run();
  if (d.get_reward())
    {
      Reward *reward = d.get_reward();
      Gtk::TreeIter i = rewards_list->append();
      (*i)[rewards_columns.name] = reward->getName();
      (*i)[rewards_columns.reward] = reward;
      Rewardlist::getInstance()->push_back(reward);
    }

}

void RewardlistDialog::on_remove_clicked()
{
  //erase the selected row from the treeview
  //remove the reward from the rewardlist
  Glib::RefPtr<Gtk::TreeSelection> selection = rewards_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Reward *a = row[rewards_columns.reward];
      rewards_list->erase(iterrow);
      Rewardlist::getInstance()->remove(a);
    }
}

void RewardlistDialog::on_edit_clicked()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = 
    rewards_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Reward *reward = row[rewards_columns.reward];
      Player *neutral = Playerlist::getInstance()->getNeutral();
      RewardDialog d(neutral, true, reward);
      d.run();
      if (d.get_reward())
	{
	  Rewardlist::getInstance()->remove(reward);
	  reward = d.get_reward();
	  (*iterrow)[rewards_columns.name] = reward->getName();
	  (*iterrow)[rewards_columns.reward] = reward;
	  Rewardlist::getInstance()->push_back(reward);
	}
      else
	{
	  rewards_list->erase(iterrow);
	  Rewardlist::getInstance()->remove(reward);
	}
    }

}

void RewardlistDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void RewardlistDialog::run()
{
    dialog->show_all();
    int response = dialog->run();
}

