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

#include "item-bonus-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "Item.h"
#include "Itemlist.h"

ItemBonusDialog::ItemBonusDialog()
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/item-bonus-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    decorate(dialog.get());
    window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));

    items_list = Gtk::ListStore::create(items_columns);
    xml->get_widget("treeview", items_treeview);
    items_treeview->set_model(items_list);
    items_treeview->append_column("", items_columns.name);
    items_treeview->append_column("Bonus", items_columns.bonus);

    Itemlist::iterator iter = Itemlist::getInstance()->begin();
    for (;iter != Itemlist::getInstance()->end(); iter++)
      addItem((*iter).second);
}

void ItemBonusDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
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

void ItemBonusDialog::addItem(Item *item)
{
    if (item->isPlantable())
      return;
    Gtk::TreeIter i = items_list->append();
    (*i)[items_columns.name] = item->getName();
    (*i)[items_columns.bonus] = item->getBonusDescription();
}
