// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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
#include "GameScenarioOptions.h"
#include "xmlhelper.h"

#include "armysetlist.h"
#include "armyset.h"
#include "GameMap.h"
#include "Configuration.h"

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

class GameScenario: public GameScenarioOptions
{
    public:

	//! The xml tag of this object in a saved-game file.
	static std::string d_tag;

        enum PlayMode 
	  {
	    HOTSEAT = 0, 
	    NETWORKED = 1,
	    PLAY_BY_MAIL = 2,
	  };

	static std::string playModeToString(const GameScenario::PlayMode mode);
	static GameScenario::PlayMode playModeFromString(const std::string str);

        /** Initializes an "empty" scenario
          * 
          * @param name     the name of the scenario
          * @param comment  the comment for the scenario
          * @param turnmode the turnmode (see NextTurn for description)
          */
        GameScenario(std::string name, std::string comment, bool turnmode,
		     GameScenario::PlayMode playmode = GameScenario::HOTSEAT);
        
        /** Load the game scenario using a specified save game
          * 
          * @param savegame     the full name of the saved-game to load
          * @param broken       set to true if something goes wrong
          */
        GameScenario(std::string savegame, bool& broken);

        GameScenario(XML_Helper &helper, bool &broken);

        ~GameScenario();

        //! Returns the number of the current turn.
        unsigned int getRound() const {return s_round;}

        //! Returns the turn mode. See NextTurn for a description.
        bool getTurnmode() const {return d_turnmode;}
        
	std::string getId() const {return d_id;};

	void setNewRandomId();

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
        bool loadWithHelper(XML_Helper &helper);
        bool saveWithHelper(XML_Helper &helper) const;

        
	Uint32 getPlayMode() const {return d_playmode;};
	void setPlayMode(GameScenario::PlayMode mode) {d_playmode = mode;};

	bool validate(std::list<std::string> &errors, std::list<std::string> &warnings);

	void initialize(GameParameters g);

	static GameParameters loadGameParameters(std::string filename, bool &broken);

	void startRecordingEventsToFile(std::string filename);
	void stopRecordingEventsToFile();

    private:
	  /** Callback function for loading a game. See XML_Helper for details.
	   *
	   * @param tag      the tag name
	   * @param helper   the helper for parsing the save game file
	   * @return true if all went well, false otherwise.
	   */
	  bool load(std::string tag, XML_Helper* helper);
	  void quickStart();
	  bool setupFog(bool hidden_map);
	  bool setupCities(bool quick_start);
	  bool setupRewards(bool hidden_map);
	  bool setupMapRewards();
	  bool setupRuinRewards();
	  bool setupItemRewards();
	  bool setupStacks(bool hidden_map);
	  void setupDiplomacy(bool diplomacy);
	  bool autoSave();

	  // DATA
	  std::string d_name;
	  std::string d_comment;
	  bool d_turnmode; //see NextTurn for a description of this option
	  Uint32 d_playmode;
	  std::string d_id; //globally unique id identifying the scenario
	  std::string recording_file;
};

#endif // GAME_SCENARIO_H

// End of file
