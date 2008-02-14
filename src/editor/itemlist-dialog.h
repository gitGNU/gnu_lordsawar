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

#ifndef GUI_ITEMLIST_DIALOG_H
#define GUI_ITEMLIST_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <libglademm/xml.h>
#include <gtkmm/window.h>
#include <gtkmm/dialog.h>
#include <gtkmm/container.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <gtkmm/textview.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/table.h>
#include <gtkmm/checkmenuitem.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/tooltips.h>

#include "../Itemlist.h"

class ItemlistDialog: public sigc::trackable
{
 public:
    ItemlistDialog();
    ~ItemlistDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();

    void show();
    void hide();

    Gtk::Dialog &get_dialog() { return *dialog.get(); }

 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    std::string current_save_filename;
    Itemlist *d_itemlist; //current itemlist
    Item *d_item; //current item
    Gtk::Entry *name_entry;
    Gtk::TreeView *items_treeview;
    Gtk::Button *add_item_button;
    Gtk::Button *remove_item_button;
    Gtk::VBox *item_vbox;

    class ItemsColumns: public Gtk::TreeModelColumnRecord {
    public:
	ItemsColumns() 
        { add(name); add(item);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Item *> item;
    };
    const ItemsColumns items_columns;
    Glib::RefPtr<Gtk::ListStore> items_list;

    bool on_delete_event(GdkEventAny *e);

    void addItem(Item *item);
    void update_item_panel();
    void update_itemlist_buttons();

    void fill_item_info(Item *item);

    //callbacks
    void on_name_changed();
    void on_add_item_clicked();
    void on_remove_item_clicked();
    void on_item_selected();

};

#endif
