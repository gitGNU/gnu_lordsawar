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
#include <gtkmm/eventbox.h>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm/label.h>

#include "hero-offer-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../GameMap.h"
#include "../File.h"
#include "../sound.h"
#include "../city.h"

HeroOfferDialog::HeroOfferDialog(Player *player, Hero *h, City *c, int gold)
{
    city = c;
    hero = h;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/hero-offer-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("map_image", map_image);

    heromap.reset(new HeroMap(city));
    heromap->map_changed.connect(
	sigc::mem_fun(this, &HeroOfferDialog::on_map_changed));

    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);

    dialog->set_title(String::ucompose(_("Hero offer for %1"),
				       player->getName()));

    xml->get_widget("hero_image", hero_image);
    xml->get_widget("hero_male", male_radiobutton);
    male_radiobutton->signal_clicked().connect(
	sigc::mem_fun(this, &HeroOfferDialog::on_male_toggled));
    
    male_radiobutton->set_active(hero->getGender() == Army::MALE);
    on_male_toggled();
    
    xml->get_widget("name", name_entry);
    name_entry->set_text(hero->getName());

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

bool HeroOfferDialog::run()
{
    heromap->resize(GameMap::get_dim() * 2);
    heromap->draw();

    Sound::getInstance()->playMusic("hero", 1);
    dialog->show_all();
    int response = dialog->run();
    Sound::getInstance()->haltMusic();

    if (response == 0)		// accepted
      {
        hero->setName(name_entry->get_text());
        if (male_radiobutton->get_active())
          hero->setGender(Hero::MALE);
        else
          hero->setGender(Hero::FEMALE);
	return true;
      }
    else
	return false;
}

void HeroOfferDialog::on_map_changed(SDL_Surface *map)
{
    map_image->property_pixbuf() = to_pixbuf(map);
}

