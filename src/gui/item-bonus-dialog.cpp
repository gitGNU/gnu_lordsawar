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

#include "item-bonus-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "ItemProto.h"
#include "Itemlist.h"

ItemBonusDialog::ItemBonusDialog(Gtk::Window &parent)
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/item-bonus-dialog.ui");

    xml->get_widget("dialog", dialog);
    dialog->set_transient_for(parent);
    items_list = Gtk::ListStore::create(items_columns);
    xml->get_widget("treeview", items_treeview);
    items_treeview->set_model(items_list);
    items_treeview->append_column("", items_columns.name);
    items_treeview->append_column("Bonus", items_columns.bonus);

    Itemlist::iterator iter = Itemlist::getInstance()->begin();
    for (;iter != Itemlist::getInstance()->end(); iter++)
      addItemProto((*iter).second);
}

ItemBonusDialog::~ItemBonusDialog()
{
  delete dialog;
}

void ItemBonusDialog::hide()
{
  dialog->hide();
}
void ItemBonusDialog::run()
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    dialog->run();

    dialog->get_size(width, height);

}

void ItemBonusDialog::addItemProto(ItemProto *itemproto)
{
    Gtk::TreeIter i = items_list->append();
    (*i)[items_columns.name] = itemproto->getName();
    (*i)[items_columns.bonus] = itemproto->getBonusDescription();
}
