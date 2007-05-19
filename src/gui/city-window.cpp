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
#include <gtkmm/eventbox.h>
#include <gtkmm/entry.h>
#include <sigc++/functors/mem_fun.h>

#include "city-window.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../army.h"
#include "../player.h"
#include "../city.h"
#include "../GraphicsCache.h"
#include "../armysetlist.h"
#include "buy-production-dialog.h"
#include "destination-dialog.h"
#include "../GameMap.h"
#include "../citylist.h"
#include "../playerlist.h"

CityWindow::CityWindow(City *c)
{
    city = c;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/city-window.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    
    xml->get_widget("map_image", map_image);

    prodmap.reset(new VectorMap(c, VectorMap::SHOW_NO_VECTORING));
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
    xml->connect_clicked(
	"destination_button",
	sigc::mem_fun(this, &CityWindow::on_destination_clicked));
    xml->connect_clicked(
	"rename_button",
	sigc::mem_fun(this, &CityWindow::on_rename_clicked));
    xml->connect_clicked(
	"raze_button",
	sigc::mem_fun(this, &CityWindow::on_raze_clicked));

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

bool CityWindow::on_map_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS)
	return true;	// useless event
    
    prodmap->mouse_button_event(to_input_event(e));
    
    city = prodmap->getCity();
    fill_in_city_info();
    fill_in_production_info();
    return true;
}

void CityWindow::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void CityWindow::run()
{
    prodmap->resize(GameMap::get_dim() * 2);
    prodmap->draw();
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
    int type;
    Glib::RefPtr<Gdk::Pixbuf> pic;
    GraphicsCache *gc = GraphicsCache::getInstance();

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

	type = city->getArmytype(i);
        if (type != -1)
            // use GraphicsCache to load army pics because of player-specific
            // colors
            pic = to_pixbuf(gc->getArmyPic(as, type, player, 1, NULL));
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
    Player *player = city->getPlayer();
    unsigned int as = Armysetlist::getInstance()->getStandardId();
    Glib::RefPtr<Gdk::Pixbuf> pic;
    GraphicsCache *gc = GraphicsCache::getInstance();
    int slot = city->getProductionIndex();
    SDL_Surface *s
	= GraphicsCache::getInstance()->getArmyPic(as, 0, player, 1, NULL);
    Glib::RefPtr<Gdk::Pixbuf> empty_pic
	= Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, s->w, s->h);
    empty_pic->fill(0x00000000);
    


    Glib::ustring s1, s2, s3;
    Glib::ustring s4 = _("Current:");
    
    if (slot == -1)
    {
        pic = empty_pic;
	s1 = _("No production");
	s1 += "\n\n\n";
	s2 = "\n\n\n";
        s3 = "";
    }
    else
    {
        const Army* a = city->getArmy(slot);

	// fill in first column
	s1 += a->getName();
	s1 += "\n";
	// note to translators: %1/%2 is the no. of steps completed out of the
	// total no. of steps in the production
	s1 += String::ucompose(_("Time: %1"), a->getProduction());
	s1 += "\n";
	s1 += String::ucompose(_("Strength: %1"),
			      a->getStat(Army::STRENGTH, false));
	
	// fill in second column
	s2 += "\n";
	s2 += String::ucompose(_("Moves: %1"), a->getStat(Army::MOVES, false));
	s2 += "\n";
	s2 += String::ucompose(_("Cost: %1"), a->getUpkeep());

        s3 = String::ucompose(_("%1t"), a->getProduction() - 
                              city->getDuration() + 1);
        if (city->getVectoring() != Vector<int>(-1, -1))
          {
            Citylist *cl = Citylist::getInstance();
            City *dest = cl->getObjectAt(city->getVectoring());
            s3 += String::ucompose(_(", then to %1"), dest->getName());
          }
      pic = to_pixbuf(gc->getArmyPic(as, slot, player, 1, NULL));
    }
    
    current_image->set(pic);
    production_info_label1->set_markup(s1);
    production_info_label2->set_markup(s2);
    turns_left_label->set_markup("<i>" + s3 + "</i>");
    current_label->set_markup("<i>" + s4 + "</i>");
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

void CityWindow::on_on_hold_clicked() //stop button
{
    city->setVectoring(Vector<int>(-1,-1));
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
    DestinationDialog d(city);
    d.set_parent_window(*dialog.get());
    d.run();
}

void CityWindow::on_map_changed(SDL_Surface *map)
{
    map_image->property_pixbuf() = to_pixbuf(map);
}

void CityWindow::on_rename_clicked ()
{
    std::auto_ptr<Gtk::Dialog> renamedialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> renamexml
	= Gnome::Glade::Xml::create(get_glade_path() + "/city-rename-dialog.glade");
	
    Gtk::Dialog *d;
    renamexml->get_widget("dialog", d);
    renamedialog.reset(d);
    renamedialog->set_transient_for(*dialog.get());
    
    Glib::ustring s = _("Rename City");

    Gtk::Label *l;
    renamexml->get_widget("label", l);
    Gtk::Entry *e;
    renamexml->get_widget("name_entry", e);

    d->set_title(s);
    s = _("Type the new name for this city:");
    l->set_text(s);

    e->set_text(city->getName());
    d->show_all();
    int response = d->run();

    if (response == 0)		// changed city name
      {
        city->setName(e->get_text());
        fill_in_city_info();
      }
  return;
}

void CityWindow::on_raze_clicked ()
{
    std::auto_ptr<Gtk::Dialog> razedialog;
    
    Glib::RefPtr<Gnome::Glade::Xml> razexml
	= Gnome::Glade::Xml::create(get_glade_path() + "/city-raze-dialog.glade");
	
    Gtk::Dialog *d;
    razexml->get_widget("dialog", d);
    razedialog.reset(d);
    razedialog->set_transient_for(*dialog.get());
    
    Glib::ustring s = _("Raze City");

    Gtk::Label *l;
    razexml->get_widget("label", l);

    d->set_title(s);
    s = String::ucompose(_("Are you sure that you want to raze %1?"), 
                           city->getName());
    s += "\n";
    s += _("You won't be popular!");
    l->set_text(s);

    d->show_all();
    int response = d->run();

    if (response == 0)		// burn it to the ground ralphie boy!
      {
        Playerlist *pl = Playerlist::getInstance();
        Player *p = pl->getActiveplayer();
        p->cityRaze(city);
dialog->hide();
      }
  return;
}
