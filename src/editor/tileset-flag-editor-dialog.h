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

#ifndef TILESET_FLAG_EDITOR_DIALOG_H
#define TILESET_FLAG_EDITOR_DIALOG_H

#include <memory>
#include <map>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>
#include <gtkmm.h>
#include "tileset.h"


//! Tileset flag editor.  
//! Shows and manages the flags that appear on stacks
class TilesetFlagEditorDialog: public sigc::trackable
{
 public:
    TilesetFlagEditorDialog(Tileset * tileset);
    ~TilesetFlagEditorDialog();

    std::string get_selected_filename() {return selected_filename;};
    void set_icon_from_file(std::string name) {dialog->set_icon_from_file(name);};

    void set_parent_window(Gtk::Window &parent);

    int run();
    
 private:
    Gtk::Dialog* dialog;
    Gtk::FileChooserButton *flag_filechooserbutton;
    Gtk::ComboBoxText *shield_theme_combobox;
    Gtk::Table *preview_table;
    Tileset *d_tileset;
    std::string selected_filename;

    void setup_shield_theme_combobox(Gtk::Box *box);
    void shieldset_changed();
    void on_image_chosen();
    void update_flag_panel();
    void show_preview_flags(std::string filename);

    bool loadFlag(std::string filename);
    void clearFlag();
    std::map< guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >* > flags;
    sigc::connection heartbeat;
    std::map<guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >::iterator> frame;

    void on_heartbeat();

};

#endif
