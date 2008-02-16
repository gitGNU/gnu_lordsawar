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

#include "select-item-dialog.h"

#include "glade-helpers.h"
#include "../gui/input-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../Item.h"
#include "../Itemlist.h"

SelectItemDialog::SelectItemDialog()
{
    selected_item = 0;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/select-item-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    
    xml->get_widget("select_button", select_button);

    xml->get_widget("item_toggles_table", toggles_table);

    fill_in_item_toggles();
}

void SelectItemDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void SelectItemDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response != 1)
	selected_item = 0;
}

void SelectItemDialog::on_item_toggled(Gtk::ToggleButton *toggle)
{
    if (ignore_toggles)
	return;
    
    selected_item = 0;
    ignore_toggles = true;
    for (unsigned int i = 0; i < item_toggles.size(); ++i) {
	if (toggle == item_toggles[i])
	    selected_item = selectable[i];
	
	item_toggles[i]->set_active(toggle == item_toggles[i]);
    }
    ignore_toggles = false;

    set_select_button_state();
}

void SelectItemDialog::fill_in_item_toggles()
{
    // fill in selectable armies
    selectable.clear();
    Itemlist::iterator iter = Itemlist::getInstance()->begin();
    for (;iter != Itemlist::getInstance()->end(); iter++)
      selectable.push_back((*iter).second);

    // fill in item options
    item_toggles.clear();
    toggles_table->foreach(sigc::mem_fun(toggles_table, &Gtk::Container::remove));
    toggles_table->resize(1, 1);
    const int no_columns = 5;
    for (unsigned int i = 0; i < selectable.size(); ++i)
    {
	Gtk::ToggleButton *toggle = manage(new Gtk::ToggleButton);
	
	std::string name = selectable[i]->getName() + "\n" +
	  selectable[i]->getBonusDescription();
	toggle->add(*manage(new Gtk::Label(name)));
	item_toggles.push_back(toggle);
	int x = i % no_columns;
	int y = i / no_columns;
	toggles_table->attach(*toggle, x, x + 1, y, y + 1,
			      Gtk::SHRINK, Gtk::SHRINK);
	toggle->show_all();

	toggle->signal_toggled().connect(
	    sigc::bind(sigc::mem_fun(this, &SelectItemDialog::on_item_toggled),
		       toggle));
    }

    ignore_toggles = false;
    if (!item_toggles.empty())
	item_toggles[0]->set_active(true);
}

void SelectItemDialog::set_select_button_state()
{
    select_button->set_sensitive(selected_item);
}
