//  Copyright (C) 2011 Ben Asselstine
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

#ifndef GAMELIST_H
#define GAMELIST_H

#include <gtkmm.h>
#include <string>
#include <list>
#include <sigc++/trackable.h>

#include "xmlhelper.h"

class GameScenario;
class Profile;
class AdvertisedGame;
class HostedGame;
class RecentlyPlayedGameList;

//! A list of games that we've recently hosted or advertised.
/** 
 * It is implemented as a singleton.
 *
 */
class Gamelist: public std::list<HostedGame*>, public sigc::trackable
{
    public:

	//! The xml tag of this object in a game list file.
	static std::string d_tag; 
	
	static const int TEN_DAYS_OLD = 864000; /* seconds */

	static const int MAX_NUMBER_OF_ADVERTISED_GAMES = 100;

	// Methods that operate on the class data and do not modify the class.

        //! Save game list to the game list file.
        bool save() const;

	//! Save the game list to an opened file.
	bool save(XML_Helper* helper) const;

        //! Get the list, with some identifying information removed.
        RecentlyPlayedGameList* getList(bool scrub_profile_id = true) const;

        HostedGame *findGameByScenarioId(std::string scenario_id) const;

	// Methods that operate on the class data and modify the class.

        bool add(HostedGame *g);

	//! Load the game list from the games file.
        bool load();

	//! Add an entry to the list of games.
	void addEntry(AdvertisedGame *advertised_game);

	//! Touch the game in the games list.
	void updateEntry(std::string scenario_id, guint32 round);

	//! Remove a game entry from the list, by it's scenario id.
	bool removeEntry(std::string scenario_id);

	//! Removes games from the list that are too old, or just too numerous.
	void pruneGames();
	

	// Static Methods

        //! return the singleton instance of this class.
        static Gamelist * getInstance();

        //! Loads the singleton instance from an opened file.
        static Gamelist * getInstance(XML_Helper *helper);

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

    protected:
        //! Default Constructor.
        Gamelist();

	//! Loading constructor
        Gamelist(XML_Helper *helper);
        
        //! Destructor.
        ~Gamelist();

    private:
        //! Callback for loading the games into this list.
	bool load_tag(std::string tag, XML_Helper *helper);

	//! Helper method to sort the list by it's last-played time.
	static bool orderByTime(HostedGame*rhs, HostedGame *lhs);

	//! Remove the old games from the list.
	void pruneOldGames(int stale = TEN_DAYS_OLD);

        void pruneTooManyGames(int too_many = 100);

	//! Load the game list from the given file.
	bool loadFromFile(std::string filename);

	//! Save the game list to the given file.
	bool saveToFile(std::string filename) const;

        void remove_all();
	// DATA

        //! A static pointer for the singleton instance.
        static Gamelist* s_instance;
};

#endif // GAMELIST_H

