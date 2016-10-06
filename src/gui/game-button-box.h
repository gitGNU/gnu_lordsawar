//  Copyright (C) 2011 Ben Asselstine
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

#pragma once
#ifndef GAME_BUTTON_BOX_H
#define GAME_BUTTON_BOX_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include <glibmm.h>
#include "Configuration.h"

class Game;
// shows the game buttons in the main game window
class GameButtonBox: public Gtk::Box
{
 public:
     //! Constructor for building this object with gtk::builder
    GameButtonBox(BaseObjectType* base, const Glib::RefPtr<Gtk::Builder> &xml);

    //!Destructor.
    ~GameButtonBox();

    void give_some_cheese();
    bool get_end_turn_button_sensitive();
    void setup_signals(Game *game, guint32 factor);

    //Signals
    sigc::signal<void> diplomacy_clicked;

    // Statics
    static int get_icon_size(guint32 factor);
    static GameButtonBox * create(guint32 factor);

 protected:

 private:
    std::list<sigc::connection> connections;
    guint32 d_factor;
    Gtk::Button *next_movable_button;
    Gtk::Button *center_button;
    Gtk::Button *diplomacy_button;
    Gtk::Button *defend_button;
    Gtk::Button *park_button;
    Gtk::Button *deselect_button;
    Gtk::Button *search_button;
    Gtk::Button *move_button;
    Gtk::Button *move_all_button;
    Gtk::Button *end_turn_button;
    Gtk::Button *nw_keypad_button;
    Gtk::Button *n_keypad_button;
    Gtk::Button *ne_keypad_button;
    Gtk::Button *e_keypad_button;
    Gtk::Button *w_keypad_button;
    Gtk::Button *sw_keypad_button;
    Gtk::Button *s_keypad_button;
    Gtk::Button *se_keypad_button;
    static Glib::ustring get_file(Configuration::UiFormFactor factor);

    void setup_button(Gtk::Button *button, sigc::slot<void> slot,
                      sigc::signal<void, bool> &game_signal);

    void change_diplomacy_button_image (bool proposals_present);
    void update_diplomacy_button (bool sensitive);

    void add_pictures_to_buttons(guint32 factor);
    void drop_connections();
    void pad_image(Gtk::Image *image);
    void add_picture_to_button (guint32 icontype, Gtk::Button *button, bool arrow = false);
};

#endif // GAME_BUTTON_BOX
