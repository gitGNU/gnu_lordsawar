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

#ifndef TILE_PREVIEW_DIALOG_H
#define TILE_PREVIEW_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/textview.h>
#include <gtkmm/entry.h>
#include "../Tile.h"

//! Scenario editor.  Edits the description of the scenario.
class TilePreviewDialog: public sigc::trackable
{
 public:
    TilePreviewDialog(Tile *tile);

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    Tile *d_tile;
    std::auto_ptr<Gtk::Dialog> dialog;
};

#endif
