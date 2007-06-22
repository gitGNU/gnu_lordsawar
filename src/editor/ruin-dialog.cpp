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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.*

#include <config.h>

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>

#include "ruin-dialog.h"

#include "glade-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../ruin.h"
#include "../stack.h"
#include "../army.h"


RuinDialog::RuinDialog(Ruin *r)
{
    ruin = r;
    
    // copy occupant to be able to undo if the dialog is cancelled
    Stack *occupant = ruin->getOccupant();
    if (occupant)
	keeper = new Stack(*occupant);
    else
	keeper = new Stack(0, ruin->getPos());
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/ruin-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("name_entry", name_entry);
    name_entry->set_text(ruin->getName());

    xml->get_widget("keeper_button", keeper_button);
    keeper_button->signal_clicked().connect(
	sigc::mem_fun(this, &RuinDialog::on_keeper_clicked));

    set_keeper_name();
}

void RuinDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void RuinDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == 0)		// accepted
    {
        ruin->setName(name_entry->get_text());

	// get rid of old occupant and insert new
	Stack *occupant = ruin->getOccupant();
	if (occupant)
	    delete occupant;

	ruin->setOccupant(keeper);
	keeper = 0;
    }
    else
    {
	delete keeper;
	keeper = 0;
    }
}

void RuinDialog::set_keeper_name()
{
    Glib::ustring name;
    if (keeper && !keeper->empty())
	name = keeper->getStrongestArmy()->getName();
    else
	name = _("No keeper");
    
    keeper_button->set_label(name);
}

void RuinDialog::on_keeper_clicked()
{
    // FIXME: run stack dialog with keeper

    set_keeper_name();
}

