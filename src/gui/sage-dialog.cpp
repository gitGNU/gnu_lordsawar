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

#include <config.h>

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "SightMap.h"
#include "sage-dialog.h"

#include "defs.h"
#include "snd.h"
#include "ruin.h"
#include "rewardlist.h"
#include "playerlist.h"
#include "Item.h"

SageDialog::SageDialog(Gtk::Window &parent, Sage *sage, Hero *h, Ruin *r)
 : LwDialog(parent, "sage-dialog.ui")
{
    ruin = r;
    hero = h;
    
    rewards_list = Gtk::ListStore::create(rewards_columns);
    xml->get_widget("rewardtreeview", rewards_treeview);
    rewards_treeview->set_model(rewards_list);
    rewards_treeview->append_column("", rewards_columns.name);
    rewards_treeview->get_selection()->signal_changed().connect
      (sigc::mem_fun(*this, &SageDialog::on_reward_selected));

    xml->get_widget("map_image", map_image);
    xml->get_widget("continue_button", continue_button);

    ruinmap = new RuinMap(ruin);
    ruinmap->map_changed.connect(
	sigc::mem_fun(this, &SageDialog::on_map_changed));

    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);

    dialog->set_title(_("A Sage!"));

    for(Sage::iterator it = sage->begin(); it != sage->end(); it++)
      addReward(*it);

  continue_button->set_sensitive(false);

}

SageDialog::~SageDialog()
{
  delete ruinmap;
}

Reward *SageDialog::grabSelectedReward()
{
    Glib::RefPtr<Gtk::TreeView::Selection> sel;
    sel = rewards_treeview->get_selection();
    Gtk::TreeModel::iterator it = sel->get_selected();
    Gtk::TreeModel::Row row = *it;
    return row[rewards_columns.reward];
}

void SageDialog::hide()
{
}

Reward *SageDialog::run()
{
    ruinmap->resize();
    ruinmap->draw(Playerlist::getActiveplayer());

    Snd::getInstance()->play("hero", 1);
    dialog->show_all();
    dialog->run();
    Snd::getInstance()->halt();
    //okay, we have a reward selected
   //now we return it (somehow)
  
    Reward *reward = grabSelectedReward();
    //is this in our one-time list anywhere?

    Rewardlist *rlist = Rewardlist::getInstance();
    Rewardlist::iterator it = 
      std::find (rlist->begin(), rlist->end(), reward);
    if (it != rlist->end())
      {
	//yes, it's something on our one-time reward list!
	//take if off our list, so we can't award it again
	rlist->erase(it);
      }

    // fixme: remove all common rewards that isn't the one we selected
    // the contents of the common rewards are a memory leak

    return reward;
}

void SageDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, ruinmap->get_width(), ruinmap->get_height());
  map_image->property_pixbuf() = pixbuf;
}

void SageDialog::addReward(Reward *reward)
{
  Gtk::TreeIter i = rewards_list->append();
  switch (reward->getType())
    {
    case Reward::GOLD:
      (*i)[rewards_columns.name] = _("Gold");
      break;
    case Reward::ITEM:
    case Reward::ALLIES:
      break;
    case Reward::MAP:
	{
	  Glib::ustring name;
	  Reward_Map *m = static_cast<Reward_Map*>(reward);
	  name  = m->getName();
	  if (name == "")
	    {
	      switch (rand() % 6)
		{
		case 0: name = _("parchment map"); break;
		case 1: name = _("vellum map"); break;
		case 2: name = _("paper map"); break;
		case 3: name = _("torn paper map"); break;
		case 4: name = _("dusty map"); break;
		case 5: name = _("blood-stained map"); break;
		}
	    }
	    
	  (*i)[rewards_columns.name] = name;
	}

      break;
    case Reward::RUIN:
	{
	  Ruin *r = static_cast<Reward_Ruin*>(reward)->getRuin();
	  if (r->getReward() == NULL)
	    {
	      Reward *rew  = NULL;
	      if (rand() % 2 == 0)
		{
		  rew = Rewardlist::getInstance()->popRandomItemReward();
		  if (!rew)
		    rew = Rewardlist::getInstance()->popRandomMapReward();
		}
	      else
		{
		  rew = Rewardlist::getInstance()->popRandomMapReward();
		  if (!rew)
		    rew = Rewardlist::getInstance()->popRandomItemReward();
		}
	      if (!rew)
		r->populateWithRandomReward();
	      else
		r->setReward(rew);
	    }
	  if (r->getReward()->getType() == Reward::ITEM)
	    (*i)[rewards_columns.name] = 
	      static_cast<Reward_Item*>(r->getReward())->getItem()->getName();
	  else if (r->getReward()->getType() == Reward::ALLIES)
	    (*i)[rewards_columns.name] = _("Allies");
	  else if (r->getReward()->getType() == Reward::MAP)
	    (*i)[rewards_columns.name] = r->getReward()->getName();
	}
      break;
    }
  (*i)[rewards_columns.reward] = reward;
}

void SageDialog::on_reward_selected()
{
  continue_button->set_sensitive(true);
}

