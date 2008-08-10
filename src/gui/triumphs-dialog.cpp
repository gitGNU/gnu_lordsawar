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

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/notebook.h>
#include <gtkmm/image.h>

#include "triumphs-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../File.h"
#include "../GameMap.h"
#include "../GraphicsCache.h"
#include "../armysetlist.h"
#include "../playerlist.h"
#include "../player.h"

TriumphsDialog::TriumphsDialog(Player *player)
{
  d_player = player;
  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/triumphs-dialog.glade");

  Gtk::Dialog *d = 0;
  xml->get_widget("dialog", d);
  dialog.reset(d);

  Gtk::HBox *contents;
  xml->get_widget("outer_hbox", contents);
  notebook = Gtk::manage(new Gtk::Notebook());
  contents->pack_start(*notebook, true, true, 0);
  fill_in_info();
  //set the notebook to start off on the player's own page
  notebook->set_current_page(d_player->getId());
}

void TriumphsDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
  //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void TriumphsDialog::hide()
{
  dialog->hide();
}
void TriumphsDialog::run()
{
  dialog->show_all();
  dialog->run();
}

Uint32 TriumphsDialog::tally(Player *p, Player::TriumphType type)
{
  Playerlist *pl = Playerlist::getInstance();
  Uint32 count = 0;
  if (p == d_player)
    {
      // add up what the other players did to us
      for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
	count += p->getTriumphTally(*it, type);
    }
  else
    {
      // add up what we did to that player
      count = d_player->getTriumphTally(p, type);
    }
  return count;
}

void TriumphsDialog::fill_in_page(Player *p)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  //here we tally up the stats, make a vbox and append it as a new page
  //tally it up differently when p == d_player
	
  Uint32 count;
  Glib::ustring s;
  count = tally(p, Player::TALLY_HERO);
  if (p == d_player)
    s = String::ucompose (ngettext("%1 hero earned fates worthy of legend!",
				   "%1 heroes earned fates worthy of legend!",
				   count), count);
  else
    s = String::ucompose (ngettext
			  ("%1 so-called hero slaughtered without mercy!",
			   "%1 so-called heroes slaughtered without mercy!",
			   count), count);
  Gtk::Label *hero_label = new Gtk::Label(s);

  const Army *hero = NULL;
  const Armysetlist* al = Armysetlist::getInstance();
  //let's go find the hero army
  for (unsigned int j = 0; j < al->getSize(p->getArmyset()); j++)
    {
      const Army *a = al->getArmy (p->getArmyset(), j);
      if (a->isHero())
	{
	  hero = a;
	  break;
	}
    }
  Gtk::Image *hero_image = new Gtk::Image
    (to_pixbuf(gc->getArmyPic(p->getArmyset(), hero->getType(), p, NULL)));
  Gtk::HBox *hero_hbox = new Gtk::HBox();
  hero_hbox->pack_start(*manage(hero_image), Gtk::PACK_SHRINK, 10);
  hero_hbox->pack_start(*manage(hero_label), Gtk::PACK_SHRINK, 10);

  count = tally(p, Player::TALLY_SHIP);
  if (p == d_player)
    s = String::ucompose (ngettext("%1 navy not currently in service!",
				   "%1 navies not currently in service!",
				   count), count);
  else
    s = String::ucompose (ngettext("%1 navy rests with the fishes!",
				   "%1 navies rest with the fishes!",
				   count), count);
  Gtk::Label *ship_label = new Gtk::Label(s);
  Gtk::Image *ship_image = new Gtk::Image (to_pixbuf(gc->getShipPic(p)));
  Gtk::HBox *ship_hbox = new Gtk::HBox();
  ship_hbox->pack_start(*manage(ship_image), Gtk::PACK_SHRINK, 10);
  ship_hbox->pack_start(*manage(ship_label), Gtk::PACK_SHRINK, 10);

  count = tally(p, Player::TALLY_NORMAL);
  if (p == d_player)
    s = String::ucompose (ngettext("%1 army died to ensure final victory!",
				   "%1 armies died to ensure final victory!",
				   count), count);
  else
    s = String::ucompose (ngettext("%1 army smote like sheep!",
				   "%1 armies smote like sheep!",
				   count), count);
  Gtk::Label *normal_label = new Gtk::Label(s);
  Gtk::Image *normal_image = new Gtk::Image
    (to_pixbuf(gc->getArmyPic(p->getArmyset(), 0, p, NULL)));
  Gtk::HBox *normal_hbox = new Gtk::HBox();
  normal_hbox->pack_start(*manage(normal_image), Gtk::PACK_SHRINK, 10);
  normal_hbox->pack_start(*manage(normal_label), Gtk::PACK_SHRINK, 10);

  count = tally(p, Player::TALLY_SPECIAL);
  if (p == d_player)
    s = String::ucompose 
      (ngettext ("%1 unnatural creature returned from whence it came!",
		 "%1 unnatural creatures returned from whence they came!",
		 count), count);
  else
    s = String::ucompose (ngettext ("%1 unnatural creature dispatched!",
				    "%1 unnatural creatures dispatched!",
				    count), count);
  Gtk::Label *special_label = new Gtk::Label(s);
  //let's go find a special army
  const Army *special = NULL;
  for (unsigned int j = 0; j < al->getSize(p->getArmyset()); j++)
    {
      const Army *a = al->getArmy (p->getArmyset(), j);
      if (a->getAwardable())
	{
	  special = a;
	  break;
	}
    }
  Gtk::Image *special_image = new Gtk::Image
    (to_pixbuf(gc->getArmyPic(p->getArmyset(), 
			      special->getType(), p, NULL)));
  Gtk::HBox *special_hbox = new Gtk::HBox();
  special_hbox->pack_start(*manage(special_image), Gtk::PACK_SHRINK, 10);
  special_hbox->pack_start(*manage(special_label), Gtk::PACK_SHRINK, 10);

  count = tally(p, Player::TALLY_FLAG);
  if (p == d_player)
    s = String::ucompose (ngettext ("%1 standard betrayed by it's guardian!",
				    "%1 standards betrayed by it's guardian!",
				    count), count);
  else
    s = String::ucompose (ngettext 
			  ("%1 standard wrested from a vanquished foe!",
			   "%1 standards wrested from a vanquished foe!",
			   count), count);
  Gtk::Label *flag_label = new Gtk::Label(s);
  Gtk::Image *flag_image = new Gtk::Image 
    (to_pixbuf(gc->getPlantedStandardPic(p)));
  Gtk::HBox *flag_hbox = new Gtk::HBox();
  flag_hbox->pack_start(*manage(flag_image), Gtk::PACK_SHRINK, 10);
  flag_hbox->pack_start(*manage(flag_label), Gtk::PACK_SHRINK, 10);

  Gtk::VBox *contents = new Gtk::VBox();
  contents->add(*manage(normal_hbox));
  contents->add(*manage(special_hbox));
  contents->add(*manage(hero_hbox));
  contents->add(*manage(ship_hbox));
  contents->add(*manage(flag_hbox));
  notebook->append_page 
    (*manage(contents), 
     *manage(new Gtk::Image(to_pixbuf(gc->getShieldPic(2, p)))));
}

void TriumphsDialog::fill_in_info()
{
  Playerlist::iterator pit = Playerlist::getInstance()->begin();
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      fill_in_page(*pit);
    }
}
