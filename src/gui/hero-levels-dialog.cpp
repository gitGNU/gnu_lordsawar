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

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>

#include "hero-levels-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../playerlist.h"
#include "../player.h"
#include "../army.h"
#include "../armysetlist.h"
#include "../stacklist.h"
#include "../stack.h"
#include "../hero.h"
#include "../GraphicsCache.h"

HeroLevelsDialog::HeroLevelsDialog(Player *theplayer)
{
    player = theplayer;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/hero-levels-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    decorate(dialog.get());
    window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));

    heroes_list = Gtk::ListStore::create(heroes_columns);
    xml->get_widget("treeview", heroes_treeview);
    heroes_treeview->set_model(heroes_list);
    heroes_treeview->append_column("", heroes_columns.image);
    heroes_treeview->append_column("Hero", heroes_columns.name);
    heroes_treeview->append_column("Level", heroes_columns.level);
    heroes_treeview->append_column("Exp", heroes_columns.exp);
    heroes_treeview->append_column("Needs", heroes_columns.needs);
    heroes_treeview->append_column("Str", heroes_columns.str);
    heroes_treeview->append_column("Move", heroes_columns.move);
    heroes_treeview->set_headers_visible(true);

    const Stacklist* sl = theplayer->getStacklist();
    for (Stacklist::const_iterator it = sl->begin(); it != sl->end(); it++)
      for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
        if ((*sit)->isHero()) 
          addHero(dynamic_cast<Hero*>(*sit));
     
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
    (*i)[heroes_columns.image] = to_pixbuf(gc->getArmyPic(player->getArmyset(),
                                           h->getType(), player, NULL));
    (*i)[heroes_columns.level] = String::ucompose(_("%1"), h->getLevel());
    (*i)[heroes_columns.exp] = (Uint32)h->getXP();
    (*i)[heroes_columns.needs] = (Uint32)h->getXpNeededForNextLevel();
    (*i)[heroes_columns.str] = h->getStat(Army::STRENGTH, true);
    (*i)[heroes_columns.move] = h->getStat(Army::MOVES, true);
}
