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

#include "surrender-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"

SurrenderDialog::SurrenderDialog(int numEnemies)
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/surrender-dialog.ui");

    xml->get_widget("dialog", dialog);
    decorate(dialog);
    window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));

    Gtk::Label *label;
    xml->get_widget("label", label);
    xml->get_widget("image", image);
    
    Glib::ustring s;
    s = String::ucompose
      (ngettext("Your final opponent comes on bended knee and offers surrender!",
		"%1 opponents come on bended knee and offer surrender!",
		numEnemies), numEnemies);
    s += _("\nDo you accept?");
    label->set_text(s);
    image->property_file()
      = File::getMiscFile("various/parley_offered.png");
}

SurrenderDialog::~SurrenderDialog()
{
  delete dialog;
}

void SurrenderDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
  //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void SurrenderDialog::hide()
{
  dialog->hide();
}
bool SurrenderDialog::run()
{
  return dialog->run() == Gtk::RESPONSE_ACCEPT;
}
