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

#ifndef GAME_H
#define GAME_H

#include <memory>
#include <sigc++/signal.h>
#include <glibmm/ustring.h>
#include "rectangle.h"

#include "input-events.h"
#include "rectangle.h"
#include "sidebar-stats.h"
#include "map-tip-position.h"
#include "callback-enums.h"
#include "army.h"
#include "fight.h"

#include "bigmap.h"
#include "smallmap.h"

class SmallMap;
class BigMap;
class NextTurn;
class GameScenario;
class Hero;
class City;
class Stack;
class Player;
class Ruin;
class Fight;
class Quest;
class Stack;

/** Controls a game.
  * 
  * Manages the big and small map, the game scenario and who's turn it is, etc.
  * It's mostly a puppeteer class that connects the various other classes with
  * signals and callbacks. 
  *
  */
class Game
{
 public:
    Game(GameScenario* gameScenario);
    ~Game();

    //! Possibly recruit a new hero at the beginning of a turn
    void maybeRecruitHero(Player *p);

    void redraw();
    void size_changed();
    
    void smallmap_mouse_button_event(MouseButtonEvent e);
    void smallmap_mouse_motion_event(MouseMotionEvent e);
    
    void mouse_button_event(MouseButtonEvent e);
    void mouse_motion_event(MouseMotionEvent e);
    void key_press_event(KeyPressEvent e);

    void select_prev_stack();
    void select_next_stack();
    void select_next_movable_stack();
    void center_selected_stack();
    void defend_selected_stack();
    void search_selected_stack();
    void move_selected_stack();
    void move_all_stacks();
    void end_turn();

    void center_view(Vector<int> p);

    //! Callback which redraws the map and checks if the active stack has died
    bool stackRedraw();
    

    /** Cares for correct connecting of the events. There are enough signals to
     * care for that a separate function seems justified.
     */
    void connectEvents();

    // initiate game flow
    void startGame();
    void loadGame();
    // stop game flow, clean up
    void stopGame();

    // save current game, returns true if successful
    bool saveGame(std::string file);

    //! Called whenever a stack has moved, updates the map etc.
    void stackUpdate(Stack* s);

    //! Called whenever a stack has died; updates bigmap as well
    void stackDied(Stack* s);

    //! Callback when the army of a human player reaches a new level.
    Army::Stat newLevelArmy(Army* a);

    //! Callback when an army gets a new medal.
    void newMedalArmy(Army* a);

    // locks/unlocks the input widgets during computer turns
    void lock_inputs();
    void unlock_inputs();

    //! Function that puts all units on the tile into the current active stack
    void selectAllStack();

    //! Function that unselects the current active stack
    void unselectStack();

    sigc::signal<void, Vector<int> > current_map_position;
    sigc::signal<void, SDL_Surface *> smallmap_changed;
    sigc::signal<void, SidebarStats> sidebar_stats_changed;
    sigc::signal<void, bool>
	can_select_prev_stack,
	can_select_next_stack,
	can_select_next_movable_stack,
	can_center_selected_stack,
	can_defend_selected_stack,
	can_search_selected_stack,
	can_move_selected_stack,
	can_move_all_stacks,
	can_end_turn;
    sigc::signal<void, Stack *> stack_info_changed;
    sigc::signal<void, Glib::ustring, MapTipPosition> map_tip_changed;
    sigc::signal<void, Ruin*, int> ruin_searched;
    sigc::signal<void, Fight &> fight_started;
    sigc::signal<void, Stack *, Stack *> ruinfight_started;
    sigc::signal<void, Fight::Result> ruinfight_finished;
    sigc::signal<bool, Player *, Hero *, City *, int> hero_offers_service;
    sigc::signal<bool, Temple *, int> temple_visited;
    sigc::signal<void, Hero *, Quest *> quest_assigned;
    sigc::signal<CityDefeatedAction, City *, int> city_defeated;
    sigc::signal<void, City *, int> city_pillaged;
    sigc::signal<void, City *, int> city_sacked;
    sigc::signal<void, City *> city_razed;
    sigc::signal<void, City *> city_visited;
    sigc::signal<void, Player *, unsigned int> next_player_turn;
    sigc::signal<void, int> hero_arrives;
    sigc::signal<void, Army *> medal_awarded_to_army;
    sigc::signal<Army::Stat, Army *> army_gains_level;
    sigc::signal<void, Player *> game_loaded;
    
 private:

    /**
    \brief The function reads in the heronames file and produces a
    set of hero templates to be randomly selected from
    */
    int loadHeroTemplates();

    // centers the map on a city of the active player
    void center_view_on_city();

    void update_control_panel();
    void update_sidebar_stats();
    void update_stack_info();	// emit stack_info_changed
    void clear_stack_info();

    //! Loads the button images
    void loadImages();

    // bigmap callbacks
    void on_mouse_on_tile(Vector<int> pos);
    void on_stack_selected(Stack* s);
    void on_city_selected(City* c, MapTipPosition pos, bool brief);
    void on_ruin_selected(Ruin* r, MapTipPosition pos);
    void on_temple_selected(Temple* t, MapTipPosition pos);
    void on_signpost_selected(Signpost* s, MapTipPosition pos);
    void on_bigmap_view_changed(Rectangle view);

    // smallmap callbacks
    void on_smallmap_view_changed(Rectangle view);
    void on_smallmap_changed(SDL_Surface *map);

    // misc. callbacks
    void invading_city(City* city);
    // returns true if the next turn loop should stop
    bool init_turn_for_player(Player* p);

    
    void looting_city(City *city, int &gold);
    
    // the part of the map that currently is visible, measured in tiles
    Rectangle map_view;
    
    GameScenario* d_gameScenario;
    NextTurn* d_nextTurn;
    std::auto_ptr<BigMap> bigmap;
    std::auto_ptr<SmallMap> smallmap;

    /* the contents of the heronames data file */
    std::vector<Hero*> d_herotemplates[MAX_PLAYERS];

    bool input_locked;
};

#endif
