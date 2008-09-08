//  Copyright (C) 2008, Ben Asselstine
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

#ifndef RECENTLY_PLAYED_GAME_H
#define RECENTLY_PLAYED_GAME_H

#include <SDL.h>
#include <string>


#include <sys/time.h>

#include "GameScenario.h"
class XML_Helper;

//! A single game entry in the recently played games list.
/**
 *
 */
class RecentlyPlayedGame
{
    public:

	//! Loading constructor.
        /**
	 * Make a new recently played game object by reading it in from an 
	 * opened recently played games list file.
	 *
         * @param helper  The opened recently played games list file to read the
	 *                game entry from.
         */
        RecentlyPlayedGame(XML_Helper* helper);

	//! Default constructor.
	/**
	 * Make a new recently played game object by taking values from the
	 * GameScenario.
	 */
        RecentlyPlayedGame(GameScenario *game_scenario);
        
	//! Destructor.
        virtual ~RecentlyPlayedGame();

        //! Get the scenario id of the recently played game entry.
	std::string getId() const {return d_id;};

	//! Get time of when this game was last played (seconds past the epoch)
	time_t getTimeOfLastPlay() const { return d_time;};

	//! Set the last time we saw something happen in this game.
	void setTimeOfLastPlay(time_t then) { d_time = then;};

	//! Get the round that we last saw this game at.
	Uint32 getRound() const { return d_round;};

	//! Set the round that we last saw this game at.
	void setRound(Uint32 round) { d_round = round;};
	//! Get the number of cities in the game
	Uint32 getNumberOfCities() const {return d_number_of_cities;};

	//! Get the number of players in the game
	Uint32 getNumberOfPlayers() const {return d_number_of_players;};

	//! Get the kind of game
	GameScenario::PlayMode getPlayMode() const {return d_playmode;};

	//! Get the name of the scenario
	std::string getName() const {return d_name;};

	//! Save the game entry to an opened file.
	bool save(XML_Helper* helper) const;

	//! Save the game entry, but not the enclosing tags.
	bool saveContents(XML_Helper *helper) const;


	/**
	 * static load function (see XML_Helper)
	 * 
	 * Whenever a game entry is loaded, this function is called. It
	 * examines the stored id and calls the constructor of the appropriate
	 * recently played game class.
	 *
	 * @param helper       the XML_Helper instance for the savegame
	 */
	static RecentlyPlayedGame* handle_load(XML_Helper *helper);
    protected:
	virtual bool doSave(XML_Helper *helper) const = 0;

	std::string d_id;
	time_t d_time;
	Uint32 d_round;
	Uint32 d_number_of_cities;
	Uint32 d_number_of_players;
	GameScenario::PlayMode d_playmode;
	std::string d_name;

};

class RecentlyPlayedHotseatGame : public RecentlyPlayedGame
{
    public:
	//! Make a new hotseat game entry.
	RecentlyPlayedHotseatGame(GameScenario *game_scenario);
	//! Load a new hotseat game from an opened saved-game file.
	RecentlyPlayedHotseatGame(XML_Helper *helper);
	//! Destroy a hotseat game entry.
	~RecentlyPlayedHotseatGame();

	virtual bool doSave(XML_Helper *helper) const;
	bool fillData(std::string filename);
    private:
	std::string d_filename;
};

class RecentlyPlayedPbmGame : public RecentlyPlayedGame
{
    public:
	//! Make a new pbm game entry.
	RecentlyPlayedPbmGame(GameScenario *game_scenario);
	//! Load a new pbm game from an opened saved-game file.
	RecentlyPlayedPbmGame(XML_Helper *helper);
	//! Destroy a pbm game entry.
	~RecentlyPlayedPbmGame();

	virtual bool doSave(XML_Helper *helper) const;
	bool fillData(std::string filename);
    private:
	std::string d_filename;
};

class RecentlyPlayedNetworkedGame : public RecentlyPlayedGame
{
    public:
	//! Make a new networked game entry.
	RecentlyPlayedNetworkedGame(GameScenario *game_scenario);
	//! Load a new networked game from an opened saved-game file.
	RecentlyPlayedNetworkedGame(XML_Helper *helper);
	//! Destroy a networked game entry.
	~RecentlyPlayedNetworkedGame();

	virtual bool doSave(XML_Helper *helper) const;
	bool fillData(std::string host, Uint32 port);

	std::string getHost() const {return d_host;};
	Uint32 getPort() const {return d_port;};
    private:
	std::string d_host;
	Uint32 d_port;
};

#endif // RECENTLY_PLAYED_GAME_H
