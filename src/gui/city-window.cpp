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
#include <gtkmm/image.h>
#include <sigc++/functors/mem_fun.h>

#include "city-window.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../army.h"
#include "../player.h"
#include "../city.h"
#include "../GraphicsCache.h"
#include "../armysetlist.h"
#include "buy-production-dialog.h"

CityWindow::CityWindow(City *c)
{
    city = c;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/city-window.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    
    xml->get_widget("city_label", city_label);
    xml->get_widget("status_label", status_label);
    xml->get_widget("production_info_label1", production_info_label1);
    xml->get_widget("production_info_label2", production_info_label2);
    xml->get_widget("buy_button", buy_button);
    xml->get_widget("on_hold_button", on_hold_button);
    on_hold_button->signal_clicked().connect(
	    sigc::mem_fun(this, &CityWindow::on_on_hold_clicked));
    buy_button->signal_clicked().connect(
	sigc::mem_fun(this, &CityWindow::on_buy_clicked));
    xml->connect_clicked(
	"destination_button",
	sigc::mem_fun(this, &CityWindow::on_destination_clicked));

    for (int i = 1; i <= city->getMaxNoOfBasicProd(); ++i) {
	Gtk::ToggleButton *toggle;
	xml->get_widget(String::ucompose("production_toggle%1", i), toggle);
	production_toggles.push_back(toggle);
	toggle->signal_toggled().connect(
	    sigc::bind(sigc::mem_fun(this, &CityWindow::on_production_toggled),
		       toggle));
    }

    fill_in_city_info();
    fill_in_production_toggles();

    ignore_toggles = false;
}

void CityWindow::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void CityWindow::run()
{
    dialog->show();
    dialog->run();
}

void CityWindow::fill_in_city_info()
{
    // fill in titles
    Glib::ustring title;
    switch (city->getDefenseLevel())
    {
    case 1: title = _("Village of %1"); break;
    case 2: title = _("Town of %1"); break;
    case 3: title = _("City of %1"); break;
    case 4: title = _("Fortress of %1"); break;
    }
    title = String::ucompose(title, city->getName());
    
    dialog->set_title(title);
    city_label->set_markup("<b>" + title + "</b>");

    // fill in status label
    Glib::ustring s;
    if (city->isCapital())
    {
	s += String::ucompose(_("Home city of %1"),
			      city->getPlayer()->getName());
	s += "\n";
    }

    s += String::ucompose(_("Defence: %1"), city->getDefenseLevel());
    s += "\n";
    s += String::ucompose(_("Income: %1"), city->getGold());
    status_label->set_text(s);
}

void CityWindow::fill_in_production_toggles()
{
    Player *player = city->getPlayer();
    unsigned int as = Armysetlist::getInstance()->getStandardId();
    int production_index = city->getProductionIndex();

    SDL_Surface *s
	= GraphicsCache::getInstance()->getArmyPic(as, 0, player, 1, NULL);
    Glib::RefPtr<Gdk::Pixbuf> empty_pic
	= Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, s->w, s->h);
    empty_pic->fill(0x00000000);
    
    ignore_toggles = true;
    for (int i = 0; i < city->getMaxNoOfBasicProd(); i++)
    {
	Gtk::ToggleButton *toggle = production_toggles[i];
	toggle->foreach(sigc::mem_fun(toggle, &Gtk::Container::remove));

	Glib::RefPtr<Gdk::Pixbuf> pic;
	int type = city->getArmytype(i);
        if (type != -1)
            // use GraphicsCache to load army pics because of player-specific
            // colors
            pic = to_pixbuf(GraphicsCache::getInstance()->getArmyPic(
				as, type, player, 1, NULL));
	else
	    pic = empty_pic;
	
	toggle->add(*manage(new Gtk::Image(pic)));

	toggle->set_active(i == production_index);
	toggle->show_all();
    }
    ignore_toggles = false;

    on_hold_button->set_sensitive(production_index != -1);
    fill_in_production_info();
    set_buy_button_state();
}

void CityWindow::on_production_toggled(Gtk::ToggleButton *toggle)
{
    if (ignore_toggles)
	return;
    
    int slot = -1;
    ignore_toggles = true;
    for (unsigned int i = 0; i < production_toggles.size(); ++i) {
	if (toggle == production_toggles[i])
	    slot = i;
	
	production_toggles[i]->set_active(toggle == production_toggles[i]);
    }
    ignore_toggles = false;

    bool is_empty = city->getArmytype(slot) == -1;
    
    if (is_empty)
	city->getPlayer()->cityChangeProduction(city, -1);
    else
	city->getPlayer()->cityChangeProduction(city, slot);

    on_hold_button->set_sensitive(!is_empty);
    
    fill_in_production_info();
    set_buy_button_state();
}

void CityWindow::fill_in_production_info()
{
    int slot = city->getProductionIndex();

    Glib::ustring s1, s2;
    
    if (slot == -1)
    {
	s1 = _("No production");
	s1 += "\n\n\n";
	s2 = "\n\n\n";
    }
    else
    {
        const Army* a = city->getArmy(slot);

	// fill in first column
	s1 += a->getName();
	s1 += "\n";
	// note to translators: %1/%2 is the no. of steps completed out of the
	// total no. of steps in the production
	s1 += String::ucompose(_("Duration: %1/%2"),
			      city->getDuration(), a->getProduction());
	s1 += "\n";
	// note to translators: %1 is melee strength, %2 is ranged strength
	s1 += String::ucompose(_("Attack: %1/%2"),
			      a->getStat(Army::STRENGTH, false),
			      a->getStat(Army::RANGED, false));
	s1 += "\n";
	s1 += String::ucompose(_("Defence: %1"),
			      a->getStat(Army::DEFENSE, false));
	
	// fill in second column
	s2 += "\n";
	s2 += String::ucompose(_("Moves: %1"), a->getStat(Army::MOVES, false));
	s2 += "\n";
	s2 += String::ucompose(_("Hitpoints: %1"), a->getStat(Army::HP, false));
	s2 += "\n";
	s2 += String::ucompose(_("Upkeep: %1"), a->getUpkeep());
	
    }
    
    production_info_label1->set_markup("<i>" + s1 + "</i>");
    production_info_label2->set_markup("<i>" + s2 + "</i>");
}

void CityWindow::set_buy_button_state()
{
    int selected_index = -1;
    bool selected_empty = false;
    bool one_empty = false;
    
    // loop through toggles, we only want buy to be sensitive if a slot is
    // selected and empty unless there are no empty slots left
    for (int i = 0; i < int(production_toggles.size()); ++i)
    {
	bool is_empty = city->getArmytype(i) == -1;

	if (is_empty)
	    one_empty = true;

	if (production_toggles[i]->get_active())
	{
	    selected_index = i;
	    if (is_empty)
		selected_empty = true;
	}
	
    }

    bool res = false;
    if (selected_index != -1)
    {
	if (selected_empty)
	    res = true;
	else if (selected_index < city->getMaxNoOfBasicProd() && !one_empty)
	    res = true;
    }

    buy_button->set_sensitive(res);
}

void CityWindow::on_on_hold_clicked()
{
    city->getPlayer()->cityChangeProduction(city, -1);
    on_hold_button->set_sensitive(false);
    ignore_toggles = true;
    for (unsigned int i = 0; i < production_toggles.size(); ++i)
	production_toggles[i]->set_active(false);
    ignore_toggles = false;
    fill_in_production_info();
    set_buy_button_state();
}

void CityWindow::on_buy_clicked()
{
    BuyProductionDialog d(city);
    d.set_parent_window(*dialog.get());
    d.run();

    int army = d.get_selected_army();
    if (army != BuyProductionDialog::NO_ARMY_SELECTED)
    {
	int slot = -1;
	for (unsigned int i = 0; i < production_toggles.size(); ++i)
	    if (production_toggles[i]->get_active()) {
		slot = i;
		break;
	    }
	
	city->getPlayer()->cityBuyProduction(city, slot, army);
	city->getPlayer()->cityChangeProduction(city, slot);

	fill_in_production_toggles();
	fill_in_production_info();

	set_buy_button_state();
    }
}

void CityWindow::on_destination_clicked()
{
}

