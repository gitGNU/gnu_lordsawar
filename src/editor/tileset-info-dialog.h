//  Copyright (C) 2008, 2009 Ben Asselstine
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

#ifndef TILESET_INFO_DIALOG_H
#define TILESET_INFO_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include "tileset.h"

//! Scenario editor.  Edits the description of the scenario.
class TileSetInfoDialog: public sigc::trackable
{
 public:
    TileSetInfoDialog(Tileset *tileset);
    ~TileSetInfoDialog();

    void set_parent_window(Gtk::Window &parent);

    bool run();
    
 private:
    Tileset *d_tileset;
    Gtk::Dialog* dialog;
    Gtk::Entry *name_entry;
    Gtk::TextView *description_textview;
    Gtk::ColorButton *road_colorbutton;

};

#endif
