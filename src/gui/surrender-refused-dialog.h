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

#ifndef SURRENDER_REFUSED_DIALOG_H
#define SURRENDER_REFUSED_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm.h>


// dialog for showing the refusal of surrender
class SurrenderRefusedDialog: public sigc::trackable
{
 public:
    SurrenderRefusedDialog(Gtk::Window &parent);
    ~SurrenderRefusedDialog();

    void run();
    void hide();
    
 private:
    Gtk::Dialog* dialog;
    Gtk::Image *image;

};

#endif
