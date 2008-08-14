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

#include <libglademm/xml.h>
#include <gtkmm/image.h>
#include <gtkmm/table.h>
#include <gtkmm/alignment.h>
#include <sigc++/functors/mem_fun.h>
#include <assert.h>

#include "select-hidden-ruin-dialog.h"

#include "glade-helpers.h"
#include "gui/input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "ruin.h"
#include "ruinlist.h"
#include "playerlist.h"

SelectHiddenRuinDialog::SelectHiddenRuinDialog()
{
    selected_hidden_ruin = 0;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/select-hidden-ruin-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    
    xml->get_widget("select_button", select_button);

    xml->get_widget("hidden_ruins_treeview", hidden_ruins_treeview);
    hidden_ruins_list = Gtk::ListStore::create(hidden_ruins_columns);
    hidden_ruins_treeview->set_model(hidden_ruins_list);
    hidden_ruins_treeview->append_column("", hidden_ruins_columns.name);
    hidden_ruins_treeview->set_headers_visible(false);

    Ruinlist *ruinlist = Ruinlist::getInstance();
    Ruinlist::iterator iter = ruinlist->begin();
    for (;iter != ruinlist->end(); iter++)
      if ((*iter).isHidden() && 
	  (*iter).getOwner() == Playerlist::getInstance()->getNeutral())
	addHiddenRuin(&*iter);
      
    Uint32 max = ruinlist->size();
    if (max)
      {
	Gtk::TreeModel::Row row;
	row = hidden_ruins_treeview->get_model()->children()[0];
	if(row)
	  hidden_ruins_treeview->get_selection()->select(row);
      }
    else
      select_button->set_sensitive(false);

}

void SelectHiddenRuinDialog::addHiddenRuin(Ruin *ruin)
{
    
  Glib::ustring s;
  Gtk::TreeIter i = hidden_ruins_list->append();
  s = String::ucompose("%1 (%2, %3)", ruin->getName(), ruin->getPos().x,
		       ruin->getPos().y);
  (*i)[hidden_ruins_columns.name] = s;
  (*i)[hidden_ruins_columns.ruin] = ruin;
}

void SelectHiddenRuinDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void SelectHiddenRuinDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response != 1)
	selected_hidden_ruin = 0;
    else
      {
	Glib::RefPtr<Gtk::TreeSelection> selection = 
	  hidden_ruins_treeview->get_selection();
	Gtk::TreeModel::iterator iterrow = selection->get_selected();

	if (iterrow) 
	  {
	    Gtk::TreeModel::Row row = *iterrow;
	    selected_hidden_ruin = row[hidden_ruins_columns.ruin];
	  }
      }
}

