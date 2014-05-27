//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2014 Ben Asselstine
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

#include "army-gains-level-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "army.h"
#include "GraphicsCache.h"
#include "hero.h"
#include "shield.h"

//give a hero some more abilities
ArmyGainsLevelDialog::ArmyGainsLevelDialog(Gtk::Window &parent, Hero *a, bool show_sight_stat)
{
    GraphicsCache *gc = GraphicsCache::getInstance();
    hero = a;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/army-gains-level-dialog.ui");

    xml->get_widget("dialog", dialog);
    dialog->set_transient_for(parent);
    Gtk::Image *image;
    xml->get_widget("image", image);
    image->property_pixbuf() = 
      gc->getCircledArmyPic(hero, false, Shield::NEUTRAL, true)->to_pixbuf();
    Gtk::Image *hero_image;
    xml->get_widget("hero_image", hero_image);
    hero_image->property_pixbuf() = 
      gc->getNewLevelPic(Playerlist::getActiveplayer(),
			 dynamic_cast<Hero*>(a)->getGender())->to_pixbuf();
    
    Gtk::Label *label;
    xml->get_widget("label", label);
    Glib::ustring s;
    s = String::ucompose(_("%1 has advanced to level %2!"), a->getName(),
			 a->getLevel() + 1);
    dialog->set_title(s);
    s += "\n\n";
    s += _("Choose an attribute to improve:");
    label->set_text(s);

    xml->get_widget("stats_vbox", stats_vbox);

    add_item(Army::MOVES, _("Moves: %1"));
    if (show_sight_stat == true)
      add_item(Army::SIGHT, _("Sight: %1"));
    if (a->getStat(Army::STRENGTH, false) < MAX_ARMY_STRENGTH)
      add_item(Army::STRENGTH, _("Strength: %1"));

    stat_items[0].radio->set_active(true);
    on_stat_toggled();
}

ArmyGainsLevelDialog::~ArmyGainsLevelDialog()
{
  delete dialog;
}

void ArmyGainsLevelDialog::hide()
{
  dialog->hide();
}

void ArmyGainsLevelDialog::run()
{
    dialog->show_all();
    dialog->run();
}

void ArmyGainsLevelDialog::add_item(Army::Stat stat, Glib::ustring desc)
{
    StatItem item;
    item.stat = stat;
    item.desc = desc;
    if (stat_items.empty())
	item.radio = manage(new Gtk::RadioButton);
    else
    {
	Gtk::RadioButton::Group group = stat_items[0].radio->get_group();
	item.radio = manage(new Gtk::RadioButton(group));
    }

    stat_items.push_back(item);

    item.radio->signal_toggled().connect(
	sigc::mem_fun(this, &ArmyGainsLevelDialog::on_stat_toggled));

    stats_vbox->pack_start(*item.radio, Gtk::PACK_SHRINK);
}

void ArmyGainsLevelDialog::on_stat_toggled()
{
    for (unsigned int i = 0; i < stat_items.size(); ++i)
	if (stat_items[i].radio->get_active())
	{
	    selected_stat = stat_items[i].stat;
	    break;
	}
    	    
    fill_in_descriptions();
}

void ArmyGainsLevelDialog::fill_in_descriptions()
{
    for (unsigned int i = 0; i < stat_items.size(); ++i)
    {
	StatItem &item = stat_items[i];

	int v = hero->getStat(item.stat, false);

	if (item.radio->get_active())
	    v += hero->computeLevelGain(item.stat);
	    
	item.radio->set_label(String::ucompose(item.desc, v));
    }
}
