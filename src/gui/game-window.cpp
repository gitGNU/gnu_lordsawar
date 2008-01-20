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

#include <config.h>

#include <iomanip>
#include <queue>
#include <SDL_video.h>
#include <assert.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/hide.h>

#include <libglademm/xml.h>
#include <gtkmm/widget.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/progressbar.h>
#include <gdkmm/cursor.h>
#include <gtkmm/frame.h>
#include <gtkmm/dialog.h>
#include <gtkmm/stock.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>
#include <gtkmm/alignment.h>

#include "game-window.h"

#include "gtksdl.h"
#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "error-utils.h"

#include "fight-window.h"
#include "city-window.h"
#include "army-gains-level-dialog.h"
#include "hero-dialog.h"
#include "sage-dialog.h"
#include "ruin-rewarded-dialog.h"
#include "hero-offer-dialog.h"
#include "quest-report-dialog.h"
#include "quest-assigned-dialog.h"
#include "quest-completed-dialog.h"
#include "preferences-dialog.h"
#include "fight-order-dialog.h"
#include "hero-levels-dialog.h"
#include "ruin-report-dialog.h"
#include "army-bonus-dialog.h"
#include "item-bonus-dialog.h"
#include "history-report-dialog.h"
#include "report-dialog.h"
#include "triumphs-dialog.h"

#include "../ucompose.hpp"
#include "../defs.h"
#include "../sound.h"
#include "../File.h"
#include "../game.h"
#include "../gamebigmap.h"
#include "../smallmap.h"
#include "../GameScenario.h"
#include "../army.h"
#include "../ruin.h"
#include "../path.h"
#include "../player.h"
#include "../stacklist.h"
#include "../signpostlist.h"
#include "../playerlist.h"
#include "../hero.h"
#include "../temple.h"
#include "../city.h"
#include "../Quest.h"
#include "../QuestsManager.h"
#include "../stack.h"
#include "../GraphicsCache.h"
#include "../QuestsManager.h"
#include "../Quest.h"
#include "../QCitySack.h"
#include "../QCityRaze.h"
#include "../QCityOccupy.h"
#include "../QPillageGold.h"
#include "../counter.h"
#include "../armysetlist.h"
#include "../CreateScenario.h"
#include "../reward.h"
#include "../Configuration.h"
#include "../GameMap.h"
#include "../Item.h"


GameWindow::GameWindow()
{
    sdl_inited = false;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/game-window.glade");

    Gtk::Window *w = 0;
    xml->get_widget("window", w);
    window.reset(w);

      w->signal_delete_event().connect
       (sigc::mem_fun(*this, &GameWindow::on_delete_event));

    xml->get_widget("sdl_container", sdl_container);
    xml->get_widget("stack_info_box", stack_info_box);
    xml->get_widget("stack_info_container", stack_info_container);
    xml->get_widget("group_moves_label", group_moves_label);
    xml->get_widget("group_togglebutton", group_ungroup_toggle);
    group_ungroup_toggle->signal_toggled().connect
      (sigc::bind(sigc::mem_fun(*this, &GameWindow::on_group_toggled),
		  group_ungroup_toggle));
    xml->get_widget("terrain_image", terrain_image);
    xml->get_widget("stats_box", stats_box);
    stack_info_box->hide();
    stats_box->show();

    // the map image
    xml->get_widget("map_image", map_image);
    xml->get_widget("map_eventbox", map_eventbox);
    map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
			     Gdk::POINTER_MOTION_MASK);
      map_eventbox->signal_button_press_event().connect
       (sigc::mem_fun(*this, &GameWindow::on_map_mouse_button_event));
      map_eventbox->signal_button_release_event().connect
       (sigc::mem_fun(*this, &GameWindow::on_map_mouse_button_event));
      map_eventbox->signal_motion_notify_event().connect
       (sigc::mem_fun(*this, &GameWindow::on_map_mouse_motion_event));

    // the stats
    Gtk::Image *image;
    xml->get_widget("cities_stats_image", image);
    image->property_file() = File::getMiscFile("various/smallcity.png");
    xml->get_widget("gold_stats_image", image);
    image->property_file() = File::getMiscFile("various/smalltreasury.png");
    xml->get_widget("income_stats_image", image);
    image->property_file() = File::getMiscFile("various/smallincome.png");
    xml->get_widget("upkeep_stats_image", image);
    image->property_file() = File::getMiscFile("various/smallupkeep.png");
    
    xml->get_widget("cities_stats_label", cities_stats_label);
    xml->get_widget("gold_stats_label", gold_stats_label);
    xml->get_widget("income_stats_label", income_stats_label);
    xml->get_widget("upkeep_stats_label", upkeep_stats_label);
    xml->get_widget("turn_label", turn_label);
    xml->get_widget("turn_hbox", turn_hbox);
    xml->get_widget("shield_image_0", shield_image[0]);
    xml->get_widget("shield_image_1", shield_image[1]);
    xml->get_widget("shield_image_2", shield_image[2]);
    xml->get_widget("shield_image_3", shield_image[3]);
    xml->get_widget("shield_image_4", shield_image[4]);
    xml->get_widget("shield_image_5", shield_image[5]);
    xml->get_widget("shield_image_6", shield_image[6]);
    xml->get_widget("shield_image_7", shield_image[7]);

    // the control panel
    xml->get_widget("next_movable_button", next_movable_button);
    xml->get_widget("center_button", center_button);
    xml->get_widget("defend_button", defend_button);
    xml->get_widget("park_button", park_button);
    xml->get_widget("deselect_button", deselect_button);
    xml->get_widget("search_button", search_button);
    xml->get_widget("move_button", move_button);
    xml->get_widget("move_all_button", move_all_button);
    xml->get_widget("end_turn_button", end_turn_button);

    // fill in imagery
    std::vector<Glib::RefPtr<Gdk::Pixbuf> > button_images
	= disassemble_row(File::getMiscFile("various/buttons.png"), 11);
    next_movable_button->add(*manage(new Gtk::Image(button_images[2])));
    center_button->add(*manage(new Gtk::Image(button_images[5])));
    defend_button->add(*manage(new Gtk::Image(button_images[6])));
    park_button->add(*manage(new Gtk::Image(button_images[1])));
    deselect_button->add(*manage(new Gtk::Image(button_images[7])));
    search_button->add(*manage(new Gtk::Image(button_images[9])));
    move_button->add(*manage(new Gtk::Image(button_images[3])));
    move_all_button->add(*manage(new Gtk::Image(button_images[4])));
    end_turn_button->add(*manage(new Gtk::Image(button_images[10])));

    // connect callbacks for the menu
    xml->connect_clicked("load_game_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_load_game_activated));
    xml->connect_clicked("save_game_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_save_game_activated));
    xml->connect_clicked("save_game_as_menuitem", 
			 sigc::mem_fun(*this, &GameWindow::on_save_game_as_activated));
    xml->connect_clicked("quit_menuitem", 
			 sigc::mem_fun(*this, &GameWindow::on_quit_activated));
    xml->connect_clicked("army_report_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_army_report_activated));
    xml->connect_clicked("city_report_menuitem", 
			 sigc::mem_fun(*this, &GameWindow::on_city_report_activated));
    xml->connect_clicked("gold_report_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_gold_report_activated));
    xml->connect_clicked("winning_report_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_winning_report_activated));
    xml->connect_clicked("quests_menuitem", 
			 sigc::mem_fun(*this, &GameWindow::on_quests_activated));
    xml->connect_clicked("fullscreen_menuitem", 
			 sigc::mem_fun(*this, &GameWindow::on_fullscreen_activated));
    xml->get_widget("fullscreen_menuitem", fullscreen_menuitem);
    xml->connect_clicked("preferences_menuitem", 
			 sigc::mem_fun(*this, &GameWindow::on_preferences_activated));

    xml->get_widget("end_turn_menuitem", end_turn_menuitem);
    xml->get_widget("move_all_menuitem", move_all_menuitem);
    xml->get_widget("disband_menuitem", disband_menuitem);
    xml->get_widget("signpost_menuitem", signpost_menuitem);
    xml->get_widget("search_menuitem", search_menuitem);
    xml->get_widget("inspect_menuitem", inspect_menuitem);
    xml->get_widget("plant_standard_menuitem", plant_standard_menuitem);
    xml->get_widget("city_history_menuitem", city_history_menuitem);
    xml->get_widget("event_history_menuitem", event_history_menuitem);
    xml->get_widget("gold_history_menuitem", gold_history_menuitem);
    xml->get_widget("winner_history_menuitem", winner_history_menuitem);
    xml->get_widget("group_ungroup_menuitem", group_ungroup_menuitem);
    xml->get_widget("leave_menuitem", leave_menuitem);
    xml->get_widget("next_menuitem", next_menuitem);

    xml->connect_clicked("fight_order_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_fight_order_activated));
    xml->connect_clicked("resign_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_resign_activated));
    xml->connect_clicked("levels_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_levels_activated));
    xml->connect_clicked("ruin_report_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_ruin_report_activated));
    xml->connect_clicked("army_bonus_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_army_bonus_activated));
    xml->connect_clicked("item_bonus_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_item_bonus_activated));
    xml->connect_clicked("production_report_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_production_report_activated));
    xml->connect_clicked("triumphs_menuitem",
			 sigc::mem_fun(*this, &GameWindow::on_triumphs_activated));
    d_quick_fights = false;
}

GameWindow::~GameWindow()
{
  std::list<sigc::connection>::iterator it = connections.begin();
  for (; it != connections.end(); it++) 
    (*it).disconnect();
  connections.clear();
    clear_army_buttons();
}

void GameWindow::show()
{
    next_movable_button->show_all();
    center_button->show_all();
    defend_button->show_all();
    park_button->show_all();
    deselect_button->show_all();
    search_button->show_all();
    move_button->show_all();
    move_all_button->show_all();
    end_turn_button->show_all();
    
    sdl_container->show_all();
    window->show();
}

void GameWindow::hide()
{
    window->hide();
}

namespace 
{
    void surface_attached_helper(GtkSDL *gtksdl, gpointer data)
    {
	static_cast<GameWindow *>(data)->on_sdl_surface_changed();
    }
}

void GameWindow::init(int width, int height)
{
    sdl_widget
	= Gtk::manage(Glib::wrap(gtk_sdl_new(width, height, 0, SDL_SWSURFACE)));

    sdl_widget->set_flags(Gtk::CAN_FOCUS);

    sdl_widget->grab_focus();
    sdl_widget->add_events(Gdk::KEY_PRESS_MASK | 
		  Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
	          Gdk::POINTER_MOTION_MASK);

    sdl_widget->signal_key_press_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_sdl_key_event));
    sdl_widget->signal_key_release_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_sdl_key_event));
    sdl_widget->signal_button_press_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_sdl_mouse_button_event));
    sdl_widget->signal_button_release_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_sdl_mouse_button_event));
    sdl_widget->signal_motion_notify_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_sdl_mouse_motion_event));
    
    // connect to the special signal that signifies that a new surface has been
    // generated and attached to the widget
    g_signal_connect(G_OBJECT(sdl_widget->gobj()), "surface-attached",
		     G_CALLBACK(surface_attached_helper), this);
    
    sdl_container->add(*sdl_widget);
}

std::string
create_and_dump_scenario(const std::string &file, const GameParameters &g)
{
    CreateScenario creator;

    // then fill the other players
    int c = 0;
    int army_id = Armysetlist::getInstance()->getArmysetId(g.army_theme);
    for (std::vector<GameParameters::Player>::const_iterator
	     i = g.players.begin(), end = g.players.end();
	 i != end; ++i, ++c) {
	
	if (i->type == GameParameters::Player::OFF)
	{
            fl_counter->getNextId();
	    continue;
	}
	
	Player::Type type;
	if (i->type == GameParameters::Player::EASY)
	    type = Player::AI_FAST;
	else if (i->type == GameParameters::Player::HARD)
	    type = Player::AI_SMART;
	else
	    type = Player::HUMAN;

	creator.addPlayer(i->name, army_id, Player::get_color_for_no(c), type);
    }

    // the neutral player must come last so it has the highest id among players
    creator.addNeutral(_("Neutral"), army_id, Player::get_color_for_neutral(),
		       Player::AI_DUMMY);

    // now fill in some map information
    creator.setMapTiles(g.tile_theme);
    creator.setNoCities(g.map.cities);
    creator.setNoRuins(g.map.ruins);
    creator.setNoTemples(g.map.temples);

    // terrain: the scenario generator also accepts input with a sum of
    // more than 100%, so the thing is rather easy here
    creator.setPercentages(g.map.grass, g.map.water, g.map.forest, g.map.swamp,
			   g.map.hills, g.map.mountains);

    int area = g.map.width * g.map.height;
    creator.setNoSignposts(int(area * (g.map.grass / 100.0) * 0.0030));

    // tell it the dimensions
    creator.setWidth(g.map.width);
    creator.setHeight(g.map.height);

    // and tell it the turn mode
    if (g.process_armies == GameParameters::PROCESS_ARMIES_AT_PLAYERS_TURN)
        creator.setTurnmode(true);
    else
	creator.setTurnmode(false);
	
    // now create the map and dump the created map
    std::string path = File::getSavePath();
    path += file;
    
    creator.create(g);
    creator.dump(path);
    
    return path;
}

void GameWindow::new_game(GameParameters g)
{
    if (g.map_path.empty()) {
	// construct new random scenario if we're not going to load the game
	std::string path = create_and_dump_scenario("random.map", g);
	g.map_path = path;
    }

    setup_game(g.map_path);
    setup_signals();
    game->startGame();
}

void GameWindow::load_game(std::string file_path)
{
    current_save_filename = file_path;
    setup_game(file_path);
    setup_signals();
    game->loadGame();
    game->redraw();
}

void GameWindow::setup_button(Gtk::Button *button,
			      sigc::slot<void> slot,
			      sigc::signal<void, bool> &game_signal)
{
  connections.push_back (button->signal_clicked().connect(slot));
  connections.push_back 
    (game_signal.connect(sigc::mem_fun(button, &Gtk::Widget::set_sensitive)));
}

void GameWindow::setup_menuitem(Gtk::MenuItem *item,
				sigc::slot<void> slot,
				sigc::signal<void, bool> &game_signal)
{
  connections.push_back (item->signal_activate().connect(slot));
  connections.push_back 
    (game_signal.connect(sigc::mem_fun(item, &Gtk::Widget::set_sensitive)));
}

void GameWindow::setup_signals()
{
  // get rid of the connections that might be still around from last time
  std::list<sigc::connection>::iterator it = connections.begin();
  for (; it != connections.end(); it++) 
    (*it).disconnect();
  connections.clear();

  setup_button(next_movable_button,
	       sigc::mem_fun(game.get(), &Game::select_next_movable_stack),
	       game->can_select_next_movable_stack);
  setup_button(center_button,
	       sigc::mem_fun(game.get(), &Game::center_selected_stack),
	       game->can_center_selected_stack);
  setup_button(defend_button,
	       sigc::mem_fun(game.get(), &Game::defend_selected_stack),
	       game->can_defend_selected_stack);
  setup_button(park_button,
	       sigc::mem_fun(game.get(), &Game::park_selected_stack),
	       game->can_park_selected_stack);
  setup_button(deselect_button,
	       sigc::mem_fun(game.get(), &Game::deselect_selected_stack),
	       game->can_deselect_selected_stack);
  setup_button(search_button,
	       sigc::mem_fun(game.get(), &Game::search_selected_stack),
	       game->can_search_selected_stack);
  setup_button(move_button,
	       sigc::mem_fun(game.get(), &Game::move_selected_stack),
	       game->can_move_selected_stack);
  setup_button(move_all_button,
	       sigc::mem_fun(game.get(), &Game::move_all_stacks),
	       game->can_move_all_stacks);
  setup_menuitem(move_all_menuitem,
		 sigc::mem_fun(game.get(), &Game::move_all_stacks),
		 game->can_move_all_stacks);
  setup_button(end_turn_button,
	       sigc::mem_fun(game.get(), &Game::end_turn),
	       game->can_end_turn);
  setup_menuitem(end_turn_menuitem,
		 sigc::mem_fun(game.get(), &Game::end_turn),
		 game->can_end_turn);
  setup_menuitem(disband_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_disband_activated),
		 game->can_disband_stack);
  setup_menuitem(signpost_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_signpost_activated),
		 game->can_change_signpost);
  setup_menuitem(search_menuitem,
		 sigc::mem_fun(game.get(), &Game::search_selected_stack),
		 game->can_search_selected_stack);
  setup_menuitem(inspect_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_inspect_activated),
		 game->can_inspect_selected_stack);
  setup_menuitem(plant_standard_menuitem,
		 sigc::mem_fun(*this, 
			       &GameWindow::on_plant_standard_activated),
		 game->can_plant_standard_selected_stack);
  setup_menuitem(city_history_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_city_history_activated),
		 game->can_see_history);
  setup_menuitem(gold_history_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_gold_history_activated),
		 game->can_see_history);
  setup_menuitem(event_history_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_event_history_activated),
		 game->can_see_history);
  setup_menuitem(winner_history_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_winner_history_activated),
		 game->can_see_history);
  setup_menuitem(group_ungroup_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_group_ungroup_activated),
		 game->can_group_ungroup_selected_stack);
  setup_menuitem(leave_menuitem,
		 sigc::mem_fun(game.get(), &Game::park_selected_stack),
		 game->can_park_selected_stack);
  setup_menuitem(next_menuitem,
		 sigc::mem_fun(game.get(), &Game::select_next_movable_stack),
		 game->can_select_next_movable_stack);

  // setup game callbacks
  connections.push_back 
    (game->sidebar_stats_changed.connect
     (sigc::mem_fun(*this, &GameWindow::on_sidebar_stats_changed)));
  connections.push_back
    (game->smallmap_changed.connect
     (sigc::mem_fun(*this, &GameWindow::on_smallmap_changed)));
  connections.push_back
    (game->get_smallmap().view_slid.connect
     (sigc::mem_fun(*this, &GameWindow::on_smallmap_slid)));
  connections.push_back
    (game->stack_info_changed.connect
     (sigc::mem_fun(*this, &GameWindow::on_stack_info_changed)));
  connections.push_back
    (game->map_tip_changed.connect
     (sigc::mem_fun(*this, &GameWindow::on_map_tip_changed)));
  connections.push_back
    (game->stack_tip_changed.connect
     (sigc::mem_fun(*this, &GameWindow::on_stack_tip_changed)));
  connections.push_back
    (game->ruin_searched.connect
     (sigc::mem_fun(*this, &GameWindow::on_ruin_searched)));
  connections.push_back
    (game->sage_visited.connect
     (sigc::mem_fun(*this, &GameWindow::on_sage_visited)));
  connections.push_back
    (game->fight_started.connect
     (sigc::mem_fun(*this, &GameWindow::on_fight_started)));
  connections.push_back
    (game->ruinfight_started.connect
     (sigc::mem_fun(*this, &GameWindow::on_ruinfight_started)));
  connections.push_back
    (game->ruinfight_finished.connect
     (sigc::mem_fun(*this, &GameWindow::on_ruinfight_finished)));
  connections.push_back
    (game->hero_offers_service.connect
     (sigc::mem_fun(*this, &GameWindow::on_hero_offers_service)));
  connections.push_back
    (game->temple_searched.connect
     (sigc::mem_fun(*this, &GameWindow::on_temple_searched)));
  connections.push_back
    (game->quest_assigned.connect
     (sigc::mem_fun(*this, &GameWindow::on_quest_assigned)));
  connections.push_back
    (game->city_defeated.connect
     (sigc::mem_fun(*this, &GameWindow::on_city_defeated)));
  connections.push_back
    (game->city_pillaged.connect
     (sigc::mem_fun(*this, &GameWindow::on_city_pillaged)));
  connections.push_back
    (game->city_sacked.connect
     (sigc::mem_fun(*this, &GameWindow::on_city_sacked)));
  connections.push_back
    (game->city_razed.connect
     (sigc::mem_fun(*this, &GameWindow::on_city_razed)));
  connections.push_back
    (game->city_visited.connect
     (sigc::mem_fun(*this, &GameWindow::on_city_visited)));
  connections.push_back
    (game->ruin_visited.connect
     (sigc::mem_fun(*this, &GameWindow::on_ruin_visited)));
  connections.push_back
    (game->temple_visited.connect
     (sigc::mem_fun(*this, &GameWindow::on_temple_visited)));
  connections.push_back
    (game->next_player_turn.connect
     (sigc::mem_fun(*this, &GameWindow::on_next_player_turn)));
  connections.push_back
    (game->hero_arrives.connect
     (sigc::mem_fun(*this, &GameWindow::on_hero_brings_allies)));
  connections.push_back
    (game->medal_awarded_to_army.connect
     (sigc::mem_fun(*this, &GameWindow::on_medal_awarded_to_army)));
  connections.push_back
    (game->army_gains_level.connect
     (sigc::mem_fun(*this, &GameWindow::on_army_gains_level)));
  connections.push_back
    (game->game_loaded.connect
     (sigc::mem_fun(*this, &GameWindow::on_game_loaded)));
  connections.push_back
    (game->game_over.connect
     (sigc::mem_fun(*this, &GameWindow::on_game_over)));
  connections.push_back
    (game->player_died.connect
     (sigc::mem_fun(*this, &GameWindow::on_player_died)));
  connections.push_back
    (game->advice_asked.connect
     (sigc::mem_fun(*this, &GameWindow::on_advice_asked)));

  // misc callbacks
  QuestsManager *q = QuestsManager::getInstance();
  connections.push_back
    (q->quest_completed.connect
     (sigc::mem_fun(this, &GameWindow::on_quest_completed)));
  connections.push_back
    (q->quest_expired.connect
     (sigc::mem_fun(this, &GameWindow::on_quest_expired)));
  if (game.get())
    {
      connections.push_back
	(game->get_bigmap().cursor_changed.connect
	 (sigc::mem_fun(*this, &GameWindow::on_bigmap_cursor_changed)));
    }

}

void GameWindow::setup_game(std::string file_path)
{
  stop_game();

  bool broken = false;
  GameScenario* game_scenario = new GameScenario(file_path, broken);

  if (broken)
    // FIXME: we should not die here, but simply return to the splash screen
    show_fatal_error(_("Map was broken when re-reading. Exiting..."));

  Sound::getInstance()->haltMusic(false);
  Sound::getInstance()->enableBackground();

  game.reset(new Game(game_scenario));

  show_shield_turn();
}

bool GameWindow::on_delete_event(GdkEventAny *e)
{
  on_quit_activated();

  return true;
}

bool GameWindow::on_sdl_mouse_button_event(GdkEventButton *e)
{
  if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
    return true;	// useless event

  if (game.get())
    game->get_bigmap().mouse_button_event(to_input_event(e));

  return true;
}

bool GameWindow::on_sdl_mouse_motion_event(GdkEventMotion *e)
{
  if (game.get())
    {
      game->get_bigmap().mouse_motion_event(to_input_event(e));
      sdl_widget->grab_focus();
    }
  return true;
}

void GameWindow::on_bigmap_cursor_changed(GraphicsCache::CursorType cursor)
{
  sdl_widget->get_window()->set_cursor 
    (Gdk::Cursor(Gdk::Display::get_default(), to_pixbuf
		 (GraphicsCache::getInstance()->getCursorPic
		  (cursor)), 4, 4));
}

bool GameWindow::on_sdl_key_event(GdkEventKey *e)
{
  static int left_shift_down = 0;
  static int right_shift_down = 0;
  if (e->keyval == GDK_Shift_L) 
    left_shift_down = !left_shift_down;
  else if (e->keyval == GDK_Shift_R)
    right_shift_down = !right_shift_down;

  if (e->keyval == GDK_Shift_L || e->keyval == GDK_Shift_R)
    game->get_bigmap().set_shift_key_down (right_shift_down || left_shift_down);

  return true;
}

bool GameWindow::on_map_mouse_button_event(GdkEventButton *e)
{
  if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
    return true;	// useless event

  if (game.get())
    game->get_smallmap().mouse_button_event(to_input_event(e));

  return true;
}

bool GameWindow::on_map_mouse_motion_event(GdkEventMotion *e)
{
  if (game.get())
    {
      game->get_smallmap().mouse_motion_event(to_input_event(e));

      map_eventbox->get_window()->set_cursor 
	(Gdk::Cursor(Gdk::Display::get_default(), to_pixbuf
		     (GraphicsCache::getInstance()->getCursorPic
		      (GraphicsCache::MAGNIFYING_GLASS)), 3, 3));
    }

  return true;
}

void GameWindow::on_sdl_surface_changed()
{
  if (!sdl_inited) {
    sdl_inited = true;
    sdl_initialized.emit();
  }

  if (game.get()) {
    game->get_bigmap().screen_size_changed();
    game->redraw();
  }
}

void GameWindow::on_load_game_activated()
{
  return;
  Gtk::FileChooserDialog chooser(*window.get(), _("Choose Game to Load"));
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*.sav");
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(Configuration::s_savePath);

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      std::string filename = chooser.get_filename();
      chooser.hide();
      load_game(filename);
    }
}

void GameWindow::on_save_game_activated()
{
  if (current_save_filename.empty())
    on_save_game_as_activated();
  else
    {
      if (game.get())
	{
	  bool success = game->saveGame(current_save_filename);
	  if (!success)
	    show_error(_("Game was not saved!"));
	}
    }
}

void GameWindow::on_save_game_as_activated()
{
  Gtk::FileChooserDialog chooser(*window.get(), _("Choose a Name"),
				 Gtk::FILE_CHOOSER_ACTION_SAVE);
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*.sav");
  chooser.set_filter(sav_filter);
  chooser.set_current_folder(Configuration::s_savePath);

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      std::string filename = chooser.get_filename();
      chooser.hide();

      current_save_filename = filename;

      if (game.get())
	{
	  bool success = game->saveGame(current_save_filename);
	  if (!success)
	    show_error(_("Error saving game!"));
	}
    }
}

void GameWindow::on_quit_activated()
{
  // FIXME: ask
  bool end = true;

  if (end) {
    stop_game();
    game_ended.emit();
  }
}

void GameWindow::on_quests_activated()
{
  Player *player = Playerlist::getActiveplayer();
  std::vector<Quest*> quests
    = QuestsManager::getInstance()->getPlayerQuests(player);
  if (quests.size() > 0)
    {
      QuestReportDialog d(quests[0]);
      d.set_parent_window(*window.get());
      return d.run();
    }
  else //no quest!
    {
      QuestReportDialog d(NULL);
      d.set_parent_window(*window.get());
      return d.run();
    }
}

void GameWindow::on_fullscreen_activated()
{
  if (fullscreen_menuitem->get_active())
    window->fullscreen();
  else
    window->unfullscreen();
}

void GameWindow::on_signpost_activated()
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/signpost-change-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  dialog->set_title(_("Signpost"));

  Stack *stack = Playerlist::getActiveplayer()->getActivestack();
  if (!stack)
    return;
  Signpost *s = Signpostlist::getInstance()->getObjectAt(stack->getPos());
  if (!s)
    return;
  Gtk::Label *l;
  xml->get_widget("label", l);
  l->set_text(_("Change the message on this sign:"));
  Gtk::Entry *e;
  xml->get_widget("message_entry", e);
  e->set_text(s->getName());
  dialog->show_all();
  int response = dialog->run();

  if (response == 0)
    Playerlist::getActiveplayer()->signpostChange(s, e->get_text());

  return;
}

void GameWindow::on_disband_activated()
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/disband-stack-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  dialog->set_title(_("Disband"));

  Gtk::Label *l;
  xml->get_widget("label", l);

  l->set_text(_("Are you sure you want to disband this group?"));
  dialog->show_all();
  int response = dialog->run();

  if (response == 0) //disband the active stack
    Playerlist::getActiveplayer()->stackDisband(NULL);

  return;
}

void GameWindow::on_resign_activated()
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/player-resign-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  dialog->set_title(_("Resign"));

  Gtk::Label *l;
  xml->get_widget("label", l);

  l->set_text(_("Are you sure you want to resign?"));
  dialog->show_all();
  int response = dialog->run();

  if (response == 0) //disband all stacks, raze all cities
    Playerlist::getActiveplayer()->resign();

  return;
}

void GameWindow::on_preferences_activated()
{
  PreferencesDialog d;
  d.set_parent_window(*window.get());
  d.run(game.get());
}

void GameWindow::on_group_ungroup_activated()
{
  group_ungroup_toggle->set_active(!group_ungroup_toggle->get_active());
}

void GameWindow::on_fight_order_activated()
{
  FightOrderDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_levels_activated()
{
  HeroLevelsDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_ruin_report_activated()
{
  Vector<int> pos;
  pos.x = 0;
  pos.y = 0;
  if (currently_selected_stack)
    pos = currently_selected_stack->getPos();

  RuinReportDialog d(pos);
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_army_bonus_activated()
{
  ArmyBonusDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_item_bonus_activated()
{
  ItemBonusDialog d;
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_army_report_activated()
{
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::ARMY);
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_city_report_activated()
{
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::CITY);
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_gold_report_activated()
{
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::GOLD);
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_production_report_activated()
{
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::PRODUCTION);
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_winning_report_activated()
{
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::WINNING);
  d.set_parent_window(*window.get());
  d.run();
}
void GameWindow::on_city_history_activated()
{
  HistoryReportDialog d(Playerlist::getActiveplayer(), 
			HistoryReportDialog::CITY);
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_event_history_activated()
{
  HistoryReportDialog d(Playerlist::getActiveplayer(),
			HistoryReportDialog::EVENTS);
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_gold_history_activated()
{
  HistoryReportDialog d(Playerlist::getActiveplayer(),
			HistoryReportDialog::GOLD);
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_winner_history_activated()
{
  HistoryReportDialog d(Playerlist::getActiveplayer(),
			HistoryReportDialog::WINNING);
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_triumphs_activated()
{
  TriumphsDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::stop_game()
{
  Sound::getInstance()->disableBackground();
  if (game.get())
    {
      game->stopGame();
      game.reset();
      current_save_filename = "";
    }
}

void GameWindow::on_game_over(Player *winner)
{

  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/game-over-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  Gtk::Image *image;
  xml->get_widget("image", image);

  SDL_Surface *win_unmasked = File::getMiscPicture("win.jpg", false);
  // mask pics need a special format
  SDL_Surface* tmp = File::getMiscPicture("win_mask.png");
  SDL_Surface *mask = SDL_CreateRGBSurface(
					   SDL_SWSURFACE, tmp->w, tmp->h, 32, 0xFF000000, 0xFF0000, 0xFF00, 0xFF);
  SDL_SetAlpha(tmp, 0, 0);
  SDL_BlitSurface(tmp, 0, mask, 0);
  SDL_FreeSurface(tmp);

  SDL_Surface* win = GraphicsCache::getInstance()->applyMask(
							     win_unmasked, mask, Playerlist::getActiveplayer());

  image->property_pixbuf() = to_pixbuf(win);

  SDL_FreeSurface(win);
  SDL_FreeSurface(win_unmasked);
  SDL_FreeSurface(mask);

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s += String::ucompose(_("Congratulations to %1 for conquering the world!"), 
	winner->getName());
  label->set_text(s);

  dialog->show_all();
  dialog->run();

  stop_game();
  game_ended.emit();
}

void GameWindow::on_player_died(Player *player)
{
  assert(player);

  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/player-died-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s += String::ucompose(_("The rule of %1 has permanently ended!"),
			player->getName());
  if (Playerlist::getInstance()->countHumanPlayersAlive() == 0 &&
      player->getType() == Player::HUMAN)
    {
      s += "\n";
      s += _("No further human resistance is possible");
      s += "\n";
      s += _("but the battle will continue!");
      s += "\n";
      s += _("Press `CTRL-P' to stop the war");
      s += "\n";
      s += _("and visit the sites of thy old battles.");
    }
  label->set_text(s);

  dialog->show_all();
  dialog->run();
}

void GameWindow::on_message_requested(std::string msg)
{
  // FIXME: this is a bit crude, maybe beef it up
  Gtk::MessageDialog dialog(*window.get(), msg);
  dialog.show_all();
  dialog.run();
}

void GameWindow::on_army_toggled(Gtk::ToggleButton *toggle, Army *army)
{
  Player *p = Playerlist::getActiveplayer();
  Stack *s = p->getStacklist()->getActivestack();
  group_ungroup_toggle->set_sensitive(false);
  army->setGrouped(toggle->get_active());
  ensure_one_army_button_active();
  on_stack_info_changed(s);
  group_ungroup_toggle->set_sensitive(true);
  s->getPath()->recalculate(s);
}

void GameWindow::on_group_toggled(Gtk::ToggleButton *toggle)
{
  if (toggle->sensitive() == false)
    return;
  if (toggle->get_active() == false)
    currently_selected_stack->ungroup();
  else
    currently_selected_stack->group();
  currently_selected_stack->getPath()->recalculate(currently_selected_stack);
  update_army_buttons();
}

bool GameWindow::on_army_button_event(GdkEventButton *e,
				      Gtk::ToggleButton *toggle, Army *army)
{
  // if a hero is right-clicked, pop up the hero dialog, otherwise show the
  // army info tip
  MouseButtonEvent event = to_input_event(e);
  if (event.button == MouseButtonEvent::RIGHT_BUTTON
      && event.state == MouseButtonEvent::PRESSED) {

    army_info_tip.reset(new ArmyInfoTip(toggle, army, 
					ArmyInfoTip::ARMY_INSTANCE));

    return true;
  }
  else if (event.button == MouseButtonEvent::RIGHT_BUTTON
	   && event.state == MouseButtonEvent::RELEASED) {
    army_info_tip.reset();
    return true;
  }

  return false;
}

void GameWindow::clear_army_buttons()
{
  for (army_buttons_type::iterator i = army_buttons.begin(),
       end = army_buttons.end(); i != end; ++i)
    delete *i;
  army_buttons.clear();
}

void GameWindow::update_army_buttons()
{
  Stack::iterator j = currently_selected_stack->begin();
  for (army_buttons_type::iterator i = army_buttons.begin(),
       end = army_buttons.end(); i != end; ++i, j++)
    {
      if (!*j || !*i)
	continue; //fixme: why is this required?
      (*i)->set_active((*j)->isGrouped());
    }
}

void GameWindow::ensure_one_army_button_active()
{
  if (army_buttons.empty())
    return;

  // determine number of active buttons
  int sum = 0;
  for (army_buttons_type::iterator i = army_buttons.begin(),
       end = army_buttons.end(); i != end; ++i)
    if ((*i)->get_active())
      ++sum;

  if (sum == 0)
    {
      // must have at least one active, so pick the first
      army_buttons.front()->set_active();
      sum = 1;
    }
}

void GameWindow::on_sidebar_stats_changed(SidebarStats s)
{
  cities_stats_label->set_text(String::ucompose("%1", s.cities));
  gold_stats_label->set_text(String::ucompose("%1", s.gold));
  income_stats_label->set_text(String::ucompose("%1", s.income));
  upkeep_stats_label->set_text(String::ucompose("%1", s.upkeep));
  turn_label->set_text(String::ucompose("Turn %1", s.turns + 1));
}

void GameWindow::on_smallmap_changed(SDL_Surface *map)
{
  map_image->property_pixbuf() = to_pixbuf(map);
}

void GameWindow::on_smallmap_slid(Rectangle view)
{
  on_smallmap_changed(game->get_smallmap().get_surface());
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
}

void GameWindow::on_stack_info_changed(Stack *s)
{
  clear_army_buttons();

  currently_selected_stack = s;

  if (!s)
    show_stats();
  else
    {
      if (s->getPlayer()->getType() == Player::HUMAN)
	show_stack(s);
      else
	show_stats();
    }
}

void GameWindow::show_stats()
{
  Armysetlist *al = Armysetlist::getInstance();
  int height = al->getTileSize(Playerlist::getActiveplayer()->getArmyset());
  height += turn_label->get_height();
  height += 20;
  stack_info_box->get_parent()->property_height_request() = height;
  stats_box->get_parent()->property_height_request() = height;
  stack_info_container->hide();
  stats_box->show();
}

void GameWindow::fill_in_group_info (Stack *s)
{
  Uint32 bonus = s->calculateMoveBonus();
  GraphicsCache *gc = GraphicsCache::getInstance();
  SDL_Surface *terrain = gc->getMoveBonusPic(bonus, s->hasShip());
  terrain_image->property_pixbuf() = to_pixbuf(terrain);
  group_moves_label->set_text(String::ucompose("%1",
					       s->getGroupMoves()));
  group_ungroup_toggle->set_active(s->isGrouped());
}

void GameWindow::show_stack(Stack *s)
{
  s->sortForViewing (true);
  stats_box->hide();

  army_buttons.clear(); 
  for (Stack::iterator i = s->begin(), end = s->end(); i != end; ++i)
    {
      // construct a toggle button
      Army *army = *i;
      Gtk::VBox *toggle_box = manage(new Gtk::VBox);

      // image
      toggle_box->add(*manage(new Gtk::Image(to_pixbuf(army->getPixmap()))));
      // number of moves
      Glib::ustring moves_str = String::ucompose("%1", army->getMoves());
      toggle_box->add(*manage(new Gtk::Label(moves_str,
					     Gtk::ALIGN_CENTER, Gtk::ALIGN_TOP)));

      // the button itself
      Gtk::ToggleButton *toggle = new Gtk::ToggleButton;
      toggle->add(*toggle_box);
      toggle->set_active(army->isGrouped());
      toggle->signal_toggled().connect(
				       sigc::bind(sigc::mem_fun(*this, &GameWindow::on_army_toggled),
						  toggle, army));
      toggle->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
      toggle->signal_button_press_event().connect(
						  sigc::bind(sigc::mem_fun(*this, &GameWindow::on_army_button_event),
							     toggle, army), false);
      toggle->signal_button_release_event().connect(
						    sigc::bind(sigc::mem_fun(*this, &GameWindow::on_army_button_event),
							       toggle, army), false);
      // add it
      stack_info_box->pack_start(*toggle, Gtk::PACK_SHRINK);
      army_buttons.push_back(toggle);

    }

  fill_in_group_info(s);
  ensure_one_army_button_active();
  stack_info_container->show_all();
}

void GameWindow::on_stack_tip_changed(Stack *stack, MapTipPosition mpos)
{
  if (stack == NULL)
    stack_info_tip.reset();
  else
    {
      //_crapola
      stack_info_tip.reset(new StackInfoTip(sdl_widget, mpos, stack));
    }
}

void GameWindow::on_map_tip_changed(Glib::ustring tip, MapTipPosition pos)
{
  if (tip.empty())
    hide_map_tip();
  else
    show_map_tip(tip, pos);
}

void GameWindow::show_map_tip(Glib::ustring msg, MapTipPosition pos)
{
  // init the map tip
  map_tip.reset(new Gtk::Window(Gtk::WINDOW_POPUP));
  Gtk::Frame *f = manage(new Gtk::Frame);
  f->property_shadow_type() = Gtk::SHADOW_ETCHED_OUT;

  Gtk::Label *l = manage(new Gtk::Label);
  l->set_padding(6, 6);
  l->set_text(msg);
  f->add(*l);

  map_tip->add(*f);
  f->show_all();

  // get screen position
  Vector<int> p;
  sdl_widget->get_window()->get_origin(p.x, p.y);
  p += pos.pos;

  Vector<int> size(0, 0);
  map_tip->get_size(size.x, size.y);

  switch (pos.justification)
    {
    case MapTipPosition::LEFT:
      map_tip->set_gravity(Gdk::GRAVITY_NORTH_WEST);
      break;
    case MapTipPosition::RIGHT:
      map_tip->set_gravity(Gdk::GRAVITY_NORTH_EAST);
      p.x -= size.x;
      break;
    case MapTipPosition::TOP:
      map_tip->set_gravity(Gdk::GRAVITY_NORTH_WEST);
      break;
    case MapTipPosition::BOTTOM:
      map_tip->set_gravity(Gdk::GRAVITY_SOUTH_WEST);
      p.y -= size.y;
      break;
    }

  // and action
  map_tip->move(p.x, p.y);
  map_tip->show();
}

void GameWindow::hide_map_tip()
{
  map_tip.reset();
}

void GameWindow::on_sage_visited (Ruin *ruin, Stack *stack)
{
  SageDialog d(stack->getFirstHero()->getPlayer(), 
	       static_cast<Hero*>(stack->getFirstHero()), ruin);
  d.set_parent_window(*window.get());
  Reward *reward = d.run();
  ruin->setReward(reward);
}

void GameWindow::on_ruin_rewarded (Reward_Ruin *reward)
{
  RuinRewardedDialog d(reward);
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_ruin_searched(Ruin *ruin, Stack *stack, Reward *reward)
{
  std::auto_ptr<Gtk::Dialog> dialog;
  if (ruin->hasSage())
    {
      if (reward->getType() == Reward::RUIN)
	return on_ruin_rewarded(static_cast<Reward_Ruin*>(reward));
    }

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/ruin-searched-dialog.glade");


  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  dialog->set_title(ruin->getName());

  Gtk::Image *image;
  xml->get_widget("ruin_image", image);
  image->property_file() = File::getMiscFile("various/ruin_1.jpg");

  Gtk::Label *label;
  xml->get_widget("label", label);

  Glib::ustring s = label->get_text();
  s += "\n\n";
  s += String::ucompose("%1 finds ", stack->getFirstHero()->getName());
  if (reward->getType() == Reward::GOLD)
    {
      Reward_Gold *gold = dynamic_cast<Reward_Gold*>(reward);
      s += String::ucompose("%1 gold pieces.", gold->getGold());
    }
  else if (reward->getType() == Reward::ALLIES)
    {
      Reward_Allies *allies = dynamic_cast<Reward_Allies*>(reward);
      s += String::ucompose("%1 allies.", allies->getNoOfAllies());
    }
  else if (reward->getType() == Reward::ITEM)
    {
      Reward_Item *item = dynamic_cast<Reward_Item*>(reward);
      s += String::ucompose("the %1.", item->getItem()->getName());
    }
  else if (reward->getType() == Reward::MAP)
    {
      Reward_Map *map = dynamic_cast<Reward_Map*>(reward);
      s += String::ucompose("the %2.", map->getLocation()->getName());
    }

  label->set_text(s);

  dialog->show_all();
  dialog->run();
}

void GameWindow::on_ruinfight_started(Stack *attackers, Stack *defenders)
{
  //so and so encounters a wolf...
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/ruinfight-started-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  dialog->set_title(_("Searching"));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = label->get_text();
  s = "\n\n";
  s += attackers->getFirstHero()->getName() + " encounters some ";
  s += defenders->getStrongestArmy()->getName() + "...";
  label->set_text(s);

  dialog->show_all();
  dialog->run();
}
void GameWindow::on_ruinfight_finished(Fight::Result result)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/ruinfight-finished-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  if (result == Fight::ATTACKER_WON)
    dialog->set_title(_("Hero Victorious"));
  else
    dialog->set_title(_("Hero Defeated"));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = label->get_text();
  s = "\n\n";
  if (result == Fight::ATTACKER_WON)
    s += _("...and is victorious!");
  else
    s += _("...and is slain by it!");
  label->set_text(s);

  dialog->show_all();
  dialog->run();
}

void GameWindow::on_fight_started(Fight &fight)
{
  FightWindow d(fight);

  d.set_parent_window(*window.get());
  d.run(&d_quick_fights);
  if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
    d_quick_fights = false;
}

void GameWindow::on_hero_brings_allies (int numAllies)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/hero-brings-allies-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  dialog->set_title(_("Hero brings allies!"));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s = String::ucompose(
		       ngettext("The hero brings %1 ally!",
				"The hero brings %1 allies!", numAllies), numAllies);
  label->set_text(s);

  dialog->show_all();
  dialog->run();
}

bool GameWindow::on_hero_offers_service(Player *player, Hero *hero, City *city, int gold)
{
  HeroOfferDialog d(player, hero, city, gold);
  d.set_parent_window(*window.get());
  return d.run();
}


void GameWindow::on_temple_visited(Temple *temple)
{
  RuinReportDialog d(temple->getPos());
  d.set_parent_window(*window.get());
  d.run();
}

bool GameWindow::on_temple_searched(bool hasHero, Temple *temple, int blessCount)
{
  QuestsManager *qm = QuestsManager::getInstance();
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/temple-visit-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  dialog->set_title(temple->getName());

  Gtk::Label *l;
  Gtk::Button *close_button;
  Gtk::Button *accept_button;
  xml->get_widget("label", l);
  xml->get_widget("close_button", close_button);
  xml->get_widget("accept_button", accept_button);

  if (qm->getPlayerQuests(Playerlist::getActiveplayer()).size() > 0)
    accept_button->set_sensitive(false);

  Glib::ustring s;
  if (blessCount > 0)
    s += String::ucompose(
			  ngettext("%1 army has been blessed!",
				   "%1 armies have been blessed!", blessCount), blessCount);
  else
    s += _("We have already blessed thee!");

  l->set_text(s);
  s = l->get_text() + "\n" + _("Seek more blessings in far temples!");
  l->set_text(s);
  if (GameScenario::s_play_with_quests == true)
    {
      if (hasHero)
	s = l->get_text() + "\n\n" + _("Do you seek a quest?");
      l->set_text(s);
    }

  dialog->show_all();

  if (hasHero == false || GameScenario::s_play_with_quests == false)
    {
      close_button->hide();
      s = _("_Close");
      accept_button->set_label(s);
    }

  int response = dialog->run();

  if (hasHero == false || GameScenario::s_play_with_quests == false)
    response = 1;

  if (response == 0)		// accepted a quest
    return true;
  else
    return false;
}

void GameWindow::on_quest_assigned(Hero *hero, Quest *quest)
{
  QuestAssignedDialog d(hero, quest);
  d.set_parent_window(*window.get());
  return d.run();
}

static bool
hero_has_quest_here (Stack *s, City *c, bool *sack, bool *raze, bool *occupy)
{
  Player *p = Playerlist::getActiveplayer();
  std::vector<Quest*> questlist;
  *sack = false;
  *raze = false;
  *occupy = false;

  QuestsManager *q_mgr = QuestsManager::getInstance();
  questlist = q_mgr->getPlayerQuests(p);
  /* loop over all quests */
  /* for each quest, check the quest type */
  for (std::vector<Quest*>::iterator i = questlist.begin();
       i != questlist.end(); ++i)
    {
      switch ((*i)->getType())
	{
	case Quest::CITYSACK:
	case Quest::CITYRAZE:
	case Quest::CITYOCCUPY:
	  if ((*i)->getType() == Quest::CITYSACK)
	    {
	      if (dynamic_cast<QuestCitySack*>((*i))->getCity() != c)
		continue;
	    }
	  else if ((*i)->getType() == Quest::CITYOCCUPY)
	    {
	      if (dynamic_cast<QuestCityOccupy*>((*i))->getCity() != c)
		continue;
	    }
	  else if ((*i)->getType() == Quest::CITYRAZE)
	    {
	      if (dynamic_cast<QuestCityRaze*>((*i))->getCity() != c)
		continue;
	    }
	  /* now check if the quest's hero is in our stack */
	  for (Stack::iterator it = s->begin(); it != s->end(); ++it)
	    {
	      if ((*it)->isHero())
		{
		  if ((*it)->getId() == (*i)->getHeroId())
		    {
		      /* hey we found one, set the corresponding boolean */
		      if ((*i)->getType() == Quest::CITYSACK)
			*sack = true;
		      else if ((*i)->getType() == Quest::CITYRAZE)
			*raze = true;
		      else if ((*i)->getType() == Quest::CITYOCCUPY)
			*occupy = true;
		    }
		}
	    }
	  break;
	case Quest::PILLAGEGOLD:
	  *sack = true;
	  break;
	}
    }
  if ((*raze) || (*sack) || (*occupy))
    return true;
  else
    return false;
}

CityDefeatedAction GameWindow::on_city_defeated(City *city, int gold)
{
  std::auto_ptr<Gtk::Dialog> dialog;
  if (gold)
    on_city_looted (city, gold);
  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/city-defeated-dialog.glade");


  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  Gtk::Image *image;
  xml->get_widget("city_image", image);
  image->property_file() = File::getMiscFile("various/city_occupied.jpg");
  image->show();

  Gtk::Label *label;
  xml->get_widget("label", label);
  Gtk::Requisition req = image->size_request();
  label->set_size_request(req.width);

  Glib::ustring name;
  Player *p = Playerlist::getActiveplayer();
  Army *h = p->getStacklist()->getActivestack()->getFirstHero();
  if (h)
    name = h->getName();
  else
    name = p->getName(false);

  Glib::ustring s;
  switch (rand() % 4)
    {
    case 0: s = _("%1, you have triumphed in the battle of %2."); break;
    case 1: s = _("%1, you have claimed victory in the battle of %2."); break;
    case 2: s = _("%1, you have shown no mercy in the battle of %2."); break;
    case 3: s = _("%1, you have slain the foe in the battle of %2."); break;
    }

  s = String::ucompose(s, name, city->getName());
  s += "\n\n";
  s += label->get_text();
  label->set_text(s);

  if (h) /* if there was a hero in the stack */
    {
      bool sack, raze, occupy;
      if (hero_has_quest_here (p->getStacklist()->getActivestack(), city, 
			       &sack, &raze, &occupy))
	{
	  Gtk::Button *button;
	  if (sack)
	    {
	      xml->get_widget("sack_button", button);
	      button->grab_default();
	      //button->set_label(">" + button->get_label() +"<");
	    }
	  if (raze)
	    {
	      xml->get_widget("raze_button", button);
	      //button->set_label(">" + button->get_label() +"<");
	      button->grab_default();
	    }
	  if (occupy)
	    {
	      xml->get_widget("occupy_button", button);
	      //button->set_label(">" + button->get_label() +"<");
	      button->grab_default();
	    }
	}
    }

  if (city->getNoOfBasicProd() <= 0) {
    Gtk::Button *b;
    xml->get_widget("pillage_button", b);
    b->hide();
  }

  if (city->getNoOfBasicProd() <= 1) {
    Gtk::Button *b;
    xml->get_widget("sack_button", b);
    b->hide();
  }

  dialog->show();

  int response = dialog->run();
  switch (response) {
  default:
  case 0: return CITY_DEFEATED_OCCUPY;
  case 1: return CITY_DEFEATED_RAZE;
  case 2: return CITY_DEFEATED_PILLAGE;
  case 3: return CITY_DEFEATED_SACK;
  }
}

void GameWindow::on_city_looted (City *city, int gold)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/city-looted-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  dialog->set_title(String::ucompose(_("%1 Looted"), city->getName()));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = label->get_text();
  s += "\n\n";
  s += String::ucompose(
			ngettext("Your armies loot %1 gold piece.",
				 "Your armies loot %1 gold pieces.", gold), gold);
  label->set_text(s);

  dialog->show_all();
  dialog->run();
}
void GameWindow::on_city_pillaged(City *city, int gold, int pillaged_army_type)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  std::auto_ptr<Gtk::Dialog> dialog;
  Player *player = city->getPlayer();
  unsigned int as = player->getArmyset();

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/city-pillaged-dialog.glade");

  Gtk::Dialog *d;
  Gtk::Image *pillaged_army_type_image;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  dialog->set_title(String::ucompose(_("Pillaged %1"), city->getName()));

  Gtk::Label *pillaged_army_type_cost_label;
  xml->get_widget("pillaged_army_type_cost_label", pillaged_army_type_cost_label);
  xml->get_widget("pillaged_army_type_image", pillaged_army_type_image);
  if (gold == 0)
    {
      SDL_Surface *s
	= GraphicsCache::getInstance()->getArmyPic(as, 0, player, NULL);
      Glib::RefPtr<Gdk::Pixbuf> empty_pic
	= Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, s->w, s->h);
      empty_pic->fill(0x00000000);
      pillaged_army_type_image->set(empty_pic);
      pillaged_army_type_cost_label->set_text("");
    }
  else
    {
      Glib::RefPtr<Gdk::Pixbuf> pic;
      pic = to_pixbuf(gc->getArmyPic(as, pillaged_army_type, player, NULL));
      pillaged_army_type_image->set(pic);
      pillaged_army_type_cost_label->set_text(String::ucompose("%1 gp", gold));
    }
  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = label->get_text();
  s += "\n\n";
  s += String::ucompose(
			ngettext("The loot is worth %1 gold piece.",
				 "The loot is worth %1 gold pieces.",
				 gold), gold);
  label->set_text(s);

  dialog->show_all();
  dialog->run();
}

void GameWindow::on_city_sacked(City *city, int gold, std::list<Uint32> sacked_types)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Player *player = city->getPlayer();
  unsigned int as = player->getArmyset();
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/city-sacked-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  dialog->set_title(String::ucompose(_("Sacked %1"), city->getName()));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = label->get_text();
  s = String::ucompose("The city of %1 is sacked\nfor %2 gold!\n\n",
		       city->getName(), gold);
  s += String::ucompose(
			ngettext("Ability to produce %1 unit has been lost",
				 "Ability to produce %1 units has been lost",
				 sacked_types.size()), sacked_types.size());
  s += "\nand only 1 unit remains";
  label->set_text(s);

  Gtk::Image *sacked_army_1_image;
  Gtk::Image *sacked_army_2_image;
  Gtk::Image *sacked_army_3_image;
  Gtk::Label *sacked_army_1_cost_label;
  Gtk::Label *sacked_army_2_cost_label;
  Gtk::Label *sacked_army_3_cost_label;
  xml->get_widget("sacked_army_1_image", sacked_army_1_image);
  xml->get_widget("sacked_army_2_image", sacked_army_2_image);
  xml->get_widget("sacked_army_3_image", sacked_army_3_image);
  xml->get_widget("sacked_army_1_cost_label", sacked_army_1_cost_label);
  xml->get_widget("sacked_army_2_cost_label", sacked_army_2_cost_label);
  xml->get_widget("sacked_army_3_cost_label", sacked_army_3_cost_label);

  Glib::RefPtr<Gdk::Pixbuf> pic;
  SDL_Surface *surf
    = GraphicsCache::getInstance()->getArmyPic(as, 0, player, NULL);
  Glib::RefPtr<Gdk::Pixbuf> empty_pic
    = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, surf->w, surf->h);
  empty_pic->fill(0x00000000);
  int i = 0;
  Gtk::Label *sack_label = NULL;
  Gtk::Image *sack_image = NULL;
  for (std::list<Uint32>::iterator it = sacked_types.begin(); it != sacked_types.end(); it++)
    {
      switch (i)
	{
	case 0:
	  sack_label = sacked_army_1_cost_label;
	  sack_image = sacked_army_1_image;
	  break;
	case 1:
	  sack_label = sacked_army_2_cost_label;
	  sack_image = sacked_army_2_image;
	  break;
	case 2:
	  sack_label = sacked_army_3_cost_label;
	  sack_image = sacked_army_3_image;
	  break;
	}
      pic = to_pixbuf(gc->getArmyPic(as, *it, player, NULL));
      sack_image->set(pic);
      const Army *a = 
	Armysetlist::getInstance()->getArmy (player->getArmyset(), *it);
      s = String::ucompose("%1 gp", a->getProductionCost() / 2);
      sack_label->set_text(s);
      i++;
    }
  for (i = sacked_types.size(); i < 3; i++)
    {
      switch (i)
	{
	case 0:
	  sack_label = sacked_army_1_cost_label;
	  sack_image = sacked_army_1_image;
	  break;
	case 1:
	  sack_label = sacked_army_2_cost_label;
	  sack_image = sacked_army_2_image;
	  break;
	case 2:
	  sack_label = sacked_army_3_cost_label;
	  sack_image = sacked_army_3_image;
	  break;
	}
      sack_image->set(empty_pic);
      sack_label->set_text("");
    }
  dialog->show_all();
  dialog->run();
}

void GameWindow::on_city_razed (City *city)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/city-razed-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  dialog->set_title(String::ucompose(_("Razed %1"), city->getName()));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s = String::ucompose(_("The city of %1 is in ruins!"), city->getName());
  label->set_text(s);

  dialog->show_all();
  dialog->run();
}

void GameWindow::on_city_visited(City *city)
{
  CityWindow d(city);

  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::on_ruin_visited(Ruin *ruin)
{
  RuinReportDialog d(ruin->getPos());
  d.set_parent_window(*window.get());
  d.run();
}

void GameWindow::show_shield_turn()
{
  Playerlist* pl = Playerlist::getInstance();
  GraphicsCache *gc = GraphicsCache::getInstance();
  unsigned int c = 0;
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); ++i)
    {
      if (pl->getNeutral() == (*i))
	continue;
      if ((*i)->isDead())
	{
	  shield_image[c]->clear();
	  turn_hbox->remove(dynamic_cast<Gtk::Widget&>(*shield_image[c]));
	  turn_hbox->resize_children();
	  continue;
	}
      if (*i == pl->getActiveplayer())
	shield_image[c]->get_parent()->modify_bg(Gtk::STATE_NORMAL, shield_image[c]->get_parent()->get_style()->get_black());
      else
	shield_image[c]->get_parent()->modify_bg(Gtk::STATE_NORMAL, shield_image[c]->get_parent()->get_style()->get_white());
      shield_image[c]->property_pixbuf()=to_pixbuf(gc->getShieldPic(1,(*i)));
      c++;
    }
  for (unsigned int i = c; i < MAX_PLAYERS; i++)
    shield_image[i]->clear();
}

void GameWindow::on_next_player_turn(Player *player, unsigned int turn_number)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  while (g_main_context_iteration(NULL, FALSE)); //doEvents

  d_quick_fights = false;
  show_shield_turn();
  if (player->getType() != Player::HUMAN)
    return;
  if (Configuration::s_showNextPlayer == false)
    return;
  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/next-player-turn-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  Gtk::Image *image;
  xml->get_widget("image", image);
  image->property_file() = File::getMiscFile("various/ship.png");

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = String::ucompose(_("%1\nTurn %2"), player->getName(), 
				     turn_number);
  label->set_text(s);

  dialog->show_all();
  dialog->run();
}

void GameWindow::on_medal_awarded_to_army(Army *army)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/medal-awarded-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  Gtk::Image *image;
  xml->get_widget("image", image);
  image->property_pixbuf() = to_pixbuf(army->getPixmap());

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s += String::ucompose(_("%1 is awarded a medal!"), army->getName());
  s += "\n\n";
  s += String::ucompose(_("Experience: %1"), std::setprecision(2), army->getXP());
  s += "\n";
  s += String::ucompose(_("Level: %1"), army->getLevel());
  label->set_text(s);

  dialog->show_all();
  dialog->run();
}

Army::Stat GameWindow::on_army_gains_level(Army *army)
{
  ArmyGainsLevelDialog d(army);

  d.set_parent_window(*window.get());
  d.run();

  return d.get_selected_stat();
}

void GameWindow::on_game_loaded(Player *player)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/game-loaded-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s += String::ucompose(_("%1, your turn continues."), player->getName());
  label->set_text(s);

  dialog->show_all();
  dialog->run();
}

void GameWindow::on_quest_completed(Quest *quest, Reward *reward)
{
  QuestCompletedDialog d(quest, reward);
  d.set_parent_window(*window.get());
  return d.run();
}

void GameWindow::on_quest_expired(Quest *quest)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/quest-expired-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s += String::ucompose(_("%1 did not complete the quest."),
			quest->getHeroName());
  s += "\n\n";

  // add messages from the quest
  std::queue<std::string> msgs;
  quest->getExpiredMsg(msgs);
  while (!msgs.empty())
    {
      s += msgs.front();
      msgs.pop();
      if (!msgs.empty())
	s += "\n\n";
    }

  label->set_text(s);

  dialog->show_all();
  dialog->run();
}

void GameWindow::on_inspect_activated ()
{
  HeroDialog d(dynamic_cast<Hero*>(currently_selected_stack->getFirstHero()), 
	       currently_selected_stack->getPos());
  d.set_parent_window(*window.get());
  d.run();
}
void GameWindow::on_plant_standard_activated ()
{
  Playerlist::getActiveplayer()->heroPlantStandard(NULL);
}
    
void GameWindow::on_advice_asked(float percent)
{
  //we asked for advice on a fight, and we're being told that we 
  //have a PERCENT chance of winning the fight
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/military-advisor-dialog.glade");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());

  dialog->set_title(_("Advisor!"));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;

  int num = rand() % 5;
  if (num == 0)
    s += _("My Good Lord!");
  else if (num == 1)
    s += _("Great and Worthy Lord!");
  else if (num == 2)
    s += _("O Champion of Justice!");
  else if (num == 3)
    s += _("O Mighty Leader!");
  else if (num == 4)
    s += _("O Great Warlord!");
  s += "\n";

  num = rand() % 7;
  if (num == 0)
    s += _("This battle will surely be");
  else if (num == 1)
    s += _("A battle here would be");
  else if (num == 2)
    s += _("I believe this battle will surely be");
  else if (num == 3)
    s += _("This battle would be");
  else if (num == 4)
    s += _("A battle here would be");
  else if (num == 5)
    s += _("I believe this battle will be");
  else if (num == 6)
    s += _("This battle shall be");
  s += "\n";


  if (percent >= 90.0)
    s += _("as simple as butchering sleeping cattle!");
  else if (percent >= 80.0)
    s += _("an easy victory!  We cannot lose!");
  else if (percent >= 70.0)
    s += _("a comfortable victory");
  else if (percent >= 60.0)
    s += _("a hard fought victory! But we shall win!");
  else if (percent >= 50.0)
    s += _("very evenly matched!");
  else if (percent >= 40.0)
    s += _("difficult but not impossible to win!");
  else if (percent >= 30.0)
    s += _("a brave choice! I leave it to thee!");
  else if (percent >= 20.0)
    s += _("a foolish decision!");
  else if (percent >= 10.0)
    s += _("sheerest folly!  Thou shouldst not attack!");
  else
    s += _("complete and utter suicide!");
  label->set_text(s);

  dialog->show_all();
  dialog->run();
  return;
}
