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

#include "driver.h"

#include "error-utils.h"
#include "main.h"
#include "splash-window.h"
#include "game-window.h"
#include "../defs.h"
#include "../GraphicsCache.h"
#include "../GameScenario.h"
#include "game-lobby-dialog.h"
#include "../CreateScenario.h"
#include "../counter.h"
#include "../shieldsetlist.h"
#include "../File.h"
#include "../armysetlist.h"
#include "../playerlist.h"
#include "timed-message-dialog.h"
#include "new-game-progress-window.h"

#include "../game-client.h"

static GameClient *game_client = 0;

Driver::Driver()
{
    splash_window.reset(new SplashWindow);
    splash_window->new_game_requested.connect(
	sigc::mem_fun(*this, &Driver::on_new_game_requested));
    splash_window->new_hosted_network_game_requested.connect(
	sigc::mem_fun(*this, &Driver::on_new_hosted_network_game_requested));
    splash_window->new_remote_network_game_requested.connect(
	sigc::mem_fun(*this, &Driver::on_new_remote_network_game_requested));
    splash_window->load_requested.connect(
	sigc::mem_fun(*this, &Driver::on_load_requested));
    splash_window->quit_requested.connect(
	sigc::mem_fun(*this, &Driver::on_quit_requested));

    if (Main::instance().start_test_scenario) {
        game_client = new GameClient();
        game_client->game_scenario_received.connect(
          sigc::mem_fun(this, &Driver::on_load_requested));
        game_client->start("localhost", 12345);
        splash_window->show();
        return;
      
	// quick load a test scenario
	GameParameters g;
	GameParameters::Player p;
	p.type = GameParameters::Player::HUMAN;
	p.name = "Mr. Test";
	g.players.push_back(p);
	p.type = GameParameters::Player::EASY;
	p.name = "Evail";
	g.players.push_back(p);
	g.map.width = 75;
	g.map.height = 100;
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
	g.tile_theme = "default";
	g.army_theme = "Default";
	g.process_armies = GameParameters::PROCESS_ARMIES_AT_PLAYERS_TURN;
	
	g.difficulty = GameScenario::calculate_difficulty_rating(g);
	on_new_game_requested(g);
    }
    else
	splash_window->show();
}

Driver::~Driver()
{
}

void Driver::on_new_hosted_network_game_requested(GameParameters g, bool has_ops)
{
    if (splash_window.get())
	splash_window->hide();

    NewGameProgressWindow pw(g);
    Gtk::Main::instance()->run(pw);
    GameScenario *game_scenario = pw.getGameScenario();

    if (game_scenario == NULL)
      {
	TimedMessageDialog dialog(*splash_window->get_window(),
				  _("Corrupted saved game file."), 0);
	return;
      }

  game_lobby_dialog.reset(new GameLobbyDialog(game_scenario, has_ops));
  game_lobby_dialog->set_parent_window(*splash_window.get()->get_window());
  int response = game_lobby_dialog->run();
  game_lobby_dialog->hide();
  if (splash_window.get())
    splash_window->show();
}

void Driver::on_new_remote_network_game_requested(std::string filename, bool has_ops)
{
  if (splash_window.get())
    splash_window->hide();
  game_lobby_dialog.reset(new GameLobbyDialog(filename, has_ops));
  game_lobby_dialog->set_parent_window(*splash_window.get()->get_window());
  int response = game_lobby_dialog->run();
  game_lobby_dialog->hide();
  if (splash_window.get())
    splash_window->show();
}

void Driver::on_new_game_requested(GameParameters g)
{
    if (splash_window.get())
	splash_window->hide();

    NewGameProgressWindow pw(g);
    Gtk::Main::instance()->run(pw);
    GameScenario *game_scenario = pw.getGameScenario();

    if (game_scenario == NULL)
      {
	TimedMessageDialog dialog(*splash_window->get_window(),
				  _("Corrupted saved game file."), 0);
	return;
      }

    init_game_window();
    
    game_window->show();
    game_window->new_game(game_scenario);
}

void Driver::on_load_requested(std::string filename)
{
    if (splash_window.get())
	splash_window->hide();

    GameScenario *game_scenario = load_game(filename);
    if (game_scenario == NULL)
      return;

    init_game_window();
    
    //game_window->sdl_initialized.connect(
	//sigc::bind(sigc::mem_fun(game_window.get(), &GameWindow::load_game),
		   //filename));
    game_window->show();
    game_window->load_game(game_scenario);

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
    game_window->hide();
    game_window.reset();

    GraphicsCache::deleteInstance();

    splash_window->show();
}

void Driver::init_game_window()
{
    game_window.reset(new GameWindow);

    game_window->game_ended.connect(
	sigc::mem_fun(*this, &Driver::on_game_ended));
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
	return NULL;
      }
    return game_scenario;
}
