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

#include <assert.h>

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "city-window.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "army.h"
#include "armyprodbase.h"
#include "player.h"
#include "city.h"
#include "GraphicsCache.h"
#include "armysetlist.h"
#include "buy-production-dialog.h"
#include "destination-dialog.h"
#include "GameMap.h"
#include "citylist.h"
#include "playerlist.h"
#include "File.h"

CityWindow::CityWindow(City *c, bool razing_possible, 
		       bool see_opponents_production)
{
  army_info_tip = NULL;
    city = c;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/city-window.ui");

    xml->get_widget("dialog", dialog);
    decorate(dialog);
    window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
    set_title(c->getName());
    
    xml->get_widget("map_image", map_image);

    prodmap = new VectorMap(c, VectorMap::SHOW_ORIGIN_CITY_VECTORING,
		  see_opponents_production);
    prodmap->map_changed.connect(
	sigc::mem_fun(this, &CityWindow::on_map_changed));

    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);
    map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK);
    map_eventbox->signal_button_press_event().connect(
	sigc::mem_fun(*this, &CityWindow::on_map_mouse_button_event));
    xml->get_widget("status_label", status_label);
    xml->get_widget("turns_left_label", turns_left_label);
    xml->get_widget("current_label", current_label);
    xml->get_widget("current_image", current_image);
    xml->get_widget("production_info_label1", production_info_label1);
    xml->get_widget("production_info_label2", production_info_label2);
    xml->get_widget("buy_button", buy_button);
    xml->get_widget("on_hold_button", on_hold_button);
    on_hold_button->signal_clicked().connect(
	    sigc::mem_fun(this, &CityWindow::on_on_hold_clicked));
    buy_button->signal_clicked().connect(
	sigc::mem_fun(this, &CityWindow::on_buy_clicked));
    xml->get_widget("destination_button", destination_button);
    destination_button->signal_clicked().connect(
	sigc::mem_fun(this, &CityWindow::on_destination_clicked));
    xml->get_widget("rename_button", rename_button);
    rename_button->signal_clicked().connect(
	sigc::mem_fun(this, &CityWindow::on_rename_clicked));
    xml->get_widget("raze_button", raze_button);
    raze_button->signal_clicked().connect(
	sigc::mem_fun(this, &CityWindow::on_raze_clicked));

    xml->get_widget("production_toggles_hbox", production_toggles_hbox);
    for (unsigned int i = 1; i <= city->getMaxNoOfProductionBases(); ++i) {
	Gtk::ToggleButton *toggle = new Gtk::ToggleButton();
	production_toggles_hbox->pack_start(*manage(toggle), false, false, 0);
	production_toggles.push_back(toggle);
	toggle->signal_toggled().connect(
	    sigc::bind(sigc::mem_fun(this, &CityWindow::on_production_toggled),
		       toggle));
	toggle->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	toggle->signal_button_press_event().connect(
	    sigc::bind(sigc::mem_fun(*this, &CityWindow::on_production_button_event),
		       toggle), false);
	
	toggle->signal_button_release_event().connect(
	    sigc::bind(sigc::mem_fun(*this, &CityWindow::on_production_button_event),
		       toggle), false);
	
    }

    d_razing_possible = razing_possible;
    fill_in_city_info();
    fill_in_production_toggles();

    ignore_toggles = false;
}

CityWindow::~CityWindow()
{
  delete dialog;
  delete prodmap;
  if (army_info_tip != NULL)
    delete army_info_tip;
}

bool CityWindow::on_map_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS)
	return true;	// useless event
    
    prodmap->mouse_button_event(to_input_event(e));
    
    city = prodmap->getCity();
    fill_in_city_info();
    fill_in_production_toggles();
    fill_in_production_info();
    return true;
}

void CityWindow::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void CityWindow::hide()
{
  dialog->hide();
}

void CityWindow::run()
{
    prodmap->resize();
    prodmap->draw(Playerlist::getActiveplayer());
    dialog->show();
    dialog->run();
}

void CityWindow::fill_in_city_info()
{
    set_title(city->getName());

    // fill in status label
    Glib::ustring s;
    if (city->isCapital())
    {
	s += String::ucompose(_("Capital city of %1"),
			      city->getCapitalOwner()->getName());
	s += "\n";
    }

    s += String::ucompose(_("Defence: %1"), city->getDefenseLevel());
    s += "\n";
    s += String::ucompose(_("Income: %1"), city->getGold());
    status_label->set_text(s);
}

void CityWindow::fill_in_production_toggles()
{
    int production_index = city->getActiveProductionSlot();

    ignore_toggles = true;
    for (unsigned int i = 0; i < city->getMaxNoOfProductionBases(); i++)
    {
	Gtk::ToggleButton *toggle = production_toggles[i];
	toggle->foreach(sigc::mem_fun(toggle, &Gtk::Container::remove));
        update_toggle_picture(i);
	toggle->set_active((int)i == production_index);
	toggle->show_all();
    }
    ignore_toggles = false;

    on_hold_button->set_sensitive(production_index != -1);
    fill_in_production_info();
}

void CityWindow::update_toggle_picture(int slot)
{
  Player *player = city->getOwner();
  unsigned int as = player->getArmyset();
  Glib::RefPtr<Gdk::Pixbuf> pic;
  GraphicsCache *gc = GraphicsCache::getInstance();
  Gtk::ToggleButton *toggle = production_toggles[slot];
  if (city->getArmytype(slot) == -1)
    pic = gc->getCircledArmyPic(as, 0, player, NULL, false, 
                                Shield::NEUTRAL, false)->to_pixbuf();
  else
    {
      int type = city->getArmytype(slot);
      pic = gc->getCircledArmyPic (as, type, player, NULL, false,
                                   slot == city->getActiveProductionSlot() ? 
                                   player->getId(): Shield::NEUTRAL, 
                                   true)->to_pixbuf();
    }
  Gtk::Image *image = new Gtk::Image();
  image->property_pixbuf() = pic;
  toggle->remove();
  toggle->add(*manage(image));
  toggle->show_all();
}

void CityWindow::on_production_toggled(Gtk::ToggleButton *toggle)
{
    if (city->getOwner() != Playerlist::getActiveplayer())
    {
        toggle->set_active(false);
        return;
    }
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
	city->getOwner()->cityChangeProduction(city, -1);
    else
	city->getOwner()->cityChangeProduction(city, slot);

    on_hold_button->set_sensitive(!is_empty);
    
    for (unsigned int i = 0; i < production_toggles.size(); ++i) 
      update_toggle_picture(i);
    fill_in_production_info();
}

void CityWindow::fill_in_production_info()
{
    Player *player = city->getOwner();
    unsigned int as = player->getArmyset();
    Glib::RefPtr<Gdk::Pixbuf> pic;
    GraphicsCache *gc = GraphicsCache::getInstance();
    int slot = city->getActiveProductionSlot();
    //Glib::RefPtr<Gdk::Pixbuf> s
	//= GraphicsCache::getInstance()->getCircledArmyPic(as, 0, player, NULL, false, Shield::NEUTRAL, true)->to_pixbuf();
    Glib::RefPtr<Gdk::Pixbuf> empty_pic =
      GraphicsCache::getInstance()->getCircledArmyPic(as, 0, player, NULL, false, Shield::NEUTRAL, false)->to_pixbuf();
    
    Glib::ustring s1, s2, s3;
    Glib::ustring s4 = _("Current:");
    
    if (slot == -1)
    {
        pic = empty_pic;
	s1 = _("No production");
	s1 += "\n\n";
	s2 = "\n\n";
        s3 = "";
    }
    else
    {
        const ArmyProdBase * a = city->getProductionBase(slot);

	// fill in first column
	s1 += a->getName();
	s1 += "\n";
	// note to translators: %1/%2 is the no. of steps completed out of the
	// total no. of steps in the production
	s1 += String::ucompose(_("Time: %1"), a->getProduction());
	s1 += "\n";
	s1 += String::ucompose(_("Strength: %1"),
			      a->getStrength());
	
	// fill in second column
	s2 += "\n";
	s2 += String::ucompose(_("Moves: %1"), a->getMaxMoves());
	s2 += "\n";
	s2 += String::ucompose(_("Cost: %1"), a->getUpkeep());

        s3 = String::ucompose(_("%1t"), city->getDuration());
        if (city->getVectoring() != Vector<int>(-1, -1))
          {
            Citylist *cl = Citylist::getInstance();
            City *dest = cl->getNearestFriendlyCity(city->getVectoring(), 4);
            s3 += String::ucompose(_(", then to %1"), 
                                   dest ? dest->getName() : "Standard");
          }
      pic = gc->getCircledArmyPic(as, a->getTypeId(), player, NULL, false,
                                  Shield::NEUTRAL, true)->to_pixbuf();
    }
    
    current_image->property_pixbuf() = pic;
    production_info_label1->set_markup(s1);
    production_info_label2->set_markup(s2);
    turns_left_label->set_markup("<i>" + s3 + "</i>");
    current_label->set_markup("<i>" + s4 + "</i>");

    if (city->getOwner () != Playerlist::getActiveplayer())
      {
        turns_left_label->hide();
        current_label->hide();
        current_image->hide();
        buy_button->set_sensitive(false);
        raze_button->set_sensitive(false);
        rename_button->set_sensitive(false);
        destination_button->set_sensitive(false);
        on_hold_button->set_sensitive(false);
        for (unsigned int i = 0; i < production_toggles.size(); ++i) 
          {
           // production_toggles[i]->set_sensitive(false);
	    production_toggles[i]->set_active(false);
          }
        production_info_label1->hide();
        production_info_label2->hide();
      }
    else
      {
        turns_left_label->show();
        current_label->show();
        current_image->show();
        buy_button->set_sensitive(true);
        raze_button->set_sensitive (d_razing_possible);
        rename_button->set_sensitive(true);
        destination_button->set_sensitive(true);
        on_hold_button->set_sensitive(true);
        //for (unsigned int i = 0; i < production_toggles.size(); ++i) 
          //production_toggles[i]->set_sensitive(true);
        production_info_label1->show();
        production_info_label2->show();
      }
}

bool CityWindow::on_production_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle)
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

	const ArmyProdBase *prodbase = city->getProductionBase(slot);

	if (prodbase)
	  {
	    if (army_info_tip != NULL)
		delete army_info_tip;
	    army_info_tip = new ArmyInfoTip(toggle, prodbase, city);
	  }
	return true;
    }
    else if (event.button == MouseButtonEvent::RIGHT_BUTTON
	     && event.state == MouseButtonEvent::RELEASED) {
	{
	  if (army_info_tip != NULL)
	    {
	      delete army_info_tip;
	      army_info_tip = NULL;
	    }
	}
	return true;
    }
    
    return false;
}

void CityWindow::on_on_hold_clicked() //stop button
{
    city->setVectoring(Vector<int>(-1,-1));
    city->getOwner()->cityChangeProduction(city, -1);
    on_hold_button->set_sensitive(false);
    ignore_toggles = true;
    for (unsigned int i = 0; i < production_toggles.size(); ++i)
	production_toggles[i]->set_active(false);
    ignore_toggles = false;
    fill_in_production_info();
    prodmap->draw(Playerlist::getActiveplayer());
}

void CityWindow::on_buy_clicked()
{
    BuyProductionDialog d(city);
    d.set_parent_window(*dialog);
    d.run();

    int army = d.get_selected_army();
    d.hide();
    if (army != BuyProductionDialog::NO_ARMY_SELECTED)
    {
	int slot = -1;
	slot = city->getFreeSlot();
	
	if  (slot == -1)
	  {
	    //no free slots available.  change the one we're on.
	    slot = city->getActiveProductionSlot();
	    if (slot == -1) 
	      slot = 0;
	  }
	city->getOwner()->cityBuyProduction(city, slot, army);
	city->getOwner()->cityChangeProduction(city, slot);

	fill_in_production_toggles();
	fill_in_production_info();

    }
}

void CityWindow::on_destination_clicked()
{
    DestinationDialog d(city, &d_see_all);
    d.set_parent_window(*dialog);
    d.run();
    fill_in_production_info();
    prodmap->draw(Playerlist::getActiveplayer());
}

void CityWindow::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, prodmap->get_width(), prodmap->get_height());
  map_image->property_pixbuf() = pixbuf;
}

void CityWindow::on_rename_clicked ()
{
    Gtk::Dialog* subdialog;
    
    Glib::RefPtr<Gtk::Builder> renamexml
	= Gtk::Builder::create_from_file(get_glade_path() + "/city-rename-dialog.ui");
	
    renamexml->get_widget("dialog", subdialog);
    Decorated decorator;
    decorator.decorate(subdialog);
    decorator.window_closed.connect(sigc::mem_fun(subdialog, &Gtk::Dialog::hide));
    subdialog->set_transient_for(*dialog);
    
    Glib::ustring s = _("Rename City");

    Gtk::Label *l;
    renamexml->get_widget("label", l);
    Gtk::Entry *e;
    renamexml->get_widget("name_entry", e);

    decorator.set_title(s);
    s = _("Type the new name for this city:");
    l->set_text(s);

    e->set_text(city->getName());
    subdialog->show_all();
    int response = subdialog->run();

    if (response == Gtk::RESPONSE_ACCEPT)		// changed city name
      {
        if (String::utrim(e->get_text()) != "")
          Playerlist::getActiveplayer()->cityRename
            (city, String::utrim(e->get_text()));
        fill_in_city_info();
      }
    subdialog->hide();
    delete subdialog;
  return;
}

void CityWindow::on_raze_clicked ()
{
  on_raze_clicked (city, dialog);
}

bool CityWindow::on_raze_clicked (City *city, Gtk::Dialog *parent)
{
    Gtk::Dialog* subdialog;
    
    Glib::RefPtr<Gtk::Builder> razexml
	= Gtk::Builder::create_from_file(get_glade_path() + "/city-raze-dialog.ui");
	
    razexml->get_widget("dialog", subdialog);
    Decorated decorator;
    decorator.decorate(subdialog);
    decorator.window_closed.connect(sigc::mem_fun(subdialog, &Gtk::Dialog::hide));
    subdialog->set_transient_for(*parent);
    
    Glib::ustring s = _("Raze City");

    Gtk::Label *l;
    razexml->get_widget("label", l);

    decorator.set_title(s);
    s = String::ucompose(_("Are you sure that you want to raze %1?"), 
                           city->getName());
    s += "\n";
    s += _("You won't be popular!");
    l->set_text(s);

    subdialog->show_all();
    int response = subdialog->run();
    subdialog->hide();
    delete subdialog;

    if (response == Gtk::RESPONSE_ACCEPT) // burn it to the ground ralphie boy!
      {
	Playerlist::getActiveplayer()->cityRaze(city);
	parent->hide();
	return true;
      }
  return false;
}
