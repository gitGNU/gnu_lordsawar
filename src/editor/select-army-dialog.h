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

#ifndef SELECT_ARMY_DIALOG_H
#define SELECT_ARMY_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include <SDL/SDL_types.h>

#include "gui/army-info-tip.h"

class ArmyProto;
class City;
class Player;

//! Scenario editor.  Select an Army prototype.
class SelectArmyDialog: public sigc::trackable
{
 public:
    SelectArmyDialog(Player *p, bool defends_ruins = false,
		     bool rewardable = false);

    void set_parent_window(Gtk::Window &parent);

    void run();

    const ArmyProto *get_selected_army() { return selected_army; }
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    std::auto_ptr<ArmyInfoTip> army_info_tip;
    Gtk::Label *army_info_label1;
    Gtk::Label *army_info_label2;
    Gtk::Table *toggles_table;
    Gtk::Button *select_button;
    std::vector<Uint32> armysets;

    const ArmyProto *selected_army;
    Player *player;
    bool d_defends_ruins;
    bool d_awardable;

    std::vector<Gtk::ToggleButton *> army_toggles;
    bool ignore_toggles;
    std::vector<const ArmyProto*> selectable;

    void on_army_toggled(Gtk::ToggleButton *toggle);
    bool on_army_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle);
    
    void fill_in_army_toggles();
    void fill_in_army_info();
    void set_select_button_state();
};

#endif
