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
#include <gtkmm/table.h>
#include <gtkmm/alignment.h>
#include <sigc++/functors/mem_fun.h>
#include <assert.h>

#include "select-army-dialog.h"

#include "glade-helpers.h"
#include "../gui/image-helpers.h"
#include "../gui/input-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../army.h"
#include "../GraphicsCache.h"
#include "../armysetlist.h"

SelectArmyDialog::SelectArmyDialog(Player *p)
{
    player = p;
    selected_army = 0;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/select-army-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    
    xml->get_widget("army_info_label1", army_info_label1);
    xml->get_widget("army_info_label2", army_info_label2);
    xml->get_widget("select_button", select_button);

    xml->get_widget("army_toggles_table", toggles_table);

    // setup the armyset combo
    armyset_combobox = manage(new Gtk::ComboBoxText);

    armysets = Armysetlist::getInstance()->getArmysets();
    for (std::vector<Uint32>::iterator i = armysets.begin(),
	     end = armysets.end(); i != end; ++i)
	armyset_combobox->append_text(Armysetlist::getInstance()->getName(*i));

    Gtk::Alignment *alignment;
    xml->get_widget("armyset_alignment", alignment);
    alignment->add(*armyset_combobox);

    armyset_combobox->signal_changed().connect(
	sigc::mem_fun(this, &SelectArmyDialog::on_armyset_changed));
    armyset_combobox->set_active(0);
}

void SelectArmyDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void SelectArmyDialog::run()
{
    dialog->show_all();
    int response = dialog->run();

    if (response != 1)
	selected_army = 0;
}

void SelectArmyDialog::on_army_toggled(Gtk::ToggleButton *toggle)
{
    if (ignore_toggles)
	return;
    
    selected_army = 0;
    ignore_toggles = true;
    for (unsigned int i = 0; i < army_toggles.size(); ++i) {
	if (toggle == army_toggles[i])
	    selected_army = selectable[i];
	
	army_toggles[i]->set_active(toggle == army_toggles[i]);
    }
    ignore_toggles = false;

    fill_in_army_info();
    set_select_button_state();
}

void SelectArmyDialog::fill_in_army_toggles()
{
    const Armysetlist* al = Armysetlist::getInstance();

    Uint32 armyset = armysets[armyset_combobox->get_active_row_number()];

    // fill in selectable armies
    selectable.clear();
    for (unsigned int j = 0; j < al->getSize(armyset); j++)
    {
	const Army *a = al->getArmy(armyset, j);
	selectable.push_back(a);
    }

    // fill in army options
    army_toggles.clear();
    toggles_table->foreach(sigc::mem_fun(toggles_table, &Gtk::Container::remove));
    toggles_table->resize(1, 1);
    const int no_columns = 4;
    for (unsigned int i = 0; i < selectable.size(); ++i)
    {
	Gtk::ToggleButton *toggle = manage(new Gtk::ToggleButton);
	
	Glib::RefPtr<Gdk::Pixbuf> pixbuf
	    = to_pixbuf(GraphicsCache::getInstance()->getArmyPic(armyset,
                                       selectable[i]->getType(),
                                       player, NULL));
	
	toggle->add(*manage(new Gtk::Image(pixbuf)));
	army_toggles.push_back(toggle);
	int x = i % no_columns;
	int y = i / no_columns;
	toggles_table->attach(*toggle, x, x + 1, y, y + 1,
			      Gtk::SHRINK, Gtk::SHRINK);
	toggle->show_all();

	toggle->signal_toggled().connect(
	    sigc::bind(sigc::mem_fun(this, &SelectArmyDialog::on_army_toggled),
		       toggle));
	toggle->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	toggle->signal_button_press_event().connect(
	    sigc::bind(sigc::mem_fun(*this, &SelectArmyDialog::on_army_button_event),
		       toggle), false);
	
	toggle->signal_button_release_event().connect(
	    sigc::bind(sigc::mem_fun(*this, &SelectArmyDialog::on_army_button_event),
		       toggle), false);
    }

    ignore_toggles = false;
    if (!army_toggles.empty())
	army_toggles[0]->set_active(true);
}

void SelectArmyDialog::fill_in_army_info()
{
    Glib::ustring s1, s2;
    
    if (!selected_army)
    {
	s1 = _("No army");
	s1 += "\n\n\n";
	s2 = "\n\n\n";
    }
    else
    {
	const Army *a = selected_army;

	// fill in first column
	s1 += a->getName();
	s1 += "\n";
	s1 += String::ucompose(_("Strength: %1"),
			      a->getStat(Army::STRENGTH, false));
	s1 += "\n";
	s1 += String::ucompose(_("Moves: %1"), a->getStat(Army::MOVES, false));
	
	// fill in second column
	s2 += "\n";
	s2 += String::ucompose(_("Hitpoints: %1"), a->getStat(Army::HP, false));
	s2 += "\n";
	s2 += String::ucompose(_("Upkeep: %1"), a->getUpkeep());
    }
    
    army_info_label1->set_markup("<i>" + s1 + "</i>");
    army_info_label2->set_markup("<i>" + s2 + "</i>");
}

void SelectArmyDialog::set_select_button_state()
{
    select_button->set_sensitive(selected_army);
}

bool SelectArmyDialog::on_army_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle)
{
    MouseButtonEvent event = to_input_event(e);
    if (event.button == MouseButtonEvent::RIGHT_BUTTON
	&& event.state == MouseButtonEvent::PRESSED) {
	int slot = -1;
	for (unsigned int i = 0; i < army_toggles.size(); ++i) {
	    if (toggle == army_toggles[i])
		slot = i;
	}
	assert(slot != -1);

	const Army *army = selectable[slot];

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

void SelectArmyDialog::on_armyset_changed()
{
    fill_in_army_toggles();
}
