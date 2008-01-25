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

#ifndef GAME_OPTIONS_DIALOG_H
#define GAME_OPTIONS_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/combobox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/widget.h>

#include "../game-parameters.h"

// dialog for setting game options before the game starts
class GameOptionsDialog: public sigc::trackable
{
 public:
    GameOptionsDialog();

    void set_parent_window(Gtk::Window &parent);

    bool run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::CheckButton *quests_checkbutton;
    Gtk::CheckButton *view_enemies_checkbutton;
    Gtk::CheckButton *view_production_checkbutton;
    Gtk::CheckButton *hidden_map_checkbutton;
    Gtk::ComboBox *neutral_cities_combobox;
    Gtk::CheckButton *diplomacy_checkbutton;
    Gtk::CheckButton *intense_combat_checkbutton;
    Gtk::CheckButton *military_advisor_checkbutton;
    Gtk::CheckButton *random_turns_checkbutton;
    Gtk::CheckButton *quick_start_checkbutton;
    Gtk::CheckButton *cusp_of_war_checkbutton;
    Gtk::ToggleButton *beginner_toggle;
    Gtk::ToggleButton *intermediate_toggle;
    Gtk::ToggleButton *advanced_toggle;
    Gtk::ToggleButton *greatest_toggle;
    void on_beginner_toggled(Gtk::ToggleButton *toggle);
    void on_intermediate_toggled(Gtk::ToggleButton *toggle);
    void on_advanced_toggled(Gtk::ToggleButton *toggle);
    void on_greatest_toggled(Gtk::ToggleButton *toggle);
    void on_option_clicked();
    void on_diplomacy_clicked();
    void fill_in_options();

};

#endif
