//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include "armyset-info-dialog.h"

#include "glade-helpers.h"
#include "ucompose.hpp"
#include "defs.h"


ArmySetInfoDialog::ArmySetInfoDialog(Armyset *armyset)
{
  d_armyset = armyset;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/armyset-info-dialog.gtk");

    xml->get_widget("dialog", dialog);

    xml->get_widget("name_entry", name_entry);
    name_entry->set_text(armyset->getName());
    
    xml->get_widget("id_spinbutton", id_spinbutton);
    id_spinbutton->set_value(armyset->getId());
}

ArmySetInfoDialog::~ArmySetInfoDialog()
{
  delete dialog;
}
void ArmySetInfoDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

bool ArmySetInfoDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
      d_armyset->setName(name_entry->get_text());
      d_armyset->setId(int(id_spinbutton->get_value()));
      return true;
    }
    return false;
}

