//  Copyright (C) 2009, 2010 Ben Asselstine
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

#ifndef TILESET_EXPLOSION_PICTURE_EDITOR_DIALOG_H
#define TILESET_EXPLOSION_PICTURE_EDITOR_DIALOG_H

#include <memory>
#include <map>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>
#include <gtkmm.h>
#include "tileset.h"
#include "tile-preview-scene.h"


//! Tileset explosion picture editor.  
class TilesetExplosionPictureEditorDialog: public sigc::trackable
{
 public:
    TilesetExplosionPictureEditorDialog(Tileset * tileset);
    ~TilesetExplosionPictureEditorDialog();

    void set_parent_window(Gtk::Window &parent);

    void set_icon_from_file(std::string name) {dialog->set_icon_from_file(name);};
    std::string get_selected_filename() {return selected_filename;};
    int run();
    
 private:
    Gtk::Dialog* dialog;
    Gtk::RadioButton *large_explosion_radiobutton;
    Gtk::RadioButton *small_explosion_radiobutton;
    Gtk::FileChooserButton *explosion_filechooserbutton;
    Gtk::Image *scene_image;
    Tileset *d_tileset;
    std::string selected_filename;
    std::list<std::string> delfiles;

    void on_image_chosen();
    void on_large_toggled();
    void on_small_toggled();
    void show_explosion_image(std::string filename);
    void update_panel();

    void update_scene(TilePreviewScene *scene, std::string filename);

};

#endif
