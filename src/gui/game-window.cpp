//  Copyright (C) 2007, 2008, Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2014 Ben Asselstine
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

#include <config.h>

#include <iomanip>
#include <queue>
#include <assert.h>
#include <string.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/hide.h>

#include <gtkmm.h>
#include <glib.h>

#include "game-window.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "error-utils.h"

#include "driver.h"

#include "fight-window.h"
#include "city-window.h"
#include "army-gains-level-dialog.h"
#include "hero-dialog.h"
#include "sage-dialog.h"
#include "ruin-rewarded-dialog.h"
#include "hero-offer-dialog.h"
#include "surrender-dialog.h"
#include "surrender-refused-dialog.h"
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
#include "diplomacy-report-dialog.h"
#include "diplomacy-dialog.h"
#include "stack-info-dialog.h"
#include "timed-message-dialog.h"
#include "destination-dialog.h"
#include "decorated.h"
#include "item-report-dialog.h"
#include "use-item-dialog.h"
#include "use-item-on-player-dialog.h"
#include "use-item-on-city-dialog.h"
#include "game-button-box.h"
#include "status-box.h"

#include "ucompose.hpp"
#include "defs.h"
#include "sound.h"
#include "File.h"
#include "game.h"
#include "gamebigmap.h"
#include "smallmap.h"
#include "GameScenarioOptions.h"
#include "army.h"
#include "ruin.h"
#include "ruinlist.h"
#include "path.h"
#include "player.h"
#include "signpostlist.h"
#include "playerlist.h"
#include "citylist.h"
#include "hero.h"
#include "heroproto.h"
#include "temple.h"
#include "templelist.h"
#include "city.h"
#include "Quest.h"
#include "QuestsManager.h"
#include "stack.h"
#include "GraphicsCache.h"
#include "QuestsManager.h"
#include "Quest.h"
#include "QCitySack.h"
#include "QCityRaze.h"
#include "QCityOccupy.h"
#include "QPillageGold.h"
#include "counter.h"
#include "armysetlist.h"
#include "tilesetlist.h"
#include "CreateScenario.h"
#include "reward.h"
#include "Configuration.h"
#include "GameMap.h"
#include "Item.h"
#include "shieldsetlist.h"
#include "game-server.h"
#include "game-client.h"
#include "NextTurnHotseat.h"
#include "NextTurnPbm.h"
#include "NextTurnNetworked.h"
#include "pbm-game-server.h"
#include "network_player.h"
#include "stacktile.h"
#include "MapBackpack.h"
#include "select-city-map.h"
#include "shield.h"

GameWindow::GameWindow()
{
  game_winner = NULL;
  stack_info_tip = NULL;
  city_info_tip = NULL;
  map_tip = NULL;
  stack_tip = NULL;
  game = NULL;
  game_button_box = NULL;
    sdl_inited = false;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/game-window.ui");

    Gtk::Window *w = 0;
    xml->get_widget("window", w);
    window = w;
    w->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
    decorate(window, GraphicsCache::SPLASH_BACKGROUND);
    
    window_closed.connect
      (sigc::mem_fun(*this, &GameWindow::on_quit_activated));

    w->signal_delete_event().connect
      (sigc::mem_fun(*this, &GameWindow::on_delete_event));

    xml->get_widget("menubar", menubar);
    xml->get_widget("bigmap_image", bigmap_image);
    //bigmap_drawingarea->set_double_buffered(false);
    bigmap_image->signal_event().connect
      (sigc::mem_fun(*this, &GameWindow::on_bigmap_exposed));
    bigmap_image->signal_size_allocate().connect
      (sigc::mem_fun(*this, &GameWindow::on_bigmap_surface_changed));
    bigmap_image->grab_focus();
    xml->get_widget("bigmap_eventbox", bigmap_eventbox);
    bigmap_eventbox->add_events(Gdk::KEY_PRESS_MASK | 
		  Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
	          Gdk::POINTER_MOTION_MASK);
    bigmap_eventbox->signal_key_press_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_bigmap_key_event));
    bigmap_eventbox->signal_key_release_event().connect(
	sigc::mem_fun(*this, &GameWindow::on_bigmap_key_event));
    bigmap_eventbox->signal_button_press_event().connect
     (sigc::mem_fun(*this, &GameWindow::on_bigmap_mouse_button_event));
    bigmap_eventbox->signal_button_release_event().connect
     (sigc::mem_fun(*this, &GameWindow::on_bigmap_mouse_button_event));
    bigmap_eventbox->signal_motion_notify_event().connect
     (sigc::mem_fun(*this, &GameWindow::on_bigmap_mouse_motion_event));
    bigmap_eventbox->signal_scroll_event().connect
       (sigc::mem_fun(*this, &GameWindow::on_bigmap_scroll_event));
    xml->get_widget("status_box_container", status_box_container);
    //Gtk::Viewport *vp;
    //xml->get_widget("bigmap_viewport", vp);
    //decorate_border(vp, 175);

    status_box = StatusBox::create(Configuration::s_ui_form_factor);
    status_box->reparent(*status_box_container);
    status_box_container->add(*manage(status_box));

    // the map image
    xml->get_widget("smallmap_image", smallmap_image);
    //map_drawingarea->set_double_buffered(false);
    smallmap_image->signal_event().connect
      (sigc::mem_fun(*this, &GameWindow::on_smallmap_exposed));
    xml->get_widget("map_eventbox", map_eventbox);
    xml->get_widget("map_container", map_container);
    map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
			     Gdk::POINTER_MOTION_MASK | Gdk::SCROLL_MASK);
      map_eventbox->signal_button_press_event().connect
       (sigc::mem_fun(*this, &GameWindow::on_smallmap_mouse_button_event));
      map_eventbox->signal_button_release_event().connect
       (sigc::mem_fun(*this, &GameWindow::on_smallmap_mouse_button_event));
      map_eventbox->signal_motion_notify_event().connect
       (sigc::mem_fun(*this, &GameWindow::on_smallmap_mouse_motion_event));
    //Gtk::Viewport *mvp;
    //xml->get_widget("map_viewport", mvp);
    //decorate_border(mvp, 175);
    xml->get_widget("control_panel_viewport", control_panel_viewport);
    decorate_border(control_panel_viewport, 175);
    game_button_box = GameButtonBox::create(Configuration::s_ui_form_factor);
    game_button_box->reparent(*control_panel_viewport);
    control_panel_viewport->add(*manage(game_button_box));

    // the stats
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

    // connect callbacks for the menu
    xml->get_widget("new_game_menuitem", new_game_menuitem);
    new_game_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_new_game_activated));
    xml->get_widget("load_game_menuitem", load_game_menuitem);
    load_game_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_load_game_activated));
    xml->get_widget("save_game_menuitem", save_game_menuitem);
    save_game_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_save_game_activated));
    xml->get_widget("save_game_as_menuitem", save_game_as_menuitem);
    save_game_as_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_save_game_as_activated));
    xml->get_widget("quit_menuitem", quit_menuitem);
    quit_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_quit_activated));
    xml->get_widget("toggle_grid_menuitem", toggle_grid_menuitem);
    toggle_grid_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_grid_toggled));
    xml->get_widget("army_report_menuitem", army_report_menuitem);
    army_report_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_army_report_activated));
    xml->get_widget("item_report_menuitem", item_report_menuitem);
    item_report_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_item_report_activated));
    xml->get_widget("city_report_menuitem", city_report_menuitem);
    city_report_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_city_report_activated));
    xml->get_widget("gold_report_menuitem", gold_report_menuitem);
    gold_report_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_gold_report_activated));
    xml->get_widget("winning_report_menuitem", winning_report_menuitem);
    winning_report_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_winning_report_activated));
    xml->get_widget("diplomacy_report_menuitem", diplomacy_report_menuitem);
    xml->get_widget("quests_menuitem", quests_menuitem);
    quests_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_quests_activated));
    xml->get_widget("fullscreen_menuitem", fullscreen_menuitem);
    fullscreen_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_fullscreen_activated));
    xml->get_widget("preferences_menuitem", preferences_menuitem);
    preferences_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_preferences_activated));

    xml->get_widget("show_lobby_menuitem", show_lobby_menuitem);
    show_lobby_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_show_lobby_activated));
    xml->get_widget("end_turn_menuitem", end_turn_menuitem);
    xml->get_widget("move_all_menuitem", move_all_menuitem);
    xml->get_widget("disband_menuitem", disband_menuitem);
    xml->get_widget("stack_info_menuitem", stack_info_menuitem);
    xml->get_widget("signpost_menuitem", signpost_menuitem);
    xml->get_widget("search_menuitem", search_menuitem);
    xml->get_widget("use_menuitem", use_menuitem);
    xml->get_widget("inspect_menuitem", inspect_menuitem);
    xml->get_widget("plant_standard_menuitem", plant_standard_menuitem);
    xml->get_widget("city_history_menuitem", city_history_menuitem);
    xml->get_widget("ruin_history_menuitem", ruin_history_menuitem);
    xml->get_widget("event_history_menuitem", event_history_menuitem);
    xml->get_widget("gold_history_menuitem", gold_history_menuitem);
    xml->get_widget("winner_history_menuitem", winner_history_menuitem);
    xml->get_widget("group_ungroup_menuitem", group_ungroup_menuitem);
    xml->get_widget("leave_menuitem", leave_menuitem);
    xml->get_widget("next_menuitem", next_menuitem);

    xml->get_widget("fight_order_menuitem", fight_order_menuitem);
    fight_order_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_fight_order_activated));
    xml->get_widget("resign_menuitem", resign_menuitem);
    resign_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_resign_activated));
    xml->get_widget("production_menuitem", production_menuitem);
    production_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_production_activated));
    xml->get_widget("cities_menuitem", cities_menuitem);
    cities_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_production_activated));
    xml->get_widget("build_menuitem", build_menuitem);
    build_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_production_activated));
    xml->get_widget("vectoring_menuitem", vectoring_menuitem);
    vectoring_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_vectoring_activated));
    xml->get_widget("levels_menuitem", levels_menuitem);
    xml->get_widget("inspect_menuitem", inspect_menuitem);
    xml->get_widget("ruin_report_menuitem", ruin_report_menuitem);
    ruin_report_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_ruin_report_activated));
    xml->get_widget("army_bonus_menuitem", army_bonus_menuitem);
    army_bonus_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_army_bonus_activated));
    xml->get_widget("item_bonus_menuitem", item_bonus_menuitem);
    item_bonus_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_item_bonus_activated));
    xml->get_widget("production_report_menuitem", production_report_menuitem);
    production_report_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_production_report_activated));
    xml->get_widget("triumphs_menuitem", triumphs_menuitem);
    triumphs_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_triumphs_activated));
    xml->get_widget("help_about_menuitem", help_about_menuitem);
    help_about_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_help_about_activated));
    xml->get_widget("online_help_menuitem", online_help_menuitem);
    online_help_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_online_help_activated));
    d_quick_fights = false;

    if (Configuration::s_decorated == true)
      {
        /*
	//colour the menubar
	Glib::RefPtr<Gtk::Style> copy;
	Glib::RefPtr<Gdk::Pixbuf> back;
	Glib::RefPtr<Gdk::Pixmap> pixmap;
	Glib::RefPtr<Gdk::Bitmap> bitmap;
	copy = menubar->get_style()->copy();
	back = GraphicsCache::getInstance()->getBackgroundPic(GraphicsCache::SPLASH_BACKGROUND)->to_pixbuf();
	pixmap = Gdk::Pixmap::create
	  (window->get_window(), back->get_width(), back->get_height());
	back->composite_color(back, 0, 0, 
			      back->get_width(), back->get_height(), 
			      0.0, 0.0, 1.0, 1.0, Gdk::INTERP_NEAREST, 127, 
			      0, 0, 64, 0, 0);
	back->render_pixmap_and_mask(pixmap, bitmap, 10);
	copy->set_bg_pixmap(Gtk::STATE_NORMAL, pixmap);
	menubar->set_style(copy);
        */
      }

}

GameWindow::~GameWindow()
{
  std::list<sigc::connection>::iterator it = connections.begin();
  for (; it != connections.end(); it++) 
    (*it).disconnect();
  connections.clear();
    if (city_info_tip)
      {
	delete city_info_tip;
	city_info_tip = NULL;
      }
    if (stack_info_tip)
      {
	delete stack_info_tip;
	stack_info_tip = NULL;
      }
    if (game)
      {
	delete game;
	game = NULL;
      }
    if (game_button_box)
      {
        delete game_button_box;
        game_button_box = NULL;
      }
    if (status_box)
      {
        delete status_box;
        status_box = NULL;
      }
  delete window;
}

void GameWindow::show()
{
  if (game_button_box)
    {
      control_panel_viewport->show_all();
      game_button_box->show_all();
    }
    
    bigmap_image->show_all();
    window->show();
    if (status_box)
      {
        status_box->setHeightFudgeFactor(turn_label->get_height());
        status_box->enforce_height();
        status_box->show_stats();
      }
      
    //seems unnecessary, but this is for starting up a second game after
    //closing the first game window.
    //fixme: find out why this line is necessary
    on_bigmap_surface_changed(bigmap_image->get_allocation());
}

void GameWindow::hide()
{
    window->hide();
}

void GameWindow::init(int width, int height)
{
    bigmap_image->set_size_request(width, height);
}

void GameWindow::new_network_game(GameScenario *game_scenario, NextTurn *next_turn)
{
  if (GameServer::getInstance()->isListening() == true)
    GameServer::getInstance()->round_begins.connect(sigc::mem_fun(this, &GameWindow::on_remote_next_player_turn));
  else
    GameClient::getInstance()->playerlist_reorder_received.connect(sigc::mem_fun(this, &GameWindow::on_remote_next_player_turn));
  bool success = false;
  //stop_game();
  success = setup_game(game_scenario, next_turn);
  if (!success)
    return;
  setup_signals(game_scenario);
  game->redraw();
  game->startGame();
  if (Playerlist::getActiveplayer() && GameServer::getInstance()->isListening() == false)
    if (Playerlist::getActiveplayer()->getType() != Player::NETWORKED)
      {
        dynamic_cast<NextTurnNetworked*>(next_turn)->start_player(Playerlist::getActiveplayer());
      }
}

void GameWindow::continue_network_game(NextTurn *next_turn)
{
  next_turn->start();
}

void GameWindow::new_game(GameScenario *game_scenario, NextTurn *next_turn)
{
  bool success = false;
  success = setup_game(game_scenario, next_turn);
  if (!success)
    return;
  setup_signals(game_scenario);
  game->startGame();
  //we don't get here until the game ends.
}

void GameWindow::load_game(GameScenario *game_scenario, NextTurn *next_turn)
{
  bool success = false;
  success = setup_game(game_scenario, next_turn);
  if (!success)
    return;

  game->get_bigmap().screen_size_changed(bigmap_image->get_allocation());
  setup_signals(game_scenario);
  game->loadGame();
  //we don't get here until the game ends, or a human player ends a turn.
  if (Playerlist::getInstance()->countPlayersAlive())
    game->redraw();
}

void GameWindow::setup_menuitem(Gtk::MenuItem *item,
				sigc::slot<void> slot,
				sigc::signal<void, bool> &game_signal)
{
  connections.push_back (item->signal_activate().connect(slot));
  connections.push_back 
    (game_signal.connect(sigc::mem_fun(item, &Gtk::Widget::set_sensitive)));
}

void GameWindow::setup_signals(GameScenario *game_scenario)
{
  // get rid of the connections that might be still around from last time
  std::list<sigc::connection>::iterator it = connections.begin();
  for (; it != connections.end(); it++) 
    (*it).disconnect();
  connections.clear();

  connections.push_back (game_button_box->diplomacy_clicked.connect
   (sigc::mem_fun (*this, &GameWindow::on_diplomacy_button_clicked)));
  connections.push_back 
    (game->city_too_poor_to_produce.connect 
     (sigc::mem_fun(*this, &GameWindow::show_city_production_report)));
  connections.push_back
    (game->commentator_comments.connect
     (sigc::mem_fun(*this, &GameWindow::on_commentator_comments)));

  setup_menuitem(move_all_menuitem,
		 sigc::mem_fun(game, &Game::move_all_stacks),
		 game->can_move_all_stacks);
  setup_menuitem(end_turn_menuitem,
		 sigc::mem_fun(game, &Game::end_turn),
		 game->can_end_turn);
  if (game_scenario->getPlayMode() == GameScenario::PLAY_BY_MAIL)
    setup_menuitem(end_turn_menuitem,
		   sigc::mem_fun(*this, &GameWindow::end_turn_play_by_mail),
		   game->can_end_turn);
  if (game_scenario->getPlayMode() == GameScenario::NETWORKED)
    {
      load_game_menuitem->set_sensitive(false);
      if (GameServer::getInstance()->isListening() == false)
	{
	  save_game_menuitem->set_sensitive(false);
	  save_game_as_menuitem->set_sensitive(false);
	}
    }
  else
    show_lobby_menuitem->set_sensitive(false);

  setup_menuitem(disband_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_disband_activated),
		 game->can_disband_stack);
  setup_menuitem(stack_info_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_stack_info_activated),
		 game->can_deselect_selected_stack);
  setup_menuitem(signpost_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_signpost_activated),
		 game->can_change_signpost);
  setup_menuitem(search_menuitem,
		 sigc::mem_fun(game, &Game::search_selected_stack),
		 game->can_search_selected_stack);
  setup_menuitem(use_menuitem,
		 sigc::mem_fun(game, &Game::select_item_to_use),
		 game->can_use_item);
  setup_menuitem(inspect_menuitem,
		 sigc::mem_fun(*this, 
			       &GameWindow::on_inspect_activated),
		 game->can_inspect);
  setup_menuitem(levels_menuitem,
		 sigc::mem_fun(*this, 
			       &GameWindow::on_levels_activated),
		 game->can_see_hero_levels);
  setup_menuitem(plant_standard_menuitem,
		 sigc::mem_fun(*this, 
			       &GameWindow::on_plant_standard_activated),
		 game->can_plant_standard_selected_stack);
  setup_menuitem(city_history_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_city_history_activated),
		 game->can_see_history);
  setup_menuitem(ruin_history_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_ruin_history_activated),
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
  setup_menuitem(diplomacy_report_menuitem,
		 sigc::mem_fun(*this, 
			       &GameWindow::on_diplomacy_report_activated),
		 game->can_see_diplomacy); 
  setup_menuitem(group_ungroup_menuitem,
		 sigc::mem_fun(*this, &GameWindow::on_group_ungroup_activated),
		 game->can_group_ungroup_selected_stack);
  setup_menuitem(leave_menuitem,
		 sigc::mem_fun(game, &Game::park_selected_stack),
		 game->can_park_selected_stack);
  setup_menuitem(next_menuitem,
		 sigc::mem_fun(game, &Game::select_next_movable_stack),
		 game->can_select_next_movable_stack);

  // setup game callbacks
  connections.push_back
    (game->game_stopped.connect
     (sigc::mem_fun(*this, &GameWindow::on_game_stopped)));
  connections.push_back 
    (game->sidebar_stats_changed.connect
     (sigc::mem_fun(*this, &GameWindow::on_sidebar_stats_changed)));
  connections.push_back 
    (game->progress_status_changed.connect
     (sigc::mem_fun(*this, &GameWindow::on_progress_status_changed)));
  connections.push_back 
    (game->progress_changed.connect
     (sigc::mem_fun(*this, &GameWindow::on_progress_changed)));
  connections.push_back
    (game->bigmap_changed.connect
     (sigc::mem_fun(*this, &GameWindow::on_bigmap_changed)));
  connections.push_back
    (game->smallmap_changed.connect
     (sigc::mem_fun(*this, &GameWindow::on_smallmap_changed)));
  connections.push_back
    (game->get_smallmap().view_slid.connect
     (sigc::mem_fun(*this, &GameWindow::on_smallmap_slid)));
  connections.push_back
    (game->stack_info_changed.connect
     (sigc::mem_fun(*status_box, &StatusBox::on_stack_info_changed)));
  connections.push_back
    (game->map_tip_changed.connect
     (sigc::mem_fun(*this, &GameWindow::on_bigmap_tip_changed)));
  connections.push_back
    (game->stack_tip_changed.connect
     (sigc::mem_fun(*this, &GameWindow::on_stack_tip_changed)));
  connections.push_back
    (game->city_tip_changed.connect
     (sigc::mem_fun(*this, &GameWindow::on_city_tip_changed)));
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
    (game->abbreviated_fight_started.connect
     (sigc::mem_fun(*this, &GameWindow::on_abbreviated_fight_started)));
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
    (game->enemy_offers_surrender.connect
     (sigc::mem_fun(*this, &GameWindow::on_enemy_offers_surrender)));
  connections.push_back
    (game->surrender_answered.connect
     (sigc::mem_fun(*this, &GameWindow::on_surrender_answered)));
  connections.push_back
    (game->stack_considers_treachery.connect
     (sigc::mem_fun(*this, &GameWindow::on_stack_considers_treachery)));
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
    (game->hero_gains_level.connect
     (sigc::mem_fun(*this, &GameWindow::on_hero_gains_level)));
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
  connections.push_back
    (game->sunk_ships.connect
     (sigc::mem_fun(*this, &GameWindow::on_ships_sunk)));
  connections.push_back
    (game->bags_picked_up.connect
     (sigc::mem_fun(*this, &GameWindow::on_bags_picked_up)));
  connections.push_back
    (game->mp_added_to_hero_stack.connect
     (sigc::mem_fun(*this, &GameWindow::on_mp_added_to_hero_stack)));
  connections.push_back
    (game->worms_killed.connect
     (sigc::mem_fun(*this, &GameWindow::on_worms_killed)));
  connections.push_back
    (game->bridge_burned.connect
     (sigc::mem_fun(*this, &GameWindow::on_bridge_burned)));
  connections.push_back
    (game->keeper_captured.connect
     (sigc::mem_fun(*this, &GameWindow::on_keeper_captured)));
  connections.push_back
    (game->monster_summoned.connect
     (sigc::mem_fun(*this, &GameWindow::on_monster_summoned)));
  connections.push_back
    (game->stole_gold.connect
     (sigc::mem_fun(*this, &GameWindow::on_gold_stolen)));
  connections.push_back
    (game->stack_moves.connect
     (sigc::mem_fun(*this, &GameWindow::on_stack_moves)));
  connections.push_back
    (game->select_item.connect
     (sigc::mem_fun(*this, &GameWindow::on_select_item)));
  connections.push_back
    (game->select_item_victim_player.connect
     (sigc::mem_fun(*this, &GameWindow::on_select_item_victim_player)));
  connections.push_back
    (game->select_city_to_use_item_on.connect
     (sigc::mem_fun(*this, &GameWindow::on_select_city_to_use_item_on)));
  connections.push_back
    (game->city_diseased.connect
     (sigc::mem_fun(*this, &GameWindow::on_city_diseased)));
  connections.push_back
    (game->city_defended.connect
     (sigc::mem_fun(*this, &GameWindow::on_city_defended)));
  connections.push_back
    (game->city_persuaded.connect
     (sigc::mem_fun(*this, &GameWindow::on_city_persuaded)));
  connections.push_back
    (game->stack_teleported.connect
     (sigc::mem_fun(*this, &GameWindow::on_stack_teleported)));

  // misc callbacks
  QuestsManager *q = QuestsManager::getInstance();
  connections.push_back
    (q->quest_completed.connect
     (sigc::mem_fun(this, &GameWindow::on_quest_completed)));
  connections.push_back
    (q->quest_expired.connect
     (sigc::mem_fun(this, &GameWindow::on_quest_expired)));
  if (game)
    {
      connections.push_back
	(game->get_bigmap().cursor_changed.connect
	 (sigc::mem_fun(*this, &GameWindow::on_bigmap_cursor_changed)));
    }

  connections.push_back
    (game->remote_next_player_turn.connect
     (sigc::mem_fun(*this, &GameWindow::on_remote_next_player_turn)));
}

void GameWindow::show_city_production_report (bool destitute)
{
  if (!destitute)
    return;
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::PRODUCTION);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::end_turn_play_by_mail ()
{
  //prompt to save the turn file!
  Gtk::FileChooserDialog chooser(*window, _("Save your Turn file and mail it back"),
				 Gtk::FILE_CHOOSER_ACTION_SAVE);
  Glib::RefPtr<Gtk::FileFilter> trn_filter = Gtk::FileFilter::create();
  trn_filter->set_name(_("LordsAWar Turn files (*.trn)"));
  trn_filter->add_pattern("*" + PBM_EXT);
  chooser.add_filter(trn_filter);
  chooser.set_current_folder(Glib::get_home_dir());

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();
  chooser.hide();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      std::string filename = chooser.get_filename();

      game->saveTurnFile(filename);
      TimedMessageDialog dialog
	(*window, ("Now send the turn file back to the game master."), 0);
      dialog.run();
      dialog.hide();
    }
  game_ended.emit();
}

bool GameWindow::setup_game(GameScenario *game_scenario, NextTurn *nextTurn)
{
  status_box->clear_selected_stack();

  Sound::getInstance()->haltMusic(false);
  Sound::getInstance()->enableBackground();

  if (game)
    delete game;
  game = new Game(game_scenario, nextTurn);

  game_button_box->setup_signals(game, Configuration::s_ui_form_factor);
    
  status_box->stack_composition_modified.connect
      (sigc::mem_fun(game, &Game::recalculate_moves_for_stack));
  status_box->stack_tile_group_toggle.connect
    (sigc::mem_fun(this, &GameWindow::on_group_stack_toggled));
  show_shield_turn();
      smallmap_image->set_size_request(game->get_smallmap().get_width(),
                                                                            game->get_smallmap().get_height());
  while (g_main_context_iteration(NULL, FALSE)); //doEvents

  return true;
}
    
void GameWindow::on_group_stack_toggled(bool lock)
{
  game->get_bigmap().set_input_locked(lock);
}

bool GameWindow::on_delete_event(GdkEventAny *e)
{
  on_quit_activated();

  return true;
}

bool GameWindow::on_bigmap_mouse_button_event(GdkEventButton *e)
{
  if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
    return true;	// useless event

  if (game)
    game->get_bigmap().mouse_button_event(to_input_event(e));

  return true;
}

bool GameWindow::on_bigmap_mouse_motion_event(GdkEventMotion *e)
{
  static guint prev = 0;
  if (game)
    {
      guint delta = e->time - prev;
      if (delta > 40 || delta < 0)
	{
	  game->get_bigmap().mouse_motion_event(to_input_event(e));
	  bigmap_image->grab_focus();
	  prev = e->time;
	}
    }
  return true;
}
    
bool GameWindow::on_bigmap_scroll_event(GdkEventScroll* event)
{
  switch (event->direction) 
    {
    case GDK_SCROLL_SMOOTH:
    case GDK_SCROLL_LEFT:
    case GDK_SCROLL_RIGHT:
      break;
    case GDK_SCROLL_UP:
      //game->get_bigmap().zoom_in();
      break;
    case GDK_SCROLL_DOWN:
      //game->get_bigmap().zoom_out();
      break;
    }
  return true;
}

void GameWindow::on_bigmap_cursor_changed(GraphicsCache::CursorType cursor)
{
  bigmap_image->get_window()->set_cursor 
    (Gdk::Cursor::create
     (Gdk::Display::get_default(), 
      GraphicsCache::getInstance()->getCursorPic (cursor)->to_pixbuf(), 4, 4));
}

bool GameWindow::on_bigmap_key_event(GdkEventKey *e)
{
  if (e->keyval == GDK_KEY_Shift_L || e->keyval == GDK_KEY_Shift_R)
    game->get_bigmap().set_shift_key_down (e->type == GDK_KEY_PRESS);
  if (e->keyval == GDK_KEY_Control_L || e->keyval == GDK_KEY_Control_R)
    game->get_bigmap().set_control_key_down (e->type == GDK_KEY_PRESS);

  return true;
}

bool GameWindow::on_smallmap_mouse_button_event(GdkEventButton *e)
{
  if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
    return true;	// useless event

  if (game)
    game->get_smallmap().mouse_button_event(to_input_event(e));

  return true;
}

bool GameWindow::on_smallmap_mouse_motion_event(GdkEventMotion *e)
{
  static guint prev = 0;
  if (game)
    {
      guint delta = e->time - prev;
      if (delta > 100 || delta < 0)
	{
	  game->get_smallmap().mouse_motion_event(to_input_event(e));
	  prev = e->time;
	}

      map_eventbox->get_window()->set_cursor 
	(Gdk::Cursor::create
	 (Gdk::Display::get_default(), 
	  GraphicsCache::getInstance()->getCursorPic
		      (GraphicsCache::MAGNIFYING_GLASS)->to_pixbuf(), 8, 5));
    }

  return true;
}

bool GameWindow::on_smallmap_exposed(GdkEvent *event)
{
  Gdk::Event e(event);
  if (e.gobj()->type == GDK_EXPOSE)
    {
      Glib::RefPtr<Gdk::Window> window = smallmap_image->get_window();
      if (window)
        {
          Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
            Gdk::Pixbuf::create(game->get_smallmap().get_surface(), 0, 0, 
                                game->get_smallmap().get_width(), 
                                game->get_smallmap().get_height());
          smallmap_image->property_pixbuf() = pixbuf;
        }
    }
  return true;
}
bool GameWindow::on_bigmap_exposed(GdkEvent *event)
{
  printf("we got an event on the bigmap\n");
  Gdk::Event e(event);
  if (e.gobj()->type == GDK_EXPOSE)
    {
      printf("we got an expose event on the bigmap\n");
      Glib::RefPtr<Gdk::Window> window = bigmap_image->get_window();
      if (window)
        {
          printf("exposed\n");
          Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
            Gdk::Pixbuf::create(game->get_bigmap().get_surface(), 0, 0, 
                                bigmap_image->get_allocated_width(), 
                                bigmap_image->get_allocated_height());
          bigmap_image->property_pixbuf() = pixbuf;
        }
    }
  return true;
}

void GameWindow::on_bigmap_surface_changed(Gtk::Allocation box)
{
  static Gtk::Allocation last_box = Gtk::Allocation(0,0,1,1);
  if (!sdl_inited) {
    sdl_inited = true;
    sdl_initialized.emit();
  }

  if (game) {
    //if (box.get_width() != last_box.get_width() || box.get_height() != last_box.get_height())
      {
	//bigmap_drawingarea->set_allocation(box);
	game->get_bigmap().screen_size_changed(bigmap_image->get_allocation());
	game->redraw();
      }
  }
  last_box = box;
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
}

void GameWindow::on_load_game_activated()
{
  Gtk::FileChooserDialog chooser(*window, _("Choose Game to Load"));
  Glib::RefPtr<Gtk::FileFilter> sav_filter = Gtk::FileFilter::create();
  sav_filter->set_name(_("LordsAWar Saved Games (*.sav)"));
  sav_filter->add_pattern("*" + SAVE_EXT);
  chooser.add_filter(sav_filter);
  chooser.set_current_folder(Configuration::s_savePath);

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();
  chooser.hide();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      std::string filename = chooser.get_filename();
      current_save_filename = filename;
      if (filename ==  (File::getSavePath() + "autosave" + SAVE_EXT))
	game->inhibitAutosaveRemoval(true);
      d_load_filename = filename;
      stop_game("load-game");
      //now look at on_game_stopped.
    }
}

void GameWindow::on_save_game_activated()
{
  if (current_save_filename.empty())
    on_save_game_as_activated();
  else
    {
      if (game)
	{
	  bool success = game->saveGame(current_save_filename);
	  if (!success)
	    show_error(_("Game was not saved!"));
	}
    }
}

void GameWindow::on_save_game_as_activated()
{
  Gtk::FileChooserDialog chooser(*window, _("Choose a Name"),
				 Gtk::FILE_CHOOSER_ACTION_SAVE);
  Glib::RefPtr<Gtk::FileFilter> sav_filter = Gtk::FileFilter::create();
  sav_filter->set_name(_("LordsAWar Saved Games (*.sav)"));
  sav_filter->add_pattern("*" + SAVE_EXT);
  chooser.add_filter(sav_filter);
  chooser.set_current_folder(Configuration::s_savePath);

  chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
  chooser.set_default_response(Gtk::RESPONSE_ACCEPT);

  chooser.show_all();
  int res = chooser.run();
  chooser.hide();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      std::string filename = chooser.get_filename();

      current_save_filename = filename;

      if (game)
	{
	  bool success = game->saveGame(current_save_filename);
	  if (!success)
	    show_error(_("Error saving game!"));
	}
    }
}

void GameWindow::on_new_game_activated()
{
  Gtk::Dialog* dialog;
  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/game-quit-dialog.ui");
  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));

  dialog->set_transient_for(*window);

  int response = dialog->run();
  dialog->hide();

  if (response == Gtk::RESPONSE_ACCEPT) //end the game
    {
      stop_game("new");
    }
  delete dialog;
}

void GameWindow::on_quit_activated()
{
  Gtk::Dialog* dialog;
  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/game-quit-dialog.ui");
  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));

  dialog->set_transient_for(*window);

  int response = dialog->run();
  dialog->hide();

  if (response == Gtk::RESPONSE_ACCEPT) //end the game
    {
      stop_game("quit");
    }
  delete dialog;
}

void GameWindow::on_game_stopped()
{
  if (stop_action == "quit")
    {
      if (game)
	{
	  delete game;
	  game = NULL;
	}
      game_ended.emit();
    }
  else if (stop_action == "new")
    {
      if (game)
	{
	  delete game;
	  game = NULL;
	}
      game_ended_start_new.emit();
    }
  else if (stop_action == "game-over")
    {
      if (game_winner)
        {
          if (game_winner->getType() != Player::HUMAN)
            {
              if (game)
                {
                  delete game;
                  game = NULL;
                }
              game_ended.emit();
            }
          else
            {
              //we need to keep the game object around
              //so that we can give out some cheese
              give_some_cheese(game_winner);
            }
        }
      else
        {
          if (game)
            {
              delete game;
              game = NULL;
            }
          game_ended.emit();
        }
    }
  else if (stop_action == "load-game")
    {
      if (game)
	{
	  delete game;
	  game = NULL;
	}
      bool broken = false;
      GameScenario* game_scenario = new GameScenario(d_load_filename, broken);

      if (broken)
	{
	  on_message_requested(_("Corrupted saved game file."));
	  game_ended.emit();
	  return;
	}
      if (game_scenario->getPlayMode() == GameScenario::HOTSEAT)
	load_game(game_scenario, 
		  new NextTurnHotseat(game_scenario->getTurnmode(),
				      game_scenario->s_random_turns));
      else if (game_scenario->getPlayMode() == GameScenario::PLAY_BY_MAIL)
	{
	  PbmGameServer::getInstance()->start();
	  load_game(game_scenario, 
		    new NextTurnPbm(game_scenario->getTurnmode(),
				    game_scenario->s_random_turns));
	}
      else if (game_scenario->getPlayMode() == GameScenario::NETWORKED)
        {
          TimedMessageDialog dialog(*window, _("Can't load networked game from file."), 30);

          dialog.show_all();
          dialog.run();
          dialog.hide();
	//load_game(game_scenario, 
		  //new NextTurnNetworked(game_scenario->getTurnmode(),
					//game_scenario->s_random_turns));
        }
    }
}

void GameWindow::on_quests_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Player *player = Playerlist::getActiveplayer();
  std::vector<Quest*> quests
    = QuestsManager::getInstance()->getPlayerQuests(player);
  Stack *s = player->getActivestack();
  Hero *hero = NULL;
  if (s)
    hero = s->getFirstHeroWithAQuest();
  QuestReportDialog d(quests, hero);
  d.set_parent_window(*window);
  d.run();
  d.hide();
  return;
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
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/signpost-change-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  decorator.set_title(_("Signpost"));

  Stack *stack = Playerlist::getActiveplayer()->getActivestack();
  if (!stack)
    return;
  Signpost *s = GameMap::getSignpost(stack->getPos());
  if (!s)
    return;
  Gtk::Label *l;
  xml->get_widget("label", l);
  l->set_text(_("Change the message on this sign:"));
  Gtk::Entry *e;
  xml->get_widget("message_entry", e);
  e->set_text(s->getName());
  e->set_activates_default(true);
  dialog->show_all();
  int response = dialog->run();
  dialog->hide();

  if (response == Gtk::RESPONSE_ACCEPT)
    Playerlist::getActiveplayer()->signpostChange(s, 
                                                  String::utrim(e->get_text()));

  delete dialog;
  return;
}

  
void GameWindow::on_stack_info_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  StackInfoDialog d(status_box->get_currently_selected_stack()->getPos());
  d.set_parent_window(*window);
  d.run();
  Stack *s = d.get_selected_stack();
  d.hide();
  status_box->on_stack_info_changed(s);
  //FIXME, armies don't stay selected in the right way.  to reproduce:
  //go in with all three selected.  deselect middle one in stackinfodialog,
  //then return.
}

void GameWindow::on_disband_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Stack *stack = Playerlist::getActiveplayer()->getActivestack();
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/disband-stack-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  decorator.set_title(_("Disband"));

  Gtk::Label *l;
  xml->get_widget("label", l);

  std::vector<guint32> heroes;
  stack->getHeroes(heroes);
  Glib::ustring s = _("Are you sure you want to disband this group?");
  if (heroes.size() > 0)
    {
      s += "\n";
      s += String::ucompose( ngettext("(It contains %1 hero).",
				      "(It contains %1 heroes).", 
				      heroes.size()), heroes.size());
    }
  l->set_text(s);
  dialog->show_all();
  int response = dialog->run();
  dialog->hide();

  if (response == Gtk::RESPONSE_ACCEPT) //disband the active stack
    Playerlist::getActiveplayer()->stackDisband(NULL);

  return;
}

void GameWindow::on_resignation_completed()
{
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + 
				"/player-resign-completed-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  dialog->show_all();
  dialog->run();
  dialog->hide();
  delete dialog;

  return;
}

void GameWindow::on_resign_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;

  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/player-resign-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  decorator.set_title(_("Resign"));

  Gtk::Label *l;
  xml->get_widget("label", l);

  l->set_text(_("Are you sure you want to resign?"));
  dialog->show_all();
  int response = dialog->run();

  if (response == Gtk::RESPONSE_ACCEPT) //disband all stacks, raze all cities
    {
      Playerlist::getActiveplayer()->resign();
      dialog->hide();
      on_resignation_completed();
    }

  delete dialog;
  return;
}

void GameWindow::on_vectoring_activated()
{
  City *city;
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;

  if (status_box->get_currently_selected_stack())
    {
      Vector<int> pos = status_box->get_currently_selected_stack()->getPos();
      city = Citylist::getInstance()->getNearestVisibleFriendlyCity(pos);
    }
  else
    city = Citylist::getInstance()->getFirstCity(Playerlist::getActiveplayer());

  if (!city)
    return;
  bool see_all = true;
  DestinationDialog d(city, &see_all);

  d.set_parent_window(*window);
  d.run();
  d.hide();
  return;
}

void GameWindow::on_production_activated()
{
  City *city;
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;

  if (status_box->get_currently_selected_stack())
    {
      Vector<int> pos = status_box->get_currently_selected_stack()->getPos();
      city = Citylist::getInstance()->getNearestVisibleFriendlyCity(pos);
    }
  else
    city = Citylist::getInstance()->getFirstCity(Playerlist::getActiveplayer());

  if (!city)
    return;

  on_city_visited(city);
  return;
}

void GameWindow::on_preferences_activated()
{
  Player *current = Playerlist::getInstance()->getActiveplayer();
  bool readonly = false;
  if (game->getScenario()->getPlayMode() == GameScenario::NETWORKED)
    readonly = true;
  PreferencesDialog d(readonly);
  d.ui_form_factor_changed.connect(sigc::mem_fun(this, &GameWindow::on_ui_form_factor_changed));
  d.set_parent_window(*window);
  d.run(game);
  d.hide();
  game->get_bigmap().set_control_key_down (false);
  if (current != Playerlist::getInstance()->getActiveplayer())
    game->end_turn();
}

void GameWindow::on_ui_form_factor_changed(guint32 factor)
{
  if (game_button_box)
    {
      delete game_button_box;
      control_panel_viewport->remove();
    }
  game_button_box = GameButtonBox::create(Configuration::s_ui_form_factor);
  control_panel_viewport->add(*manage(game_button_box));
  game_button_box->reparent(*control_panel_viewport);
  game_button_box->setup_signals(game, Configuration::s_ui_form_factor);
  game_button_box->show_all();


  if (status_box)
    {
      status_box_container->remove(*status_box);
      delete status_box;
    }

  status_box = StatusBox::create(Configuration::s_ui_form_factor);
  status_box_container->pack_start(*manage(status_box), Gtk::PACK_SHRINK);
  status_box->reparent(*status_box_container);
  status_box->stack_composition_modified.connect
      (sigc::mem_fun(game, &Game::recalculate_moves_for_stack));
  connections.push_back
    (game->stack_info_changed.connect
     (sigc::mem_fun(*status_box, &StatusBox::on_stack_info_changed)));
  status_box->stack_tile_group_toggle.connect
    (sigc::mem_fun(this, &GameWindow::on_group_stack_toggled));
  status_box->setHeightFudgeFactor(turn_label->get_height());
  status_box->enforce_height();
  game->update_sidebar_stats();
  if (Playerlist::getActiveplayer()->getActivestack())
    status_box->on_stack_info_changed
      (Playerlist::getActiveplayer()->getActivestack());
  else
    status_box->show_stats();
    
  game->get_smallmap().resize();
  game->redraw();
}

void GameWindow::on_group_ungroup_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  status_box->toggle_group_ungroup();
}

void GameWindow::on_fight_order_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  FightOrderDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_levels_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  HeroLevelsDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_ruin_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Vector<int> pos;
  pos.x = 0;
  pos.y = 0;
  if (status_box->get_currently_selected_stack())
    pos = status_box->get_currently_selected_stack()->getPos();

  if (Templelist::getInstance()->size() == 0 &&
      Ruinlist::getInstance()->size() == 0)
    {
      std::string s = _("No ruins or temples to show!");
      TimedMessageDialog dialog(*window, s, 30);

      dialog.show_all();
      dialog.run();
      dialog.hide();
      return;
    }
  RuinReportDialog d(pos);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_army_bonus_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ArmyBonusDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_item_bonus_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ItemBonusDialog d;
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_army_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::ARMY);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_item_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  std::list<Stack*> stacks = Playerlist::getActiveplayer()->getStacksWithItems();
  std::list<MapBackpack*> bags = GameMap::getInstance()->getBackpacks();
  ItemReportDialog d(stacks, bags);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_city_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::CITY);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_gold_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::GOLD);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_production_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::PRODUCTION);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_winning_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::WINNING);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_city_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  HistoryReportDialog d(Playerlist::getActiveplayer(), 
			HistoryReportDialog::CITY);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_ruin_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  HistoryReportDialog d(Playerlist::getActiveplayer(), 
			HistoryReportDialog::RUIN);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_event_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  HistoryReportDialog d(Playerlist::getActiveplayer(),
			HistoryReportDialog::EVENTS);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_gold_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  HistoryReportDialog d(Playerlist::getActiveplayer(),
			HistoryReportDialog::GOLD);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_winner_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  HistoryReportDialog d(Playerlist::getActiveplayer(),
			HistoryReportDialog::WINNING);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_triumphs_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  TriumphsDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_help_about_activated()
{
  Gtk::AboutDialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/about-dialog.ui");

  xml->get_widget("dialog", dialog);
  dialog->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));

  dialog->set_version(PACKAGE_VERSION);
  dialog->set_logo(GraphicsCache::getMiscPicture("castle_icon.png")->to_pixbuf());
  dialog->set_transient_for(*window);
  dialog->show_all();
  dialog->run();
  dialog->hide();
  delete dialog;

  return;
}

void GameWindow::on_diplomacy_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  if (GameScenario::s_diplomacy == false)
    return;
  DiplomacyReportDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_diplomacy_button_clicked()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  DiplomacyDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::stop_game(std::string action)
{
  stop_action = action;
  Sound::getInstance()->disableBackground();
  if (game)
    {
      if (action == "game-over" && game->getScenario()->getPlayMode() == GameScenario::NETWORKED)
        give_some_cheese(game_winner);
      else
        game->stopGame();

      current_save_filename = "";
    }
}

void GameWindow::on_game_over(Player *winner)
{

  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/game-over-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  Gtk::Image *image;
  xml->get_widget("image", image);

  image->property_pixbuf() = GraphicsCache::getMiscPicture("win.png", false)->to_pixbuf();

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s += String::ucompose(_("Congratulations to %1 for conquering the world!"), 
	winner->getName());
  label->set_markup("<b>" + s + "</b>");

  dialog->show_all();
  dialog->run();
  dialog->hide();

  game_winner = winner;
  stop_game("game-over");
  delete dialog;
}

void GameWindow::on_player_died(Player *player)
{
  assert(player);

  Glib::ustring s;
  s += String::ucompose(_("The rule of %1 has permanently ended!"),
			player->getName());
  if (Playerlist::getInstance()->countHumanPlayersAlive() == 0 &&
      player->getType() == Player::HUMAN)
    {
      s += "\n";
      s += _("No further human resistance is possible\nbut the battle will continue!");
      s += "\n";
      s += _("Press `CTRL-P' to stop the war\nand visit the sites of thy old battles.");
    }

  TimedMessageDialog dialog(*window, s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
}

void GameWindow::on_message_requested(std::string msg)
{
  // FIXME: this is a bit crude, maybe beef it up
  Gtk::MessageDialog dialog(*window, msg);
  Decorated decorator;
  decorator.decorate(dynamic_cast<Gtk::Dialog*>(&dialog));
  //TimedMessageDialog dialog(*window, msg, 30, 5);
  dialog.show_all();
  dialog.run();
  dialog.hide();
}

void GameWindow::on_progress_status_changed(std::string string)
{
  status_box->set_progress_label(string);
}

void GameWindow::on_progress_changed()
{
  status_box->pulse();
}

void GameWindow::on_sidebar_stats_changed(SidebarStats s)
{
  status_box->update_sidebar_stats(s);

  if (Configuration::s_decorated)
      turn_label->set_markup(String::ucompose("<b>%1 %2</b>", _("Turn"), 
                                              s.turns));
  else
      turn_label->set_markup(String::ucompose("<b>%1 %2</b>", 
                                              _("Turn"), s.turns));
}

void GameWindow::on_bigmap_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, 
                        bigmap_image->get_allocated_width(), 
                        bigmap_image->get_allocated_height());
  bigmap_image->property_pixbuf() = pixbuf;
}

void GameWindow::on_smallmap_changed(Cairo::RefPtr<Cairo::Surface> map, Gdk::Rectangle r)
{
  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(map);
  double x1, x2, y1, y2;
  cr->get_clip_extents (x1, y1, x2, y2);
  int width = x2 - x1;
  int height = y2 - y1;

  if (smallmap_image->get_allocated_width() != width ||
      smallmap_image->get_allocated_height() != height)
    smallmap_image->set_size_request(width, height);
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, 
                        smallmap_image->get_allocated_width(), 
                        smallmap_image->get_allocated_height());
  smallmap_image->property_pixbuf() = pixbuf;
}

void GameWindow::on_smallmap_slid(Rectangle view)
{
  on_smallmap_changed(game->get_smallmap().get_surface(),
		      Gdk::Rectangle(view.x, view.y, view.w, view.h));
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
}

void GameWindow::on_city_tip_changed(City *city, MapTipPosition mpos)
{
  if (city == NULL)
    {
      delete city_info_tip;
      city_info_tip = NULL;
    }
  else
    {
      city_info_tip = new CityInfoTip(bigmap_image, mpos, city);
    }
}

void GameWindow::on_stack_tip_changed(StackTile *stile, MapTipPosition mpos)
{
  if (stile == NULL)
    {
      delete stack_info_tip;
      stack_info_tip = NULL;
    }
  else
    {
      stack_info_tip = new StackInfoTip(bigmap_image, mpos, stile);
    }
}

void GameWindow::on_bigmap_tip_changed(Glib::ustring tip, MapTipPosition pos)
{
  if (tip.empty())
    hide_map_tip();
  else
    show_map_tip(tip, pos);
}

void GameWindow::show_map_tip(Glib::ustring msg, MapTipPosition pos)
{
  // init the map tip
  if (map_tip != NULL)
    delete map_tip;
  map_tip = new Gtk::Window(Gtk::WINDOW_POPUP);

  Gtk::Frame *f = manage(new Gtk::Frame);
  f->property_shadow_type() = Gtk::SHADOW_ETCHED_OUT;

  Gtk::Label *l = manage(new Gtk::Label);
  l->set_padding(6, 6);
  l->set_text(msg);
  f->add(*l);

  map_tip->add(*f);
  Decorated decorator;
  decorator.decorate(map_tip, GraphicsCache::GAME_BACKGROUND, 200);
  f->show_all();

  // get screen position
  Vector<int> p;
  bigmap_image->get_window()->get_origin(p.x, p.y);
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
  if (map_tip != NULL)
    {
      delete map_tip;
      map_tip = NULL;
    }
}

Reward* GameWindow::on_sage_visited (Ruin *ruin, Sage *sage, Stack *stack)
{
  SageDialog d(sage, stack->getFirstHero()->getOwner(), 
	       static_cast<Hero*>(stack->getFirstHero()), ruin);
  d.set_parent_window(*window);
  Reward *reward = d.run();
  d.hide();
  return reward;
}

void GameWindow::on_ruin_rewarded (Reward_Ruin *reward)
{
  RuinRewardedDialog d(reward);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_ruin_searched(Ruin *ruin, Stack *stack, Reward *reward)
{
  Gtk::Dialog* dialog;
  if (ruin->hasSage())
    {
      if (reward->getType() == Reward::RUIN)
	return on_ruin_rewarded(static_cast<Reward_Ruin*>(reward));
    }

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/ruin-searched-dialog.ui");


  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  decorator.set_title(ruin->getName());

  Gtk::Label *label;
  xml->get_widget("label", label);

  Glib::ustring s = label->get_text();
  s += "\n\n";
  s += String::ucompose(_("%1 finds "), stack->getFirstHero()->getName());
  if (reward->getType() == Reward::GOLD)
    {
      Reward_Gold *gold = dynamic_cast<Reward_Gold*>(reward);
      s += String::ucompose(_("%1 gold pieces."), gold->getGold());
    }
  else if (reward->getType() == Reward::ALLIES)
    {
      Reward_Allies *allies = dynamic_cast<Reward_Allies*>(reward);
      s += String::ucompose(_("%1 allies!"), allies->getNoOfAllies());
    }
  else if (reward->getType() == Reward::ITEM)
    {
      Reward_Item *item = dynamic_cast<Reward_Item*>(reward);
      s += String::ucompose(_("the %1!"), item->getItem()->getName());
    }
  else if (reward->getType() == Reward::MAP)
    {
      Reward_Map *map = dynamic_cast<Reward_Map*>(reward);
      s += String::ucompose(_("a map!"), map->getName());
    }

  label->set_text(s);

  dialog->show_all();
  dialog->run();
  dialog->hide();
  delete dialog;
}

void GameWindow::on_ruinfight_started(Stack *attackers, Stack *defenders)
{
  //so and so encounters a wolf...
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/ruinfight-started-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  decorator.set_title(_("Searching"));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = label->get_text();
  s = "\n\n";
  s += attackers->getFirstHero()->getName() + " encounters some ";
  s += defenders->getStrongestArmy()->getName() + "...";
  label->set_text(s);

  dialog->show_all();
  dialog->run();
  dialog->hide();
  delete dialog;
}
void GameWindow::on_ruinfight_finished(Fight::Result result)
{
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/ruinfight-finished-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  if (result == Fight::ATTACKER_WON)
    decorator.set_title(_("Hero Victorious"));
  else
    decorator.set_title(_("Hero Defeated"));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s = label->get_text();
  s = "\n\n";
  if (result == Fight::ATTACKER_WON)
    s += _("...and is victorious!");
  else
    s += _("...and is slain by it!");
  label->set_text(s);

  Gtk::Image *image;
  xml->get_widget("image", image);
  if (result == Fight::ATTACKER_WON)
    image->property_file() = File::getMiscFile("various/ruin_2.png");
  else
    image->property_file() = File::getMiscFile("various/ruin_1.png");

  dialog->show_all();
  dialog->run();
  dialog->hide();
  delete dialog;
}

void GameWindow::on_fight_started(LocationBox box, Fight &fight)
{
  FightWindow d(fight);

  game->get_bigmap().setFighting(box);
  game->get_bigmap().draw(Playerlist::getViewingplayer());
  d.set_parent_window(*window);
  d.run(&d_quick_fights);
  d.hide();
  game->get_bigmap().setFighting(LocationBox(Vector<int>(-1,-1)));
  game->get_bigmap().draw(Playerlist::getViewingplayer());
  if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
    d_quick_fights = false;
}

void GameWindow::on_hero_brings_allies (int numAllies)
{
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/hero-brings-allies-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  decorator.set_title(_("Hero brings allies!"));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s = String::ucompose(
		       ngettext("The hero brings %1 ally!",
				"The hero brings %1 allies!", numAllies), numAllies);
  label->set_text(s);

  dialog->show_all();
  dialog->run();
  dialog->hide();
  delete dialog;
}

bool GameWindow::on_hero_offers_service(Player *player, HeroProto *hero, City *city, int gold)
{
  HeroOfferDialog d(player, hero, city, gold);
  d.set_parent_window(*window);
  bool retval = d.run();
  d.hide();
  return retval;
}

bool GameWindow::on_enemy_offers_surrender(int numPlayers)
{
  SurrenderDialog d(numPlayers);
  d.set_parent_window(*window);
  bool retval = d.run();
  d.hide();
  return retval;
}

void GameWindow::on_surrender_answered (bool accepted)
{
  if (accepted)
    on_message_requested
      (_("You graciously and benevolently accept their offer."));
  else
    {
      SurrenderRefusedDialog d;
      d.set_parent_window(*window);
      d.run();
      d.hide();
    }
}

bool GameWindow::on_stack_considers_treachery (Stack *stack, 
					       Player *them, Vector<int> pos)
{
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/treachery-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);
  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s = String::ucompose(_("Are you sure you want to attack %1?"), 
		       them->getName());
  s += "\n";
  s += _("Other players may not like this!");
  label->set_text(s);
  dialog->show_all();
  int response = dialog->run();
  dialog->hide();
  delete dialog;
  if (response == Gtk::RESPONSE_DELETE_EVENT)
    return false;
  else if (response == Gtk::RESPONSE_ACCEPT)
    return true;
  else
    return false;
}


void GameWindow::on_temple_visited(Temple *temple)
{
  RuinReportDialog d(temple->getPos());
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

bool GameWindow::on_temple_searched(Hero *hero, Temple *temple, int blessCount)
{
  QuestsManager *qm = QuestsManager::getInstance();
  Gtk::Dialog* dialog;
  bool hasHero = hero != NULL;
  bool ask_quest = false;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/temple-visit-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  decorator.set_title(temple->getName());

  Gtk::Label *l;
  Gtk::Button *close_button;
  Gtk::Button *accept_button;
  xml->get_widget("label", l);
  xml->get_widget("close_button", close_button);
  xml->get_widget("accept_button", accept_button);

  if (GameScenarioOptions::s_play_with_quests == 
      GameParameters::ONE_QUEST_PER_PLAYER)
    {
      if (qm->getPlayerQuests(Playerlist::getActiveplayer()).size() == 0 &&
          hasHero)
        ask_quest = true;
    }
  else if (GameScenarioOptions::s_play_with_quests == GameParameters::ONE_QUEST_PER_HERO)
    {
      if (hasHero && hero->hasQuest() == false)
        ask_quest = true;
    }

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
  if (ask_quest)
    {
      s = l->get_text() + "\n\n" + _("Do you seek a quest?");
      l->set_text(s);
    }

  dialog->show_all();

  if (ask_quest == false)
    {
      close_button->hide();
      s = _("_Close");
      accept_button->set_label(s);
    }

  if (blessCount > 0)
    Sound::getInstance()->playMusic("bless", 1);

  int response = dialog->run();
  dialog->hide();
  delete dialog;

  if (ask_quest == false)
    response = Gtk::RESPONSE_CANCEL;

  if (response == Gtk::RESPONSE_ACCEPT)		// accepted a quest
    return true;
  else
    return false;
}

void GameWindow::on_quest_assigned(Hero *hero, Quest *quest)
{
  QuestAssignedDialog d(hero, quest);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

static bool
hero_has_quest_here (Stack *s, City *c, bool *pillage, bool *sack, bool *raze, bool *occupy)
{
  Player *p = Playerlist::getActiveplayer();
  std::vector<Quest*> questlist;
  *pillage = false;
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
      if ((*i) == NULL)
	continue;
      if ((*i)->isPendingDeletion() == true)
	continue;
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
	  *pillage = true;
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
  CityDefeatedAction retval = CITY_DEFEATED_OCCUPY;
  Gtk::Button *raze_button;
  Gtk::Dialog* dialog;
  if (gold)
    on_city_looted (city, gold);
  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/city-defeated-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  Gtk::Image *image;
  xml->get_widget("city_image", image);
  image->property_file() = File::getMiscFile("various/city_occupied.png");
  image->show();

  Gtk::Label *label;
  xml->get_widget("label", label);
  int width = 0, height = 0;
  image->get_size_request(width, height);
  label->set_size_request(width, height);

  Glib::ustring name;
  Player *p = Playerlist::getActiveplayer();
  Army *h = p->getActivestack()->getFirstHero();
  if (h)
    name = h->getName();
  else
    name = p->getName();

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

  xml->get_widget("raze_button", raze_button);

  raze_button->set_sensitive
    (GameScenarioOptions::s_razing_cities == GameParameters::ON_CAPTURE || 
     GameScenarioOptions::s_razing_cities == GameParameters::ALWAYS);

  if (h) /* if there was a hero in the stack */
    {
      bool pillage, sack, raze, occupy;
      if (hero_has_quest_here (p->getActivestack(), city, 
			       &pillage, &sack, &raze, &occupy))
	{
	  Gtk::Button *button;
	  if (pillage)
	    {
	      xml->get_widget("pillage_button", button);
              button->property_can_focus() = true;
              button->property_can_default() = true;
	      button->grab_default();
	    }
	  if (sack)
	    {
	      xml->get_widget("sack_button", button);
              button->property_can_focus() = true;
              button->property_can_default() = true;
	      button->grab_default();
	    }
	  if (raze)
	    {
	      xml->get_widget("raze_button", button);
              button->property_can_focus() = true;
              button->property_can_default() = true;
	      button->grab_default();
	    }
	  if (occupy)
	    {
	      xml->get_widget("occupy_button", button);
              button->property_can_focus() = true;
              button->property_can_default() = true;
	      button->grab_default();
	    }
	}
    }

  if (city->getNoOfProductionBases() <= 0) {
    Gtk::Button *b;
    xml->get_widget("pillage_button", b);
    b->hide();
  }

  if (city->getNoOfProductionBases() <= 1) {
    Gtk::Button *b;
    xml->get_widget("sack_button", b);
    b->hide();
  }

  dialog->show();

  while (1)
    {
      int response = dialog->run();
      switch (response) 
	{
	case 1: retval = CITY_DEFEATED_OCCUPY; break;
	case 2: 
		{
		  bool razed = CityWindow::on_raze_clicked(city, dialog);
		  if (razed == false)
		    continue;
		  retval = CITY_DEFEATED_RAZE;
		  break;
		}
	case 3: retval = CITY_DEFEATED_PILLAGE; break;
	case 4: retval = CITY_DEFEATED_SACK; break;
	default: break;
	}
      if (retval)
	break;
    }
  dialog->hide();
  delete dialog;
  return retval;
}

void GameWindow::on_city_looted (City *city, int gold)
{
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/city-looted-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  decorator.set_title(String::ucompose(_("%1 Looted"), city->getName()));

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
  dialog->hide();
  delete dialog;
}
void GameWindow::on_city_pillaged(City *city, int gold, int pillaged_army_type)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Gtk::Dialog* dialog;
  Player *player = city->getOwner();
  unsigned int as = player->getArmyset();

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/city-pillaged-dialog.ui");

  Gtk::Image *pillaged_army_type_image;
  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  decorator.set_title(String::ucompose(_("Pillaged %1"), city->getName()));

  Gtk::Label *pillaged_army_type_cost_label;
  xml->get_widget("pillaged_army_type_cost_label", pillaged_army_type_cost_label);
  xml->get_widget("pillaged_army_type_image", pillaged_army_type_image);
  if (gold == 0)
    {
      Glib::RefPtr<Gdk::Pixbuf> empty_pic = 
        gc->getCircledArmyPic(as, 0, player, NULL, false, Shield::NEUTRAL, 
                              false)->to_pixbuf();
      pillaged_army_type_image->set(empty_pic);
      pillaged_army_type_cost_label->set_text("");
    }
  else
    {
      Glib::RefPtr<Gdk::Pixbuf> pic;
      pic = gc->getCircledArmyPic(as, pillaged_army_type, player, NULL, false,
                                  Shield::NEUTRAL, true)->to_pixbuf();
      pillaged_army_type_image->property_pixbuf() = pic;
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
  dialog->hide();
  delete dialog;
}

void GameWindow::on_city_sacked(City *city, int gold, std::list<guint32> sacked_types)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Player *player = city->getOwner();
  unsigned int as = player->getArmyset();
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/city-sacked-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  decorator.set_title(String::ucompose(_("Sacked %1"), city->getName()));

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
  Glib::RefPtr<Gdk::Pixbuf> empty_pic =
    gc->getCircledArmyPic(as, 0, player, NULL, false, Shield::NEUTRAL, 
                          false)->to_pixbuf();
  int i = 0;
  Gtk::Label *sack_label = NULL;
  Gtk::Image *sack_image = NULL;
  for (std::list<guint32>::iterator it = sacked_types.begin(); it != sacked_types.end(); it++)
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
      pic = gc->getCircledArmyPic(as, *it, player, NULL, false, 
                                  Shield::NEUTRAL, true)->to_pixbuf();
      sack_image->property_pixbuf() = pic;
      const ArmyProto *a = 
	Armysetlist::getInstance()->getArmy (player->getArmyset(), *it);
      s = String::ucompose("%1 gp", a->getNewProductionCost() / 2);
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
  dialog->hide();
  delete dialog;
}

void GameWindow::on_city_razed (City *city)
{
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/city-razed-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  decorator.set_title(String::ucompose(_("Razed %1"), city->getName()));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s = String::ucompose(_("The city of %1 is in ruins!"), city->getName());
  label->set_text(s);

  dialog->show_all();
  dialog->run();
  dialog->hide();
  delete dialog;
}

void GameWindow::on_city_visited(City *city)
{
  CityWindow d(city, 
	       GameScenarioOptions::s_razing_cities == GameParameters::ALWAYS,
	       GameScenarioOptions::s_see_opponents_production);

  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_ruin_visited(Ruin *ruin)
{
  RuinReportDialog d(ruin->getPos());
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::show_shield_turn() //show turn indicator
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
	shield_image[c]->property_pixbuf()=gc->getShieldPic(1,(*i))->to_pixbuf();
      else
	shield_image[c]->property_pixbuf()=gc->getShieldPic(0,(*i))->to_pixbuf();
      shield_image[c]->property_tooltip_text() = (*i)->getName();
      c++;
    }
  for (unsigned int i = c; i < MAX_PLAYERS; i++)
    shield_image[i]->clear();
}

void GameWindow::on_remote_next_player_turn()
{
  status_box->on_stack_info_changed(NULL);
  while (g_main_context_iteration(NULL, FALSE)); //doEvents

  d_quick_fights = false;
  show_shield_turn();
  turn_label->set_markup(String::ucompose("<b>%1 %2</b>", _("Turn"),
                                          GameScenarioOptions::s_round));
}

void GameWindow::on_next_player_turn(Player *player, unsigned int turn_number)
{
  Gtk::Dialog* dialog;

  status_box->on_stack_info_changed(NULL);
  while (g_main_context_iteration(NULL, FALSE)); //doEvents

  d_quick_fights = false;
  show_shield_turn();
  if (player->getType() != Player::HUMAN)
    return;
      
  if (Configuration::s_showNextPlayer == true)
    {
      Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + 
				    "/next-player-turn-dialog.ui");

      xml->get_widget("dialog", dialog);
      dialog->set_transient_for(*window);

      Decorated decorator;
      decorator.decorate(dialog);
      decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
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
      dialog->hide();
      delete dialog;
      show();
    }

}

void GameWindow::on_medal_awarded_to_army(Army *army, int medaltype)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/medal-awarded-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  Gtk::Image *image;
  xml->get_widget("image", image);
  Player *active = Playerlist::getInstance()->getActiveplayer();
  image->property_pixbuf() = 
    gc->getCircledArmyPic(active->getArmyset(), army->getTypeId(), active, 
		   army->getMedalBonuses(), false, Shield::NEUTRAL, 
                   true)->to_pixbuf();
  Gtk::Image *medal_image;
  xml->get_widget("medal_image", medal_image);
  medal_image->property_pixbuf() = 
    gc->getMedalPic(true, medaltype)->to_pixbuf();

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  if (medaltype == 0)
    s += String::ucompose(_("Your unit of %1 is awarded the avenger's medal of valour!"), army->getName());
  else if (medaltype == 1)
    s += String::ucompose(_("Your unit of %1 is awarded the defender's medal of bravery!"), army->getName());
  else if (medaltype == 2)
    s += String::ucompose(_("Your unit of %1 is awarded the veteran's medal!"), army->getName());
  else
    s += String::ucompose(_("Your unit of %1 is awarded a medal!"), army->getName());
  label->set_text(s);

  dialog->show_all();
  dialog->run();
  dialog->hide();
  delete dialog;
}

Army::Stat GameWindow::on_hero_gains_level(Hero *hero)
{
  ArmyGainsLevelDialog d(hero, GameScenario::s_hidden_map);

  d.set_parent_window(*window);
  d.run();
  d.hide();

  return d.get_selected_stat();
}

void GameWindow::on_game_loaded(Player *player)
{
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/game-loaded-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s += String::ucompose(_("%1, your turn continues."), player->getName());
  label->set_text(s);

  dialog->show_all();
  dialog->run();
  dialog->hide();
  delete dialog;
}

void GameWindow::on_quest_completed(Quest *quest, Reward *reward)
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  QuestCompletedDialog d(quest, reward);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}

void GameWindow::on_quest_expired(Quest *quest)
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/quest-expired-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

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
  dialog->hide();
  delete dialog;
}

void GameWindow::on_inspect_activated ()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  if (Playerlist::getActiveplayer()->getHeroes().size() == 0)
    return;
  Hero *hero = NULL;
  Vector<int> pos = Vector<int>(-1,-1);
  if (status_box->get_currently_selected_stack() != NULL)
    {
      hero = dynamic_cast<Hero*>(status_box->get_currently_selected_stack()->getFirstHero());
      pos = status_box->get_currently_selected_stack()->getPos();
    }
    
  HeroDialog d(hero, pos);
  d.set_parent_window(*window);
  d.run();
  d.hide();
}
void GameWindow::on_plant_standard_activated ()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Playerlist::getActiveplayer()->heroPlantStandard(NULL);
}
    
void GameWindow::on_stack_moves(Stack *stack, Vector<int> pos)
{
  Player *active = Playerlist::getInstance()->getActiveplayer();
  if (!active)
    return;
  if (active->getActivestack() != stack)
    return;
  if (GameMap::getEnemyCity(pos))
    return;
  if (GameMap::getEnemyStack(pos))
    return;
  game->get_smallmap().center_view_on_tile (pos, true);
  int step = TIMER_BIGMAP_SELECTOR * 1000;
  for (int i = 0; i < Configuration::s_displaySpeedDelay; i += step)
    {
      game->get_bigmap().draw(Playerlist::getViewingplayer());
      while (g_main_context_iteration(NULL, FALSE)); //doEvents
      Glib::usleep(step);
    }
}

void GameWindow::on_advice_asked(float percent)
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  //we asked for advice on a fight, and we're being told that we 
  //have a PERCENT chance of winning the fight
  Gtk::Dialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/military-advisor-dialog.ui");

  xml->get_widget("dialog", dialog);
  Decorated decorator;
  decorator.decorate(dialog);
  decorator.window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));
  dialog->set_transient_for(*window);

  decorator.set_title(_("Advisor!"));

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
  num = rand() % 7;
  if (percent >= 90.0)
    {
      if (num == 0)
	s += _("This battle will surely be as simple as butchering sleeping cattle!");
      else if (num == 1)
	s += _("A battle here would be as simple as butchering sleeping cattle!");
      else if (num == 2)
	s += _("I believe this battle will surely be as simple as butchering sleeping cattle!");
      else if (num == 3)
	s += _("This battle would be as simple as butchering sleeping cattle!");
      else if (num == 4)
	s += _("A battle here would be as simple as butchering sleeping cattle!");
      else if (num == 5)
	s += _("I believe this battle will be as simple as butchering sleeping cattle!");
      else if (num == 6)
	s += _("This battle shall be as simple as butchering sleeping cattle!");
    }
  else if (percent >= 80.0)
    {
      if (num == 0)
	s += _("This battle will surely be an easy victory!  We cannot lose!");
      else if (num == 1)
	s += _("A battle here would be an easy victory!  We cannot lose!");
      else if (num == 2)
	s += _("I believe this battle will surely be an easy victory!  We cannot lose!");
      else if (num == 3)
	s += _("This battle would be an easy victory!  We cannot lose!");
      else if (num == 4)
	s += _("A battle here would be an easy victory!  We cannot lose!");
      else if (num == 5)
	s += _("I believe this battle will be an easy victory!  We cannot lose!");
      else if (num == 6)
	s += _("This battle shall be an easy victory!  We cannot lose!");
    }
  else if (percent >= 70.0)
    {
      if (num == 0)
	s += _("This battle will surely be a comfortable victory!");
      else if (num == 1)
	s += _("A battle here would be a comfortable victory!");
      else if (num == 2)
	s += _("I believe this battle will surely be a comfortable victory!");
      else if (num == 3)
	s += _("This battle would be a comfortable victory!");
      else if (num == 4)
	s += _("A battle here would be a comfortable victory!");
      else if (num == 5)
	s += _("I believe this battle will be a comfortable victory!");
      else if (num == 6)
	s += _("This battle shall be a comfortable victory!");
    }
  else if (percent >= 60.0)
    {
      if (num == 0)
	s += _("This battle will surely be a hard fought victory! But we shall win!");
      else if (num == 1)
	s += _("A battle here would be a hard fought victory! But we shall win!");
      else if (num == 2)
	s += _("I believe this battle will surely be a hard fought victory! But we shall win!");
      else if (num == 3)
	s += _("This battle would be a hard fought victory! But we shall win!");
      else if (num == 4)
	s += _("A battle here would be a hard fought victory! But we shall win!");
      else if (num == 5)
	s += _("I believe this battle will be a hard fought victory! But we shall win!");
      else if (num == 6)
	s += _("This battle shall be a hard fought victory! But we shall win!");
    }
  else if (percent >= 50.0)
    {
      if (num == 0)
	s += _("This battle will surely be very evenly matched!");
      else if (num == 1)
	s += _("A battle here would be very evenly matched!");
      else if (num == 2)
	s += _("I believe this battle will surely be very evenly matched!");
      else if (num == 3)
	s += _("This battle would be very evenly matched!");
      else if (num == 4)
	s += _("A battle here would be very evenly matched!");
      else if (num == 5)
	s += _("I believe this battle will be very evenly matched!");
      else if (num == 6)
	s += _("This battle shall be very evenly matched!");
    }
  else if (percent >= 40.0)
    {
      if (num == 0)
	s += _("This battle will surely be difficult but not impossible to win!");
      else if (num == 1)
	s += _("A battle here would be difficult but not impossible to win!");
      else if (num == 2)
	s += _("I believe this battle will surely be difficult but not impossible to win!");
      else if (num == 3)
	s += _("This battle would be difficult but not impossible to win!");
      else if (num == 4)
	s += _("A battle here would be difficult but not impossible to win!");
      else if (num == 5)
	s += _("I believe this battle will be difficult but not impossible to win!");
      else if (num == 6)
	s += _("This battle shall be difficult but not impossible to win!");
    }
  else if (percent >= 30.0)
    {
      if (num == 0)
	s += _("This battle will surely be a brave choice! I leave it to thee!");
      else if (num == 1)
	s += _("A battle here would be a brave choice! I leave it to thee!");
      else if (num == 2)
	s += _("I believe this battle will surely be a brave choice! I leave it to thee!");
      else if (num == 3)
	s += _("This battle would be a brave choice! I leave it to thee!");
      else if (num == 4)
	s += _("A battle here would be a brave choice! I leave it to thee!");
      else if (num == 5)
	s += _("I believe this battle will be a brave choice! I leave it to thee!");
      else if (num == 6)
	s += _("This battle shall be a brave choice! I leave it to thee!");
    }
  else if (percent >= 20.0)
    {
      if (num == 0)
	s += _("This battle will surely be a foolish decision!");
      else if (num == 1)
	s += _("A battle here would be a foolish decision!");
      else if (num == 2)
	s += _("I believe this battle will surely be a foolish decision!");
      else if (num == 3)
	s += _("This battle would be a foolish decision!");
      else if (num == 4)
	s += _("A battle here would be a foolish decision!");
      else if (num == 5)
	s += _("I believe this battle will be a foolish decision!");
      else if (num == 6)
	s += _("This battle shall be a foolish decision!");
    }
  else if (percent >= 10.0)
    {
      if (num == 0)
	s += _("This battle will surely be sheerest folly!  Thou shouldst not attack!");
      else if (num == 1)
	s += _("A battle here would be sheerest folly!  Thou shouldst not attack!");
      else if (num == 2)
	s += _("I believe this battle will surely be sheerest folly!  Thou shouldst not attack!");
      else if (num == 3)
	s += _("This battle would be sheerest folly!  Thou shouldst not attack!");
      else if (num == 4)
	s += _("A battle here would be sheerest folly!  Thou shouldst not attack!");
      else if (num == 5)
	s += _("I believe this battle will be sheerest folly!  Thou shouldst not attack!");
      else if (num == 6)
	s += _("This battle shall be sheerest folly!  Thou shouldst not attack!");
    }
  else
    {
      if (num == 0)
	s += _("This battle will surely be complete and utter suicide!");
      else if (num == 1)
	s += _("A battle here would be complete and utter suicide!");
      else if (num == 2)
	s += _("I believe this battle will surely be complete and utter suicide!");
      else if (num == 3)
	s += _("This battle would be complete and utter suicide!");
      else if (num == 4)
	s += _("A battle here would be complete and utter suicide!");
      else if (num == 5)
	s += _("I believe this battle will be complete and utter suicide!");
      else if (num == 6)
	s += _("This battle shall be complete and utter suicide!");
    }
  label->set_text(s);

  dialog->show_all();
  dialog->run();
  dialog->hide();
  delete dialog;
  return;
}

void GameWindow::on_show_lobby_activated()
{
  show_lobby.emit();
}

//taken from go-file.c of gnucash 2.0.4
static char *
check_program (char const *prog)
{
  if (NULL == prog)
    return NULL;
  if (g_path_is_absolute (prog)) {
    if (!g_file_test (prog, G_FILE_TEST_IS_EXECUTABLE))
      return NULL;
  } else if (!g_find_program_in_path (prog))
    return NULL;
  return g_strdup (prog);
}

//taken from go-file.c of gnucash 2.0.4
GError *
go_url_show (gchar const *url)
{
  GError *err = NULL;
  char *browser = NULL;
  char *clean_url = NULL;

  /* 1) Check BROWSER env var */
  browser = check_program (getenv ("BROWSER"));

  if (browser == NULL) {
    static char const * const browsers[] = {
      "sensible-browser",	/* debian */
      "htmlview", /* fedora */
      "firefox",
      "epiphany",
      "mozilla-firebird",
      "mozilla",
      "netscape",
      "konqueror",
      "xterm -e w3m",
      "xterm -e lynx",
      "xterm -e links"
    };
    unsigned i;
    for (i = 0 ; i < G_N_ELEMENTS (browsers) ; i++)
      if (NULL != (browser = check_program (browsers[i])))
	break;
  }

  if (browser != NULL) {
    gint    argc;
    gchar **argv = NULL;
    char   *cmd_line = g_strconcat (browser, " %1", NULL);

    if (g_shell_parse_argv (cmd_line, &argc, &argv, &err)) {
      /* check for '%1' in an argument and substitute the url
       * 			 * otherwise append it */
      gint i;
      char *tmp;

      for (i = 1 ; i < argc ; i++)
	if (NULL != (tmp = strstr (argv[i], "%1"))) {
	  *tmp = '\0';
	  tmp = g_strconcat (argv[i],
			     (clean_url != NULL) ? (char const *)clean_url : url,
			     tmp+2, NULL);
	  g_free (argv[i]);
	  argv[i] = tmp;
	  break;
	}

      /* there was actually a %1, drop the one we added */
      if (i != argc-1) {
	g_free (argv[argc-1]);
	argv[argc-1] = NULL;
      }
      g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
		     NULL, NULL, NULL, &err);
      g_strfreev (argv);
    }
    g_free (cmd_line);
  }
  g_free (browser);
  g_free (clean_url);
  return err;
}

void GameWindow::on_online_help_activated()
{
  go_url_show ("http://www.nongnu.org/lordsawar/manual/" PACKAGE_VERSION "/lordsawar.html");
  return;
}

void GameWindow::on_player_replaced(Player *p)
{
  game->addPlayer(p);
}

void GameWindow::on_grid_toggled()
{
  game->get_bigmap().toggle_grid();
}

void GameWindow::give_some_cheese(Player *winner)
{
  game->endOfGameRoaming(winner);
  game_button_box->give_some_cheese();
  end_turn_menuitem->set_sensitive(false);
  save_game_menuitem->set_sensitive(false);
  save_game_as_menuitem->set_sensitive(false);
  Playerlist::getActiveplayer()->clearFogMap();
  show_shield_turn();
  game->redraw();
  on_city_history_activated();
}

void GameWindow::on_commentator_comments(std::string comment)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  TimedMessageDialog dialog (*window, comment, 0);
  dialog.set_title(_("The Warlord Says..."));
    
  PixMask *img = 
    gc->getGameButtonPic(GraphicsCache::DIPLOMACY_NO_PROPOSALS, 1)->copy();
  PixMask::scale(img, 60, 60);
  dialog.set_image(img->to_pixbuf());
  dialog.run();
  dialog.hide();
}
      
void GameWindow::on_abbreviated_fight_started(LocationBox box)
{
  game->get_bigmap().setFighting(box);
  game->get_bigmap().draw(Playerlist::getViewingplayer());
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
  Glib::usleep(TIMER_BIGMAP_SELECTOR * 1000);
  game->get_bigmap().setFighting(LocationBox(Vector<int>(-1,-1)));
  game->get_bigmap().draw(Playerlist::getViewingplayer());
}

Item* GameWindow::on_select_item(std::list<Item*> items)
{
  Item *item = NULL;
  UseItemDialog d(items);
  d.set_parent_window(*window);
  d.run();
  item = d.get_selected_item();
  d.hide();
  return item;
}
     
Player *GameWindow::on_select_item_victim_player()
{
  Player *player = NULL;
  UseItemOnPlayerDialog d;
  d.set_parent_window(*window);
  player = d.run();
  d.hide();
  return player;
}
    
City *GameWindow::on_select_city_to_use_item_on(SelectCityMap::Type type)
{
  UseItemOnCityDialog d(type);
  City *city = d.run();
  d.hide();
  return city;
}
    
void GameWindow::on_gold_stolen(Player *victim, guint32 gold_pieces)
{
  std::string s = "";
  s += 
    String::ucompose(ngettext("%1 gold piece was stolen from %2!",
                              "%1 gold pieces were stolen from %2!", 
                              gold_pieces), gold_pieces, victim->getName());
  TimedMessageDialog dialog(*window, s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
  return;
}

void GameWindow::on_ships_sunk(Player *victim, guint32 num_armies)
{
  std::string s = "";
  s += 
    String::ucompose(ngettext("%1 army unit was sunk to the watery depths!",
                              "%1 army units were sunk to the watery depths!", 
                              num_armies), num_armies);
  TimedMessageDialog dialog(*window, s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
  return;
}

void GameWindow::on_bags_picked_up(Hero *hero, guint32 num_bags)
{
  std::string s = "";
  s += 
    String::ucompose(ngettext("%1 bag was retrieved by %2!",
                              "%1 bags were retrieved by %2!", 
                              num_bags), num_bags, hero->getName());
  TimedMessageDialog dialog(*window, s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
  return;
}

void GameWindow::on_bridge_burned(Hero *hero)
{
  std::string s = "";
  s += 
    String::ucompose(_("%1 has burned a bridge!  None shall pass this way again!"), hero->getName());
  TimedMessageDialog dialog(*window, s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
  return;
}

void GameWindow::on_keeper_captured(Hero *hero, Ruin *ruin, Glib::ustring name)
{
  std::string s = "";
  s += String::ucompose(_("%1 has turned a unit of %2 from %3!"), 
                        hero->getName(), name, ruin->getName());
  TimedMessageDialog dialog(*window, s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
  return;
}

void GameWindow::on_city_diseased(Hero *hero, Glib::ustring name, guint32 num_armies)
{
  std::string s = "";
  s += String::ucompose(ngettext("%1 unit in %2 have perished!",
                                 "%1 units in %2 have perished!", num_armies),
                        num_armies, name);
  TimedMessageDialog dialog(*window, s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
  return;
}

void GameWindow::on_city_defended(Hero *hero, Glib::ustring city_name, Glib::ustring army_name, guint32 num_armies)
{
  std::string s = "";
  s += String::ucompose(ngettext("%1 unit of %2 have been raised in %3!",
                                 "%1 units of %2 have been raised in %3!", num_armies),
                        num_armies, army_name, city_name);
  TimedMessageDialog dialog(*window, s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
  return;
}

void GameWindow::on_city_persuaded(Hero *hero, Glib::ustring city_name, guint32 num_armies)
{
  if (game)
    game->redraw();
  std::string s = "";
  if (num_armies != 0)
    s += String::ucompose
      (ngettext("%1 unit in %2 have been persuaded to fly your flag!",
                "%1 units in %2 have been persuaded to fly your flag!", 
                num_armies),
       num_armies, city_name);
  else
    s += String::ucompose
      (_("The citizens of %1 have been persuaded to fly your flag!"), 
       city_name);
  TimedMessageDialog dialog(*window, s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
  return;
}

void GameWindow::on_stack_teleported(Hero *hero, Glib::ustring city_name)
{
  std::string s = "";
  s += String::ucompose(_("%1 has teleported to %2!"), 
                        hero->getName(), city_name);
  TimedMessageDialog dialog(*window, s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
  return;
}

void GameWindow::on_monster_summoned(Hero *hero, Glib::ustring name)
{
  std::string s = "";
  s += String::ucompose(_("A unit of %1 has come to the aid of %2!"), 
                        name, hero->getName());
  TimedMessageDialog dialog(*window, s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
  return;
}

void GameWindow::on_worms_killed(Hero *hero, Glib::ustring name, guint32 num_killed)
{
  std::string s = "";
  s += 
    String::ucompose(ngettext("%1 unit of %2 was banished by %3!",
                              "%1 units of %2 were banished by %3!", 
                              num_killed), num_killed, name, hero->getName());
  TimedMessageDialog dialog(*window, s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
  return;
}

void GameWindow::on_mp_added_to_hero_stack(Hero *hero, guint32 mp)
{
  std::string s = "";
  s += 
    String::ucompose(ngettext("%1 movement point was added to %2 and accompanying units!",
                              "%1 movement points were added to %2 and accompanying units!", 
                              mp), mp, hero->getName());
  TimedMessageDialog dialog(*window, s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
  return;
}
