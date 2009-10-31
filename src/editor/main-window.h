//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#ifndef GUI_MAIN_WINDOW_H
#define GUI_MAIN_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "map-tip-position.h"
#include "editorbigmap.h"
#include "RenamableLocation.h"

class EditorBigMap;
class SmallMap;
class GameScenario;
class CreateScenarioRandomize;

#include "UniquelyIdentified.h"

//! Scenario editor.  Edits a Scenario.
class MainWindow: public sigc::trackable
{
 public:
    MainWindow();
    ~MainWindow();

    void show();
    void hide();

    void init();
    void show_initial_map();
    Gtk::Window &get_window() { return *window; }


 private:
    Gtk::Window* window;
    EditorBigMap* bigmap;
    SmallMap* smallmap;
    GameScenario* game_scenario;
    CreateScenarioRandomize* d_create_scenario_names;

    Gtk::DrawingArea *bigmap_drawingarea;
    Gtk::EventBox *bigmap_eventbox;
    Gtk::CheckMenuItem *fullscreen_menuitem;
    Gtk::MenuItem *new_map_menuitem;
    Gtk::MenuItem *load_map_menuitem;
    Gtk::MenuItem *save_map_menuitem;
    Gtk::MenuItem *save_map_as_menuitem;
    Gtk::MenuItem *import_map_from_sav_menuitem;
    Gtk::MenuItem *export_as_bitmap_menuitem;
    Gtk::MenuItem *export_as_bitmap_no_game_objects_menuitem;
    Gtk::MenuItem *validate_menuitem;
    Gtk::MenuItem *quit_menuitem;
    Gtk::MenuItem *edit_players_menuitem;
    Gtk::MenuItem *edit_map_info_menuitem;
    Gtk::MenuItem *toggle_tile_graphics_menuitem;
    Gtk::MenuItem *toggle_grid_menuitem;
    Gtk::MenuItem *smooth_map_menuitem;
    Gtk::MenuItem *smooth_screen_menuitem;
    Gtk::MenuItem *switch_sets_menuitem;
    Gtk::MenuItem *edit_items_menuitem ;
    Gtk::MenuItem *edit_rewards_menuitem;
    Gtk::MenuItem *random_all_cities_menuitem;
    Gtk::MenuItem *random_unnamed_cities_menuitem;
    Gtk::MenuItem *random_all_ruins_menuitem;
    Gtk::MenuItem *random_unnamed_ruins_menuitem;
    Gtk::MenuItem *random_all_temples_menuitem;
    Gtk::MenuItem *random_unnamed_temples_menuitem;
    Gtk::MenuItem *random_all_signs_menuitem;
    Gtk::MenuItem *random_unnamed_signs_menuitem;
    Gtk::MenuItem *help_about_menuitem;
    Gtk::HBox *terrain_tile_style_hbox;
    Gtk::DrawingArea *map_drawingarea;
    std::string current_save_filename;
    bool needs_saving;
    Gtk::Table *terrain_type_table;
    Gtk::Label *mouse_position_label;
    Gtk::RadioButton *pointer_radiobutton;
    Gtk::Tooltips tooltips;
    Gtk::Box *players_hbox;

    
    GdkEventButton *button_event;

    bool on_delete_event(GdkEventAny *e);

    bool on_bigmap_mouse_button_event(GdkEventButton *e);
    bool on_bigmap_mouse_motion_event(GdkEventMotion *e);
    bool on_bigmap_key_event(GdkEventKey *e);
    bool on_bigmap_leave_event(GdkEventCrossing *e);

    bool on_smallmap_mouse_button_event(GdkEventButton *e);
    bool on_smallmap_mouse_motion_event(GdkEventMotion *e);
    
    void on_new_map_activated();
    void on_load_map_activated();
    void on_save_map_activated();
    void on_import_map_activated();
    void on_export_as_bitmap_activated();
    void on_export_as_bitmap_no_game_objects_activated();
    void on_validate_activated();
    void on_save_map_as_activated();
    void on_quit_activated();
    bool quit();
    void on_edit_map_info_activated();
    void on_edit_players_activated();
    void on_smooth_map_activated();
    void on_smooth_screen_activated();
    void on_switch_sets_activated();
    void on_edit_items_activated();
    void on_edit_rewards_activated();

    void on_fullscreen_activated();
    void on_tile_graphics_toggled();
    void on_grid_toggled();

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
    

    void clear_save_file_of_scenario_specific_data();
    Tile::Type get_terrain();
    int get_tile_style_id();
    void setup_pointer_radiobutton(Glib::RefPtr<Gtk::Builder> xml,
	std::string prefix, std::string image_file,
	EditorBigMap::Pointer pointer, int size);
    void setup_terrain_radiobuttons();

    void init_maps();
    void set_filled_map(int width, int height, int fill_style, 
			std::string tileset, std::string shieldset, 
			std::string cityset, std::string armyset);
    void set_random_map(int width, int height,
			int grass, int water, int swamp, int forest,
			int hills, int mountains,
			int cities, int ruins, int temples, int signposts,
			std::string tileset, std::string shieldset,
			std::string cityset, std::string armyset);

    void clear_map_state();
    void init_map_state();
    void remove_tile_style_buttons();
    void setup_tile_style_buttons(Tile::Type terrain);
    void randomize_signpost(Signpost *signpost);
    void randomize_city(City *city);
    void randomize_ruin(Ruin *ruin);

    // map callbacks
    void on_smallmap_changed(Glib::RefPtr<Gdk::Pixmap> map, Gdk::Rectangle r);
    void on_bigmap_changed(Glib::RefPtr<Gdk::Pixmap> map);
    void on_objects_selected(std::vector<UniquelyIdentified *> objects);
    void on_mouse_on_tile(Vector<int> tile);
    
    void popup_dialog_for_object(UniquelyIdentified *object);

    void auto_select_appropriate_pointer();

    bool on_bigmap_exposed(GdkEventExpose *event);
    bool on_smallmap_exposed(GdkEventExpose *event);
    void on_bigmap_surface_changed(Gtk::Allocation box);
    void redraw();
    void fill_players();

    struct PlayerItem
    {
	Gtk::ToggleButton *button;
	int player_id;
    };
    std::list<PlayerItem> player_buttons;
    void on_player_toggled(PlayerItem item);

    int d_width;
    int d_height;
    
};

#endif
