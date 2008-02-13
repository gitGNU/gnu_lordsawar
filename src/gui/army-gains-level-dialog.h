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

#ifndef ARMY_GAINS_LEVEL_DIALOG_H
#define ARMY_GAINS_LEVEL_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/radiobutton.h>

#include "../army.h"

// dialog for choosing what army stat to boost when a level is gained
class ArmyGainsLevelDialog: public sigc::trackable
{
 public:
    ArmyGainsLevelDialog(Army *army, bool show_sight_stat);

    void set_parent_window(Gtk::Window &parent);

    void run();

    Army::Stat get_selected_stat() { return selected_stat; }
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::VBox *stats_vbox;

    Army *army;
    Army::Stat selected_stat;

    struct StatItem
    {
	Army::Stat stat;
	Glib::ustring desc;
	Gtk::RadioButton *radio;
    };
    
    std::vector<StatItem> stat_items;

    void add_item(Army::Stat stat, Glib::ustring desc);
    void on_stat_toggled();
    void fill_in_descriptions();
};

#endif
