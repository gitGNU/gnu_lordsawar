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

#include <config.h>

#include <sigc++/functors/mem_fun.h>

#include "tile-size-editor-dialog.h"

#include "ucompose.hpp"
#include "File.h"
#include "defs.h"

TileSizeEditorDialog::TileSizeEditorDialog(Gtk::Window &parent, guint32 current, guint32 suggested)
 : LwEditorDialog(parent, "tile-size-editor-dialog.ui")
{
    xml->get_widget("label", label);
    Glib::ustring msg = 
      String::ucompose(_("Do you want to change the tile size from %1 to %2?"),
                       current, suggested);
    label->set_text(msg);
    xml->get_widget("tilesize_spinbutton", tilesize_spinbutton);
    tilesize_spinbutton->set_value(double(suggested));
}

int TileSizeEditorDialog::run()
{
    dialog->show_all();
    int response = dialog->run();
    d_tilesize = tilesize_spinbutton->get_value();
    return response;
}
