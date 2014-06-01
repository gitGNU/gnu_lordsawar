//  Copyright (C) 2014 Ben Asselstine
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

#ifndef LW_EDITOR_DIALOG_H
#define LW_EDITOR_DIALOG_H

#include <sigc++/trackable.h>
#include <gtkmm.h>

class LwEditorDialog: public sigc::trackable
{
 public:
    LwEditorDialog(Gtk::Window &parent, Glib::ustring file);
    ~LwEditorDialog();

    int run_and_hide();

 protected:
    Gtk::Dialog* dialog;
    Glib::RefPtr<Gtk::Builder> xml;
};

#endif
