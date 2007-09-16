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
#include <gtkmm/eventbox.h>
#include <gtkmm/container.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/box.h>
#include <gtkmm/checkmenuitem.h>

#include "army-info-tip.h"
#include "stack-info-tip.h"

#include "../game-parameters.h"
#include "../sidebar-stats.h"
#include "../stack.h"
#include "../fight.h"
#include "../map-tip-position.h"
#include "../callback-enums.h"
#include "../vector.h"
#include "../army.h"
#include "../GraphicsCache.h"

class Game;
class SDL_Surface;
class Ruin;
class Fight;
class Hero;
class Player;
class Temple;
class Quest;
class City;
class Reward;

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
    void load_game(std::string file_path);
    
    // emitted when the game has ended and it is time to show the splash again
    sigc::signal<void> game_ended;
    
    sigc::signal<void> quit_requested;

    sigc::signal<void> sdl_initialized;
    
 private:
    std::auto_ptr<Gtk::Window> window;
    std::auto_ptr<Gtk::Window> map_tip;	// tooltip appears over the map
    std::auto_ptr<Gtk::Window> stack_tip;// tooltip appears over the map
    Gtk::Container *sdl_container;
    Gtk::Widget *sdl_widget;
    Gtk::CheckMenuItem *fullscreen_menuitem;
    Gtk::MenuItem *end_turn_menuitem;
    Gtk::MenuItem *move_all_menuitem;
    Gtk::MenuItem *search_menuitem;
    Gtk::MenuItem *inspect_menuitem;
    Gtk::MenuItem *disband_menuitem;
    Gtk::MenuItem *signpost_menuitem;
    Gtk::MenuItem *city_history_menuitem;
    Gtk::MenuItem *event_history_menuitem;
    Gtk::MenuItem *gold_history_menuitem;
    Gtk::MenuItem *winner_history_menuitem;
    Gtk::MenuItem *plant_standard_menuitem;
    Gtk::MenuItem *group_ungroup_menuitem;
    Gtk::MenuItem *leave_menuitem;
    Gtk::MenuItem *next_menuitem;
    Gtk::Box *stack_info_box;
    Gtk::Box *stack_info_container;
    Gtk::Label *group_moves_label;
    Gtk::Image *terrain_image;
    Gtk::Box *stats_box;
    Gtk::Image *map_image;
    Gtk::Label *stats_label;
    Glib::ustring stats_text;	// the text into which the stats are inserted

    Gtk::Label *cities_stats_label;
    Gtk::Label *gold_stats_label;
    Gtk::Label *income_stats_label;
    Gtk::Label *turn_label;
    Gtk::HBox *turn_hbox;
    Gtk::Image *shield_image[MAX_PLAYERS];
    
    // the button control panel
    Gtk::Button *next_movable_button;
    Gtk::Button *center_button;
    Gtk::Button *defend_button;
    Gtk::Button *park_button;
    Gtk::Button *search_button;
    Gtk::Button *move_button;
    Gtk::Button *move_all_button;
    Gtk::Button *end_turn_button;

    Stack *currently_selected_stack;
    std::auto_ptr<ArmyInfoTip> army_info_tip;
    std::auto_ptr<StackInfoTip> stack_info_tip;
    typedef std::vector<Gtk::ToggleButton *> army_buttons_type;
    army_buttons_type army_buttons;
    Gtk::EventBox *map_eventbox;
    Gtk::ToggleButton *group_ungroup_toggle;

    std::string current_save_filename;

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
    void on_quit_activated();
    void on_quests_activated();
    void on_disband_activated();
    void on_resign_activated();
    void on_signpost_activated();
    void on_inspect_activated();
    void on_plant_standard_activated();
    void on_item_bonus_activated();
    void on_army_report_activated();
    void on_city_report_activated();
    void on_gold_report_activated();
    void on_production_report_activated();
    void on_winning_report_activated();

    void on_fullscreen_activated();
    void on_preferences_activated();
    void on_group_ungroup_activated();
    
    void on_fight_order_activated();
    void on_levels_activated();
    void on_ruin_report_activated();
    void on_army_bonus_activated();
    void on_city_history_activated();
    void on_event_history_activated();
    void on_gold_history_activated();
    void on_winner_history_activated();

    void on_message_requested(std::string msg);
    
    // info pane at the bottom
    void show_stats();
    void show_stack(Stack *s);
    void fill_in_group_info (Stack *s);
    void on_army_toggled(Gtk::ToggleButton *toggle, Army *army);
    void on_group_toggled(Gtk::ToggleButton *toggle);
    bool on_army_button_event(GdkEventButton *e,
			      Gtk::ToggleButton *toggle, Army *army);
    void on_army_button_has_size();
    void clear_army_buttons();
    void update_army_buttons();
    void ensure_one_army_button_active();

    // shield set on the top
    void show_shield_turn();

    // game callbacks
    void on_sidebar_stats_changed(SidebarStats s);
    void on_smallmap_changed(SDL_Surface *map);
    void on_bigmap_cursor_changed(GraphicsCache::CursorType cursor);
    void on_stack_info_changed(Stack *s);
    void on_map_tip_changed(Glib::ustring tip, MapTipPosition pos);
    void on_stack_tip_changed(Stack *s, MapTipPosition pos);
    void on_ruin_searched(Ruin *ruin, Stack *s, Reward *reward);
    void on_sage_visited(Ruin *ruin, Stack *s);
    void on_fight_started(Fight &fight);
    void on_ruinfight_started(Stack *attackers, Stack *defenders);
    void on_ruinfight_finished(Fight::Result result);
    bool on_hero_offers_service(Player *player, Hero *hero, City *city, int gold);
    bool on_temple_searched(bool hasHero, Temple *temple, int blessCount);
    void on_quest_assigned(Hero *hero, Quest *quest);
    CityDefeatedAction on_city_defeated(City *city, int gold);
    void on_city_pillaged(City *city, int gold, int pillaged_army_type);
    void on_city_sacked(City *city, int gold, std::list<Uint32> sacked_types);
    void on_city_razed(City *city);
    void on_city_visited(City *city);
    void on_ruin_visited(Ruin *ruin);
    void on_temple_visited(Temple *temple);
    void on_next_player_turn(Player *player, unsigned int turn_number);
    void on_hero_brings_allies(int alliesCount);
    void on_medal_awarded_to_army(Army *army);
    Army::Stat on_army_gains_level(Army *army);
    void on_game_loaded(Player *player);
    void on_game_over(Player *winner);
    void on_player_died(Player *player);

    // quest manager callbacks
    void on_quest_completed(Quest *quest, Reward *reward);
    void on_quest_expired(Quest *quest);
    
    // helpers
    void show_map_tip(Glib::ustring msg, MapTipPosition pos);
    void on_city_looted(City *city, int gold);
    void hide_map_tip();

    void setup_game(std::string file_path);
    void setup_signals();
    void stop_game();
    std::list<sigc::connection> connections;
    
    void setup_menuitem(Gtk::MenuItem*, sigc::slot<void> , sigc::signal<void, bool> &);
    void setup_button(Gtk::Button *, sigc::slot<void> slot, sigc::signal<void, bool> &);
public:
    // not part of the API, but for surface_attached_helper
    void on_sdl_surface_changed();
    bool d_quick_fights; //do we speed up fights for this player's turn?
};

#endif
