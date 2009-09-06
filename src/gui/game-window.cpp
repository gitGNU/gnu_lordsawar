//  Copyright (C) 2007, 2008, Ole Laursen
//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include <config.h>

#include <iomanip>
#include <queue>
#include <SDL_video.h>
#include <assert.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/hide.h>

#include <gtkmm.h>
#include <glib.h>

#include "game-window.h"

#include "gtksdl.h"
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
#include "stacklist.h"
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
#include "GraphicsLoader.h"
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


GameWindow::GameWindow()
{
    sdl_inited = false;
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/game-window.ui");

    Gtk::Window *w = 0;
    xml->get_widget("window", w);
    window.reset(w);
    w->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
    decorate(window.get(), File::getMiscFile("various/back.bmp"));
    
    window_closed.connect
      (sigc::mem_fun(*this, &GameWindow::on_quit_activated));

    w->signal_delete_event().connect
      (sigc::mem_fun(*this, &GameWindow::on_delete_event));

    xml->get_widget("menubar", menubar);
    xml->get_widget("sdl_container", sdl_container);
    xml->get_widget("stack_info_box", stack_info_box);
    xml->get_widget("stack_info_container", stack_info_container);
    Gtk::Viewport *vp;
    xml->get_widget("sdl_viewport", vp);
    decorate_border(vp, 175);
    xml->get_widget("group_moves_label", group_moves_label);
    xml->get_widget("group_togglebutton", group_ungroup_toggle);
    group_ungroup_toggle->signal_toggled().connect
      (sigc::bind(sigc::mem_fun(*this, &GameWindow::on_group_toggled),
		  group_ungroup_toggle));
    xml->get_widget("terrain_image", terrain_image);
    xml->get_widget("stats_box", stats_box);
    xml->get_widget("progress_box", progress_box);
    xml->get_widget("turn_progressbar", turn_progressbar);
    xml->get_widget("progress_status_label", progress_status_label);
    stack_info_box->hide();
    stats_box->hide();
    progress_box->show();

    // the map image
    xml->get_widget("map_image", map_image);
    xml->get_widget("map_eventbox", map_eventbox);
    xml->get_widget("map_container", map_container);
    map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
			     Gdk::POINTER_MOTION_MASK | Gdk::SCROLL_MASK);
      map_eventbox->signal_button_press_event().connect
       (sigc::mem_fun(*this, &GameWindow::on_map_mouse_button_event));
      map_eventbox->signal_button_release_event().connect
       (sigc::mem_fun(*this, &GameWindow::on_map_mouse_button_event));
      map_eventbox->signal_motion_notify_event().connect
       (sigc::mem_fun(*this, &GameWindow::on_map_mouse_motion_event));
    Gtk::Viewport *mvp;
    xml->get_widget("map_viewport", mvp);
    decorate_border(mvp, 175);
    xml->get_widget("control_panel_viewport", vp);
    decorate_border(vp, 175);

    // the stats
    xml->get_widget("cities_stats_image", cities_stats_image);
    cities_stats_image->property_file() = File::getMiscFile("various/smallcity.png");
    xml->get_widget("gold_stats_image", gold_stats_image);
    gold_stats_image->property_file() = File::getMiscFile("various/smalltreasury.png");
    xml->get_widget("income_stats_image", income_stats_image);
    income_stats_image->property_file() = File::getMiscFile("various/smallincome.png");
    xml->get_widget("upkeep_stats_image", upkeep_stats_image);
    upkeep_stats_image->property_file() = File::getMiscFile("various/smallupkeep.png");
    
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
    xml->get_widget("diplomacy_button", diplomacy_button);
    xml->get_widget("defend_button", defend_button);
    xml->get_widget("park_button", park_button);
    xml->get_widget("deselect_button", deselect_button);
    xml->get_widget("search_button", search_button);
    xml->get_widget("move_button", move_button);
    xml->get_widget("move_all_button", move_all_button);
    xml->get_widget("end_turn_button", end_turn_button);
    xml->get_widget("nw_keypad_button", nw_keypad_button);
    xml->get_widget("n_keypad_button", n_keypad_button);
    xml->get_widget("ne_keypad_button", ne_keypad_button);
    xml->get_widget("e_keypad_button", e_keypad_button);
    xml->get_widget("w_keypad_button", w_keypad_button);
    xml->get_widget("sw_keypad_button", sw_keypad_button);
    xml->get_widget("s_keypad_button", s_keypad_button);
    xml->get_widget("se_keypad_button", se_keypad_button);

    // fill in imagery
    d_button_images = disassemble_row(File::getMiscFile("various/buttons.png"), 11);
    next_movable_button->add(*manage(new Gtk::Image(d_button_images[2])));
    center_button->add(*manage(new Gtk::Image(d_button_images[5])));
    diplomacy_button->add(*manage(new Gtk::Image(d_button_images[0])));
    defend_button->add(*manage(new Gtk::Image(d_button_images[6])));
    park_button->add(*manage(new Gtk::Image(d_button_images[1])));
    deselect_button->add(*manage(new Gtk::Image(d_button_images[7])));
    search_button->add(*manage(new Gtk::Image(d_button_images[9])));
    move_button->add(*manage(new Gtk::Image(d_button_images[3])));
    move_all_button->add(*manage(new Gtk::Image(d_button_images[4])));
    end_turn_button->add(*manage(new Gtk::Image(d_button_images[10])));
    
    d_arrow_images = disassemble_row(File::getMiscFile("various/arrows.png"), 
				     8);
    nw_keypad_button->add(*manage(new Gtk::Image(d_arrow_images[0])));
    n_keypad_button->add(*manage(new Gtk::Image(d_arrow_images[1])));
    ne_keypad_button->add(*manage(new Gtk::Image(d_arrow_images[2])));
    e_keypad_button->add(*manage(new Gtk::Image(d_arrow_images[3])));
    w_keypad_button->add(*manage(new Gtk::Image(d_arrow_images[4])));
    sw_keypad_button->add(*manage(new Gtk::Image(d_arrow_images[5])));
    s_keypad_button->add(*manage(new Gtk::Image(d_arrow_images[6])));
    se_keypad_button->add(*manage(new Gtk::Image(d_arrow_images[7])));

    // connect callbacks for the menu
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
    xml->get_widget("inspect_menuitem", inspect_menuitem);
    xml->get_widget("plant_standard_menuitem", plant_standard_menuitem);
    xml->get_widget("city_history_menuitem", city_history_menuitem);
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
    levels_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_levels_activated));
    xml->get_widget("inspect_menuitem", inspect_menuitem);
    inspect_menuitem->signal_activate().connect
      (sigc::mem_fun(*this, &GameWindow::on_inspect_activated));
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
	//colour the menubar
	Glib::RefPtr<Gtk::Style> copy;
	Glib::RefPtr<Gdk::Pixbuf> back;
	Glib::RefPtr<Gdk::Pixmap> pixmap;
	Glib::RefPtr<Gdk::Bitmap> bitmap;
	copy = menubar->get_style()->copy();
	back = Gdk::Pixbuf::create_from_file(File::getMiscFile("various/back.bmp"));
	pixmap = Gdk::Pixmap::create
	  (window->get_window(), back->get_width(), back->get_height());
	back->composite_color(back, 0, 0, 
			      back->get_width(), back->get_height(), 
			      0.0, 0.0, 1.0, 1.0, Gdk::INTERP_NEAREST, 127, 
			      0, 0, 64, 0, 0);
	back->render_pixmap_and_mask(pixmap, bitmap, 10);
	copy->set_bg_pixmap(Gtk::STATE_NORMAL, pixmap);
	menubar->set_style(copy);
      }

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
    diplomacy_button->show_all();
    defend_button->show_all();
    park_button->show_all();
    deselect_button->show_all();
    search_button->show_all();
    move_button->show_all();
    move_all_button->show_all();
    end_turn_button->show_all();
    nw_keypad_button->show_all();
    n_keypad_button->show_all();
    ne_keypad_button->show_all();
    e_keypad_button->show_all();
    w_keypad_button->show_all();
    sw_keypad_button->show_all();
    s_keypad_button->show_all();
    se_keypad_button->show_all();
    
    sdl_container->show_all();
    window->show();
    on_sdl_surface_changed();
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
    sdl_widget->signal_scroll_event().connect
       (sigc::mem_fun(*this, &GameWindow::on_sdl_scroll_event));
    
    // connect to the special signal that signifies that a new surface has been
    // generated and attached to the widget
    g_signal_connect(G_OBJECT(sdl_widget->gobj()), "surface-attached",
		     G_CALLBACK(surface_attached_helper), this);
    
    sdl_container->add(*sdl_widget);

	    

}

void GameWindow::new_network_game(GameScenario *game_scenario, NextTurn *next_turn)
{
  if (GameServer::getInstance()->isListening() == true)
    GameServer::getInstance()->round_begins.connect(sigc::mem_fun(this, &GameWindow::on_remote_next_player_turn));
  else
      GameClient::getInstance()->round_begins.connect(sigc::mem_fun(this, &GameWindow::on_remote_next_player_turn));
  bool success = false;
  //stop_game();
  success = setup_game(game_scenario, next_turn);
  if (!success)
    return;
  setup_signals(game_scenario);
  game->redraw();
  game->startGame();
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

  setup_signals(game_scenario);
  game->loadGame();
  //we don't get here until the game ends.
  if (Playerlist::getInstance()->countPlayersAlive())
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

void GameWindow::setup_signals(GameScenario *game_scenario)
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
	       
  connections.push_back (diplomacy_button->signal_clicked().connect
   (sigc::mem_fun (*this, &GameWindow::on_diplomacy_button_clicked)));
  connections.push_back 
    (game->received_diplomatic_proposal.connect 
     (sigc::mem_fun(*this, &GameWindow::change_diplomacy_button_image)));
  connections.push_back 
    (game->city_too_poor_to_produce.connect 
     (sigc::mem_fun(*this, &GameWindow::show_city_production_report)));
  connections.push_back 
    (game->can_end_turn.connect 
     (sigc::mem_fun(*this, &GameWindow::update_diplomacy_button)));

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
	       sigc::mem_fun(game.get(), &Game::move_selected_stack_along_path),
	       game->can_move_selected_stack_along_path);
  setup_button(move_all_button,
	       sigc::mem_fun(game.get(), &Game::move_all_stacks),
	       game->can_move_all_stacks);
  setup_button(end_turn_button,
	       sigc::mem_fun(game.get(), &Game::end_turn),
	       game->can_end_turn);
  if (game_scenario->getPlayMode() ==  GameScenario::PLAY_BY_MAIL)
    setup_button(end_turn_button,
		 sigc::mem_fun(*this, &GameWindow::end_turn_play_by_mail),
		 game->can_end_turn);
  setup_button(nw_keypad_button,
	       sigc::mem_fun(game.get(), &Game::move_selected_stack_northwest),
	       game->can_move_selected_stack);
  setup_button(n_keypad_button,
	       sigc::mem_fun(game.get(), &Game::move_selected_stack_north),
	       game->can_move_selected_stack);
  setup_button(ne_keypad_button,
	       sigc::mem_fun(game.get(), &Game::move_selected_stack_northeast),
	       game->can_move_selected_stack);
  setup_button(e_keypad_button,
	       sigc::mem_fun(game.get(), &Game::move_selected_stack_east),
	       game->can_move_selected_stack);
  setup_button(w_keypad_button,
	       sigc::mem_fun(game.get(), &Game::move_selected_stack_west),
	       game->can_move_selected_stack);
  setup_button(sw_keypad_button,
	       sigc::mem_fun(game.get(), &Game::move_selected_stack_southwest),
	       game->can_move_selected_stack);
  setup_button(s_keypad_button,
	       sigc::mem_fun(game.get(), &Game::move_selected_stack_south),
	       game->can_move_selected_stack);
  setup_button(se_keypad_button,
	       sigc::mem_fun(game.get(), &Game::move_selected_stack_southeast),
	       game->can_move_selected_stack);
  setup_menuitem(move_all_menuitem,
		 sigc::mem_fun(game.get(), &Game::move_all_stacks),
		 game->can_move_all_stacks);
  setup_menuitem(end_turn_menuitem,
		 sigc::mem_fun(game.get(), &Game::end_turn),
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
		 sigc::mem_fun(game.get(), &Game::search_selected_stack),
		 game->can_search_selected_stack);
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
  setup_menuitem(diplomacy_report_menuitem,
		 sigc::mem_fun(*this, 
			       &GameWindow::on_diplomacy_report_activated),
		 game->can_see_diplomacy); 
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

  connections.push_back
    (game->remote_next_player_turn.connect
     (sigc::mem_fun(*this, &GameWindow::on_remote_next_player_turn)));
}

void GameWindow::show_city_production_report (bool destitute)
{
  if (!destitute)
    return;
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::PRODUCTION);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::change_diplomacy_button_image (bool proposals_present)
{
  /* switch up the image. */
  if (proposals_present)
    diplomacy_button->property_image() = new Gtk::Image(d_button_images[8]);
  else
    diplomacy_button->property_image() = new Gtk::Image(d_button_images[0]);
}

void GameWindow::end_turn_play_by_mail ()
{
  //prompt to save the turn file!
  Gtk::FileChooserDialog chooser(*window.get(), _("Save your Turn file and mail it back"),
				 Gtk::FILE_CHOOSER_ACTION_SAVE);
  Gtk::FileFilter trn_filter;
  trn_filter.add_pattern("*.trn");
  chooser.set_filter(trn_filter);
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
	(*window.get(), ("Now send the turn file back to the game master."), 0);
      dialog.run();
      dialog.hide();
    }
  game_ended.emit();
}

void GameWindow::update_diplomacy_button (bool sensitive)
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    {
      diplomacy_button->set_sensitive (false);
      return;
    }
  if (GameScenario::s_diplomacy == false)
    {
      diplomacy_button->set_sensitive (false);
      return;
    }
  diplomacy_button->set_sensitive(sensitive);
}

bool GameWindow::setup_game(GameScenario *game_scenario, NextTurn *nextTurn)
{
  currently_selected_stack = NULL;
  Playerlist *pl = Playerlist::getInstance();
  Armysetlist *al = Armysetlist::getInstance();
  for (Playerlist::iterator i = pl->begin(); i != pl->end(); i++)
    if ((*i)->getType() != Player::NETWORKED)
      GraphicsLoader::instantiatePixmaps(al->getArmyset((*i)->getArmyset()));

  GraphicsLoader::instantiatePixmaps(GameMap::getInstance()->getTileset());
  GraphicsLoader::instantiatePixmaps(GameMap::getInstance()->getShieldset());

  Sound::getInstance()->haltMusic(false);
  Sound::getInstance()->enableBackground();

  game.reset(new Game(game_scenario, nextTurn));

  show_shield_turn();
  return true;
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
  static guint prev = 0;
  if (game.get())
    {
      guint delta = e->time - prev;
      if (delta > 40 || delta < 0)
	{
	  game->get_bigmap().mouse_motion_event(to_input_event(e));
	  sdl_widget->grab_focus();
	  prev = e->time;
	}
    }
  return true;
}
    
bool GameWindow::on_sdl_scroll_event(GdkEventScroll* event)
{
  switch (event->direction) 
    {
    case GDK_SCROLL_LEFT:
    case GDK_SCROLL_RIGHT:
      break;
    case GDK_SCROLL_UP:
      game->get_bigmap().zoom_in();
      break;
    case GDK_SCROLL_DOWN:
      game->get_bigmap().zoom_out();
      break;
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

  if (e->keyval == GDK_Shift_L || e->keyval == GDK_Shift_R)
    game->get_bigmap().set_shift_key_down (e->type == GDK_KEY_PRESS);
  if (e->keyval == GDK_Control_L || e->keyval == GDK_Control_R)
    game->get_bigmap().set_control_key_down (e->type == GDK_KEY_PRESS);

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
  chooser.hide();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      std::string filename = chooser.get_filename();
      current_save_filename = filename;
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
  chooser.hide();

  if (res == Gtk::RESPONSE_ACCEPT)
    {
      std::string filename = chooser.get_filename();

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
  std::auto_ptr<Gtk::Dialog> dialog;
  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/game-quit-dialog.ui");
  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));

  dialog->set_transient_for(*window.get());

  int response = dialog->run();
  dialog->hide();

  if (response == Gtk::RESPONSE_ACCEPT) //end the game
    {
      stop_game("quit");
    }
}

void GameWindow::on_game_stopped()
{
  game.reset();
  if (stop_action == "quit" || stop_action == "game-over")
    {
      game_ended.emit();
    }
  else if (stop_action == "load-game")
    {
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
	load_game(game_scenario, 
		  new NextTurnNetworked(game_scenario->getTurnmode(),
					game_scenario->s_random_turns));
    }
}

void GameWindow::on_quests_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Player *player = Playerlist::getActiveplayer();
  std::vector<Quest*> quests
    = QuestsManager::getInstance()->getPlayerQuests(player);
  if (quests.size() > 0)
    {
      QuestReportDialog d(quests[0]);
      d.set_parent_window(*window.get());
      d.run();
      d.hide();
      return;
    }
  else //no quest!
    {
      QuestReportDialog d(NULL);
      d.set_parent_window(*window.get());
      d.run();
      d.hide();
      return;
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
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/signpost-change-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

  decorator.set_title(_("Signpost"));

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
  e->set_activates_default(true);
  dialog->show_all();
  int response = dialog->run();
  dialog->hide();

  if (response == Gtk::RESPONSE_ACCEPT)
    Playerlist::getActiveplayer()->signpostChange(s, e->get_text());

  return;
}

  
void GameWindow::on_stack_info_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  StackInfoDialog d(currently_selected_stack);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
  //on_stack_info_changed(currently_selected_stack);
  //FIXME, armies don't stay selected in the right way.  to reproduce:
  //go in with all three selected.  deselect middle one in stackinfodialog,
  //then return.
}

void GameWindow::on_disband_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Stack *stack = Playerlist::getActiveplayer()->getActivestack();
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/disband-stack-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

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
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + 
				"/player-resign-completed-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

  dialog->show_all();
  dialog->run();
  dialog->hide();

  return;
}

void GameWindow::on_resign_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;

  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/player-resign-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

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

  return;
}

void GameWindow::on_vectoring_activated()
{
  City *city;
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;

  if (currently_selected_stack)
    {
      Vector<int> pos = currently_selected_stack->getPos();
      city = Citylist::getInstance()->getNearestVisibleFriendlyCity(pos);
    }
  else
    city = Citylist::getInstance()->getFirstCity(Playerlist::getActiveplayer());

  if (!city)
    return;
  bool see_all = true;
  DestinationDialog d(city, &see_all);

  d.set_parent_window(*window.get());
  d.run();
  d.hide();
  return;
}

void GameWindow::on_production_activated()
{
  City *city;
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;

  if (currently_selected_stack)
    {
      Vector<int> pos = currently_selected_stack->getPos();
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
  PreferencesDialog d(false);
  d.set_parent_window(*window.get());
  d.run(game.get());
  d.hide();
  if (current != Playerlist::getInstance()->getActiveplayer())
    game->end_turn();
}

void GameWindow::on_group_ungroup_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  group_ungroup_toggle->set_active(!group_ungroup_toggle->get_active());
}

void GameWindow::on_fight_order_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  FightOrderDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_levels_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  HeroLevelsDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window.get());
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
  if (currently_selected_stack)
    pos = currently_selected_stack->getPos();

  if (Templelist::getInstance()->size() == 0 &&
      Ruinlist::getInstance()->size() == 0)
    {
      std::string s = _("No ruins or temples to show!");
      TimedMessageDialog dialog(*window.get(), s, 30);

      dialog.show_all();
      dialog.run();
      dialog.hide();
      return;
    }
  RuinReportDialog d(pos);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_army_bonus_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ArmyBonusDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_item_bonus_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ItemBonusDialog d;
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_army_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::ARMY);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_city_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::CITY);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_gold_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::GOLD);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_production_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::PRODUCTION);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_winning_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  ReportDialog d(Playerlist::getActiveplayer(), ReportDialog::WINNING);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}
void GameWindow::on_city_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  HistoryReportDialog d(Playerlist::getActiveplayer(), 
			HistoryReportDialog::CITY);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_event_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  HistoryReportDialog d(Playerlist::getActiveplayer(),
			HistoryReportDialog::EVENTS);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_gold_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  HistoryReportDialog d(Playerlist::getActiveplayer(),
			HistoryReportDialog::GOLD);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_winner_history_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  HistoryReportDialog d(Playerlist::getActiveplayer(),
			HistoryReportDialog::WINNING);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_triumphs_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  TriumphsDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_help_about_activated()
{
  std::auto_ptr<Gtk::AboutDialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/about-dialog.ui");

  Gtk::AboutDialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  d->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

  dialog->set_version(PACKAGE_VERSION);
  SDL_Surface *logo = GraphicsLoader::getMiscPicture("castle_icon.png");
  dialog->set_logo(to_pixbuf(logo));
  dialog->show_all();
  dialog->run();
  dialog->hide();

  return;
}

void GameWindow::on_diplomacy_report_activated()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  DiplomacyReportDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_diplomacy_button_clicked()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  DiplomacyDialog d(Playerlist::getActiveplayer());
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::stop_game(std::string action)
{
  stop_action = action;
  Sound::getInstance()->disableBackground();
  if (game.get())
    {
      game->stopGame();
      current_save_filename = "";
    }
}

void GameWindow::on_game_over(Player *winner)
{

  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/game-over-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

  Gtk::Image *image;
  xml->get_widget("image", image);

  SDL_Surface *win = GraphicsLoader::getMiscPicture("win.png", false);

  image->property_pixbuf() = to_pixbuf(win);

  SDL_FreeSurface(win);

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s += String::ucompose(_("Congratulations to %1 for conquering the world!"), 
	winner->getName());
  label->set_text(s);

  dialog->show_all();
  dialog->run();
  dialog->hide();

  stop_game("game-over");
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

  TimedMessageDialog dialog(*window.get(), s, 30);

  dialog.show_all();
  dialog.run();
  dialog.hide();
}

void GameWindow::on_message_requested(std::string msg)
{
  // FIXME: this is a bit crude, maybe beef it up
  Gtk::MessageDialog dialog(*window.get(), msg);
  //TimedMessageDialog dialog(*window.get(), msg, 30, 5);
  dialog.show_all();
  dialog.run();
  dialog.hide();
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
  game->recalculate_moves_for_stack(s);
}

void GameWindow::on_group_toggled(Gtk::ToggleButton *toggle)
{
  if (toggle->sensitive() == false)
    return;
  bool active = toggle->get_active();
      
  clear_army_buttons();
  if (active)
    currently_selected_stack->group();
  else
    currently_selected_stack->ungroup();

  on_stack_info_changed(currently_selected_stack);
  game->recalculate_moves_for_stack(currently_selected_stack);
}

bool GameWindow::on_army_button_event(GdkEventButton *e,
				      Gtk::ToggleButton *toggle, Army *army)
{
  // if a hero is right-clicked, pop up the hero dialog, otherwise show the
  // army info tip
  MouseButtonEvent event = to_input_event(e);
  if (event.button == MouseButtonEvent::RIGHT_BUTTON
      && event.state == MouseButtonEvent::PRESSED) {

    army_info_tip.reset(new ArmyInfoTip(toggle, army));

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

void GameWindow::on_progress_status_changed(std::string string)
{
  progress_status_label->set_markup("<b>" + string + "</b>");
}

void GameWindow::on_progress_changed()
{
  turn_progressbar->pulse();
}

void GameWindow::on_sidebar_stats_changed(SidebarStats s)
{
  if (Configuration::s_decorated)
    {
      cities_stats_label->set_markup(String::ucompose("<b>%1</b>", s.cities));
      gold_stats_label->set_markup(String::ucompose("<b>%1</b>", s.gold));
      income_stats_label->set_markup(String::ucompose("<b>%1</b>", s.income));
      upkeep_stats_label->set_markup(String::ucompose("<b>%1</b>", s.upkeep));
      turn_label->set_markup(String::ucompose("<b>Turn %1</b>", s.turns));
    }
  else
    {
      cities_stats_label->set_markup(String::ucompose("%1", s.cities));
      gold_stats_label->set_markup(String::ucompose("%1", s.gold));
      income_stats_label->set_markup(String::ucompose("%1", s.income));
      upkeep_stats_label->set_markup(String::ucompose("%1", s.upkeep));
      turn_label->set_markup(String::ucompose("Turn %1", s.turns));
    }
  Glib::ustring tip;
  tip = String::ucompose(
			 ngettext("You have %1 city!",
				  "You have %1 cities!", s.cities), s.cities);
  cities_stats_image->set_tooltip_text(tip);
  cities_stats_label->set_tooltip_text(tip);
  tip = String::ucompose(
			 ngettext("You have %1 gold piece in your treasury!",
				  "You have %1 gold pieces in your treasury!", s.gold), s.gold);
  gold_stats_image->set_tooltip_text(tip);
  gold_stats_label->set_tooltip_text(tip);
  tip = String::ucompose(
			 ngettext("You earn %1 gold piece in income!",
				  "You earn %1 gold pieces in income!", s.income), s.income);
  income_stats_image->set_tooltip_text(tip);
  income_stats_label->set_tooltip_text(tip);
  tip = String::ucompose(
			 ngettext("You pay %1 gold piece in upkeep!",
				  "You pay %1 gold pieces in upkeep!", s.upkeep), s.upkeep);
  upkeep_stats_image->set_tooltip_text(tip);
  upkeep_stats_label->set_tooltip_text(tip);
}

void GameWindow::on_smallmap_changed(SDL_Surface *map)
{
  map_container->property_height_request() = map->h;
  map_container->property_width_request() = map->w;
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
    {
      if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
	show_stats();
      else
	show_progress();
    }
  else
    {
      if (s->getOwner()->getType() == Player::HUMAN)
	{
	  s->setDefending(false);
	  s->setParked(false);
	  show_stack(s);
	}
      else
	show_progress();
    }
  return;
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
  progress_box->hide();
  stats_box->show();
}

void GameWindow::show_progress()
{
  turn_progressbar->property_fraction() = 0.0;
  if (Playerlist::getActiveplayer() == Playerlist::getInstance()->getNeutral())
    progress_status_label->set_text("");
  else
    progress_status_label->set_markup("<b>" + Playerlist::getActiveplayer()->getName() + "</b>");
  stats_box->hide();
  stack_info_container->hide();
  progress_box->show();
}

void GameWindow::fill_in_group_info (Stack *s)
{
  guint32 bonus = s->calculateMoveBonus();
  GraphicsCache *gc = GraphicsCache::getInstance();
  SDL_Surface *terrain = gc->getMoveBonusPic(bonus, s->hasShip());
  terrain_image->property_pixbuf() = to_pixbuf(terrain);
  if (Configuration::s_decorated == true)
    group_moves_label->set_markup(String::ucompose("<b>%1</b>",
						   s->getGroupMoves()));
  else
    group_moves_label->set_markup(String::ucompose("%1",
						   s->getGroupMoves()));
  //printf ("toggling group/ungroup!\n");
  group_ungroup_toggle->set_sensitive(false);
  group_ungroup_toggle->set_active(s->isGrouped());
  if (group_ungroup_toggle->get_active() == true)
    group_ungroup_toggle->set_label(_("UnGrp"));
  else
    group_ungroup_toggle->set_label(_("Grp"));
  group_ungroup_toggle->set_sensitive(true);
}

void GameWindow::show_stack(Stack *s)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  s->sortForViewing (true);
  stats_box->hide();
  progress_box->hide();

  army_buttons.clear(); 
  for (Stack::iterator i = s->begin(), end = s->end(); i != end; ++i)
    {
      // construct a toggle button
      Army *army = *i;
      Gtk::VBox *toggle_box = manage(new Gtk::VBox);

      // image
      toggle_box->add(*manage(new Gtk::Image(to_pixbuf(gc->getArmyPic(army)))));
      // number of moves
      Glib::ustring moves_str = String::ucompose("%1", army->getMoves());
      toggle_box->add(*manage(new Gtk::Label(moves_str,
					     Gtk::ALIGN_CENTER, Gtk::ALIGN_TOP)));

      // the button itself
      Gtk::ToggleButton *toggle = new Gtk::ToggleButton;
      toggle->add(*toggle_box);
      toggle->set_active(army->isGrouped());
      toggle->signal_toggled().connect
	(sigc::bind(sigc::mem_fun(*this, &GameWindow::on_army_toggled),
		    toggle, army));
      toggle->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
      toggle->signal_button_press_event().connect
	(sigc::bind(sigc::mem_fun(*this, &GameWindow::on_army_button_event),
		    toggle, army), false);
      toggle->signal_button_release_event().connect
	(sigc::bind(sigc::mem_fun(*this, &GameWindow::on_army_button_event),
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
  Decorated decorator;
  decorator.decorate(map_tip.get(),File::getMiscFile("various/background.png"), 200);
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
  SageDialog d(stack->getFirstHero()->getOwner(), 
	       static_cast<Hero*>(stack->getFirstHero()), ruin);
  d.set_parent_window(*window.get());
  Reward *reward = d.run();
  d.hide();
  ruin->setReward(reward);
}

void GameWindow::on_ruin_rewarded (Reward_Ruin *reward)
{
  RuinRewardedDialog d(reward);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_ruin_searched(Ruin *ruin, Stack *stack, Reward *reward)
{
  std::auto_ptr<Gtk::Dialog> dialog;
  if (ruin->hasSage())
    {
      if (reward->getType() == Reward::RUIN)
	return on_ruin_rewarded(static_cast<Reward_Ruin*>(reward));
    }

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/ruin-searched-dialog.ui");


  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

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
}

void GameWindow::on_ruinfight_started(Stack *attackers, Stack *defenders)
{
  //so and so encounters a wolf...
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/ruinfight-started-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

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
}
void GameWindow::on_ruinfight_finished(Fight::Result result)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/ruinfight-finished-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

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
}

void GameWindow::on_fight_started(Fight &fight)
{
  FightWindow d(fight);

  d.set_parent_window(*window.get());
  d.run(&d_quick_fights);
  d.hide();
  if (Playerlist::getActiveplayer()->getType() == Player::HUMAN)
    d_quick_fights = false;
}

void GameWindow::on_hero_brings_allies (int numAllies)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/hero-brings-allies-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

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
}

bool GameWindow::on_hero_offers_service(Player *player, HeroProto *hero, City *city, int gold)
{
  HeroOfferDialog d(player, hero, city, gold);
  d.set_parent_window(*window.get());
  bool retval = d.run();
  d.hide();
  return retval;
}

bool GameWindow::on_enemy_offers_surrender(int numPlayers)
{
  SurrenderDialog d(numPlayers);
  d.set_parent_window(*window.get());
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
      d.set_parent_window(*window.get());
      d.run();
      d.hide();
    }
}

bool GameWindow::on_stack_considers_treachery (Player *me, Stack *stack, 
					       Player *them, Vector<int> pos)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/treachery-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());
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
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

bool GameWindow::on_temple_searched(bool hasHero, Temple *temple, int blessCount)
{
  QuestsManager *qm = QuestsManager::getInstance();
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/temple-visit-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

  decorator.set_title(temple->getName());

  Gtk::Label *l;
  Gtk::Button *close_button;
  Gtk::Button *accept_button;
  xml->get_widget("label", l);
  xml->get_widget("close_button", close_button);
  xml->get_widget("accept_button", accept_button);

  if (qm->getPlayerQuests(Playerlist::getActiveplayer()).size() > 0 &&
      hasHero)
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
  dialog->hide();

  if (hasHero == false || GameScenario::s_play_with_quests == false)
    response = Gtk::RESPONSE_ACCEPT;

  if (response == Gtk::RESPONSE_ACCEPT)		// accepted a quest
    return true;
  else
    return false;
}

void GameWindow::on_quest_assigned(Hero *hero, Quest *quest)
{
  QuestAssignedDialog d(hero, quest);
  d.set_parent_window(*window.get());
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
  Gtk::Button *raze_button;
  std::auto_ptr<Gtk::Dialog> dialog;
  if (gold)
    on_city_looted (city, gold);
  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/city-defeated-dialog.ui");


  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

  Gtk::Image *image;
  xml->get_widget("city_image", image);
  image->property_file() = File::getMiscFile("various/city_occupied.png");
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

  xml->get_widget("raze_button", raze_button);

  raze_button->set_sensitive
    (GameScenarioOptions::s_razing_cities == GameParameters::ON_CAPTURE || 
     GameScenarioOptions::s_razing_cities == GameParameters::ALWAYS);

  if (h) /* if there was a hero in the stack */
    {
      bool pillage, sack, raze, occupy;
      if (hero_has_quest_here (p->getStacklist()->getActivestack(), city, 
			       &pillage, &sack, &raze, &occupy))
	{
	  Gtk::Button *button;
	  if (pillage)
	    {
	      xml->get_widget("pillage_button", button);
	      button->grab_default();
	    }
	  if (sack)
	    {
	      xml->get_widget("sack_button", button);
	      button->grab_default();
	    }
	  if (raze)
	    {
	      xml->get_widget("raze_button", button);
	      button->grab_default();
	    }
	  if (occupy)
	    {
	      xml->get_widget("occupy_button", button);
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
	default:
	case 1: return CITY_DEFEATED_OCCUPY;
	case 2: 
		{
		  bool razed = CityWindow::on_raze_clicked(city, dialog.get());
		  if (razed == false)
		    continue;
		  return CITY_DEFEATED_RAZE;
		}
	case 3: return CITY_DEFEATED_PILLAGE;
	case 4: return CITY_DEFEATED_SACK;
	}
    }
  dialog->hide();
}

void GameWindow::on_city_looted (City *city, int gold)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/city-looted-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

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
}
void GameWindow::on_city_pillaged(City *city, int gold, int pillaged_army_type)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  std::auto_ptr<Gtk::Dialog> dialog;
  Player *player = city->getOwner();
  unsigned int as = player->getArmyset();

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/city-pillaged-dialog.ui");

  Gtk::Dialog *d;
  Gtk::Image *pillaged_army_type_image;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

  decorator.set_title(String::ucompose(_("Pillaged %1"), city->getName()));

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
  dialog->hide();
}

void GameWindow::on_city_sacked(City *city, int gold, std::list<guint32> sacked_types)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Player *player = city->getOwner();
  unsigned int as = player->getArmyset();
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/city-sacked-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

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
  SDL_Surface *surf
    = GraphicsCache::getInstance()->getArmyPic(as, 0, player, NULL);
  Glib::RefPtr<Gdk::Pixbuf> empty_pic
    = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, surf->w, surf->h);
  empty_pic->fill(0x00000000);
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
      pic = to_pixbuf(gc->getArmyPic(as, *it, player, NULL));
      sack_image->set(pic);
      const ArmyProto *a = 
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
  dialog->hide();
}

void GameWindow::on_city_razed (City *city)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/city-razed-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

  decorator.set_title(String::ucompose(_("Razed %1"), city->getName()));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s = String::ucompose(_("The city of %1 is in ruins!"), city->getName());
  label->set_text(s);

  dialog->show_all();
  dialog->run();
  dialog->hide();
}

void GameWindow::on_city_visited(City *city)
{
  CityWindow d(city, 
	       GameScenarioOptions::s_razing_cities == GameParameters::ALWAYS,
	       GameScenarioOptions::s_see_opponents_production);

  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_ruin_visited(Ruin *ruin)
{
  RuinReportDialog d(ruin->getPos());
  d.set_parent_window(*window.get());
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
	shield_image[c]->property_pixbuf()=to_pixbuf(gc->getShieldPic(1,(*i)));
      else
	shield_image[c]->property_pixbuf()=to_pixbuf(gc->getShieldPic(0,(*i)));
      shield_image[c]->property_tooltip_text() = (*i)->getName();
      c++;
    }
  for (unsigned int i = c; i < MAX_PLAYERS; i++)
    shield_image[i]->clear();
}

void GameWindow::on_remote_next_player_turn()
{
  std::auto_ptr<Gtk::Dialog> dialog;

  on_stack_info_changed(NULL);
  while (g_main_context_iteration(NULL, FALSE)); //doEvents

  d_quick_fights = false;
  show_shield_turn();
  turn_label->set_markup(String::ucompose("Turn %1", 
					  GameScenarioOptions::s_round));
}

void GameWindow::on_next_player_turn(Player *player, unsigned int turn_number)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  on_stack_info_changed(NULL);

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

      Gtk::Dialog *d;
      xml->get_widget("dialog", d);
      dialog.reset(d);
      dialog->set_transient_for(*window.get());

      Decorated decorator;
      decorator.decorate(dialog.get());
      decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
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
      show();
    }

}

void GameWindow::on_medal_awarded_to_army(Army *army)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/medal-awarded-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

  Gtk::Image *image;
  xml->get_widget("image", image);
  image->property_pixbuf() = to_pixbuf(gc->getArmyPic(army));

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s += String::ucompose(_("%1 is awarded a medal!"), army->getName());
  s += "\n\n";
  s += String::ucompose(_("Experience: %1"), std::setprecision(3), army->getXP());
  s += "\n";
  s += String::ucompose(_("Level: %1"), army->getLevel());
  label->set_text(s);

  dialog->show_all();
  dialog->run();
  dialog->hide();
}

Army::Stat GameWindow::on_army_gains_level(Army *army)
{
  ArmyGainsLevelDialog d(army, GameScenario::s_hidden_map);

  d.set_parent_window(*window.get());
  d.run();
  d.hide();

  return d.get_selected_stat();
}

void GameWindow::on_game_loaded(Player *player)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/game-loaded-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

  Gtk::Label *label;
  xml->get_widget("label", label);
  Glib::ustring s;
  s += String::ucompose(_("%1, your turn continues."), player->getName());
  label->set_text(s);

  dialog->show_all();
  dialog->run();
  dialog->hide();
}

void GameWindow::on_quest_completed(Quest *quest, Reward *reward)
{
  QuestCompletedDialog d(quest, reward);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}

void GameWindow::on_quest_expired(Quest *quest)
{
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/quest-expired-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
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
  dialog->hide();
}

void GameWindow::on_inspect_activated ()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  if (Playerlist::getActiveplayer()->getStacklist()->getHeroes().size() == 0)
    return;
  Hero *hero = NULL;
  Vector<int> pos = Vector<int>(-1,-1);
  if (currently_selected_stack != NULL)
    {
      hero = dynamic_cast<Hero*>(currently_selected_stack->getFirstHero());
      pos = currently_selected_stack->getPos();
    }
    
  HeroDialog d(hero, pos);
  d.set_parent_window(*window.get());
  d.run();
  d.hide();
}
void GameWindow::on_plant_standard_activated ()
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  Playerlist::getActiveplayer()->heroPlantStandard(NULL);
}
    
void GameWindow::on_advice_asked(float percent)
{
  if (Playerlist::getActiveplayer()->getType() != Player::HUMAN)
    return;
  //we asked for advice on a fight, and we're being told that we 
  //have a PERCENT chance of winning the fight
  std::auto_ptr<Gtk::Dialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/military-advisor-dialog.ui");

  Gtk::Dialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  Decorated decorator;
  decorator.decorate(dialog.get());
  decorator.window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));
  dialog->set_transient_for(*window.get());

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
	s += _("This battle will surely be a comfortable victory");
      else if (num == 1)
	s += _("A battle here would be a comfortable victory");
      else if (num == 2)
	s += _("I believe this battle will surely be a comfortable victory");
      else if (num == 3)
	s += _("This battle would be a comfortable victory");
      else if (num == 4)
	s += _("A battle here would be a comfortable victory");
      else if (num == 5)
	s += _("I believe this battle will be a comfortable victory");
      else if (num == 6)
	s += _("This battle shall be a comfortable victory");
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

