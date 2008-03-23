//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef GUI_MAIN_WINDOW_H
#define GUI_MAIN_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <libglademm/xml.h>
#include <gtkmm/window.h>
#include <gtkmm/dialog.h>
#include <gtkmm/container.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>
#include <gtkmm/table.h>
#include <gtkmm/checkmenuitem.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/tooltips.h>

#include "../map-tip-position.h"
#include "editorbigmap.h"
#include "RenamableLocation.h"

class SDL_Surface;
class EditorBigMap;
class SmallMap;
class GameScenario;
class CreateScenarioRandomize;

#include "../UniquelyIdentified.h"

//! Scenario editor.  Edits a Scenario.
class MainWindow: public sigc::trackable
{
 public:
    MainWindow();
    ~MainWindow();

    void show();
    void hide();

    // initialize the SDL widget 
    void init(int width, int height);

    sigc::signal<void> sdl_initialized;
    
    void show_initial_map();
    Gtk::Window &get_window() { return *window.get(); }

 private:
    std::auto_ptr<Gtk::Window> window;
    Gtk::Container *sdl_container;
    Gtk::Widget *sdl_widget;
    Gtk::CheckMenuItem *fullscreen_menuitem;
    Gtk::HBox *terrain_tile_style_hbox;
    Gtk::Image *map_image;
    std::string current_save_filename;
    Gtk::Table *terrain_type_table;
    Gtk::Label *mouse_position_label;
    Gtk::RadioButton *pointer_radiobutton;
    Gtk::Tooltips tooltips;

    std::auto_ptr<EditorBigMap> bigmap;
    std::auto_ptr<SmallMap> smallmap;
    
    bool sdl_inited;

    std::auto_ptr<GameScenario> game_scenario;
    std::auto_ptr<CreateScenarioRandomize> d_create_scenario_names;
    GdkEventButton *button_event;

    bool on_delete_event(GdkEventAny *e);

    bool on_sdl_mouse_button_event(GdkEventButton *e);
    bool on_sdl_mouse_motion_event(GdkEventMotion *e);
    bool on_sdl_key_event(GdkEventKey *e);
    bool on_sdl_leave_event(GdkEventCrossing *e);

    bool on_map_mouse_button_event(GdkEventButton *e);
    bool on_map_mouse_motion_event(GdkEventMotion *e);
    
    void on_new_map_activated();
    void on_load_map_activated();
    void on_save_map_activated();
    void on_save_map_as_activated();
    void on_quit_activated();
    void on_edit_map_info_activated();
    void on_edit_players_activated();
    void on_smooth_map_activated();
    void on_smooth_screen_activated();
    void on_edit_items_activated();
    void on_edit_rewards_activated();

    void on_fullscreen_activated();

    void on_random_all_cities_activated();
    void on_random_unnamed_cities_activated();
    void on_random_all_ruins_activated();
    void on_random_unnamed_ruins_activated();
    void on_random_all_temples_activated();
    void on_random_unnamed_temples_activated();
    void on_random_all_signs_activated();
    void on_random_unnamed_signs_activated();

    void on_help_about_activated();
    
    struct PointerItem
    {
	Gtk::RadioButton *button;
	EditorBigMap::Pointer pointer;
	int size;
    };

    std::vector<PointerItem> pointer_items;
    
    struct TerrainItem
    {
	Gtk::RadioButton *button;
	Tile::Type terrain;
    };

    struct TileStyleItem
    {
	Gtk::RadioButton *button;
	int tile_style_id;
    };

    std::vector<TerrainItem> terrain_items;
    std::vector<TileStyleItem> tile_style_items;
    
    void on_pointer_radiobutton_toggled();
    void on_terrain_radiobutton_toggled();
    void on_tile_style_radiobutton_toggled();
    
    Tile::Type get_terrain();
    int get_tile_style_id();
    void setup_pointer_radiobutton(Glib::RefPtr<Gnome::Glade::Xml> xml,
	std::string prefix, std::string image_file,
	EditorBigMap::Pointer pointer, int size);
    void setup_terrain_radiobuttons();

    void init_maps();
    void set_filled_map(int width, int height, int fill_style, 
			std::string tileset, std::string shieldset, 
			std::string cityset);
    void set_random_map(int width, int height,
			int grass, int water, int swamp, int forest,
			int hills, int mountains,
			int cities, int ruins, int temples, int signposts,
			std::string tileset, std::string shieldset,
			std::string cityset);

    void clear_map_state();
    void init_map_state();
    void remove_tile_style_buttons();
    void setup_tile_style_buttons(Tile::Type terrain);
    void randomize_signpost(Signpost *signpost);
    void randomize_city(City *city);
    void randomize_ruin(Ruin *ruin);

    // map callbacks
    void on_smallmap_changed(SDL_Surface *map);
    void on_objects_selected(std::vector<UniquelyIdentified *> objects);
    void on_mouse_on_tile(Vector<int> tile);
    
    void popup_dialog_for_object(UniquelyIdentified *object);

    int d_width;
    int d_height;
    
public:
    // not part of the API, but for surface_attached_helper
    void on_sdl_surface_changed();
};

#endif
