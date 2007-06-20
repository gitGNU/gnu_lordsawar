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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

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

class SDL_Surface;
class EditorBigMap;
class SmallMap;
class GameScenario;

// the top-level window of the map with menu, bigmap, smallmap, drawing
// controls
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
    Gtk::Image *map_image;
    std::string current_save_filename;
    Gtk::Table *terrain_type_table;
    Gtk::Tooltips tooltips;

    std::auto_ptr<EditorBigMap> bigmap;
    std::auto_ptr<SmallMap> smallmap;
    
    bool sdl_inited;

    std::auto_ptr<GameScenario> game_scenario;

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
    void on_edit_info_activated();
    void on_edit_players_activated();

    void on_fullscreen_activated();

    
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

    std::vector<TerrainItem> terrain_items;
    
    void on_pointer_radiobutton_toggled();
    
    Tile::Type get_terrain();
    void setup_pointer_radiobutton(Glib::RefPtr<Gnome::Glade::Xml> xml,
	std::string prefix, std::string image_file,
	EditorBigMap::Pointer pointer, int size);
    void setup_terrain_radiobuttons();

    void init_maps();
    void on_smallmap_changed(SDL_Surface *map);

    
public:
    // not part of the API, but for surface_attached_helper
    void on_sdl_surface_changed();
};

#endif
