//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008 Ben Asselstine
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#ifndef BUY_PRODUCTION_DIALOG_H
#define BUY_PRODUCTION_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/button.h>

#include "army-info-tip.h"

class Army;
class City;

#include "decorated.h"
// dialog for buying a production slot for a city
class BuyProductionDialog: public Decorated
{
 public:
    BuyProductionDialog(City *city);

    void set_parent_window(Gtk::Window &parent);

    void run();
    void hide();

    enum { NO_ARMY_SELECTED = -1 };
    int get_selected_army() { return selected_army; }
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    std::auto_ptr<ArmyInfoTip> army_info_tip;
    Gtk::Label *production_info_label1;
    Gtk::Label *production_info_label2;
    Gtk::Button *buy_button;

    City *city;
    int selected_army;

    std::vector<Gtk::ToggleButton *> production_toggles;
    bool ignore_toggles;
    std::vector<const Army*> purchasables;

    void on_production_toggled(Gtk::ToggleButton *toggle);
    bool on_production_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle);
    void fill_in_production_info();
    void set_buy_button_state();
    const Army *army_id_to_army();
};

#endif
