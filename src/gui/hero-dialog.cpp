//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008 Ben Asselstine
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

#include <iomanip>

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm/image.h>

#include "hero-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "hero.h"
#include "Item.h"
#include "GameMap.h"
#include "GraphicsCache.h"
#include "Backpack.h"
#include "MapBackpack.h"
#include "history.h"

HeroDialog::HeroDialog(Hero *h, Vector<int> p)
{
    GraphicsCache *gc = GraphicsCache::getInstance();
    hero = h;
    pos = p;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/hero-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    decorate(dialog.get());
    window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
    set_title(hero->getName());

    Gtk::Label *hero_label;
    xml->get_widget("hero_label", hero_label);
    hero_label->set_markup("<b>" + hero->getName() + "</b>");

    Gtk::Image *hero_image;
    xml->get_widget("hero_image", hero_image);
    hero_image->property_pixbuf() = to_pixbuf(gc->getArmyPic(hero));

    xml->get_widget("info_label1", info_label1);
    xml->get_widget("info_label2", info_label2);
    fill_in_info_labels();
	
    xml->get_widget("drop_button", drop_button);
    xml->get_widget("pickup_button", pickup_button);

    drop_button->signal_clicked()
	.connect(sigc::mem_fun(this, &HeroDialog::on_drop_clicked));
    pickup_button->signal_clicked()
	.connect(sigc::mem_fun(this, &HeroDialog::on_pickup_clicked));
    
    item_list = Gtk::ListStore::create(item_columns);
    xml->get_widget("treeview", item_treeview);
    item_treeview->set_model(item_list);
    item_treeview->append_column("", item_columns.image);
    item_treeview->append_column(_("Name"), item_columns.name);
    item_treeview->append_column(_("Attributes"), item_columns.attributes);
    item_treeview->append_column(_("Status"), item_columns.status);

    item_treeview->get_selection()->signal_changed()
	.connect(sigc::mem_fun(this, &HeroDialog::on_selection_changed));

    events_list = Gtk::ListStore::create(events_columns);
    xml->get_widget("events_treeview", events_treeview);
    events_treeview->append_column("", events_columns.desc);
    events_treeview->set_model(events_list);
    events_list->clear();
    std::list<History* > events;
    events = hero->getOwner()->getHistoryForHeroId(hero->getId());
    for (std::list<History*>::iterator i = events.begin(); i != events.end();
	 i++)
      addHistoryEvent(*i);

    on_selection_changed();

    // populate the item list
    Backpack *backpack = hero->getBackpack();
    for (Backpack::iterator i = backpack->begin(); i != backpack->end(); ++i)
	add_item(*i, true);

    MapBackpack *ground = GameMap::getInstance()->getTile(pos)->getBackpack();
    for (MapBackpack::iterator i = ground->begin(); i != ground->end(); i++)
      add_item(*i, false);
}

void HeroDialog::addHistoryEvent(History *history)
{
  Glib::ustring s = "";
  Gtk::TreeIter i = events_list->append();

  switch (history->getType())
    {
    case History::FOUND_SAGE: 
	{
	  History_FoundSage *ev;
	  ev = static_cast<History_FoundSage *>(history);
	  s = String::ucompose(_("%1 finds a sage!"), ev->getHeroName());
	  break;
	}
    case History::HERO_EMERGES:
	{
	  History_HeroEmerges *ev;
	  ev = static_cast<History_HeroEmerges *>(history);
	  s = String::ucompose(_("%1 emerges in %2!"), ev->getHeroName(),
			       ev->getCityName());
	  break;
	}
    case History::HERO_QUEST_STARTED:
	{
	  History_HeroQuestStarted *ev;
	  ev = static_cast<History_HeroQuestStarted*>(history);
	  s = String::ucompose(_("%1 begins a quest!"), ev->getHeroName());
	  break;
	}
    case History::HERO_QUEST_COMPLETED:
	{
	  History_HeroQuestCompleted *ev;
	  ev = static_cast<History_HeroQuestCompleted *>(history);
	  s = String::ucompose(_("%1 finishes a quest!"), ev->getHeroName());
	  break;
	}
    case History::HERO_KILLED_IN_CITY:
	{
	  History_HeroKilledInCity *ev;
	  ev = static_cast<History_HeroKilledInCity *>(history);
	  s = String::ucompose(_("%1 is killed in %2!"), ev->getHeroName(),
			       ev->getCityName());
	  break;
	}
    case History::HERO_KILLED_IN_BATTLE:
	{
	  History_HeroKilledInBattle *ev;
	  ev = static_cast<History_HeroKilledInBattle *>(history);
	  s = String::ucompose(_("%1 is killed in battle!"), ev->getHeroName());
	  break;
	}
    case History::HERO_KILLED_SEARCHING:
	{
	  History_HeroKilledSearching *ev;
	  ev = static_cast<History_HeroKilledSearching *>(history);
	  s = String::ucompose(_("%1 is killed while searching!"), 
			       ev->getHeroName());
	  break;
	}
    case History::HERO_CITY_WON:
	{
	  History_HeroCityWon *ev;
	  ev = static_cast<History_HeroCityWon *>(history);
	  s = String::ucompose(_("%1 conquers %2!"), ev->getHeroName(), 
			       ev->getCityName());
	  break;
	}
    case History::HERO_FINDS_ALLIES:
	{
	  History_HeroFindsAllies *ev;
	  ev = static_cast<History_HeroFindsAllies*>(history);
	  s = String::ucompose(_("%1 finds allies!"), ev->getHeroName());
	  break;
	}
    default:
      s = _("unknown");
      break;
    }

  (*i)[events_columns.desc] = s;
  (*i)[events_columns.history] = history;
}

void HeroDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void HeroDialog::hide()
{
  dialog->hide();
}

void HeroDialog::run()
{
    GameMap *gm = GameMap::getInstance();
    dialog->show();
    dialog->run();
    if (gm->getTile(pos)->getBackpack()->size() > 0 && 
        gm->getTile(pos)->getMaptileType() == Tile::WATER)
      {
        // splash, items lost forever
        while (gm->getTile(pos)->getBackpack()->size())
          {
	    MapBackpack::iterator i = gm->getTile(pos)->getBackpack()->begin();
            gm->getTile(pos)->getBackpack()->removeFromBackpack(*i);
          }
      }
}

void HeroDialog::on_selection_changed()
{
    Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
    if (i)
    {
	bool droppable = (*i)[item_columns.status] == _("In backpack");
	drop_button->set_sensitive(droppable);
	pickup_button->set_sensitive(!droppable);
    }
    else
    {
	drop_button->set_sensitive(false);
	pickup_button->set_sensitive(false);
    }
}

void HeroDialog::on_drop_clicked()
{
    Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
    if (i)
    {
	Item *item = (*i)[item_columns.item];
	hero->getOwner()->heroDropItem (hero, item, pos);
	(*i)[item_columns.status] = _("On the ground");
	on_selection_changed();
	fill_in_info_labels();
    }
}

void HeroDialog::on_pickup_clicked()
{
    Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
    if (i)
    {
	Item *item = (*i)[item_columns.item];
        if (item->getPlanted() == true)
          item->setPlanted(false);
	hero->getOwner()->heroPickupItem (hero, item, pos);
	(*i)[item_columns.status] = _("In backpack");
	on_selection_changed();
	fill_in_info_labels();
    }
}

void HeroDialog::add_item(Item *item, bool in_backpack)
{
    Gtk::TreeIter i = item_list->append();
    (*i)[item_columns.name] = item->getName();

    (*i)[item_columns.attributes] = item->getBonusDescription();
    
    if (in_backpack)
	(*i)[item_columns.status] = _("In backpack");
    else
	(*i)[item_columns.status] = _("On the ground");

    (*i)[item_columns.item] = item;
}

void HeroDialog::fill_in_info_labels()
{
    Uint32 bonus = 0;
    Glib::ustring s;
    // fill in first column
    Backpack *backpack = hero->getBackpack();
    for (Backpack::iterator i = backpack->begin(); i != backpack->end(); ++i)
      {
        if ((*i)->getBonus(Item::ADD1STR))
          bonus += 1;
        if ((*i)->getBonus(Item::ADD2STR))
          bonus += 2;
        if ((*i)->getBonus(Item::ADD3STR))
          bonus += 3;
      }
    s += String::ucompose(_("Battle: %1"), bonus);
    s += "\n";

    bonus = 0;
    for (Backpack::iterator i = backpack->begin(); i != backpack->end(); ++i)
      {
        if ((*i)->getBonus(Item::ADD1STACK))
          bonus += 1;
        if ((*i)->getBonus(Item::ADD2STACK))
          bonus += 2;
        if ((*i)->getBonus(Item::ADD3STACK))
          bonus += 3;
      }

    //now add natural command
    bonus += hero->calculateNaturalCommand ();

    s += String::ucompose(_("Command: %1"), bonus);
    s += "\n";
    s += String::ucompose(_("Level: %1"), hero->getLevel());
    s += "\n";
    s += String::ucompose(_("Experience: %1"),
			  std::setprecision(3), hero->getXP());
    info_label1->set_text(s);

    // fill in second column
    s = "";
    // note to translators: %1 is melee strength, %2 is ranged strength
    s += String::ucompose(_("Strength: %1"),
			  hero->getStat(Army::STRENGTH));
    s += "\n";
    // note to translators: %1 is remaining moves, %2 is total moves
    s += String::ucompose(_("Moves: %1/%2"),
			  hero->getMoves(), hero->getStat(Army::MOVES));
    s += "\n";
    s += String::ucompose(_("Upkeep: %1"), hero->getUpkeep());
    info_label2->set_text(s);
}
