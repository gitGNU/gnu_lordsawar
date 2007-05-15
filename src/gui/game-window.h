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

#ifndef GUI_GAME_WINDOW_H
#define GUI_GAME_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <gtkmm/window.h>
#include <gtkmm/dialog.h>
#include <gtkmm/container.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/box.h>
#include <gtkmm/tooltips.h>

#include "../game-parameters.h"
#include "../sidebar-stats.h"
#include "../stack-info.h"
#include "../stack.h"
#include "../fight.h"
#include "../map-tip-position.h"
#include "../callback-enums.h"
#include "../vector.h"
#include "../army.h"

class Game;
class SDL_Surface;
class Ruin;
class Fight;
class Hero;
class Player;
class Temple;
class Quest;
class City;

/** The main window in which all the game action is going on.
  *
  * This class takes cares of the widget side of the things around the action
  * area, delegating the actual drawing to the model classes, but is also
  * responsible for popping up dialog windows in response to the action.
  * It has a pane at the bottom with information about the currently selected
  * stack and a pane to the right with the small map, buttons and statistics. 
  */
class GameWindow: public sigc::trackable
{
 public:
    GameWindow();
    ~GameWindow();

    void show();
    void hide();

    // initialize the SDL widget 
    void init(int width, int height);

    // setup a new game
    void new_game(GameParameters g);
    
    // setup and use the game stored under file_path
    void load_game(const std::string &file_path, bool start);
    
    // emitted when the game has ended and it is time to show the splash again
    sigc::signal<void> game_ended;
    
    sigc::signal<void> quit_requested;

    sigc::signal<void> sdl_initialized;
    
 private:
    std::auto_ptr<Gtk::Window> window;
    std::auto_ptr<Gtk::Window> map_tip;	// tooltip appears over the map
    std::auto_ptr<Gtk::Window> stack_info_tip; // tooltip for stack info
    Gtk::Container *sdl_container;
    Gtk::Widget *sdl_widget;
    Gtk::Box *stack_info_box;
    Gtk::Image *map_image;
    Gtk::Label *stats_label;
    Glib::ustring stats_text;	// the text into which the stats are inserted

    // the button control panel
    Gtk::Button *prev_button;
    Gtk::Button *next_button;
    Gtk::Button *next_movable_button;
    Gtk::Button *center_button;
    Gtk::Button *defend_button;
    Gtk::Button *search_button;
    Gtk::Button *move_button;
    Gtk::Button *move_all_button;
    Gtk::Button *end_turn_button;
    
    Gtk::Tooltips tooltips;
    typedef std::vector<Gtk::ToggleButton *> army_buttons_type;
    army_buttons_type army_buttons;

    std::auto_ptr<Game> game;
    
    bool sdl_inited;

    bool on_delete_event(GdkEventAny *e);

    bool on_sdl_mouse_button_event(GdkEventButton *e);
    bool on_sdl_mouse_motion_event(GdkEventMotion *e);
    bool on_sdl_key_event(GdkEventKey *e);

    bool on_map_mouse_button_event(GdkEventButton *e);
    bool on_map_mouse_motion_event(GdkEventMotion *e);
    
    void on_load_game_activated();
    void on_save_game_activated();
    void on_save_game_as_activated();
    void on_resign_game_activated();
    void on_quit_activated();

    // stack info pane at the bottom
    void on_army_toggled(Gtk::ToggleButton *toggle, Army *army);
    bool on_army_button_event(GdkEventButton *e, Army *army);
    void on_army_button_has_size();
    void clear_army_buttons();
    void ensure_one_army_button_active();
    void set_army_button_tooltip(Gtk::ToggleButton *toggle);

    // game callbacks
    void on_sidebar_stats_changed(SidebarStats s);
    void on_smallmap_changed(SDL_Surface *map);
    void on_stack_info_changed(StackInfo s);
    void on_map_tip_changed(Glib::ustring tip, MapTipPosition pos);
    void on_ruin_searched(Ruin *ruin, int gold_found);
    void on_fight_started(Fight &fight);
    void on_ruinfight_started(Stack *attackers, Stack *defenders);
    void on_ruinfight_finished(Fight::Result result);
    bool on_hero_offers_service(Player *player, Hero *hero, City *city, int gold);
    bool on_temple_visited(Temple *temple, int blessCount);
    void on_quest_assigned(Hero *hero, Quest *quest);
    CityDefeatedAction on_city_defeated(City *city, int gold);
    void on_city_pillaged(City *city, int gold);
    void on_city_sacked(City *city, int gold);
    void on_city_visited(City *city);
    void on_next_player_turn(Player *player, unsigned int turn_number);
    void on_hero_brings_allies(int alliesCount);
    void on_medal_awarded_to_army(Army *army);
    Army::Stat on_army_gains_level(Army *army);

    // helpers
    void show_map_tip(Glib::ustring msg, MapTipPosition pos);
    void on_city_looted(City *city, int gold);
    void hide_map_tip();
    
public:
    // not part of the API, but for surface_attached_helper
    void on_sdl_surface_changed();
};

#endif
