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

#ifndef NEW_NETWORK_GAME_DIALOG_H
#define NEW_NETWORK_GAME_DIALOG_H

#include <memory>
#include <string>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "ucompose.hpp"
#include "decorated.h"
// dialog for choosing between a hosted game and a game we connect to
class NewNetworkGameDialog: public Decorated
{
 public:
    NewNetworkGameDialog(Glib::ustring network_game_nickname);
    ~NewNetworkGameDialog();

    Glib::ustring getNickname() {return String::utrim(nick_entry->get_text());}
    bool isClient() {return client_radiobutton->get_active();}

    void set_parent_window(Gtk::Window &parent);

    void hide();
    bool run();
    
 private:
  Gtk::Dialog* dialog;
  Gtk::Entry *nick_entry;
  Gtk::RadioButton *client_radiobutton;
  Gtk::Button *accept_button;

  void update_buttons();
  void on_nickname_changed();
};

#endif
