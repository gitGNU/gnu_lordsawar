//  Copyright (C) 2007, 2008, 2009, 2010, 2014 Ben Asselstine
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

#include "cityset-info-dialog.h"
#include "citysetlist.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"

#define method(x) sigc::mem_fun(*this, &CitySetInfoDialog::x)

CitySetInfoDialog::CitySetInfoDialog(Gtk::Window &parent, Set *cityset, Glib::ustring dir, Glib::ustring file, bool readonly, Glib::ustring title)
 : LwEditorDialog(parent, "cityset-info-dialog.ui")
{
  d_cityset = cityset;
  d_readonly = readonly;
    
    if (title != "")
      dialog->set_title(title);

    xml->get_widget("accept_button", accept_button);
    xml->get_widget("status_label", status_label);
    xml->get_widget("dir_label", dir_label);

    xml->get_widget("name_entry", name_entry);
    name_entry->set_text(cityset->getName());
    if (readonly == false)
      name_entry->signal_changed().connect (method(on_name_changed));
    
    xml->get_widget("filename_entry", filename_entry);
    if (file != "")
      filename_entry->set_text(file);
    else
      {
        guint32 num = 0;
        Glib::ustring basename = Citysetlist::getInstance()->findFreeBaseName(_("untitled"), 100, num);
        filename_entry->set_text(basename);

        Glib::ustring name = String::ucompose("%1 %2", _("Untitled"), num);
        name_entry->set_text(name);
      }
    if (readonly == false)
      filename_entry->signal_changed().connect (method(on_filename_changed));

    xml->get_widget("id_spinbutton", id_spinbutton);
    id_spinbutton->set_value(cityset->getId());
    id_spinbutton->set_sensitive(false);

    xml->get_widget("copyright_textview", copyright_textview);
    copyright_textview->get_buffer()->set_text(d_cityset->getCopyright());
    xml->get_widget("license_textview", license_textview);
    license_textview->get_buffer()->set_text(d_cityset->getLicense());
    xml->get_widget("description_textview", description_textview);
    description_textview->get_buffer()->set_text(cityset->getInfo());

    dir_label->set_text (dir);
    if (readonly)
      filename_entry->set_sensitive(false);

    update_buttons();
}

void CitySetInfoDialog::on_filename_changed()
{
  update_buttons();
}

void CitySetInfoDialog::on_name_changed()
{
  char *s = File::sanify(name_entry->get_text().c_str());
  filename_entry->set_text(s);
  free (s);
  update_buttons();
}

int CitySetInfoDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
      d_cityset->setName(name_entry->get_text());
      d_cityset->setId(int(id_spinbutton->get_value()));
      if (d_readonly == false)
	d_cityset->setBaseName(filename_entry->get_text());
      d_cityset->setCopyright(copyright_textview->get_buffer()->get_text());
      d_cityset->setLicense(license_textview->get_buffer()->get_text());
      d_cityset->setInfo(description_textview->get_buffer()->get_text());
      return response;
    }
    return response;
}

void CitySetInfoDialog::update_buttons()
{
  if (d_readonly)
    {
      accept_button->set_sensitive(true);
      return;
    }

  if (Citysetlist::getInstance()->get(filename_entry->get_text()))
    {
      accept_button->set_sensitive(false);
      status_label->set_markup(String::ucompose("<b>%1</b>", 
						_("That filename is already used.")));
    }
  else if (filename_entry->get_text() == "" || name_entry->get_text() == "")
    accept_button->set_sensitive(false);
  else if (Citysetlist::getInstance()->contains(name_entry->get_text()) && 
           name_entry->get_text() != "")
    {
      status_label->set_markup(String::ucompose("<b>%1</b>", 
						_("That name is already in use.")));
      accept_button->set_sensitive(true);
    }
  else
    {
      status_label->set_text("");
      accept_button->set_sensitive(true);
    }
}
