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

#include "shieldset-info-dialog.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "glade-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"


ShieldSetInfoDialog::ShieldSetInfoDialog(Shieldset *shieldset, std::string dir, bool readonly)
{
  d_shieldset = shieldset;
  d_readonly = readonly;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/shieldset-info-dialog.ui");

    xml->get_widget("dialog", dialog);

    xml->get_widget("accept_button", accept_button);
    xml->get_widget("status_label", status_label);
    xml->get_widget("dir_label", dir_label);

    xml->get_widget("name_entry", name_entry);
    name_entry->set_text(shieldset->getName());
    if (readonly == false)
      name_entry->signal_changed().connect
	(sigc::mem_fun(this, &ShieldSetInfoDialog::on_name_changed));
    
    xml->get_widget("subdir_entry", subdir_entry);
    subdir_entry->set_text(shieldset->getSubDir());
    if (readonly == false)
      subdir_entry->signal_changed().connect
	(sigc::mem_fun(this, &ShieldSetInfoDialog::on_subdir_changed));

    xml->get_widget("id_spinbutton", id_spinbutton);
    id_spinbutton->set_value(shieldset->getId());
    id_spinbutton->set_sensitive(false);

    xml->get_widget("copyright_textview", copyright_textview);
    copyright_textview->get_buffer()->set_text(d_shieldset->getCopyright());
    xml->get_widget("license_textview", license_textview);
    license_textview->get_buffer()->set_text(d_shieldset->getLicense());
    xml->get_widget("description_textview", description_textview);
    description_textview->get_buffer()->set_text(shieldset->getInfo());

    dir_label->set_text (dir);
    if (readonly)
      subdir_entry->set_sensitive(false);
    else
      accept_button->set_sensitive(false);
}

void ShieldSetInfoDialog::on_subdir_changed()
{
  std::string dir = File::getUserShieldsetDir() + subdir_entry->get_text();
  if (File::exists(dir) == true && subdir_entry->get_text() != "")
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
  if (subdir_entry->get_text() != "")
    dir_label->set_text(dir + "/");
  else
    {
      dir_label->set_text(File::getUserShieldsetDir() + "<subdir>/");
      accept_button->set_sensitive(false);
    }
}
void ShieldSetInfoDialog::on_name_changed()
{
  char *s = File::sanify(name_entry->get_text().c_str());
  subdir_entry->set_text(s);
  free (s);
}

ShieldSetInfoDialog::~ShieldSetInfoDialog()
{
  delete dialog;
}
void ShieldSetInfoDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

int ShieldSetInfoDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
      d_shieldset->setName(name_entry->get_text());
      d_shieldset->setId(int(id_spinbutton->get_value()));
      if (d_readonly == false)
	d_shieldset->setSubDir(subdir_entry->get_text());
      d_shieldset->setCopyright(copyright_textview->get_buffer()->get_text());
      d_shieldset->setLicense(license_textview->get_buffer()->get_text());
      d_shieldset->setInfo(description_textview->get_buffer()->get_text());
      return response;
    }
    return response;
}

