//  Copyright (C) 2007, 2008, 2009, 2011, 2012, 2014 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "fight-order-dialog.h"

#include <gtkmm.h>
#include "glade-helpers.h"
#include "image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "playerlist.h"
#include "player.h"
#include "army.h"
#include "armysetlist.h"
#include "GraphicsCache.h"
#include "armyset.h"

FightOrderDialog::FightOrderDialog(Gtk::Window &parent, Player *theplayer)
{
    player = theplayer;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/fight-order-dialog.ui");

    xml->get_widget("dialog", dialog);
    dialog->set_transient_for(parent);
    armies_list = Gtk::ListStore::create(armies_columns);
    xml->get_widget("treeview", armies_treeview);
    armies_treeview->set_model(armies_list);
    armies_treeview->append_column("", armies_columns.image);
    armies_treeview->append_column("", armies_columns.name);

    std::list<guint32> fight_order = theplayer->getFightOrder();
    std::list<guint32>::iterator it = fight_order.begin();
    for (; it != fight_order.end(); it++)
      addArmyType(*it);
    armies_treeview->set_reorderable(true);
    xml->get_widget("reverse_button", reverse_button);
    reverse_button->signal_clicked().connect
      (sigc::mem_fun (*this, &FightOrderDialog::on_reverse_button_clicked));
    xml->get_widget("reset_button", reset_button);
    reset_button->signal_clicked().connect
      (sigc::mem_fun (*this, &FightOrderDialog::on_reset_button_clicked));
}
FightOrderDialog::~FightOrderDialog()
{
  delete dialog;
}

void FightOrderDialog::hide()
{
  dialog->hide();
}

void FightOrderDialog::run()
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    int response = dialog->run();

    dialog->get_size(width, height);

    if (response == Gtk::RESPONSE_ACCEPT)
      {
        std::list<guint32> fight_order;
        for (Gtk::TreeIter i = armies_list->children().begin(),
	     end = armies_list->children().end(); i != end; ++i) 
          fight_order.push_back((*i)[armies_columns.army_type]);
        player->setFightOrder(fight_order);

      }
}

void FightOrderDialog::addArmyType(guint32 army_type)
{
    GraphicsCache *gc = GraphicsCache::getInstance();
    Gtk::TreeIter i = armies_list->append();
    Armysetlist *alist = Armysetlist::getInstance();
    const ArmyProto *a = alist->getArmy(player->getArmyset(), army_type);
    (*i)[armies_columns.name] = a->getName();
    (*i)[armies_columns.image] = 
      gc->getCircledArmyPic(player->getArmyset(), army_type, player, NULL,
                            false, player->getId(), true)->to_pixbuf();
    (*i)[armies_columns.army_type] = a->getId();
}

void FightOrderDialog::on_reverse_button_clicked()
{
  std::vector<int> new_order;
  Gtk::TreeModel::Children kids = armies_list->children();
  for (unsigned int i = 0; i < kids.size(); i++)
    new_order.push_back(kids.size() - i - 1);
  armies_list->reorder(new_order);
}

void FightOrderDialog::on_reset_button_clicked()
{
  std::vector<int> new_order;
  Gtk::TreeModel::Children kids = armies_list->children();
  for (unsigned int i = 0; i < kids.size(); i++)
    {
      int index = 0;
      for (unsigned int j = 0; j < kids.size(); j++)
        {
          if (i == (kids[j])[armies_columns.army_type])
            break;
          index++;
        }
      new_order.push_back(index);
    }
  armies_list->reorder(new_order);
}
