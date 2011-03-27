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

#include "army-bonus-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "army.h"
#include "armysetlist.h"
#include "player.h"
#include "GraphicsCache.h"
#include "File.h"

ArmyBonusDialog::ArmyBonusDialog(Player *p)
{
    d_player = p;
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/army-bonus-dialog.ui");

    xml->get_widget("dialog", dialog);
    decorate(dialog);
    window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
    dialog->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));

    armies_list = Gtk::ListStore::create(armies_columns);
    xml->get_widget("treeview", armies_treeview);
    armies_treeview->set_model(armies_list);
    armies_treeview->append_column("", armies_columns.image);
    armies_treeview->append_column("", armies_columns.name);
    armies_treeview->append_column("Str", armies_columns.str);
    armies_treeview->append_column("Move", armies_columns.move);
    armies_treeview->append_column("", armies_columns.move_image);
    armies_treeview->append_column("Bonus", armies_columns.bonus);
    armies_treeview->set_headers_visible(true);

    guint32 max = Armysetlist::getInstance()->getSize(d_player->getArmyset());
    for (unsigned int i = 0; i < max; i++)
      addArmyType(i);
}

ArmyBonusDialog::~ArmyBonusDialog()
{
  delete dialog;
}
void ArmyBonusDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void ArmyBonusDialog::hide()
{
  dialog->hide();
}

void ArmyBonusDialog::run()
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    dialog->run();

    dialog->get_size(width, height);

}

void ArmyBonusDialog::addArmyType(guint32 army_type)
{
    GraphicsCache *gc = GraphicsCache::getInstance();
    Player *p = d_player;
    const ArmyProto *a;
    a = Armysetlist::getInstance()->getArmy(p->getArmyset(), army_type);
    if (a->isHero())
      return; //we don't want to show heroes in this list
    Gtk::TreeIter i = armies_list->append();
    (*i)[armies_columns.name] = a->getName();
    (*i)[armies_columns.image] = 
      gc->getCircledArmyPic(p->getArmyset(), army_type, p, NULL, false,
                            p->getId(), true)->to_pixbuf();
    (*i)[armies_columns.str] = a->getStrength();
    (*i)[armies_columns.move] = a->getMaxMoves();
    guint32 b = a->getMoveBonus();
    (*i)[armies_columns.move_image] = gc->getMoveBonusPic(b, false)->to_pixbuf();
    (*i)[armies_columns.bonus] = "-";

    std::string s = a->getArmyBonusDescription();
    if (s == "")
      (*i)[armies_columns.bonus] = "-";
    else
      (*i)[armies_columns.bonus] = s;
}
