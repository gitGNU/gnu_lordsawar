//  Copyright (C) 2010, 2014 Ben Asselstine
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

#ifndef TILE_SIZE_EDITOR_DIALOG_H
#define TILE_SIZE_EDITOR_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"

class TileSizeEditorDialog: public LwEditorDialog
{
 public:
    TileSizeEditorDialog(Gtk::Window &parent, guint32 current, guint32 suggested);
    ~TileSizeEditorDialog();

    guint32 get_selected_tilesize() const {return d_tilesize;}

    int run();
    void hide();
    
 private:
    Gtk::Label *label;
    Gtk::SpinButton *tilesize_spinbutton;
    guint32 d_tilesize;
};

#endif
