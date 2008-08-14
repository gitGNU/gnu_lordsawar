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

#include "temple-dialog.h"

#include "glade-helpers.h"
#include "ucompose.hpp"
#include "CreateScenarioRandomize.h"
#include "defs.h"
#include "temple.h"
#include "RenamableLocation.h"

TempleDialog::TempleDialog(Temple *t, CreateScenarioRandomize *randomizer)
{
    d_randomizer = randomizer;
    temple = t;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/temple-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("name_entry", name_entry);
    name_entry->set_text(temple->getName());

    xml->get_widget("type_entry", type_entry);
    type_entry->set_value(temple->getType());
    xml->get_widget("randomize_name_button", randomize_name_button);
    randomize_name_button->signal_clicked().connect(
	sigc::mem_fun(this, &TempleDialog::on_randomize_name_clicked));
}

void TempleDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void TempleDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == 0)		// accepted
    {
      Location *l = temple;
      RenamableLocation *renamable_temple = static_cast<RenamableLocation*>(l);
      renamable_temple->setName(name_entry->get_text());
      temple->setType(type_entry->get_value_as_int());
    }
    else
      {
	if (name_entry->get_text() != DEFAULT_TEMPLE_NAME)
	  d_randomizer->pushRandomTempleName(name_entry->get_text());
      }
}

void TempleDialog::on_randomize_name_clicked()
{
  std::string existing_name = name_entry->get_text();
  if (existing_name == "Shrine")
    name_entry->set_text(d_randomizer->popRandomTempleName());
  else
    {
      name_entry->set_text(d_randomizer->popRandomTempleName());
      d_randomizer->pushRandomTempleName(existing_name);
    }
}
