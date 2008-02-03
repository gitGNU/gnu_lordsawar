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
Driver::Driver()
{
    splash_window.reset(new SplashWindow);
    splash_window->new_game_requested.connect(
	sigc::mem_fun(*this, &Driver::on_new_game_requested));
    splash_window->load_requested.connect(
	sigc::mem_fun(*this, &Driver::on_load_requested));
    splash_window->quit_requested.connect(
	sigc::mem_fun(*this, &Driver::on_quit_requested));

    if (Main::instance().start_test_scenario) {
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
	g.map.temples = 25;
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

void Driver::on_new_game_requested(GameParameters g)
{
    init_game_window();
    
    game_window->sdl_initialized.connect(
	sigc::bind(sigc::mem_fun(game_window.get(), &GameWindow::new_game), g));
    game_window->show();
}

void Driver::on_load_requested(std::string filename)
{
    init_game_window();
    
    game_window->sdl_initialized.connect(
	sigc::bind(sigc::mem_fun(game_window.get(), &GameWindow::load_game),
		   filename));
    game_window->show();
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
    if (splash_window.get())
	splash_window->hide();

    game_window.reset(new GameWindow);

    game_window->game_ended.connect(
	sigc::mem_fun(*this, &Driver::on_game_ended));
    game_window->quit_requested.connect(
	sigc::mem_fun(*this, &Driver::on_quit_requested));

    game_window->init(640, 480);
}

