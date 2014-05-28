//  Copyright (C) 2008, 2009, 2010, 2014 Ben Asselstine
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

#ifndef TILESET_SELECTOR_EDITOR_DIALOG_H
#define TILESET_SELECTOR_EDITOR_DIALOG_H

#include <memory>
#include <map>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>
#include <gtkmm.h>
#include "tileset.h"


//! Tileset selector editor.  
//! Shows and manages the large and small army unit selector animation.
class TilesetSelectorEditorDialog: public sigc::trackable
{
 public:
    TilesetSelectorEditorDialog(Gtk::Window &parent, Tileset * tileset);
    ~TilesetSelectorEditorDialog();

    Glib::ustring get_small_selector_filename() {return small_filename;};
    Glib::ustring get_large_selector_filename() {return large_filename;};
    int run();
    
 private:
    Gtk::Dialog* dialog;
    Gtk::RadioButton *large_selector_radiobutton;
    Gtk::RadioButton *small_selector_radiobutton;
    Gtk::FileChooserButton *selector_filechooserbutton;
    Gtk::ComboBoxText *shield_theme_combobox;
    Gtk::Table *preview_table;
    Tileset *d_tileset;
    std::list<Glib::ustring> delfiles;

    void setup_shield_theme_combobox(Gtk::Box *box);
    void shieldset_changed();
    void on_image_chosen();
    void on_large_toggled();
    void on_small_toggled();
    void update_selector_panel();
    void show_preview_selectors(Glib::ustring filename);

    bool loadSelector(Glib::ustring filename);
    void clearSelector();
    std::map< guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >* > selectors;
    sigc::connection heartbeat;
    std::map<guint32, std::list<Glib::RefPtr<Gdk::Pixbuf> >::iterator> frame;

    void on_heartbeat();
    Glib::ustring small_filename;
    Glib::ustring large_filename;

};

#endif
