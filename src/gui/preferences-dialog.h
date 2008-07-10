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

#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm/dialog.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/image.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/scale.h>
#include <gtkmm/box.h>

#include "../vector.h"

class Game;
// dialog for showing sound and game preferences
class PreferencesDialog: public sigc::trackable
{
 public:
    PreferencesDialog(bool readonly);

    void set_parent_window(Gtk::Window &parent);

    void run(Game *game);

 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::CheckButton *show_turn_popup_checkbutton;
    Gtk::CheckButton *play_music_checkbutton;
    Gtk::Scale *music_volume_scale;
    Gtk::Box *music_volume_hbox;
    Gtk::VBox *players_vbox;

    bool d_readonly;
    void on_show_turn_popup_toggled();
    void on_play_music_toggled();
    void on_music_volume_changed();
    std::list<Gtk::ComboBoxText *> player_types;
};

#endif
