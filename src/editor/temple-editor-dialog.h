//  Copyright (C) 2007 Ole Laursen
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

#ifndef TEMPLE_EDITOR_DIALOG_H
#define TEMPLE_EDITOR_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>

class CreateScenarioRandomize;

class Temple;

//! Scenario editor.  Edits a Temple object.
class TempleEditorDialog: public sigc::trackable
{
 public:
    TempleEditorDialog(Temple *temple, CreateScenarioRandomize *randomizer);
    ~TempleEditorDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    Gtk::Dialog* dialog;
    Gtk::Entry *name_entry;
    Gtk::Entry *description_entry;
    Gtk::SpinButton *type_entry;
    Temple *temple;
    Gtk::Button *randomize_name_button;
    CreateScenarioRandomize *d_randomizer;

    void on_randomize_name_clicked();
};

#endif
