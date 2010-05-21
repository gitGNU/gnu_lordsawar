//  Copyright (C) 2007, 2008, 2009, 2010 Ben Asselstine
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

#ifndef SHIELDSET_INFO_DIALOG_H
#define SHIELDSET_INFO_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include "shieldset.h"

//! Shieldset Editor.  Edit the description of the Shieldset.
class ShieldSetInfoDialog: public sigc::trackable
{
 public:
    ShieldSetInfoDialog(Shieldset *shieldset, std::string dir,
                        bool readonly = false, std::string title = "");
    ~ShieldSetInfoDialog();

    void set_parent_window(Gtk::Window &parent);

    int run();
    
 private:
    Gtk::Dialog* dialog;
    Shieldset *d_shieldset;
    Gtk::Entry *name_entry;
    Gtk::TextView *copyright_textview;
    Gtk::TextView *license_textview;
    Gtk::Entry *filename_entry;
    Gtk::SpinButton *id_spinbutton;
    Gtk::Button *accept_button;
    Gtk::Label *status_label;
    Gtk::TextView *description_textview;
    Gtk::Label *dir_label;

    void on_name_changed();
    void on_filename_changed();
    void update_buttons();

    bool d_readonly;
};

#endif
