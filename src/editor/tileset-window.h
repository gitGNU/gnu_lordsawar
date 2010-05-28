//  Copyright (C) 2008, 2009, 2010 Ben Asselstine
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

#ifndef GUI_TILELIST_WINDOW_H
#define GUI_TILELIST_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "Tile.h"
#include "tileset.h"
#include "PixMask.h"

//! Tileset Editor.  Edit an Tileset.
class TileSetWindow: public sigc::trackable
{
 public:
    TileSetWindow(std::string load_filename = "");
    ~TileSetWindow();

    void show();
    void hide();

    Gtk::Window &get_window() { return *window; }

    sigc::signal<void, guint32> tileset_saved;

 private:
    Gtk::Window* window;
    std::string current_save_filename;
    std::string autosave; //filename
    Tileset *d_tileset; //current tileset
    Tile *d_tile; //current tile
    bool needs_saving;
    bool inhibit_needs_saving;
    Gtk::Frame *tilestyleset_frame;
    Gtk::Frame *tilestyle_frame;
    Gtk::Entry *name_entry;
    Gtk::TreeView *tiles_treeview;
    Gtk::Button *add_tile_button;
    Gtk::Button *remove_tile_button;
    Gtk::VBox *tile_vbox;
    Gtk::Entry *tile_name_entry;
    Gtk::ComboBoxText *tile_type_combobox;
    Gtk::SpinButton *tile_moves_spinbutton;
    Gtk::ComboBoxText *tile_smallmap_pattern_combobox;
    Gtk::ColorButton *tile_smallmap_first_colorbutton;
    Gtk::ColorButton *tile_smallmap_second_colorbutton;
    Gtk::ColorButton *tile_smallmap_third_colorbutton;
    Gtk::Image *tile_smallmap_image;
    Gtk::Button *add_tilestyleset_button;
    Gtk::Button *remove_tilestyleset_button;
    Gtk::MenuItem *new_tileset_menuitem;
    Gtk::MenuItem *load_tileset_menuitem;
    Gtk::MenuItem *save_tileset_menuitem;
    Gtk::MenuItem *save_as_menuitem;
    Gtk::MenuItem *edit_tileset_info_menuitem;
    Gtk::MenuItem *roads_picture_menuitem;
    Gtk::MenuItem *bridges_picture_menuitem;
    Gtk::MenuItem *fog_picture_menuitem;
    Gtk::MenuItem *flags_picture_menuitem;
    Gtk::MenuItem *army_unit_selector_menuitem;
    Gtk::MenuItem *explosion_picture_menuitem;
    Gtk::MenuItem *preview_tile_menuitem;
    Gtk::MenuItem *quit_menuitem;
    Gtk::MenuItem *help_about_menuitem;
    Gtk::ComboBoxText *tilestyle_combobox;
    Gtk::Image *tilestyle_image;
    Gtk::Button *image_button;

    std::vector<PixMask* > tilestyle_standard_images;
    std::vector<PixMask* > tilestyle_images;
    Gtk::Image *tilestyle_standard_image;

    class TilesColumns: public Gtk::TreeModelColumnRecord {
    public:
	TilesColumns() 
        { add(name); add(tile);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Tile *> tile;
    };
    const TilesColumns tiles_columns;
    Glib::RefPtr<Gtk::ListStore> tiles_list;

    Gtk::TreeView *tilestylesets_treeview;
    class TileStyleSetsColumns: public Gtk::TreeModelColumnRecord {
    public:
	TileStyleSetsColumns() 
        { add(name); add(bname); add(tilestyleset);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Glib::ustring> bname;
	Gtk::TreeModelColumn<TileStyleSet *> tilestyleset;
    };
    const TileStyleSetsColumns tilestylesets_columns;
    Glib::RefPtr<Gtk::ListStore> tilestylesets_list;
    Glib::RefPtr<Gdk::Pixmap> tile_smallmap_surface;
    Glib::RefPtr<Gdk::GC> tile_smallmap_surface_gc;
    Gtk::TreeView *tilestyles_treeview;
    class TileStylesColumns: public Gtk::TreeModelColumnRecord {
    public:
	TileStylesColumns() 
        { add(name); add(tilestyle);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<TileStyle *> tilestyle;
    };
    const TileStylesColumns tilestyles_columns;
    Glib::RefPtr<Gtk::ListStore> tilestyles_list;

    Tile * get_selected_tile ();
    TileStyleSet * get_selected_tilestyleset ();
    TileStyle * get_selected_tilestyle ();

    bool on_delete_event(GdkEventAny *e);

    void update_tile_panel();
    void update_tilestyleset_panel();
    void update_tilestyle_panel();
    void update_tileset_buttons();
    void update_tilestyleset_buttons();
    void update_tileset_menuitems();
    void update_tile_preview_menuitem();

    void on_new_tileset_activated();
    void on_load_tileset_activated();
    void on_save_tileset_activated();
    void on_save_as_activated();
    void on_quit_activated();
    bool on_window_closed(GdkEventAny *);
    bool quit();
    void on_edit_tileset_info_activated();
    void on_army_unit_selector_activated();
    void on_explosion_picture_activated();
    void on_roads_picture_activated();
    void on_bridges_picture_activated();
    void on_fog_picture_activated();
    void on_flags_picture_activated();
    void on_preview_tile_activated();
    void on_help_about_activated();
    void on_tile_selected();
    void on_tilestyleset_selected();
    void on_tilestyle_selected();

    void fill_tile_info(Tile *tile);
    void fill_tile_smallmap(Tile *tile);
    void fill_colours(Tile *);
    void fill_tilestylesets();
    void fill_tilestyleset_info(TileStyleSet *t);

    //callbacks
    void on_tile_type_changed();
    void on_tile_name_changed();
    void on_tile_moves_changed();
    void on_tile_first_color_changed();
    void on_tile_second_color_changed();
    void on_tile_third_color_changed();
    void on_tile_pattern_changed();
    void on_tilestyle_changed();
    void on_image_chosen();

    void on_add_tile_clicked();
    void on_remove_tile_clicked();
    void on_add_tilestyleset_clicked();
    void on_remove_tilestyleset_clicked();

    bool load_tileset(std::string filename);
    void update_window_title();

};

#endif
