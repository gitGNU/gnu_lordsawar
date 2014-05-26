//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014 Ben Asselstine
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

#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H

#include <map>
#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm.h>

#include "vector.h"

class Game;
class Player;
// dialog for showing sound and game preferences
class PreferencesDialog: public sigc::trackable
{
 public:
    PreferencesDialog(bool readonly);
    ~PreferencesDialog();

    void set_parent_window(Gtk::Window &parent);

    void run(Game *game);
    void hide();

    sigc::signal<void, guint32> ui_form_factor_changed;

 private:
    Gtk::Dialog* dialog;
    Gtk::CheckButton *commentator_checkbutton;
    Gtk::Scale *speed_scale;
    Gtk::ComboBox *ui_combobox;
    Gtk::CheckButton *play_music_checkbutton;
    Gtk::Scale *music_volume_scale;
    Gtk::Box *music_volume_hbox;
    Gtk::VBox *players_vbox;

    bool d_readonly;
    void on_show_commentator_toggled();
    void on_play_music_toggled();
    void on_music_volume_changed();
    void on_speed_changed();
    void on_observe_toggled(Gtk::CheckButton *button);
    void on_type_changed(Gtk::ComboBoxText *combo);
    void on_ui_form_factor_changed();

    typedef std::map<Player*, Gtk::ComboBoxText*> PlayerTypeMap;
    PlayerTypeMap player_types;

    typedef std::map<Player*, Gtk::CheckButton*> PlayerObserveMap;
    PlayerObserveMap player_observed;
};

#endif
