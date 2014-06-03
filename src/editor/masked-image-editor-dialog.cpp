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

#include "masked-image-editor-dialog.h"

#include "gui/image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "shieldsetlist.h"
#include "shieldset.h"
#include "ImageCache.h"


MaskedImageEditorDialog::MaskedImageEditorDialog(Gtk::Window &parent, Glib::ustring filename, int only, Shieldset *shieldset)
 : LwEditorDialog(parent, "masked-image-editor-dialog.ui")
{
  if (only >= 0)
    {
      only_show = true;
      only_show_colour = Shield::Colour(only);
    }
  d_shieldset = shieldset;
    xml->get_widget("filechooserbutton", filechooserbutton);
    xml->get_widget("image_white", image_white);
    xml->get_widget("image_green", image_green);
    xml->get_widget("image_yellow", image_yellow);
    xml->get_widget("image_light_blue", image_light_blue);
    xml->get_widget("image_red", image_red);
    xml->get_widget("image_dark_blue", image_dark_blue);
    xml->get_widget("image_orange", image_orange);
    xml->get_widget("image_black", image_black);
    xml->get_widget("image_neutral", image_neutral);
    show_image(filename);
    target_filename = filename;
    update_panel();
    filechooserbutton->signal_file_set().connect
       (sigc::mem_fun(*this, &MaskedImageEditorDialog::on_image_chosen));
}

int MaskedImageEditorDialog::run()
{
    dialog->show_all();
    int response = dialog->run();
    if (response != Gtk::RESPONSE_ACCEPT)
      target_filename = "";

    return response;
}

void MaskedImageEditorDialog::on_image_chosen()
{
  Glib::ustring selected_filename = filechooserbutton->get_filename();
  if (selected_filename.empty())
    return;

  target_filename = selected_filename;
  show_image(selected_filename);
}


void MaskedImageEditorDialog::update_panel()
{
  if (target_filename != "")
    filechooserbutton->set_filename (target_filename);
}

void MaskedImageEditorDialog::show_image(Glib::ustring filename)
{
  if (filename == "")
    return;
  bool broken = false;
  std::vector<PixMask*> half = disassemble_row(filename, 2, broken);
  if (broken)
    return;
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    {
      Gtk::Image *image = NULL;
      switch (i)
	{
	case Shield::WHITE: image = image_white; break;
	case Shield::GREEN: image = image_green; break;
	case Shield::YELLOW: image = image_yellow; break;
	case Shield::LIGHT_BLUE: image = image_light_blue; break;
	case Shield::RED: image = image_red; break;
	case Shield::DARK_BLUE: image = image_dark_blue; break;
	case Shield::ORANGE: image = image_orange; break;
	case Shield::BLACK: image = image_black; break;
	case Shield::NEUTRAL: image = image_neutral; break;
	default : break;
	}

      if (only_show)
        {
          if (i != only_show_colour)
            {
              image->property_visible() = false;
              image->property_no_show_all() = true;
              continue;
            }
        }
      if (d_shieldset == NULL)
        d_shieldset = Shieldsetlist::getInstance()->getShieldset(1);
      Gdk::RGBA colour = d_shieldset->getColor(i);
      PixMask *army_image = ImageCache::applyMask(half[0],  half[1],
						     colour, false);
      image->property_pixbuf() = army_image->to_pixbuf();
      delete army_image;
    }
  delete half[0];
  delete half[1];
}
