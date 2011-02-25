//  Copyright (C) 2008 Ben Asselstine
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

#ifndef RECENTLY_PLAYED_GAME_H
#define RECENTLY_PLAYED_GAME_H

#include <gtkmm.h>
#include <string>


#include <sys/time.h>

#include "GameScenario.h"
class XML_Helper;
class Profile;

//! A single game entry in the recently played games list.
/**
 *
 */
class RecentlyPlayedGame
{
    public:

	//! The xml tag of this object in a recently played game file.
	static std::string d_tag; 

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
        RecentlyPlayedGame(GameScenario *game_scenario, Profile *profile);
        
	//! Destructor.
        virtual ~RecentlyPlayedGame();

	// Get Methods

        //! Get the scenario id of the recently played game entry.
	std::string getId() const {return d_id;};

        //! Get the id of the profile who made the entry.
	std::string getProfileId() const {return d_profile_id;};

	//! Get time of when this game was last played (seconds past the epoch).
	time_t getTimeOfLastPlay() const { return d_time;};

	//! Get the round that we last saw this game at..
	guint32 getRound() const { return d_round;};

	//! Get the number of cities in the game.
	guint32 getNumberOfCities() const {return d_number_of_cities;};

	//! Get the number of players in the game.
	guint32 getNumberOfPlayers() const {return d_number_of_players;};

	//! Get the kind of game.
	GameScenario::PlayMode getPlayMode() const {return d_playmode;};

	//! Get the name of the scenario.
	std::string getName() const {return d_name;};


	// Set Methods

	//! Set the last time we saw something happen in this game.
	void setTimeOfLastPlay(time_t then) { d_time = then;};

	//! Set the round that we last saw this game at.
	void setRound(guint32 round) { d_round = round;};


	// Methods that operate on the class data but do not modify it.

	//! Save the game entry to an opened file.
	bool save(XML_Helper* helper) const;

	//! Save the game entry, but not the enclosing tags.
	bool saveContents(XML_Helper *helper) const;


	// Static Methods

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

	//! Save the entry to an opened file.
	virtual bool doSave(XML_Helper *helper) const = 0;

	// DATA
	
	//! The id of the game.
	std::string d_id;

	//! When the game was last played.
	time_t d_time;

	//! What round the game was at.
	guint32 d_round;

	//! How many cities the game has.
	guint32 d_number_of_cities;

	//! How many players the game had at the start of the game.
	guint32 d_number_of_players;

	//! The kind of game.
	GameScenario::PlayMode d_playmode;

	//! The name of the game.
	std::string d_name;

        //! The id of the profile who played the game.
        std::string d_profile_id;
};

class RecentlyPlayedHotseatGame : public RecentlyPlayedGame
{
    public:
	//! Make a new hotseat game entry.
	RecentlyPlayedHotseatGame(GameScenario *game_scenario, Profile *p);

	//! Load a new hotseat game from an opened file.
	RecentlyPlayedHotseatGame(XML_Helper *helper);

	//! Destroy a hotseat game entry.
	~RecentlyPlayedHotseatGame();


	// Methods that operate on the class data but do not modify it.
	
	//! Save the hotseat game entry to an opened file.
	virtual bool doSave(XML_Helper *helper) const;


	// Methods that operate on the class data and modify it.

	//! Assign the filename to the entry.
	bool fillData(std::string filename);

    private:
	std::string d_filename;
};

class RecentlyPlayedPbmGame : public RecentlyPlayedGame
{
    public:
	//! Make a new pbm game entry.
	RecentlyPlayedPbmGame(GameScenario *game_scenario, Profile *p);

	//! Load a new pbm game from an opened saved-game file.
	RecentlyPlayedPbmGame(XML_Helper *helper);

	//! Destroy a pbm game entry.
	~RecentlyPlayedPbmGame();


	// Methods that operate on the class data but do not modify it.
	
	//! Save the play-by-mail game entry to an opened file.
	virtual bool doSave(XML_Helper *helper) const;


	// Methods that operate on the class data and modify it.

	//! Assign the filename to the entry.
	bool fillData(std::string filename);
    private:

	// DATA
	
	//! The filename of the play-by-mail game for this entry.
	std::string d_filename;
};

class RecentlyPlayedNetworkedGame : public RecentlyPlayedGame
{
    public:
	//! Make a new networked game entry.
	RecentlyPlayedNetworkedGame(GameScenario *game_scenario, Profile *p);

	//! Load a new networked game from an opened file.
	RecentlyPlayedNetworkedGame(XML_Helper *helper);

	//! Destroy a networked game entry.
	~RecentlyPlayedNetworkedGame();


	// Get Methods
	
	//! Get the hostname associated with the game.
	std::string getHost() const {return d_host;};

	//! Get the port associated with the host, and game.
	guint32 getPort() const {return d_port;};


	// Methods that operate on the class data but do not modify it.

	//! Save the networked game entry to an opened file.
	virtual bool doSave(XML_Helper *helper) const;


	// Methods that operate on the class data and modify it.

	bool fillData(std::string host, guint32 port);

    private:

	// DATA
	
	//! The hostname that the network game was hosted at.
	std::string d_host;

	//! The port on the hostname that the network game was hosted at.
	guint32 d_port;
};

#endif // RECENTLY_PLAYED_GAME_H
