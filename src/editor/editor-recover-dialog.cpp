//  Copyright (C) 2010 Ben Asselstine
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

#include "editor-recover-dialog.h"

#include "glade-helpers.h"
#include "ucompose.hpp"
#include "File.h"
#include "defs.h"

EditorRecoverDialog::EditorRecoverDialog(std::string question)
{
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/editor-recover-dialog.ui");

    xml->get_widget("dialog", dialog);
    dialog->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
    xml->get_widget("label", label);
    label->set_text(question);
}

EditorRecoverDialog::~EditorRecoverDialog()
{
  delete dialog;
}
void EditorRecoverDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
}

int EditorRecoverDialog::run()
{
    dialog->show_all();
    return dialog->run();
}

void EditorRecoverDialog::hide()
{
  dialog->hide();
}
