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

#ifndef GAME_PREFERENCES_DIALOG_H
#define GAME_PREFERENCES_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <gtkmm/window.h>
#include <gtkmm/dialog.h>
#include <gtkmm/combobox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/widget.h>
#include <gtkmm/scale.h>

#include "../game-parameters.h"
#include "game-options-dialog.h"

class XML_Helper;

// dialog for choosing parameters for starting a new game
class GamePreferencesDialog
{
 public:
    GamePreferencesDialog();
    ~GamePreferencesDialog();

    void set_parent_window(Gtk::Window &parent);

    sigc::signal<void, GameParameters> game_started;
    
    void run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;

    Gtk::Button *start_game_button;
    Gtk::ComboBoxText *tile_theme_combobox;
    Gtk::ComboBoxText *army_theme_combobox;
    Gtk::ComboBoxText *shield_theme_combobox;
    Gtk::ComboBoxText *city_theme_combobox;
    Gtk::Label *difficulty_label;
    Gtk::RadioButton *random_map_radio;
    Gtk::FileChooserButton *load_map_filechooser;
    Gtk::Widget *random_map_container;
    Gtk::ComboBox *map_size_combobox;
    Gtk::Scale *grass_scale;
    Gtk::ToggleButton *grass_random_togglebutton;
    Gtk::Scale *water_scale;
    Gtk::ToggleButton *water_random_togglebutton;
    Gtk::Scale *swamp_scale;
    Gtk::ToggleButton *swamp_random_togglebutton;
    Gtk::Scale *forest_scale;
    Gtk::ToggleButton *forest_random_togglebutton;
    Gtk::Scale *hills_scale;
    Gtk::ToggleButton *hills_random_togglebutton;
    Gtk::Scale *mountains_scale;
    Gtk::ToggleButton *mountains_random_togglebutton;
    Gtk::Scale *cities_scale;
    Gtk::ToggleButton *cities_random_togglebutton;
    Gtk::Scale *ruins_scale;
    Gtk::ToggleButton *ruins_random_togglebutton;
    Gtk::Scale *temples_scale;
    Gtk::ToggleButton *temples_random_togglebutton;
    Gtk::CheckButton *cities_can_produce_allies_checkbutton;
    Gtk::ComboBox *difficulty_combobox;

    enum { MAP_SIZE_NORMAL = 0, MAP_SIZE_SMALL, MAP_SIZE_TINY };
    enum { BEGINNER = 0, INTERMEDIATE, ADVANCED, I_AM_THE_GREATEST, CUSTOM};

    Gtk::VBox *players_vbox;

    typedef std::vector<Glib::ustring> player_name_seq;
    player_name_seq default_player_names;
    player_name_seq::iterator current_player_name;
    
    std::list<Gtk::ComboBoxText *> player_types;
    std::list<Gtk::Entry *> player_names;

    GameOptionsDialog *game_options_dialog;

    void add_player(const Glib::ustring &type, const Glib::ustring &name);
    void on_add_player_clicked();
    void on_random_map_toggled();
    void on_map_size_changed();
    void on_difficulty_changed();
    void on_start_game_clicked();
    void on_edit_options_clicked();
    void on_player_type_changed();
    void on_grass_random_toggled();
    void on_water_random_toggled();
    void on_swamp_random_toggled();
    void on_forest_random_toggled();
    void on_hills_random_toggled();
    void on_mountains_random_toggled();
    void on_cities_random_toggled();
    void on_ruins_random_toggled();
    void on_temples_random_toggled();
    void on_map_chosen();
    bool is_beginner();
    bool is_intermediate();
    bool is_advanced();
    bool is_greatest();
    void update_difficulty_combobox();
    void update_difficulty_rating();
    bool scan_players(std::string tag, XML_Helper* helper);
    GameParameters load_map_parameters;
};

#endif
