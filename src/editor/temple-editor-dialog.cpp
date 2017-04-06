//  Copyright (C) 2007 Ole Laursen
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

#include "temple-editor-dialog.h"

#include "ucompose.hpp"
#include "CreateScenarioRandomize.h"
#include "defs.h"
#include "temple.h"
#include "RenamableLocation.h"

TempleEditorDialog::TempleEditorDialog(Gtk::Window &parent, Temple *t, CreateScenarioRandomize *randomizer)
 : LwEditorDialog(parent, "temple-editor-dialog.ui")
{
    d_randomizer = randomizer;
    temple = t;
    
    xml->get_widget("name_entry", name_entry);
    name_entry->set_text(temple->getName());
    name_entry->set_max_length (MAX_LENGTH_FOR_RUIN_NAME);

    xml->get_widget("description_entry", description_entry);
    description_entry->set_text(temple->getDescription());

    xml->get_widget("type_entry", type_entry);
    type_entry->set_value(temple->getType());
    xml->get_widget("randomize_name_button", randomize_name_button);
    randomize_name_button->signal_clicked().connect(
	sigc::mem_fun(this, &TempleEditorDialog::on_randomize_name_clicked));
}

int TempleEditorDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
      Location *l = temple;
      RenamableLocation *renamable_temple = static_cast<RenamableLocation*>(l);
      renamable_temple->setName(name_entry->get_text());
      temple->setType(type_entry->get_value_as_int());
      renamable_temple->setDescription(description_entry->get_text());
    }
    else
      {
	if (name_entry->get_text() != Temple::getDefaultName())
	  d_randomizer->pushRandomTempleName(name_entry->get_text());
      }
    return response;
}

void TempleEditorDialog::on_randomize_name_clicked()
{
  Glib::ustring existing_name = name_entry->get_text();
  if (existing_name == Temple::getDefaultName())
    name_entry->set_text(d_randomizer->popRandomTempleName());
  else
    {
      name_entry->set_text(d_randomizer->popRandomTempleName());
      d_randomizer->pushRandomTempleName(existing_name);
    }
}
