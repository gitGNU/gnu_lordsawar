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

#include "masked-image-editor-dialog.h"

#include "glade-helpers.h"
#include "gui/image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "shieldsetlist.h"
#include "GraphicsCache.h"


MaskedImageEditorDialog::MaskedImageEditorDialog(std::string filename)
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/masked-image-editor-dialog.ui");

    xml->get_widget("dialog", dialog);

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
    filechooserbutton->signal_selection_changed().connect
       (sigc::mem_fun(*this, &MaskedImageEditorDialog::on_image_chosen));

}

MaskedImageEditorDialog::~MaskedImageEditorDialog()
{
  delete dialog;
}
void MaskedImageEditorDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

int MaskedImageEditorDialog::run()
{
    dialog->show_all();
    int response = dialog->run();
    if (response == Gtk::RESPONSE_ACCEPT)
      target_filename = "";

    return response;
}

void MaskedImageEditorDialog::on_image_chosen()
{
  std::string selected_filename = filechooserbutton->get_filename();
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

void MaskedImageEditorDialog::show_image(std::string filename)
{
  if (filename == "")
    return;
  std::vector<PixMask*> half = disassemble_row(filename, 2);
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

      struct rgb_shift shifts;
      shifts = Shieldsetlist::getInstance()->getMaskColorShifts(1, i);
      PixMask *army_image = GraphicsCache::applyMask(half[0],  half[1],
						     shifts, false);
      image->property_pixbuf() = army_image->to_pixbuf();
      delete army_image;
    }
  delete half[0];
  delete half[1];
}
