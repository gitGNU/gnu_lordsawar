//  Copyright (C) 2007, 2008, 2009, 2012 Ben Asselstine
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

#include "quest-completed-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "ucompose.hpp"
#include "hero.h"
#include "defs.h"
#include "ruin.h"
#include "GameMap.h"

QuestCompletedDialog::QuestCompletedDialog(Quest *q, Reward *r)
{
  reward = r;
  quest = q;
    
  Glib::RefPtr<Gtk::Builder> xml
      = Gtk::Builder::create_from_file(get_glade_path()
				  + "/quest-assigned-dialog.ui");

    xml->get_widget("dialog", dialog);
    decorate(dialog);
    window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));

    xml->get_widget("map_image", map_image);

    questmap = new QuestMap(quest);
    questmap->map_changed.connect(
	sigc::mem_fun(this, &QuestCompletedDialog::on_map_changed));

    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);

    set_title(String::ucompose(_("Quest for %1"), 
			       quest->getHero()->getName()));

    xml->get_widget("label", label);
    Glib::ustring s;
    s += String::ucompose(_("%1 completed the quest!"),
			  quest->getHero()->getName());
    s += "\n\n";
    // add messages from the quest
    std::queue<std::string> msgs;
    quest->getSuccessMsg(msgs);
    while (!msgs.empty())
    {
        s += msgs.front();
        s += "\n\n";
	msgs.pop();
    }
    if (reward->getType() == Reward::GOLD)
      {
        guint32 gold = dynamic_cast<Reward_Gold*>(reward)->getGold();
        s += String::ucompose(
	    ngettext("You have been rewarded with %1 gold piece.",
		     "You have been rewarded with %1 gold pieces.",
		     gold), gold);
      }
    else if (reward->getType() == Reward::ALLIES)
      {
        guint32 num = dynamic_cast<Reward_Allies*>(reward)->getNoOfAllies();
        s += String::ucompose(
	    ngettext("You have been rewarded with %1 ally.",
		     "You have been rewarded with %1 allies.",
		     num), num);
      }
    else if (reward->getType() == Reward::ITEM)
      {
	Item *item = dynamic_cast<Reward_Item*>(reward)->getItem();
	s += String::ucompose("You have been rewarded with the %1.", 
			      item->getName());
      }
    else if (reward->getType() == Reward::RUIN)
      {
	Ruin *ruin = dynamic_cast<Reward_Ruin*>(reward)->getRuin();
	s += String::ucompose("You are shown the site of %1\n", 
			      ruin->getName());
	questmap->set_target(ruin->getPos());
	if (ruin->getReward() == NULL)
	  ruin->populateWithRandomReward();
	Reward *ruin_reward = ruin->getReward();
	if (ruin_reward->getType() == Reward::ALLIES)
	  s += _("where powerful allies can be found!");
	else if (ruin_reward->getType() == Reward::ITEM)
	  {
	    Item *item = dynamic_cast<Reward_Item*>(ruin_reward)->getItem();
	    s += String::ucompose(_("where the %1 can be found!"), 
				  item->getName());
	  }
	else if (ruin_reward->getType() == Reward::MAP)
	  s += _("where a map can be found!");
	else if (ruin_reward->getType() == Reward::RUIN)
	  s += _("where nothing can be found!");
	else if (ruin_reward->getType() == Reward::GOLD)
	  s += _("where gold can be found!");
	else //this one shouldn't happen
	  s += _("where something important can be found!");
      }

    label->set_text(s);

}

QuestCompletedDialog::~QuestCompletedDialog()
{
  delete dialog;
  delete questmap;
}
void QuestCompletedDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
  //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void QuestCompletedDialog::hide()
{
  dialog->hide();
}

void QuestCompletedDialog::run()
{
  questmap->resize();
  questmap->draw(quest->getHero()->getOwner());

  dialog->show_all();
  dialog->run();
}

void QuestCompletedDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, 
                        questmap->get_width(), questmap->get_height());
    map_image->property_pixbuf() = pixbuf;
}

