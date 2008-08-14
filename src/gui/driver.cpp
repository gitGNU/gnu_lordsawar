//  Copyright (C) 2007, 2008, Ole Laursen
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

#include <assert.h>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/bind.h>
#include <errno.h>

#include "driver.h"

#include <iostream>
#include "error-utils.h"
#include "main.h"
#include "splash-window.h"
#include "game-window.h"
#include "game-lobby-dialog.h"
#include "defs.h"
#include "GraphicsCache.h"
#include "GameScenario.h"
#include "CreateScenario.h"
#include "counter.h"
#include "shieldsetlist.h"
#include "File.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "xmlhelper.h"
#include "Configuration.h"
#include "ucompose.hpp"
#include "sound.h"
#include "timed-message-dialog.h"
#include "new-game-progress-window.h"
#include "game-preferences-dialog.h"

#include "game-client.h"
#include "pbm-game-client.h"
#include "game-server.h"
#include "pbm-game-server.h"
#include "NextTurnHotseat.h"
#include "NextTurnPbm.h"
#include "NextTurnNetworked.h"
#include "pbm/pbm.h"

Driver::Driver(std::string load_filename)
{
  game_scenario_downloaded = "";
    splash_window.reset(new SplashWindow);
    splash_window->new_game_requested.connect(
	sigc::mem_fun(*this, &Driver::on_new_game_requested));
    splash_window->new_hosted_network_game_requested.connect(
	sigc::mem_fun(*this, &Driver::on_new_hosted_network_game_requested));
    splash_window->new_remote_network_game_requested.connect(
	sigc::mem_fun(*this, &Driver::on_new_remote_network_game_requested));
    splash_window->new_pbm_game_requested.connect(
	sigc::mem_fun(*this, &Driver::on_new_pbm_game_requested));
    splash_window->load_requested.connect(
	sigc::mem_fun(*this, &Driver::on_load_requested));
    splash_window->quit_requested.connect(
	sigc::mem_fun(*this, &Driver::on_quit_requested));

    d_load_filename = load_filename;
    if (Main::instance().start_stress_test) 
      {
	Sound::deleteInstance();
	stress_test();
	exit(0);
      }
    splash_window->sdl_initialized.connect(sigc::mem_fun(*this, &Driver::run));
    splash_window->show();
}

void Driver::run()
{
  if (Main::instance().start_test_scenario) 
    {
      // quick load a test scenario
      GameParameters g;
      GameParameters::Player p;
      p.type = GameParameters::Player::HUMAN;
      p.id = 0;
      p.name = "Mr. Test";
      g.players.push_back(p);
      p.type = GameParameters::Player::EASY;
      p.id = 1;
      p.name = "Evil";
      g.players.push_back(p);
      for (int i = 2; i < MAX_PLAYERS; i++)
	{
	  p.type = GameParameters::Player::OFF;
	  p.id = i;
	  p.name = "";
	  g.players.push_back(p);
	}
      g.map.width = 125;
      g.map.height = 125;
      g.map.grass = 78;
      g.map.water = 7;
      g.map.swamp = 2;
      g.map.forest = 3;
      g.map.hills = 5;
      g.map.mountains = 5;
      g.map.cities = 20;
      g.map.ruins = 25;
      g.map_path = "";
      g.play_with_quests = false;
      g.hidden_map = false;
      g.neutral_cities = GameParameters::STRONG;
      g.razing_cities = GameParameters::ALWAYS;
      g.diplomacy = false;
      g.random_turns = false;
      g.quick_start = false;
      g.intense_combat = false;
      g.military_advisor = false;
      g.army_theme = "Default";
      g.tile_theme = "default";
      g.shield_theme = "default";
      g.city_theme = "default";
      g.process_armies = GameParameters::PROCESS_ARMIES_AT_PLAYERS_TURN;
      g.difficulty = GameScenario::calculate_difficulty_rating(g);
      g.cities_can_produce_allies = false;
      g.cusp_of_war = false;
      g.see_opponents_stacks = true;
      g.see_opponents_production = true;
      on_new_game_requested(g);
    }
  else if (Main::instance().turn_filename != "") 
    {
      PbmGameClient *pbm_game_client = PbmGameClient::getInstance();
      GameScenario *game_scenario = load_game(d_load_filename);
      if (game_scenario == NULL)
	return;
      //now apply the actions in the turn file
      bool broken;

      //if the active player isn't a network player than don't do anything
      if (Playerlist::getActiveplayer()->getType() != Player::NETWORKED)
	return;
      //load the file, and decode them as we go.
      XML_Helper helper(Main::instance().turn_filename, std::ios::in, 
			Configuration::s_zipfiles);
      NextTurnPbm *nextTurn;
      nextTurn = new NextTurnPbm(game_scenario->getTurnmode(),
			      game_scenario->s_random_turns);
      broken = pbm_game_client->loadWithHelper (helper, 
						Playerlist::getActiveplayer());
      helper.close();
      if (!broken)
	{
	  game_scenario->saveGame(d_load_filename);
	  if (Playerlist::getActiveplayer()->getType() != Player::NETWORKED)
	    {
	      if (splash_window.get())
		splash_window->hide();

	      init_game_window();
	      game_window->show();
	      game_window->load_game(game_scenario, nextTurn);
	    }
	}
    }
  else
    {
      splash_window->show();
      if (d_load_filename.empty() == false)
	{
	  GameParameters g;
	  g.map_path = d_load_filename;
	  size_t found = d_load_filename.find(".map");
	  if (found != std::string::npos)
	    {
	      GamePreferencesDialog d(d_load_filename);
	      d.set_parent_window(*splash_window->get_window());
	      d.game_started.connect(sigc::mem_fun
				     (*this, &Driver::on_new_game_requested));
	      d.run();
	    }
	  else
	    {
	      found = d_load_filename.find(".sav");
	      if (found != std::string::npos)
		on_load_requested(d_load_filename);
	      else
		on_new_game_requested(g);
	    }
	}
    }
  return;
}

Driver::~Driver()
{
}

void Driver::on_hosted_player_sat_down(Player *player)
{
  Uint32 id = player->getId();
  GameServer *game_server = GameServer::getInstance();
  game_server->sit_down(player);
  player_replaced.emit(Playerlist::getInstance()->getPlayer(id));
}

void Driver::on_hosted_player_stood_up(Player *player)
{
  Uint32 id = player->getId();
  GameServer *game_server = GameServer::getInstance();
  game_server->stand_up(player);
  player_replaced.emit(Playerlist::getInstance()->getPlayer(id));
}

void Driver::on_hosted_player_chat(std::string message)
{
  GameServer *game_server = GameServer::getInstance();
  game_server->chat(message);
}

void Driver::on_client_player_sat_down(Player *player)
{
  GameClient *game_client = GameClient::getInstance();
  game_client->sit_down(player);
}

void Driver::on_client_player_stood_up(Player *player)
{
  GameClient *game_client = GameClient::getInstance();
  game_client->stand_up(player);
}

void Driver::on_client_player_chat(std::string message)
{
  GameClient *game_client = GameClient::getInstance();
  game_client->chat(message);
}

void Driver::on_new_hosted_network_game_requested(GameParameters g, int port,
						  std::string nick)
{
    if (splash_window.get())
	splash_window->hide();

    NewGameProgressWindow pw(g, GameScenario::NETWORKED);
    Gtk::Main::instance()->run(pw);
    GameScenario *game_scenario = pw.getGameScenario();

    if (game_scenario == NULL)
      {
	TimedMessageDialog dialog(*splash_window->get_window(),
				  _("Corrupted saved game file."), 0);
	dialog.run();
	dialog.hide();
	return;
      }

  GameServer *game_server = GameServer::getInstance();
  game_server->start(game_scenario, port, nick);
  NextTurnNetworked *next_turn = new NextTurnNetworked(game_scenario->getTurnmode(), game_scenario->s_random_turns);
  if (game_scenario->s_random_turns == true)
    next_turn->snextRound.connect (sigc::mem_fun(GameServer::getInstance(), 
						 &GameServer::sendTurnOrder));
  next_turn->snextPlayerUnavailable.connect(sigc::mem_fun(this, &Driver::on_player_unavailable));
  game_lobby_dialog.reset(new GameLobbyDialog(game_scenario, next_turn, 
					      game_server, true));
  game_lobby_dialog->set_parent_window(*splash_window.get()->get_window());
  game_lobby_dialog->player_sat_down.connect
    (sigc::mem_fun(this, &Driver::on_hosted_player_sat_down));
  game_lobby_dialog->player_stood_up.connect
    (sigc::mem_fun(this, &Driver::on_hosted_player_stood_up));
  game_lobby_dialog->message_sent.connect
    (sigc::mem_fun(this, &Driver::on_hosted_player_chat));
  game_lobby_dialog->start_network_game.connect
    (sigc::mem_fun(this, &Driver::start_network_game_requested));
  game_lobby_dialog->show();
  bool response = game_lobby_dialog->run();
  game_lobby_dialog->hide();
    
  if (response == false)
    {
      GameServer::deleteInstance();
      delete game_scenario;
      if (splash_window.get())
	splash_window->show();
    }
}

  
void Driver::on_server_went_away()
{
  heartbeat_conn.disconnect();
  if (game_lobby_dialog.get())
    game_lobby_dialog->hide();
  if (download_window.get())
    download_window->hide();
  if (splash_window.get())
    splash_window->show();
  GameClient::deleteInstance();
  TimedMessageDialog dialog(*splash_window->get_window(), 
			    _("Server went away."), 0);
  dialog.run();
  dialog.hide();
}

void Driver::on_client_could_not_connect()
{
  heartbeat_conn.disconnect();
  if (game_lobby_dialog.get())
    game_lobby_dialog->hide();
  if (download_window.get())
    download_window->hide();
  if (splash_window.get())
    splash_window->show();
  GameClient::deleteInstance();
  TimedMessageDialog dialog(*splash_window->get_window(), 
			    _("Could not connect."), 0);
  dialog.run();
  dialog.hide();
}

void Driver::on_new_remote_network_game_requested(std::string host, unsigned short port, std::string nick)
{
  if (splash_window.get())
    splash_window->hide();
  GameClient *game_client = GameClient::getInstance();
  game_client->game_scenario_received.connect
    (sigc::mem_fun(this, &Driver::on_game_scenario_downloaded));
  game_client->client_disconnected.connect
    (sigc::mem_fun(this, &Driver::on_server_went_away));
  game_client->client_could_not_connect.connect
    (sigc::mem_fun(this, &Driver::on_client_could_not_connect));
  game_scenario_received.connect
    (sigc::mem_fun(this, &Driver::on_game_scenario_received));
  download_window.reset(new NewNetworkGameDownloadWindow());
  download_window->pulse();
  game_client->start(host, port, nick);
  heartbeat_conn = Glib::signal_timeout().connect
    (bind_return(sigc::mem_fun(*this, &Driver::heartbeat), true), 1 * 1000);

}

void Driver::heartbeat()
{
  static bool already_done = false;
  if (already_done)
    return;
  if (game_scenario_downloaded == "")
    {
      download_window->pulse();
      return;
    }
  
  game_scenario_received.emit(game_scenario_downloaded);
  already_done = true;
}

void Driver::on_game_scenario_received(std::string path)
{
  heartbeat_conn.disconnect();
  if (download_window.get())
    download_window->hide();
  GameScenario *game_scenario = load_game(path);
  NextTurnNetworked *next_turn = new NextTurnNetworked(game_scenario->getTurnmode(), game_scenario->s_random_turns);
  next_turn->snextPlayerUnavailable.connect(sigc::mem_fun(this, &Driver::on_player_unavailable));
  game_lobby_dialog.reset(new GameLobbyDialog(game_scenario, next_turn, 
					      GameClient::getInstance(), false));
  game_lobby_dialog->set_parent_window(*splash_window.get()->get_window());
  game_lobby_dialog->player_sat_down.connect
    (sigc::mem_fun(this, &Driver::on_client_player_sat_down));
  game_lobby_dialog->player_stood_up.connect
    (sigc::mem_fun(this, &Driver::on_client_player_stood_up));
  game_lobby_dialog->message_sent.connect
    (sigc::mem_fun(this, &Driver::on_client_player_chat));
  game_lobby_dialog->start_network_game.connect
    (sigc::mem_fun(this, &Driver::start_network_game_requested));
  game_lobby_dialog->show();
  bool response = game_lobby_dialog->run();
  game_lobby_dialog->hide();

  if (response == false)
    {
      if (splash_window.get())
	splash_window->show();
    
      GameClient::deleteInstance();
      delete game_scenario;
    }
}
void Driver::on_game_scenario_downloaded(std::string path)
{
  game_scenario_downloaded = path;
  //emitting the signal doesn't work.
  //it stops the game client from doing more processing.
  //how can i bring up the game lobby dialog with this scenario?
  //...without stopping the game client from getting more messages
}

void Driver::on_new_game_requested(GameParameters g)
{
    if (splash_window.get())
	splash_window->hide();

    NewGameProgressWindow pw(g, GameScenario::HOTSEAT);
    Gtk::Main::instance()->run(pw);
    GameScenario *game_scenario = pw.getGameScenario();

    if (game_scenario == NULL)
      {
	TimedMessageDialog dialog(*splash_window->get_window(),
				  _("Corrupted saved game file."), 0);
	dialog.run();
	dialog.hide();
	return;
      }

    NextTurn *next_turn = new NextTurnHotseat(game_scenario->getTurnmode(),
					      game_scenario->s_random_turns);
    init_game_window();
    
    game_window->show();
    game_window->new_game(game_scenario, next_turn);
}

void Driver::on_load_requested(std::string filename)
{
    if (splash_window.get())
	splash_window->hide();

    GameScenario *game_scenario = load_game(filename);
    if (game_scenario == NULL)
      return;

    init_game_window();
    
    game_window->show();
    if (game_scenario->getPlayMode() == GameScenario::HOTSEAT)
      game_window->load_game
	(game_scenario, new NextTurnHotseat(game_scenario->getTurnmode(),
					    game_scenario->s_random_turns));
    else if (game_scenario->getPlayMode() == GameScenario::PLAY_BY_MAIL)
      {
	PbmGameServer::getInstance()->start();
	game_window->load_game
	  (game_scenario, new NextTurnPbm(game_scenario->getTurnmode(),
					  game_scenario->s_random_turns));
      }
    else if (game_scenario->getPlayMode() == GameScenario::NETWORKED)
      game_window->load_game
	(game_scenario, new NextTurnNetworked(game_scenario->getTurnmode(),
					      game_scenario->s_random_turns));
}

void Driver::on_quit_requested()
{
    if (splash_window.get())
	splash_window->hide();
    
    if (game_window.get())
	game_window->hide();

    Main::instance().stop_main_loop();
}

void Driver::on_game_ended()
{
  if (game_window.get())
    {
      game_window->hide();
      game_window.reset();
    }
  GameClient::deleteInstance();
  PbmGameClient::deleteInstance();
  GameServer::deleteInstance();
  PbmGameServer::deleteInstance();

  GraphicsCache::deleteInstance();

  splash_window->show();
}

void Driver::init_game_window()
{
    game_window.reset(new GameWindow);

    game_window->game_ended.connect(
	sigc::mem_fun(*this, &Driver::on_game_ended));
    game_window->show_lobby.connect(
	sigc::mem_fun(*this, &Driver::on_show_lobby_requested));
    game_window->quit_requested.connect(
	sigc::mem_fun(*this, &Driver::on_quit_requested));

    game_window->init(640, 480);

}

std::string
Driver::create_and_dump_scenario(const std::string &file, const GameParameters &g)
{
    CreateScenario creator (g.map.width, g.map.height);

    // then fill the other players
    int c = 0;
    int army_id = Armysetlist::getInstance()->getArmysetId(g.army_theme);
    Shieldsetlist *ssl = Shieldsetlist::getInstance();
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

	creator.addPlayer(i->name, army_id, ssl->getColor(g.shield_theme, 
							  c), type);
    }

    // the neutral player must come last so it has the highest id among players
    creator.addNeutral(_("Neutral"), army_id, 
		       ssl->getColor(g.shield_theme, MAX_PLAYERS),
		       Player::AI_DUMMY);

    // now fill in some map information
    creator.setMapTiles(g.tile_theme);
    creator.setShieldset(g.shield_theme);
    creator.setCityset(g.city_theme);
    creator.setNoCities(g.map.cities);
    creator.setNoRuins(g.map.ruins);
    creator.setNoTemples(4);

    // terrain: the scenario generator also accepts input with a sum of
    // more than 100%, so the thing is rather easy here
    creator.setPercentages(g.map.grass, g.map.water, g.map.forest, g.map.swamp,
			   g.map.hills, g.map.mountains);

    int area = g.map.width * g.map.height;
    creator.setNoSignposts(int(area * (g.map.grass / 100.0) * 0.0030));

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


GameScenario *Driver::load_game(std::string file_path)
{
    bool broken = false;
    GameScenario* game_scenario = new GameScenario(file_path, broken);

    if (broken)
      {
	TimedMessageDialog dialog(*splash_window->get_window(),
				  _("Corrupted saved game file."), 0);
	dialog.run();
	dialog.hide();
	return NULL;
      }
    return game_scenario;
}

void Driver::on_new_pbm_game_requested(GameParameters g)
{
  std::string filename;
  std::string temp_filename = File::getSavePath() + "pbmtmp.sav";
      
  NewGameProgressWindow pw(g, GameScenario::PLAY_BY_MAIL);
  Gtk::Main::instance()->run(pw);
  GameScenario *game_scenario = pw.getGameScenario();
  if (game_scenario == NULL)
    {
      TimedMessageDialog dialog(*splash_window->get_window(),
				_("Corrupted saved game file."), 0);
	dialog.run();
	dialog.hide();
      return;
    }
  game_scenario->saveGame(temp_filename);
  std::string player_name = Playerlist::getActiveplayer()->getName();
  delete game_scenario;
  pbm play_by_mail;
  play_by_mail.init(temp_filename);

  Gtk::FileChooserDialog chooser(*splash_window->get_window(), _("Save the scenario and mail it to the first player"),
				 Gtk::FILE_CHOOSER_ACTION_SAVE);
  Gtk::FileFilter sav_filter;
  sav_filter.add_pattern("*.sav");
  chooser.set_filter(sav_filter);
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

      remove (filename.c_str());
      if (rename(temp_filename.c_str(), filename.c_str()))
	{
	  char* err = strerror(errno);
	  std::cerr <<_("Error while trying to rename the temporary file to ")
	    << filename << "\n";
	  std::cerr <<_("Error: ") <<err <<std::endl;
	}
    }
  Glib::ustring s = String::ucompose(_("Now send the saved-game file to %1"),
				     player_name);
  TimedMessageDialog dialog(*splash_window->get_window(), s, 0);
  dialog.run();
  dialog.hide();
  return;
}

    
void Driver::stressTestNextRound()
{
  static int count = 1;
  count++;
  printf ("starting round %d!\n", count);
  sleep (1);
}

void Driver::stress_test()
{

  // quick load a test scenario
  GameParameters g;
  GameParameters::Player p;
  for (int i = 0; i < MAX_PLAYERS; i++)
    {
      p.type = GameParameters::Player::EASY;
      p.id = i;
      switch (p.id)
	{
	case 0: p.name = "one"; break;
	case 1: p.name = "two"; break;
	case 2: p.name = "three"; break;
	case 3: p.name = "four"; break;
	case 4: p.name = "five"; break;
	case 5: p.name = "six"; break;
	case 6: p.name = "seven"; break;
	case 7: p.name = "eight"; break;
	}
      g.players.push_back(p);
    }
  g.map.width = MAP_SIZE_NORMAL_WIDTH;
  g.map.height = MAP_SIZE_NORMAL_HEIGHT;
  g.map.grass = 78;
  g.map.water = 7;
  g.map.swamp = 2;
  g.map.forest = 3;
  g.map.hills = 5;
  g.map.mountains = 5;
  g.map.cities = 80;
  g.map.ruins = 15;
  g.map_path = "";
  g.play_with_quests = false;
  g.hidden_map = false;
  g.neutral_cities = GameParameters::STRONG;
  g.razing_cities = GameParameters::ALWAYS;
  g.diplomacy = false;
  g.random_turns = false;
  g.quick_start = false;
  g.intense_combat = false;
  g.military_advisor = false;
  g.army_theme = "Default";
  g.tile_theme = "default";
  g.shield_theme = "default";
  g.city_theme = "default";
  g.process_armies = GameParameters::PROCESS_ARMIES_AT_PLAYERS_TURN;
  g.difficulty = GameScenario::calculate_difficulty_rating(g);
  g.cities_can_produce_allies = false;
  g.cusp_of_war = false;
  g.see_opponents_stacks = true;
  g.see_opponents_production = true;
      
  std::string path = create_and_dump_scenario("random.map", g);
  g.map_path = path;

  bool broken = false;
  GameScenario* game_scenario = new GameScenario(g.map_path, broken);

  if (broken)
    return;

  NextTurnHotseat *nextTurn;
  nextTurn = new NextTurnHotseat(game_scenario->getTurnmode(),
				 game_scenario->s_random_turns);
    
  nextTurn->snextRound.connect
    (sigc::mem_fun(this, &Driver::stressTestNextRound));
  nextTurn->snextRound.connect
    (sigc::mem_fun(game_scenario, &GameScenario::nextRound));
  if (game_scenario->getRound() == 0)
    {
      Playerlist::getInstance()->syncPlayers(g.players);
      game_scenario->setupFog(g.hidden_map);
      game_scenario->setupCities(g.quick_start);
      game_scenario->setupDiplomacy(g.diplomacy);
      game_scenario->nextRound();
    }

  nextTurn->start();
  delete nextTurn;
  delete game_scenario;

}
	
void Driver::on_show_lobby_requested()
{
  if (game_lobby_dialog.get())
    game_lobby_dialog->show();
}
    
void Driver::start_network_game_requested(GameScenario *game_scenario, NextTurnNetworked *next_turn)
{
  if (game_window.get())
    {
      Player *active = Playerlist::getActiveplayer();
      if (active->getType() == Player::NETWORKED)
	game_window->show();
      else
	{
	  if (active->hasAlreadyInitializedTurn())
	    game_window->show();
	  else
	    game_window->continue_network_game (next_turn);
	}
    }
  else
    {
      init_game_window();
      player_replaced.connect
	(sigc::mem_fun(game_window.get(), &GameWindow::on_player_replaced));
      game_window->show();
      game_window->new_network_game (game_scenario, next_turn);
    }
}
  
void Driver::on_player_unavailable(Player *p)
{
  game_lobby_dialog->player_is_unavailable(p);
  on_show_lobby_requested();
}
