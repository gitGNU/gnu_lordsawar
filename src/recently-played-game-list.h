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

#ifndef RECENTLYPLAYEDGAMELIST_H
#define RECENTLYPLAYEDGAMELIST_H

#include <string>
#include <list>
#include <sigc++/trackable.h>

#include "xmlhelper.h"

class GameScenario;
class RecentlyPlayedGame;

//! A list of games that we've recently played.
/** 
 * This is only used for network games at the moment.
 * It is implemented as a singleton.
 *
 */
class RecentlyPlayedGameList: public std::list<RecentlyPlayedGame*>, public sigc::trackable
{
    public:
        //! return the singleton instance of this class.
        static RecentlyPlayedGameList * getInstance();

        //! Loads the singleton instance from an opened file.
        static RecentlyPlayedGameList * getInstance(XML_Helper *helper);

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

	//! Load the recently game list from the given file.
	bool loadFromFile(std::string filename);

	//! Save the recently game list to the given file.
	bool saveToFile(std::string filename);

	//! Add a game entry to the list of recently played games
	void addEntry(GameScenario *game_scenario, std::string filename);
	void addNetworkedEntry(GameScenario *game_scenario, std::string host, Uint32 port);
	void updateEntry(GameScenario *game_scenario);

	//! Remove a game entry from the list, by it's scenario id.
	bool removeEntry(std::string id);

	bool save(XML_Helper* helper) const;

	void removeAllNetworkedGames();

	void pruneGames();
	
    protected:
        //! Default Constructor.
	/**
	 */
        RecentlyPlayedGameList();
        RecentlyPlayedGameList(XML_Helper *helper);
        
        //! Destructor.
        ~RecentlyPlayedGameList();

    private:
        //! Callback for loading recentlyplayedgames into this list.
	bool load(std::string tag, XML_Helper *helper);

	//! Helper method to sort the list by it's last-played time.
	static bool orderByTime(RecentlyPlayedGame*rhs, RecentlyPlayedGame *lhs);

	static const int TWO_WEEKS_OLD = 1209600; /* seconds */
	void pruneOldGames(int stale = TWO_WEEKS_OLD);
	void pruneTooManyGames(int too_many = 10);

        //! A static pointer for the singleton instance.
        static RecentlyPlayedGameList* s_instance;
};

#endif // RECENTLYPLAYEDGAMELIST_H

