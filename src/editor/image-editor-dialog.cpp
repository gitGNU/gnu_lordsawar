//  Copyright (C) 2009 Ben Asselstine
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

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>
#include <stdlib.h>

#include "image-editor-dialog.h"

#include "glade-helpers.h"
#include "gui/image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "shieldsetlist.h"
#include "GraphicsCache.h"


ImageEditorDialog::ImageEditorDialog(std::string filename, int frames)
{
  num_frames = frames;
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/image-editor-dialog.ui");

    xml->get_widget("dialog", dialog);

    xml->get_widget("filechooserbutton", filechooserbutton);
    xml->get_widget("image", image);
    show_image(filename);
    target_filename = filename;
    update_panel();
    filechooserbutton->signal_selection_changed().connect
       (sigc::mem_fun(*this, &ImageEditorDialog::on_image_chosen));

}

ImageEditorDialog::~ImageEditorDialog()
{
  delete dialog;
}
void ImageEditorDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

int ImageEditorDialog::run()
{
    dialog->show_all();
    int response = dialog->run();
    if (response == Gtk::RESPONSE_ACCEPT)
      target_filename = "";

    return response;
}

void ImageEditorDialog::on_image_chosen()
{
  std::string selected_filename = filechooserbutton->get_filename();
  if (selected_filename.empty())
    return;

  target_filename = selected_filename;
  show_image(selected_filename);
}


void ImageEditorDialog::update_panel()
{
  if (target_filename != "")
    filechooserbutton->set_filename (target_filename);
}

void ImageEditorDialog::show_image(std::string filename)
{
  if (filename == "")
    return;

  if (heartbeat.connected())
    heartbeat.disconnect();

  frames = disassemble_row(filename, num_frames);
  active_frame = 0;
  image->clear();
  if (num_frames > 0)
    heartbeat = Glib::signal_timeout().connect
      (bind_return
       (sigc::mem_fun (*this, &ImageEditorDialog::on_heartbeat), 
	true), 500);
  else
    on_heartbeat();
}

void ImageEditorDialog::on_heartbeat()
{
  image->property_pixbuf() = frames[active_frame]->to_pixbuf();
  active_frame++;
  if (active_frame >= num_frames)
    active_frame = 0;
}
