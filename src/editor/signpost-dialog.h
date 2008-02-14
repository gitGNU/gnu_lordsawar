//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef SIGNPOST_DIALOG_H
#define SIGNPOST_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/textview.h>
#include <gtkmm/button.h>

class Signpost;
class CreateScenarioRandomize;

class SignpostDialog: public sigc::trackable
{
 public:
    SignpostDialog(Signpost *signpost, CreateScenarioRandomize *randomizer);

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::TextView *sign_textview;
    Signpost *signpost;
    Gtk::Button *randomize_button;
    CreateScenarioRandomize *d_randomizer;
    
    void on_randomize_clicked();
};

#endif
