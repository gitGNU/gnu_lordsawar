//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#ifndef TRIUMPHS_DIALOG_H
#define TRIUMPHS_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm.h>

class Player;

#include "Triumphs.h"

#include "decorated.h"
//
//
class TriumphsDialog: public Decorated
{
 public:
    TriumphsDialog(Player *player);
    ~TriumphsDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();
    void hide();
    
 private:
    Gtk::Dialog* dialog;

    Player *d_player;

    Gtk::Notebook *notebook;

    void fill_in_info();
    void fill_in_page(Player *p);
    guint32 tally(Player *p, Triumphs::TriumphType type);
};

#endif
