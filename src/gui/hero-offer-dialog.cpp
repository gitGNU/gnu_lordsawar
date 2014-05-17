//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2012 Ben Asselstine
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

#include "hero-offer-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "GameMap.h"
#include "File.h"
#include "sound.h"
#include "city.h"
#include "ucompose.hpp"

HeroOfferDialog::HeroOfferDialog(Player *player, HeroProto *h, City *c, int gold)
{
    city = c;
    hero = h;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/hero-offer-dialog.ui");

    xml->get_widget("dialog", dialog);

    decorate(dialog);
    window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));

    xml->get_widget("map_image", map_image);

    heromap = new HeroMap(city);
    heromap->map_changed.connect(
	sigc::mem_fun(this, &HeroOfferDialog::on_map_changed));

    set_title(String::ucompose(_("A Hero for %1"), player->getName()));

    xml->get_widget("hero_image", hero_image);
    xml->get_widget("hero_male", male_radiobutton);
    male_radiobutton->signal_clicked().connect(
	sigc::mem_fun(this, &HeroOfferDialog::on_male_toggled));
    
    male_radiobutton->set_active(hero->getGender() == Hero::MALE);
    on_male_toggled();
    
    xml->get_widget("name", name_entry);
    name_entry->set_text(hero->getName());
    name_entry->signal_changed().connect
      (sigc::mem_fun(*this, &HeroOfferDialog::on_name_changed));

    xml->get_widget("accept_button", accept_button);
    Gtk::Label *label;
    xml->get_widget("label", label);
    
    Glib::ustring s;
    if (gold > 0)
	s = String::ucompose(
	    ngettext("A hero in %2 wants to join you for %1 gold piece!",
		     "A hero in %2 wants to join you for %1 gold pieces!",
		     gold), gold, city->getName());
    else
	s = String::ucompose(_("A hero in %1 wants to join you!"), city->getName());
    label->set_text(s);
    update_buttons();
}

HeroOfferDialog::~HeroOfferDialog()
{
  delete heromap;
  delete dialog;
}

void HeroOfferDialog::update_buttons()
{
  if (String::utrim(name_entry->get_text()) == "")
    accept_button->set_sensitive(false);
  else
    {
      accept_button->set_sensitive(true);
      accept_button->property_can_focus() = true;
      accept_button->property_can_default() = true;
      accept_button->property_has_default() = true;
      name_entry->property_activates_default() = true;
      accept_button->property_receives_default() = true;
    }
}

void HeroOfferDialog::on_name_changed()
{
  update_buttons();
}

void HeroOfferDialog::on_male_toggled()
{
    if (male_radiobutton->get_active())
	hero_image->property_file()
	    = File::getMiscFile("various/recruit_male.png");
    else
	hero_image->property_file()
	    = File::getMiscFile("various/recruit_female.png");
}

void HeroOfferDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void HeroOfferDialog::hide()
{
  dialog->hide();
}

bool HeroOfferDialog::run()
{
    heromap->resize();
    heromap->draw(Playerlist::getActiveplayer());

    Sound::getInstance()->playMusic("hero", 1);
    dialog->show_all();
    int response = dialog->run();
    Sound::getInstance()->haltMusic();

    if (response == Gtk::RESPONSE_ACCEPT)	// accepted
      {
        hero->setName(String::utrim(name_entry->get_text()));
        if (male_radiobutton->get_active() == true && 
	    hero->getGender() == Hero::FEMALE)
          hero->setGender(Hero::MALE);
        else if (male_radiobutton->get_active() == false &&
		 hero->getGender() == Hero::MALE)
	  hero->setGender(Hero::FEMALE);
	return true;
      }
    else
	return false;
}

void HeroOfferDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, heromap->get_width(), heromap->get_height());
    map_image->property_pixbuf() = pixbuf;
}

