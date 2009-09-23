//  Copyright (C) 2007, 2008 Ole Laursen
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

#ifndef GUI_GAME_WINDOW_H
#define GUI_GAME_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>
#include <gtkmm.h>

#include "army-info-tip.h"
#include "stack-info-tip.h"
#include "decorated.h"

#include "game-parameters.h"
#include "sidebar-stats.h"
#include "stack.h"
#include "fight.h"
#include "map-tip-position.h"
#include "callback-enums.h"
#include "vector.h"
#include "army.h"
#include "GraphicsCache.h"
#include "GameScenario.h"

class Game;
class Ruin;
class Fight;
class Hero;
class HeroProto;
class Player;
class Temple;
class Quest;
class City;
class Reward;
class Reward_Ruin;
class NextTurn;

/** The main window in which all the game action is going on.
  *
  * This class takes cares of the widget side of the things around the action
  * area, delegating the actual drawing to the model classes, but is also
  * responsible for popping up dialog windows in response to the action.
  * It has a pane at the bottom with information about the currently selected
  * stack and a pane to the right with the small map, buttons and statistics. 
  */
class GameWindow: public Decorated
{
 public:
    GameWindow();
    ~GameWindow();

    void show();
    void hide();

    // initialize the big map widget 
    void init(int width, int height);

    // setup a new game
    void new_game(GameScenario *game_scenario, NextTurn *next_turn);

    // setup a new network game
    void new_network_game(GameScenario *game_scenario, NextTurn *next_turn);

    void continue_network_game(NextTurn *next_turn);
    
    // load the game
    void load_game(GameScenario *game_scenario, NextTurn *next_turn);

    // emitted when the game has ended and it is time to show the splash again
    sigc::signal<void> game_ended;
    
    sigc::signal<void> show_lobby;

    sigc::signal<void> quit_requested;

    sigc::signal<void> sdl_initialized;
    
    Gtk::Window *get_window() const {return window.get();};

    void on_player_replaced(Player *p);
 private:
    std::auto_ptr<Gtk::Window> window;
    std::auto_ptr<Gtk::Window> map_tip;	// tooltip appears over the map
    std::auto_ptr<Gtk::Window> stack_tip;// tooltip appears over the map
    Gtk::Container *bigmap_container;
    Gtk::Container *map_container;
    Gtk::MenuBar *menubar;
    Gtk::CheckMenuItem *fullscreen_menuitem;
    Gtk::MenuItem *load_game_menuitem;
    Gtk::MenuItem *save_game_menuitem;
    Gtk::MenuItem *save_game_as_menuitem;
    Gtk::MenuItem *show_lobby_menuitem;
    Gtk::MenuItem *end_turn_menuitem;
    Gtk::MenuItem *move_all_menuitem;
    Gtk::MenuItem *search_menuitem;
    Gtk::MenuItem *disband_menuitem;
    Gtk::MenuItem *stack_info_menuitem;
    Gtk::MenuItem *signpost_menuitem;
    Gtk::MenuItem *diplomacy_report_menuitem;
    Gtk::MenuItem *city_history_menuitem;
    Gtk::MenuItem *event_history_menuitem;
    Gtk::MenuItem *gold_history_menuitem;
    Gtk::MenuItem *winner_history_menuitem;
    Gtk::MenuItem *plant_standard_menuitem;
    Gtk::MenuItem *group_ungroup_menuitem;
    Gtk::MenuItem *leave_menuitem;
    Gtk::MenuItem *next_menuitem;
    Gtk::MenuItem *fight_order_menuitem;
    Gtk::MenuItem *resign_menuitem;
    Gtk::MenuItem *production_menuitem;
    Gtk::MenuItem *cities_menuitem;
    Gtk::MenuItem *build_menuitem;
    Gtk::MenuItem *vectoring_menuitem;
    Gtk::MenuItem *levels_menuitem;
    Gtk::MenuItem *inspect_menuitem;
    Gtk::MenuItem *ruin_report_menuitem;
    Gtk::MenuItem *army_bonus_menuitem;
    Gtk::MenuItem *item_bonus_menuitem;
    Gtk::MenuItem *production_report_menuitem;
    Gtk::MenuItem *triumphs_menuitem;
    Gtk::MenuItem *help_about_menuitem;
    Gtk::MenuItem *online_help_menuitem;
    Gtk::MenuItem *quit_menuitem;
    Gtk::MenuItem *toggle_grid_menuitem;
    Gtk::MenuItem *army_report_menuitem;
    Gtk::MenuItem *city_report_menuitem;
    Gtk::MenuItem *gold_report_menuitem;
    Gtk::MenuItem *winning_report_menuitem;
    Gtk::MenuItem *quests_menuitem;
    Gtk::MenuItem *preferences_menuitem;
    Gtk::Box *stack_info_box;
    Gtk::Box *stack_info_container;
    Gtk::Label *group_moves_label;
    Gtk::Image *terrain_image;
    Gtk::Box *stats_box;
    Gtk::Box *progress_box;
    Gtk::ProgressBar *turn_progressbar;
    Gtk::Label *progress_status_label;
    Gtk::DrawingArea *map_drawingarea;
    Gtk::DrawingArea *bigmap_drawingarea;

    Gtk::Label *cities_stats_label;
    Gtk::Label *gold_stats_label;
    Gtk::Label *income_stats_label;
    Gtk::Label *upkeep_stats_label;
    Gtk::Label *turn_label;
    Gtk::HBox *turn_hbox;
    Gtk::Image *shield_image[MAX_PLAYERS];
    
    // the button control panel
    Gtk::Button *next_movable_button;
    Gtk::Button *center_button;
    Gtk::Button *diplomacy_button;
    Gtk::Button *defend_button;
    Gtk::Button *park_button;
    Gtk::Button *deselect_button;
    Gtk::Button *search_button;
    Gtk::Button *move_button;
    Gtk::Button *move_all_button;
    Gtk::Button *end_turn_button;
    Gtk::Button *nw_keypad_button;
    Gtk::Button *n_keypad_button;
    Gtk::Button *ne_keypad_button;
    Gtk::Button *e_keypad_button;
    Gtk::Button *w_keypad_button;
    Gtk::Button *sw_keypad_button;
    Gtk::Button *s_keypad_button;
    Gtk::Button *se_keypad_button;

    Stack *currently_selected_stack;
    std::auto_ptr<ArmyInfoTip> army_info_tip;
    std::auto_ptr<StackInfoTip> stack_info_tip;
    typedef std::vector<Gtk::ToggleButton *> army_buttons_type;
    army_buttons_type army_buttons;
    Gtk::EventBox *map_eventbox;
    Gtk::EventBox *bigmap_eventbox;
    Gtk::ToggleButton *group_ungroup_toggle;

    std::string current_save_filename;

    std::auto_ptr<Game> game;
    
    bool sdl_inited;

    bool on_delete_event(GdkEventAny *e);

    bool on_bigmap_mouse_button_event(GdkEventButton *e);
    bool on_bigmap_mouse_motion_event(GdkEventMotion *e);
    bool on_bigmap_key_event(GdkEventKey *e);
    bool on_bigmap_scroll_event(GdkEventScroll* event);

    bool on_smallmap_mouse_button_event(GdkEventButton *e);
    bool on_smallmap_mouse_motion_event(GdkEventMotion *e);
    
    void on_load_game_activated();
    void on_save_game_activated();
    void on_save_game_as_activated();
    void on_show_lobby_activated();
    void on_quit_activated();
    void on_game_stopped();
    void on_quests_activated();
    void on_disband_activated();
    void on_stack_info_activated();
    void on_resign_activated();
    void on_resignation_completed();
    void on_signpost_activated();
    void on_inspect_activated();
    void on_plant_standard_activated();
    void on_item_bonus_activated();
    void on_army_report_activated();
    void on_city_report_activated();
    void on_gold_report_activated();
    void on_production_report_activated();
    void on_winning_report_activated();
    void on_diplomacy_report_activated();
    void on_diplomacy_button_clicked();

    void on_fullscreen_activated();
    void on_preferences_activated();
    void on_group_ungroup_activated();
    
    void on_fight_order_activated();
    void on_levels_activated();
    void on_production_activated();
    void on_vectoring_activated();
    void on_grid_toggled();
    void on_ruin_report_activated();
    void on_army_bonus_activated();
    void on_city_history_activated();
    void on_event_history_activated();
    void on_gold_history_activated();
    void on_winner_history_activated();
    void on_triumphs_activated();
    void on_help_about_activated();
    void on_online_help_activated();

    void on_message_requested(std::string msg);
    
    // info pane at the bottom
    void show_stats();
    void show_progress();
    void show_stack(Stack *s);
    void fill_in_group_info (Stack *s);
    void on_army_toggled(Gtk::ToggleButton *toggle, Army *army);
    void on_group_toggled(Gtk::ToggleButton *toggle);
    bool on_army_button_event(GdkEventButton *e,
			      Gtk::ToggleButton *toggle, Army *army);
    void clear_army_buttons();
    void update_army_buttons();
    void ensure_one_army_button_active();

    // shield set on the top
    void show_shield_turn();

    // game callbacks
    void on_sidebar_stats_changed(SidebarStats s);
    void on_progress_status_changed(std::string status);
    void on_progress_changed();
    void on_smallmap_changed(Glib::RefPtr<Gdk::Pixmap> map, Gdk::Rectangle r);
    void on_smallmap_slid(Rectangle view);
    void on_bigmap_cursor_changed(GraphicsCache::CursorType cursor);
    void on_bigmap_changed(Glib::RefPtr<Gdk::Pixmap> map);
    void on_stack_info_changed(Stack *s);
    void on_bigmap_tip_changed(Glib::ustring tip, MapTipPosition pos);
    void on_stack_tip_changed(Stack *s, MapTipPosition pos);
    void on_ruin_searched(Ruin *ruin, Stack *s, Reward *reward);
    void on_sage_visited(Ruin *ruin, Stack *s);
    void on_ruin_rewarded(Reward_Ruin *reward);
    void on_fight_started(Fight &fight);
    void on_ruinfight_started(Stack *attackers, Stack *defenders);
    void on_ruinfight_finished(Fight::Result result);
    bool on_hero_offers_service(Player *player, HeroProto *hero, City *city, int gold);
    bool on_enemy_offers_surrender(int numEnemies);
    void on_surrender_answered (bool accepted);
    bool on_stack_considers_treachery (Player *me, Stack *stack, Player *them, Vector<int> pos);
    bool on_temple_searched(bool hasHero, Temple *temple, int blessCount);
    void on_quest_assigned(Hero *hero, Quest *quest);
    CityDefeatedAction on_city_defeated(City *city, int gold);
    void on_city_pillaged(City *city, int gold, int pillaged_army_type);
    void on_city_sacked(City *city, int gold, std::list<guint32> sacked_types);
    void on_city_razed(City *city);
    void on_city_visited(City *city);
    void on_ruin_visited(Ruin *ruin);
    void on_temple_visited(Temple *temple);
    void on_next_player_turn(Player *player, unsigned int turn_number);
    void on_remote_next_player_turn();
    void on_hero_brings_allies(int alliesCount);
    void on_medal_awarded_to_army(Army *army);
    Army::Stat on_army_gains_level(Army *army);
    void on_game_loaded(Player *player);
    void on_game_over(Player *winner);
    void on_player_died(Player *player);
    void on_advice_asked(float percent);
    void end_turn_play_by_mail ();

    // quest manager callbacks
    void on_quest_completed(Quest *quest, Reward *reward);
    void on_quest_expired(Quest *quest);
    
    // helpers
    void show_map_tip(Glib::ustring msg, MapTipPosition pos);
    void on_city_looted(City *city, int gold);
    void hide_map_tip();
    void change_diplomacy_button_image (bool proposals_present);
    void show_city_production_report (bool destitute);
    void update_diplomacy_button (bool sensitive);

    bool setup_game(GameScenario *game_scenario, NextTurn *nextTurn);
    void setup_signals(GameScenario *game_scenario);
    void stop_game(std::string action);
    std::list<sigc::connection> connections;
    
    void setup_menuitem(Gtk::MenuItem*, sigc::slot<void> , sigc::signal<void, bool> &);
    void setup_button(Gtk::Button *, sigc::slot<void> slot, sigc::signal<void, bool> &);
    void on_bigmap_surface_changed(Gtk::Allocation box);
    bool on_bigmap_exposed(GdkEventExpose *event);
    bool on_smallmap_exposed(GdkEventExpose *event);

public:
    std::vector<Glib::RefPtr<Gdk::Pixbuf> > d_button_images;
    std::vector<Glib::RefPtr<Gdk::Pixbuf> > d_arrow_images;
    bool d_quick_fights; //do we speed up fights for this player's turn?
    std::string stop_action; //hackhackhack
    std::string d_scenario;
    int d_gold;
    std::list<Hero*> d_heroes;
    std::string d_player_name;
    std::string d_load_filename;
    Gtk::Image *cities_stats_image;
    Gtk::Image *gold_stats_image;
    Gtk::Image *income_stats_image;
    Gtk::Image *upkeep_stats_image;
};

#endif
