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

#ifndef ARMYSET_INFO_DIALOG_H
#define ARMYSET_INFO_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include "armyset.h"

//! Armyset Editor.  Edit the description of the Armyset.
class ArmySetInfoDialog: public sigc::trackable
{
 public:
    ArmySetInfoDialog(Armyset *armyset);
    ~ArmySetInfoDialog();

    void set_parent_window(Gtk::Window &parent);

    int run();
    
 private:
    Gtk::Dialog* dialog;
    Armyset *d_armyset;
    Gtk::Entry *name_entry;
    Gtk::SpinButton *id_spinbutton;
};

#endif
