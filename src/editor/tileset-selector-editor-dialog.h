//  Copyright (C) 2008 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
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
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include "../tileset.h"


//! Tileset selector editor.  
//! Shows and manages the large and small army unit selector animation.
class TilesetSelectorEditorDialog: public sigc::trackable
{
 public:
    TilesetSelectorEditorDialog(Tileset * tileset);

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::RadioButton *large_selector_radiobutton;
    Gtk::RadioButton *small_selector_radiobutton;
    Gtk::FileChooserButton *selector_filechooserbutton;
    Gtk::ComboBoxText *shield_theme_combobox;
    Gtk::Table *preview_table;
    Tileset *d_tileset;

    void setup_shield_theme_combobox(Gtk::Box *box);
    void shieldset_changed();
    void on_image_chosen();
    void on_large_toggled();
    void on_small_toggled();
    void update_selector_panel();
    void show_preview_selectors(std::string filename);

    bool loadSelector(std::string filename);
    void clearSelector();
    std::map< Uint32, std::list<SDL_Surface*>* > selectors;
    sigc::connection heartbeat;
    std::map<Uint32, std::list<SDL_Surface*>::iterator> frame;

    void on_heartbeat();

};

#endif
