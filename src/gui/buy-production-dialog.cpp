//  Copyright (C) 2007 Ole Laursen
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
#include <assert.h>

#include "buy-production-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "army.h"
#include "city.h"
#include "GraphicsCache.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "File.h"

BuyProductionDialog::BuyProductionDialog(City *c)
{
    GraphicsCache *gc = GraphicsCache::getInstance();
    city = c;
    selected_army = NO_ARMY_SELECTED;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/buy-production-dialog.ui");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    decorate(dialog.get());
    window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
    d->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
    
    xml->get_widget("production_info_label1", production_info_label1);
    xml->get_widget("production_info_label2", production_info_label2);
    xml->get_widget("buy_button", buy_button);

    Gtk::Table *toggles_table; 
    xml->get_widget("production_toggles_table", toggles_table);
    
    const Armysetlist* al = Armysetlist::getInstance();

    Player *p = Playerlist::getInstance()->getActiveplayer();
    // fill in purchasable armies
    for (unsigned int j = 0; j < al->getSize(p->getArmyset()); j++)
      {
        const ArmyProto *a = al->getArmy (p->getArmyset(), j);
        if (a->getProductionCost() > 0)
          purchasables.push_back(a);
      }

    // fill in production options
    const int no_columns = 4;
    for (unsigned int i = 0; i < purchasables.size(); ++i)
    {
	Gtk::ToggleButton *toggle = manage(new Gtk::ToggleButton);
	
	Glib::RefPtr<Gdk::Pixbuf> pix
	    = gc->getArmyPic(p->getArmyset(), purchasables[i]->getTypeId(), p, 
			     NULL)->to_pixbuf();
	
	Gtk::Image *image = new Gtk::Image();
	image->property_pixbuf() = pix;
	toggle->add(*manage(image));
	production_toggles.push_back(toggle);
	int x = i % no_columns;
	int y = i / no_columns;
	toggles_table->attach(*toggle, x, x + 1, y, y + 1,
			      Gtk::SHRINK, Gtk::SHRINK);
	toggle->show_all();

	toggle->signal_toggled().connect(
	    sigc::bind(sigc::mem_fun(this, &BuyProductionDialog::on_production_toggled),
		       toggle));
	toggle->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	toggle->signal_button_press_event().connect(
	    sigc::bind(sigc::mem_fun(*this, &BuyProductionDialog::on_production_button_event),
		       toggle), false);
	
	toggle->signal_button_release_event().connect(
	    sigc::bind(sigc::mem_fun(*this, &BuyProductionDialog::on_production_button_event),
		       toggle), false);
	toggle->set_sensitive
	  (city->hasProductionBase(purchasables[i]) == false);
	if ((int)purchasables[i]->getProductionCost() > c->getOwner()->getGold())
	  toggle->set_sensitive (false);
    }

    ignore_toggles = false;
    production_toggles[0]->set_active(true);
}

void BuyProductionDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void BuyProductionDialog::hide()
{
  dialog->hide();
}

void BuyProductionDialog::run()
{
    dialog->show();
    int response = dialog->run();

    if (response != Gtk::RESPONSE_ACCEPT)
	selected_army = NO_ARMY_SELECTED;
}

void BuyProductionDialog::on_production_toggled(Gtk::ToggleButton *toggle)
{
    if (ignore_toggles)
	return;
    
    selected_army = NO_ARMY_SELECTED;
    ignore_toggles = true;
    for (unsigned int i = 0; i < production_toggles.size(); ++i) {
	if (toggle == production_toggles[i])
	    selected_army = i;
	
	production_toggles[i]->set_active(toggle == production_toggles[i]);
    }
    ignore_toggles = false;

    fill_in_production_info();
    set_buy_button_state();
}

void BuyProductionDialog::fill_in_production_info()
{
    Glib::ustring s1, s2;
    
    if (selected_army == -1)
    {
	s1 = _("No production");
	s1 += "\n\n\n";
	s2 = "\n\n\n";
    }
    else
    {
	const ArmyProto *a = army_id_to_army();

	// fill in first column
	s1 += a->getName();
	s1 += "\n";
	s1 += String::ucompose(_("Duration: %1"), a->getProduction());
	s1 += "\n";
	// note to translators: %1 is melee strength, %2 is ranged strength
	s1 += String::ucompose(_("Strength: %1"),
			      a->getStrength());
	
	// fill in second column
	s2 += String::ucompose(_("Cost: %1"), a->getProductionCost());
	s2 += "\n";
	s2 += String::ucompose(_("Moves: %1"), a->getMaxMoves());
	s2 += "\n";
	s2 += String::ucompose(_("Upkeep: %1"), a->getUpkeep());
    }
    
    production_info_label1->set_markup("<i>" + s1 + "</i>");
    production_info_label2->set_markup("<i>" + s2 + "</i>");
}

void BuyProductionDialog::set_buy_button_state()
{
    bool can_buy = true;
    
    if (selected_army == NO_ARMY_SELECTED)
	can_buy = false;
    else
    {
	int gold = city->getOwner()->getGold();
	const ArmyProto *a = army_id_to_army();
	
	if (int(a->getProductionCost()) > gold ||
	    city->hasProductionBase(selected_army, 
				    city->getOwner()->getArmyset()))
	    can_buy = false;
    }
    
    buy_button->set_sensitive(can_buy);
}

const ArmyProto *BuyProductionDialog::army_id_to_army()
{
    return purchasables[selected_army];
}

bool BuyProductionDialog::on_production_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle)
{
    MouseButtonEvent event = to_input_event(e);
    if (event.button == MouseButtonEvent::RIGHT_BUTTON
	&& event.state == MouseButtonEvent::PRESSED) {
	int slot = -1;
	for (unsigned int i = 0; i < production_toggles.size(); ++i) {
	    if (toggle == production_toggles[i])
		slot = i;
	}
	assert(slot != -1);

	const ArmyProto *army = purchasables[slot];

	if (army)
	    army_info_tip.reset(new ArmyInfoTip(toggle, army));
	return true;
    }
    else if (event.button == MouseButtonEvent::RIGHT_BUTTON
	     && event.state == MouseButtonEvent::RELEASED) {
	army_info_tip.reset();
	return true;
    }
    
    return false;
}

