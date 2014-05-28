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

#ifndef GAME_PREFERENCES_DIALOG_H
#define GAME_PREFERENCES_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <gtkmm.h>

#include "game-parameters.h"
#include "game-options-dialog.h"
#include "GameScenario.h"

class XML_Helper;

// dialog for choosing parameters for starting a new game
class GamePreferencesDialog: public sigc::trackable
{
 public:
    GamePreferencesDialog(Gtk::Window &parent, Glib::ustring filename, GameScenario::PlayMode mode);
    ~GamePreferencesDialog();

    void set_title(Glib::ustring title);

    sigc::signal<void, GameParameters> game_started;
    
    bool run(Glib::ustring nickname = "guest");
    void hide();
    
 private:
    void init(Gtk::Window &parent, Glib::ustring filename);
    Gtk::Dialog* dialog;
    GameScenario::PlayMode mode;

    Gtk::Box *dialog_vbox;
    Gtk::ProgressBar *progressbar;
    Gtk::Button *start_game_button;
    Gtk::Button *edit_options_button;
    Gtk::Label *game_name_label;
    Gtk::Entry *game_name_entry;
    Gtk::Label *difficulty_label;
    Gtk::ComboBox *difficulty_combobox;

    enum { BEGINNER = 0, INTERMEDIATE, ADVANCED, I_AM_THE_GREATEST, CUSTOM};

    Gtk::VBox *players_vbox;

    typedef std::vector<Glib::ustring> player_name_seq;
    
    std::list<Gtk::ComboBoxText *> player_types;
    std::list<Gtk::Entry *> player_names;
    std::list<Gtk::Image *> player_shields;

    GameOptionsDialog *game_options_dialog;

    void add_player(GameParameters::Player::Type type,
				       const Glib::ustring &name);
    void on_difficulty_changed();
    void on_start_game_clicked();
    void on_edit_options_clicked();
    void on_player_type_changed();
    void on_player_name_changed();
    bool is_beginner();
    bool is_intermediate();
    bool is_advanced();
    bool is_greatest();
    void update_difficulty_combobox();
    void update_difficulty_rating();
    void update_shields();
    void update_buttons();
    Glib::RefPtr<Gdk::Pixbuf> getShieldPic(guint32 type, guint32 owner);
    Glib::ustring d_filename;
    guint32 d_shieldset;
};

#endif
