//  Copyright (C) 2007, 2008, 2009, 2010, 2014, 2015 Ben Asselstine
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

#ifndef GUI_SHIELDSET_WINDOW_H
#define GUI_SHIELDSET_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "shieldset.h"
#include "shieldstyle.h"

class Shield;
//! Shieldset Editor.  Edit an Shieldset.
class ShieldSetWindow: public sigc::trackable
{
 public:
    ShieldSetWindow(Glib::ustring load_filename = "");
    ~ShieldSetWindow();

    void show();
    void hide();

    Gtk::Window &get_window() { return *window; };

    sigc::signal<void, guint32> shieldset_saved;

 private:
    Gtk::Window* window;
    Glib::ustring current_save_filename;
    Shieldset *d_shieldset; //current shieldset
    Shield *d_shield; //current shield
    bool needs_saving;
    Gtk::TreeView *shields_treeview;
    Gtk::Image *small_image;
    Gtk::Image *medium_image;
    Gtk::Image *large_image;
    Gtk::MenuItem *new_shieldset_menuitem;
    Gtk::MenuItem *load_shieldset_menuitem;
    Gtk::MenuItem *save_shieldset_menuitem;
    Gtk::MenuItem *save_as_menuitem;
    Gtk::MenuItem *validate_shieldset_menuitem;
    Gtk::MenuItem *edit_shieldset_info_menuitem;
    Gtk::MenuItem *edit_copy_shields_menuitem;
    Gtk::MenuItem *quit_menuitem;
    Gtk::MenuItem *help_about_menuitem;
    Gtk::Button *change_smallpic_button;
    Gtk::Button *change_mediumpic_button;
    Gtk::Button *change_largepic_button;
    Gtk::ColorButton *player_colorbutton;
    Gtk::Alignment *shield_alignment;

    class ShieldsColumns: public Gtk::TreeModelColumnRecord {
    public:
	ShieldsColumns() 
        { add(name); add(shield);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Shield*> shield;
    };
    const ShieldsColumns shields_columns;
    Glib::RefPtr<Gtk::ListStore> shields_list;

    void on_new_shieldset_activated();
    void on_load_shieldset_activated();
    void on_save_shieldset_activated();
    void on_save_as_activated();
    void on_validate_shieldset_activated();
    void on_quit_activated();
    bool on_delete_event();
    bool on_window_closed(GdkEventAny*);
    void on_edit_shieldset_info_activated();
    void on_edit_copy_shields_activated();
    void on_help_about_activated();
    void on_shield_selected();
    void on_shieldpic_changed(ShieldStyle::Type type);
    void on_player_color_changed();

    void fill_shield_info(Shield *shield);
    bool load_shieldset(Glib::ustring filename);
    bool save_current_shieldset();
    void update_shield_panel();
    void update_shieldset_menuitems();
    bool quit();
    
    void addNewShield(Shield::Colour owner, Gdk::RGBA colour);
    void loadShield(Shield *shield);
    void update_window_title();
    void show_shield(ShieldStyle *ss, Shield *s, Gtk::Image *image);
    void refresh_shields();
    void process_shieldstyle(ShieldStyle *ss, Gtk::FileChooserDialog *d);
    Gtk::FileChooserDialog* shield_filechooser (Shield *s, ShieldStyle::Type t);
};

#endif
