//  Copyright (C) 2007, Ole Laursen
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

#ifndef GAME_PARAMETERS_H
#define GAME_PARAMETERS_H

#include <vector>
#include <string>

struct GameParameters
{
    struct Player 
    {
	enum Type { HUMAN, EASY, HARD, OFF };

	Type type;
	std::string name;
	int id;
    };

    std::vector<Player> players;

    struct Map
    {
	int width, height;
	int grass, water, swamp, forest, hills, mountains;
	int cities, ruins, temples, signposts;
    };

    Map map;

    // path to map file to load, empty if none
    std::string map_path;
    std::string tile_theme;
    std::string army_theme;
    std::string shield_theme;
    std::string city_theme;

    enum ProcessArmies {
	PROCESS_ARMIES_AT_PLAYERS_TURN = 0,
	PROCESS_ARMIES_WHEN_ROUND_BEGINS
    };
    ProcessArmies process_armies;

    bool see_opponents_stacks;
    bool see_opponents_production;
    bool play_with_quests;
    bool hidden_map;
    bool diplomacy;

    enum NeutralCities {
        AVERAGE = 0, STRONG, ACTIVE
    };
    NeutralCities neutral_cities;
    enum RazingCities {
        NEVER = 0, ON_CAPTURE, ALWAYS
    };
    RazingCities razing_cities;

    bool quick_start;
    bool cusp_of_war;
    bool intense_combat;
    bool military_advisor;
    bool random_turns;
    bool cities_can_produce_allies;
    int difficulty;
    std::string name;
};

#endif
