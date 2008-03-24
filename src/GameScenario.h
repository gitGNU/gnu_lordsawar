// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef GAME_SCENARIO_H
#define GAME_SCENARIO_H

#include <string>
#include <list>
#include <sigc++/trackable.h>
#include "game-parameters.h"

class XML_Helper;

//! A class to hold several scenario options.
/** 
 * This class has two functions. On the one hand side, it holds some data
 * about the current scenario being played (such as the name), on the other
 * hand it has a kind of supervisor function. Loading and saving works in
 * a hierarchical way with superior objects (such as the playerlist) saving
 * their data and then telling inferior objects (such as players) to save
 * their data as well. GameScenario is kind of the root of the saving or
 * loading process. For more information about the saving procedure, have
 * a look at XML_Helper.
 */

class GameScenario: public sigc::trackable
{
    public:

        /** Initializes an "empty" scenario
          * 
          * @param name     the name of the scenario
          * @param comment  the comment for the scenario
          * @param turnmode the turnmode (see NextTurn for description)
          */
        GameScenario(std::string name, std::string comment, bool turnmode);
        
        /** Load the game scenario using a specified save game
          * 
          * @param savegame     the full name of the savegame to load
          * @param broken       set to true if something goes wrong
          */
        GameScenario(std::string savegame, bool& broken);
        ~GameScenario();

        //! Returns the number of the current turn.
        unsigned int getRound() const {return d_round;}

        //! Returns the turn mode. See NextTurn for a description.
        bool getTurnmode() const {return d_turnmode;}

        //! Returns the name of the scenario.
        std::string getName(bool translate = true) const;

        //! Returns the comment for the scenario.
        std::string getComment(bool translate = true) const;

        //! Increments the turn number and does an autosave. Called by NextTurn
        //! via a signal.
        void nextRound();

        //! Sets the name of the scenario.
        void setName(std::string name) {d_name = name;}

        //! Sets the name of the scenario.
        void setComment(std::string comment) {d_comment = comment;}
        
        /** Saves the game. See XML_Helper for further explanations.
          * 
          * @param filename     the full name of the save game file
          * @return true if all went well, false otherwise
          */
        bool saveGame(std::string filename, std::string extension = "sav") const;
        static bool s_see_opponents_stacks;
        static bool s_see_opponents_production;
        static bool s_play_with_quests;
        static bool s_hidden_map;
        static bool s_diplomacy;
        static bool s_cusp_of_war;
        static GameParameters::NeutralCities s_neutral_cities;
        static GameParameters::RazingCities s_razing_cities;
        static bool s_intense_combat;
        static bool s_military_advisor;
        static bool s_random_turns;
	static bool s_surrender_already_offered;
	static int s_difficulty;

	static int calculate_difficulty_rating(GameParameters g);

    private:
        /** Callback function for loading a game. See XML_Helper for details.
          *
          * @param tag      the tag name
          * @param helper   the helper for parsing the save game file
          * @return true if all went well, false otherwise.
          */
        bool load(std::string tag, XML_Helper* helper);

        // DATA
        unsigned int d_round;
        std::string d_name;
        std::string d_comment;
        bool d_turnmode; //see NextTurn for a description of this option
};

#endif // GAME_SCENARIO_H

// End of file
