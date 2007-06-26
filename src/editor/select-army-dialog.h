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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef SELECT_ARMY_DIALOG_H
#define SELECT_ARMY_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/button.h>
#include <gtkmm/comboboxtext.h>
#include <SDL/SDL_types.h>

#include "../gui/army-info-tip.h"

class Army;
class City;

// dialog for buying a production slot for a city
class SelectArmyDialog: public sigc::trackable
{
 public:
    SelectArmyDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();

    const Army *get_selected_army() { return selected_army; }
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    std::auto_ptr<ArmyInfoTip> army_info_tip;
    Gtk::ComboBoxText *armyset_combobox;
    Gtk::Label *army_info_label1;
    Gtk::Label *army_info_label2;
    Gtk::Table *toggles_table;
    Gtk::Button *select_button;
    std::vector<Uint32> armysets;

    const Army *selected_army;

    std::vector<Gtk::ToggleButton *> army_toggles;
    bool ignore_toggles;
    std::vector<const Army*> selectable;

    void on_army_toggled(Gtk::ToggleButton *toggle);
    bool on_army_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle);
    void on_armyset_changed();
    
    void fill_in_army_toggles();
    void fill_in_army_info();
    void set_select_button_state();
};

#endif
