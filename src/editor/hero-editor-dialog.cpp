//  Copyright (C) 2009 Ben Asselstine
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

#include "hero-editor-dialog.h"

#include "glade-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "hero.h"
#include "backpack-editor-dialog.h"

HeroEditorDialog::HeroEditorDialog(Hero *hero)
{
  d_hero = hero;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/hero-editor-dialog.ui");

    xml->get_widget("dialog", dialog);
    
    xml->get_widget("edit_backpack_button", edit_backpack_button);
    edit_backpack_button->signal_clicked().connect(
	sigc::mem_fun(this, &HeroEditorDialog::on_edit_backpack_clicked));
    xml->get_widget("male_radiobutton", male_radiobutton);
    xml->get_widget("female_radiobutton", female_radiobutton);
    xml->get_widget("name_entry", name_entry);
    name_entry->set_text(d_hero->getName());
    if (d_hero->getGender() == Hero::FEMALE)
      female_radiobutton->set_active(true);
    else
      male_radiobutton->set_active(true);
}

HeroEditorDialog::~HeroEditorDialog()
{
  delete dialog;
}
void HeroEditorDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void HeroEditorDialog::run()
{
  dialog->show_all();
  int response = dialog->run();

  if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
      d_hero->setName(name_entry->get_text());
      if (male_radiobutton->get_active() == true)
	d_hero->setGender(Hero::MALE);
      else
	d_hero->setGender(Hero::FEMALE);
    }
}

void HeroEditorDialog::on_edit_backpack_clicked()
{
  BackpackEditorDialog d(d_hero->getBackpack());
  d.run();
}
