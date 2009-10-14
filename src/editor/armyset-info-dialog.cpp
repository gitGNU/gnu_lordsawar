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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "glade-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"


ArmySetInfoDialog::ArmySetInfoDialog(Armyset *armyset, bool readonly)
{
  d_armyset = armyset;
  d_readonly = readonly;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/armyset-info-dialog.ui");

    xml->get_widget("dialog", dialog);

    xml->get_widget("accept_button", accept_button);
    xml->get_widget("status_label", status_label);

    xml->get_widget("name_entry", name_entry);
    name_entry->set_text(armyset->getName());
    if (readonly == false)
      name_entry->signal_changed().connect
	(sigc::mem_fun(this, &ArmySetInfoDialog::on_name_changed));
    
    xml->get_widget("subdir_entry", subdir_entry);
    subdir_entry->set_text(armyset->getSubDir());
    if (readonly == false)
      subdir_entry->signal_changed().connect
	(sigc::mem_fun(this, &ArmySetInfoDialog::on_subdir_changed));

    xml->get_widget("id_spinbutton", id_spinbutton);
    id_spinbutton->set_value(armyset->getId());
    id_spinbutton->set_sensitive(false);

    if (readonly)
      subdir_entry->set_sensitive(false);
}

//remove spaces and lowercase the text
char *sanify(const char *string)
{
  char *result = NULL;
  size_t resultlen = 1;
  size_t len = strlen(string);
  result = (char*) malloc (resultlen);
  result[0] = '\0';
  for (unsigned int i = 0; i < len; i++)
    {
      int letter = tolower(string[i]);
      if (strchr("abcdefghijklmnopqrstuvwxyz0123456789-", letter) == NULL)
	continue;

      resultlen++;
      result = (char *) realloc (result, resultlen);
      if (result)
	{
	  result[resultlen-2] = char(letter);
	  result[resultlen-1] = '\0';
	}
    }
  return result;
}

void ArmySetInfoDialog::on_subdir_changed()
{
  std::string dir = File::getUserArmysetDir() + subdir_entry->get_text();
  if (File::exists(dir) == true)
    {
      accept_button->set_sensitive(false);
      status_label->set_markup(String::ucompose("<b>%1</b>", 
						_("That subdirectory is already in use.")));
    }
  else
    {
      accept_button->set_sensitive(true);
      status_label->set_markup("");
    }
}
void ArmySetInfoDialog::on_name_changed()
{
  char *s = sanify(name_entry->get_text().c_str());
  subdir_entry->set_text(s);
  free (s);
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

int ArmySetInfoDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
      d_armyset->setName(name_entry->get_text());
      d_armyset->setId(int(id_spinbutton->get_value()));
      if (d_readonly == false)
	d_armyset->setSubDir(subdir_entry->get_text());
      return response;
    }
    return response;
}

