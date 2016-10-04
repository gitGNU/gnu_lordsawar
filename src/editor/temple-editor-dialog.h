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

#pragma once
#ifndef TEMPLE_EDITOR_DIALOG_H
#define TEMPLE_EDITOR_DIALOG_H

#include <gtkmm.h>
#include "lw-editor-dialog.h"

class CreateScenarioRandomize;

class Temple;

//! Scenario editor.  Edits a Temple object.
class TempleEditorDialog: public LwEditorDialog
{
 public:
    TempleEditorDialog(Gtk::Window &parent, Temple *temple, CreateScenarioRandomize *randomizer);
    ~TempleEditorDialog() {};

    int run();
    
 private:
    Gtk::Entry *name_entry;
    Gtk::Entry *description_entry;
    Gtk::SpinButton *type_entry;
    Temple *temple;
    Gtk::Button *randomize_name_button;
    CreateScenarioRandomize *d_randomizer;

    void on_randomize_name_clicked();
};

#endif
