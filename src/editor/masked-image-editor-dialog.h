//  Copyright (C) 2009, 2010, 2014 Ben Asselstine
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

#ifndef MASKED_IMAGE_EDITOR_DIALOG_H
#define MASKED_IMAGE_EDITOR_DIALOG_H

#include <gtkmm.h>
#include "shield.h"
#include "lw-editor-dialog.h"


//! general masked picture editor.  
/**
 * This class doesn't actually edit the image, instead it shows the image
 * being edited in each player colour.  The user can pick a new file to be
 * the new image.
 *
 * the shieldset is required to define the mask colours.
 *
 * the only parameter means don't show the masked image in all colours,
 * just show it in one colour.  -1 means all.  non-negative is one of the
 * Shield::Colour enumerations.
 */
class Shieldset;
class MaskedImageEditorDialog: public LwEditorDialog
{
 public:
    MaskedImageEditorDialog(Gtk::Window &parent, Glib::ustring filename, int only_show_colour, Shieldset *shieldset = NULL);
    ~MaskedImageEditorDialog() {};

    void set_title(Glib::ustring t) {dialog->set_title(t);};

    void set_only_show_one(Shield::Colour c){only_show=true;only_show_colour=c;};
    Glib::ustring get_selected_filename() {return target_filename;};
    int run();
    
 private:
    Glib::ustring target_filename;
    Gtk::FileChooserButton *filechooserbutton;
    Gtk::Image *image_white;
    Gtk::Image *image_green;
    Gtk::Image *image_yellow;
    Gtk::Image *image_light_blue;
    Gtk::Image *image_red;
    Gtk::Image *image_dark_blue;
    Gtk::Image *image_orange;
    Gtk::Image *image_black;
    Gtk::Image *image_neutral;
    Shieldset * d_shieldset;
    bool only_show;
    Shield::Colour only_show_colour;
    void on_image_chosen();
    void show_image(Glib::ustring filename);
    void update_panel();

};

#endif
