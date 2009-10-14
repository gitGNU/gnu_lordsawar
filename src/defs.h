// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003, 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2004 David Sterba
// Copyright (C) 2005 Bryan Duff
// Copyright (C) 2006, 2007, 2008, 2009 Ben Asselstine
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

//This file contains the various macros used within lordsawar.

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <string>
#include <libintl.h>

#define LORDSAWAR_SAVEGAME_VERSION "0.1.6"
#define LORDSAWAR_CONFIG_VERSION "0.1.6"
#define LORDSAWAR_RECENTLY_PLAYED_VERSION "0.1.6"
#define _(string) gettext(string) // Macro for the gettext
#define __(astring) std::string(gettext(astring.c_str()))


//-----------------------------------------------------------------------------
//some standard timers. They can easier be changed here than somewhere deep
//within the code, and sometimes you have to tweak them a little bit.
const unsigned int TIMER_BIGMAP_SELECTOR=500;
const unsigned int TIMER_BIGMAP_SCROLLING=30;   //milliseconds
const unsigned int TIMER_SMALLMAP_REFRESH=50;
const unsigned int CITY_LEVELS=4; 
const unsigned int MAX_PLAYERS=8;
const unsigned int TEMPLE_TYPES=1; 
const unsigned int RUIN_TYPES=2; 
const unsigned int DIPLOMACY_TYPES=3; 
const unsigned int ROAD_TYPES=15; 
const unsigned int FOG_TYPES=15; 
const unsigned int BRIDGE_TYPES=4; 
const unsigned int CURSOR_TYPES=12; 
const unsigned int MAX_CITIES_VECTORED_TO_ONE_CITY = 4;
const unsigned int MAX_TURNS_FOR_VECTORING = 2;
const unsigned int MAX_BOAT_MOVES = 18;
const unsigned int CUSP_OF_WAR_ROUND = 9;
const unsigned int DIPLOMACY_STARTING_SCORE = 8;
const unsigned int DIPLOMACY_MAX_SCORE = 15;
const unsigned int DIPLOMACY_MIN_SCORE = 0;
const unsigned int MAX_STACK_SIZE = 8;
const unsigned int MAX_PRODUCTION_SLOTS_IN_A_CITY = 4;
const unsigned int MAX_ARMIES_PRODUCED_IN_NEUTRAL_CITY = 5;

const unsigned int MAP_SIZE_TINY_WIDTH = 50;
const unsigned int MAP_SIZE_TINY_HEIGHT = 75;
const unsigned int MAP_SIZE_SMALL_WIDTH = 70;
const unsigned int MAP_SIZE_SMALL_HEIGHT = 105;
const unsigned int MAP_SIZE_NORMAL_WIDTH = 112;
const unsigned int MAP_SIZE_NORMAL_HEIGHT = 156;

const unsigned int PRODUCTION_SHIELD_WIDTH = 10;
const unsigned int PRODUCTION_SHIELD_HEIGHT = 10;
const unsigned int PRODUCTION_SHIELD_TYPES = 8;
const unsigned int MOVE_BONUS_WIDTH = 32;
const unsigned int MOVE_BONUS_HEIGHT = 20;
const unsigned int MOVE_BONUS_TYPES = 6;
const unsigned int MEDAL_TYPES = 3;

const int MAX_GOLD_TO_CARRY_OVER_TO_NEXT_SCENARIO = 5000;
const unsigned int MAX_ARMY_STRENGTH = 9;

const unsigned short LORDSAWAR_PORT = 14998;
#define HUMAN_PLAYER_TYPE _("Human")
#define EASY_PLAYER_TYPE _("Easy")
#define HARD_PLAYER_TYPE _("Hard")
#define NO_PLAYER_TYPE _("Off")
#define NETWORKED_PLAYER_TYPE _("Network")

const std::string ARMYSETDIR = "army";
const std::string TILESETDIR = "tilesets";
const std::string CITYSETDIR = "citysets";
const std::string SHIELDSETDIR = "shield";
const std::string MAPDIR = "map";
const std::string ARMYSET_EXT = ".xml";
const std::string TILESET_EXT = ".xml";
const std::string CITYSET_EXT = ".xml";
const std::string SHIELDSET_EXT = ".xml";

const unsigned int MIN_PRODUCTION_TURNS_FOR_ARMY_UNITS = 1;
const unsigned int MAX_PRODUCTION_TURNS_FOR_ARMY_UNITS = 5;
const unsigned int MIN_UPKEEP_FOR_ARMY_UNITS = 0;
const unsigned int MAX_UPKEEP_FOR_ARMY_UNITS = 20;
const unsigned int MIN_MOVES_FOR_ARMY_UNITS = 6;
const unsigned int MAX_MOVES_FOR_ARMY_UNITS = 75;
const unsigned int MIN_STRENGTH_FOR_ARMY_UNITS = 1;
const unsigned int MAX_STRENGTH_FOR_ARMY_UNITS = 9;
#endif // DEFINITIONS_H

