//  Copyright (C) 2009, 2014 Ben Asselstine
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

#include "ucompose.hpp"
#include "defs.h"
#include "hero.h"
#include "backpack-editor-dialog.h"
#include "Backpack.h"

HeroEditorDialog::HeroEditorDialog(Gtk::Window &parent, Hero *hero)
 : LwEditorDialog(parent, "hero-editor-dialog.ui")
{
  d_hero = hero;
    
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

void HeroEditorDialog::run()
{
  dialog->show_all();
  Backpack *original_backpack = new Backpack(*d_hero->getBackpack());
  int response = dialog->run();

  if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
      d_hero->setName(name_entry->get_text());
      if (male_radiobutton->get_active() == true)
	d_hero->setGender(Hero::MALE);
      else
	d_hero->setGender(Hero::FEMALE);
    }
  else
    {
      d_hero->getBackpack()->removeAllFromBackpack();
      d_hero->getBackpack()->add(original_backpack);
    }
}

void HeroEditorDialog::on_edit_backpack_clicked()
{
  BackpackEditorDialog d(*dialog, d_hero->getBackpack());
  d.run();
  return;
}
