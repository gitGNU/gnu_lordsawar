//  Copyright (C) 2009, 2014 Ben Asselstine
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

#ifndef IMAGE_EDITOR_DIALOG_H
#define IMAGE_EDITOR_DIALOG_H

#include <memory>
#include <map>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>
#include <gtkmm.h>
#include "PixMask.h"


//! general picture editor.  
/**
 * This class doesn't actually edit the image, instead it shows the image
 * being edited in each player colour.  The user can pick a new file to be
 * the new image.
 */
class ImageEditorDialog: public sigc::trackable
{
 public:
    ImageEditorDialog(std::string filename, int num_frames);
    ~ImageEditorDialog();

    void set_parent_window(Gtk::Window &parent);

    std::string get_selected_filename() {return target_filename;};

    int run();
    
 private:
    std::string target_filename;
    int num_frames;
    std::vector<PixMask*> frames;
    int active_frame;
    sigc::connection heartbeat;
    Gtk::Dialog* dialog;
    Gtk::FileChooserButton *filechooserbutton;
    Gtk::Image *image;
    void on_image_chosen();
    void show_image(std::string filename);
    void update_panel();
    void on_heartbeat();

};

#endif
