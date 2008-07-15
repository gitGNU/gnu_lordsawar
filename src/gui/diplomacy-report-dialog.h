//  Copyright (C) 2008 Ben Asselstine
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

#ifndef DIPLOMACY_REPORT_DIALOG_H
#define DIPLOMACY_REPORT_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/table.h>

#include "../player.h"

//
//
class DiplomacyReportDialog: public sigc::trackable
{
 public:
    DiplomacyReportDialog(Player *player);

    void set_parent_window(Gtk::Window &parent);

    void run();
    void hide();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;

    Player *d_player;

    Gtk::Table *d_table;

};

#endif
