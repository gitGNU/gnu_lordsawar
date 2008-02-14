//  Copyright (C) 2008, Ben Asselstine
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

#include <iostream>
#include <iomanip>
#include <assert.h>
#include <libgen.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>

#include <gtkmm/widget.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/image.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>

#include "itemlist-dialog.h"

#include "../gui/input-helpers.h"
#include "../gui/error-utils.h"

#include "../defs.h"
#include "../Configuration.h"
#include "../Itemlist.h"
#include "../Tile.h"

#include "../ucompose.hpp"

#include "glade-helpers.h"


ItemlistDialog::ItemlistDialog()
{
  d_itemlist = Itemlist::getInstance();
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + 
				    "/itemlist-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("name_entry", name_entry);
    name_entry->signal_changed().connect
      (sigc::mem_fun(this, &ItemlistDialog::on_name_changed));
    xml->get_widget("items_treeview", items_treeview);
    xml->get_widget("add_item_button", add_item_button);
    add_item_button->signal_clicked().connect
      (sigc::mem_fun(this, &ItemlistDialog::on_add_item_clicked));
    xml->get_widget("remove_item_button", remove_item_button);
    remove_item_button->signal_clicked().connect
      (sigc::mem_fun(this, &ItemlistDialog::on_remove_item_clicked));
    xml->get_widget("item_vbox", item_vbox);

    items_list = Gtk::ListStore::create(items_columns);
    items_treeview->set_model(items_list);
    items_treeview->append_column("", items_columns.name);
    items_treeview->set_headers_visible(false);
    items_treeview->get_selection()->signal_changed().connect
      (sigc::mem_fun(*this, &ItemlistDialog::on_item_selected));

    update_item_panel();
    update_itemlist_buttons();
    Itemlist::iterator iter = d_itemlist->begin();
    for (;iter != d_itemlist->end(); iter++)
      addItem((*iter).second);
      
    Uint32 max = d_itemlist->size();
    if (max)
      {
	Gtk::TreeModel::Row row;
	row = items_treeview->get_model()->children()[0];
	if(row)
	  items_treeview->get_selection()->select(row);
      }
}

void
ItemlistDialog::update_itemlist_buttons()
{
  if (!items_treeview->get_selection()->get_selected())
    remove_item_button->set_sensitive(false);
  else
    remove_item_button->set_sensitive(true);
  if (d_itemlist == NULL)
    add_item_button->set_sensitive(false);
  else
    add_item_button->set_sensitive(true);
}

void
ItemlistDialog::update_item_panel()
{
  //if nothing selected in the treeview, then we don't show anything in
  //the item panel
  if (items_treeview->get_selection()->get_selected() == 0)
    {
      //clear all values
      name_entry->set_text("");
      item_vbox->set_sensitive(false);
      return;
    }
  item_vbox->set_sensitive(true);
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      // Row selected
      Gtk::TreeModel::Row row = *iterrow;

      Item *a = row[items_columns.item];
      fill_item_info(a);
    }
}

ItemlistDialog::~ItemlistDialog()
{
}

void ItemlistDialog::show()
{
  dialog->show();
}

void ItemlistDialog::hide()
{
  dialog->hide();
}

void ItemlistDialog::addItem(Item *item)
{
  Gtk::TreeIter i = items_list->append();
  (*i)[items_columns.name] = item->getName();
  (*i)[items_columns.item] = item;
}

void ItemlistDialog::on_item_selected()
{
  update_item_panel();
  update_itemlist_buttons();
}

void ItemlistDialog::fill_item_info(Item *item)
{
  name_entry->set_text(item->getName());
}

void ItemlistDialog::on_name_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Item *a = row[items_columns.item];
      a->setName(name_entry->get_text());
      row[items_columns.name] = name_entry->get_text();
    }
}
void ItemlistDialog::on_add_item_clicked()
{
  //add a new empty item to the itemlist
  Item *a = new Item("Untitled", false, NULL);
  //add it to the treeview
  Gtk::TreeIter i = items_list->append();
  a->setName("Untitled");
  (*i)[items_columns.name] = a->getName();
  (*i)[items_columns.item] = a;

}
void ItemlistDialog::on_remove_item_clicked()
{
  //erase the selected row from the treeview
  //remove the item from the itemlist
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      Item *a = row[items_columns.item];
      items_list->erase(iterrow);
      d_itemlist->remove(a);
    }
}
void ItemlistDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void ItemlistDialog::run()
{
    dialog->show_all();
    int response = dialog->run();
}

