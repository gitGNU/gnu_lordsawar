//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011 Ben Asselstine
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

#include "hero-levels-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "playerlist.h"
#include "player.h"
#include "army.h"
#include "armysetlist.h"
#include "stack.h"
#include "hero.h"
#include "GraphicsCache.h"
#include "shield.h"

void HeroLevelsDialog::init(Player *theplayer)
{
    player = theplayer;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/hero-levels-dialog.ui");

    xml->get_widget("dialog", dialog);
    decorate(dialog);
    window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));

    heroes_list = Gtk::ListStore::create(heroes_columns);
    xml->get_widget("treeview", heroes_treeview);
    heroes_treeview->set_model(heroes_list);
    heroes_treeview->append_column("", heroes_columns.image);
    heroes_treeview->append_column(_("Hero"), heroes_columns.name);
    heroes_treeview->append_column(_("Level"), heroes_columns.level);
    heroes_treeview->append_column(_("Exp"), heroes_columns.exp);
    heroes_treeview->append_column(_("Needs"), heroes_columns.needs);
    heroes_treeview->append_column(_("Str"), heroes_columns.str);
    heroes_treeview->append_column(_("Move"), heroes_columns.move);
    heroes_treeview->set_headers_visible(true);
}

HeroLevelsDialog::HeroLevelsDialog(std::list<Hero*> heroes)
{
  init ((*heroes.front()).getOwner());
          
  for (std::list<Hero*>::iterator it = heroes.begin(); it != heroes.end(); it++)
    addHero(*it);
}

HeroLevelsDialog::HeroLevelsDialog(Player *theplayer)
{

  init (theplayer);
  std::list<Hero*> heroes = theplayer->getHeroes();
  for (std::list<Hero*>::iterator it = heroes.begin(); it != heroes.end(); it++)
    addHero(*it);
}

HeroLevelsDialog::~HeroLevelsDialog()
{
  delete dialog;
}
void HeroLevelsDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void HeroLevelsDialog::hide()
{
  dialog->hide();
}

void HeroLevelsDialog::run()
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    dialog->run();

    dialog->get_size(width, height);

}

void HeroLevelsDialog::addHero(Hero *h)
{
    GraphicsCache *gc = GraphicsCache::getInstance();
    Gtk::TreeIter i = heroes_list->append();
    (*i)[heroes_columns.name] = h->getName();
    (*i)[heroes_columns.image] = 
      gc->getCircledArmyPic(player->getArmyset(), h->getTypeId(), player, NULL,
                            false, Shield::NEUTRAL, true)->to_pixbuf();
    (*i)[heroes_columns.level] = String::ucompose("%1", h->getLevel());
    (*i)[heroes_columns.exp] = (guint32)h->getXP();
    (*i)[heroes_columns.needs] = (guint32)h->getXpNeededForNextLevel();
    (*i)[heroes_columns.str] = h->getStat(Army::STRENGTH, true);
    (*i)[heroes_columns.move] = h->getStat(Army::MOVES, true);
}
