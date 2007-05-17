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
#include <gtkmm/radiobutton.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>

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

    Gtk::RadioButton *radio;
    Gtk::Image *image;
    xml->get_widget("hero_image", image);
    if (hero->getGender() == Army::MALE)
      {
        image->property_file() = File::getMiscFile("various/recruit_male.png");
        xml->get_widget("hero_male", radio);
      }
    else
      {
        image->property_file() = File::getMiscFile("various/recruit_female.png");
        xml->get_widget("hero_female", radio);
      }
    radio->set_active(true);

    Gtk::RadioButton *male_radio_button;
    xml->get_widget("hero_male", male_radio_button);
    male_radio_button->signal_clicked().connect(
	    sigc::mem_fun(this, &HeroOfferDialog::on_male_clicked));
    Gtk::RadioButton *female_radio_button;
    xml->get_widget("hero_female", female_radio_button);
    male_radio_button->signal_clicked().connect(
	    sigc::mem_fun(this, &HeroOfferDialog::on_female_clicked));
    
    Gtk::Entry *entry;
    xml->get_widget("name", entry);
    entry->set_text(hero->getName());

    Gtk::Label *label;
    xml->get_widget("label", label);
    
    Glib::ustring s;
    if (gold > 0)
	s = String::ucompose(
	    ngettext("A hero in %2 wants to join you for %1 gold piece!",
		     "A hero in %2 wants to join you for %1 gold pieces!",
		     gold), gold, city->getName().c_str());
    else
	s = String::ucompose(_("A hero in %1 wants to join you!"), city->getName().c_str());
    label->set_text(s);
    
}

void HeroOfferDialog::on_male_clicked()
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/hero-offer-dialog.glade");

    Gtk::Image *image;
    xml->get_widget("hero_image", image);
  
    SDL_Surface *tmp = File::getMiscPicture("recruit_male.png");
    image->property_pixbuf() = to_pixbuf(tmp);
    dialog->show_all();
    SDL_FreeSurface(tmp);
}

void HeroOfferDialog::on_female_clicked()
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/hero-offer-dialog.glade");

    Gtk::Image *image;
    xml->get_widget("hero_image", image);
  
    SDL_Surface *tmp = File::getMiscPicture("recruit_female.png");
    image->property_pixbuf() = to_pixbuf(tmp);
    dialog->show_all();
    SDL_FreeSurface(tmp);
}

void HeroOfferDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

bool HeroOfferDialog::run()
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/hero-offer-dialog.glade");
    Gtk::Entry *entry;
    Gtk::RadioButton *radio;
    xml->get_widget("name", entry);
    entry->set_text(hero->getName());
    heromap->resize(GameMap::get_dim() * 2);
    heromap->draw();

    Sound::getInstance()->playMusic("hero", 1);
    dialog->show_all();
    int response = dialog->run();
    Sound::getInstance()->haltMusic();

    if (response == 0)		// accepted
      {
        hero->setName(entry->get_text());
        xml->get_widget("hero_male", radio);
        if (radio->get_active() == true)
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

