//  Copyright (C) 2009, 2010, 2011, 2014 Ben Asselstine
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
#include "gui/image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "shieldsetlist.h"
#include "GraphicsCache.h"

ImageEditorDialog::ImageEditorDialog(Gtk::Window &parent, Glib::ustring filename, int frames)
 : LwEditorDialog(parent, "image-editor-dialog.ui")
{
  num_frames = frames;

    xml->get_widget("filechooserbutton", filechooserbutton);
    xml->get_widget("image", image);
    show_image(filename);
    target_filename = filename;
    update_panel();
    filechooserbutton->signal_file_set().connect
       (sigc::mem_fun(*this, &ImageEditorDialog::on_image_chosen));
}

ImageEditorDialog::~ImageEditorDialog()
{
}

int ImageEditorDialog::run()
{
    dialog->show_all();
    int response = dialog->run();
    if (response != Gtk::RESPONSE_ACCEPT)
      target_filename = "";

    return response;
}

void ImageEditorDialog::on_image_chosen()
{
  Glib::ustring selected_filename = filechooserbutton->get_filename();
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

void ImageEditorDialog::show_image(Glib::ustring filename)
{
  if (filename == "")
    return;

  if (heartbeat.connected())
    heartbeat.disconnect();

  bool broken = false;
  frames = disassemble_row(filename, num_frames, broken);
  if (!broken)
    {
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
}

void ImageEditorDialog::on_heartbeat()
{
  image->property_pixbuf() = frames[active_frame]->to_pixbuf();
  active_frame++;
  if (active_frame >= num_frames)
    active_frame = 0;
}
