// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004 John Farrell
// Copyright (C) 2005, 2006 Andrea Paternesi
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

#ifndef CITYLIST_H
#define CITYLIST_H

#include "LocationList.h"
#include <sigc++/trackable.h>

class City;
class Player;

//! A list of City objects on the game map.
/**
 * The citylist keeps track of the city objects located on the game map. It
 * is implemented as a singleton because many classes use it for looking 
 * up cities.
 */
class Citylist : public LocationList<City>, public sigc::trackable
{
    public:
        //! Returns the singleton instance.  Creates a new one if neccessary.
        static Citylist* getInstance();

        //! Loads the singleton instance from an opened saved-game file.
        static Citylist* getInstance(XML_Helper* helper);

        //! Deletes the singleton instance.
        static void deleteInstance();

	//! Process all City objects for the next turn.
        /**
         * This function loops through all cities belonging to player p and
         * processes them.  This method adds the income of each city to
         * the player's treasury and produces a new armies from the City
	 * object's active Army production base if they are due to be created.
         *
         * @param p        The player whose cities are processed.
         */
        void nextTurn(Player* p);

        //! Save the list of City objects to an opened saved-game file.
        bool save(XML_Helper* helper) const;
       
        //! Count the number of City objects that a given player owns. 
        int countCities(Player* p) const;

        //! Returns the closest city that is owned by an enemy player.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that is at war with the active player.
	 *
	 * @note This method will not return a razed city.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             enemy-owned City object from.
	 *
	 * @return A pointer to the nearest enemy-owned City object, or NULL 
	 *         if there aren't any City objects owned by any enemies.
	 */
        City* getNearestEnemyCity(const Vector<int>& pos);

        //! Returns the closest city that isn't owned by the active player.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that owned by a Player who isn't the active player.
	 *
	 * @note This method will not return a razed city.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             City object that isn't owned by the active player.
	 *
	 * @return A pointer to the nearest foreign-owned City object, or NULL 
	 *         if there aren't any City objects owned by any other Player.
	 */
	City* getNearestForeignCity(const Vector<int>& pos);

        //! Return the closest city owned by the active player.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that owned by the active player from the given position.
	 *
	 * @note This method will not return a razed city.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             City object that is owned by the active player.
	 *
	 * @return A pointer to the nearest City object that is owned by the 
	 *         active player, or returns NULL if the active player doesn't 
	 *         own any City objects.
	 */
        City* getNearestFriendlyCity(const Vector<int>& pos);

        //! Find the closest city owned by the active player and isn't too far.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that owned by the active player and isn't farther away than the
	 * prescribed number of tiles.
	 *
	 * @note This method will not return a razed city.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             City object that is owned by the active player.
	 * @param dist The number of tiles away that is deemed "too far".
	 *
	 * @return A pointer to the nearest City object that is owned by the 
	 *         active player, and is within the perscribed number of tiles.
	 *         Returns NULL if the active player doesn't own any City 
	 *         objects within of the prescribed number of tiles.
	 */
        City* getNearestFriendlyCity(const Vector<int>& pos, int dist);

        //! Find the closest city to the given position.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that isn't razed.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             City object from.
	 *
	 * @return A pointer to the nearest City object.  Returns NULL if there
	 *        are not any City objects in the list.
	 */
        City* getNearestCity(const Vector<int>& pos);

        //! Find the closest city that isn't too far away.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that isn't razed and isn't farther away than the presecribed
	 * number of tiles.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             City object from.
	 * @param dist The number of tiles away that is deemed "too far".
	 *
	 * @return A pointer to the nearest city that is not razed and is 
	 *         within the prescribed number of tiles.  Returns NULL if
	 *         no city could be found.
	 */
        City* getNearestCity(const Vector<int>& pos, int dist);

        //! Find the nearest city that is not obscured by fog.
	/**
	 * Scan through all cities, searching for the closest one that is
	 * not covered by fog-of-war on a hidden map.
	 *
	 * @note This method will not return a city that has been razed.
	 *
	 * @param pos  The position to find the nearest city from.
	 *
	 * @return A pointer to the nearest city that is not obscured by fog.
	 */
        City* getNearestVisibleCity(const Vector<int>& pos);

	//! Find the nearest city that is unobscured and is not too far away.
	/**
	 * Scan through all the cities, searching for the closest one that
	 * is not covered by fog-of-war on a hidden map, but is not farther
	 * away than a given distance.
	 *
	 * @note This method will not return a city that has been razed.
	 *
	 * @param pos  The position to find the nearest city from.
	 * @param dist The number of tiles away that is deemed "too far".
	 *
	 * @return A pointer to the nearest city that is not obscured by fog 
	 *         and is within the prescribed number of tiles.  Returns NULL 
	 *         if no city could be found.
	 */
        City* getNearestVisibleCity(const Vector<int>& pos, int dist);

        //! Return the closest city owned by the given player.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that owned by the given player to the given position.
	 *
	 * @note This method will not return a razed city.
	 *
	 * @param pos    The position on the game map to search for the nearest
	 *               City object that is owned by the given player.
	 * @param player The player who owns the City object this method is
	 *               searching for.
	 *
	 * @return A pointer to the nearest City object that is owned by the 
	 *         given player, or returns NULL if the given player doesn't 
	 *         own any City objects.
	 */
	City* getNearestCity(const Vector<int>& pos, Player *player);

        //! Return the closest city owned by the neutral player.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that owned by the neutral player to the given position.
	 *
	 * @note This method will not return a razed city.
	 *
	 * @param pos    The position on the game map to search for the nearest
	 *               City object that is owned by the neutral player.
	 * @param player The player who owns the City object this method is
	 *               searching for.
	 *
	 * @return A pointer to the nearest City object that is owned by the 
	 *         neutral player, or returns NULL if the neutral player 
	 *         doesn't own any City objects.
	 */
        City* getNearestNeutralCity(const Vector<int>& pos);

        //! Find the nearest unobscured city that is owned by the active player.
	/**
	 * Scan through all cities, searching for the closest one that is
	 * not covered by fog-of-war on a hidden map and is owned by the
	 * active player.
	 *
	 * @note This method will not return a city that has been razed.
	 *
	 * @param pos  The position to find the nearest city from.
	 *
	 * @return A pointer to the nearest city that is not obscured by fog,
	 *         and is owned by the active player.
	 */
        City* getNearestVisibleFriendlyCity(const Vector<int>& pos);

        //! Get the nearest unfogged city of active player's that isn't too far.
	/**
	 * Scan through all cities, searching for the closest one that is
	 * not covered by fog-of-war on a hidden map and is owned by the
	 * active player, and is within the prescribed number of tiles.
	 *
	 * @note This method will not return a city that has been razed.
	 *
	 * @param pos  The position to find the nearest city from.
	 * @param dist The number of tiles away that is deemed "too far".
	 *
	 * @return A pointer to the nearest city that is not obscured by fog,
	 *         and is owned by the active player and is within the 
	 *         prescribed number of tiles.  Returns NULL if no city 
	 *         fitting those parameters could be found.
	 */
        City* getNearestVisibleFriendlyCity(const Vector<int>& pos, int dist);

        //! Returns the first City object owned by the given player.
	/**
	 * Scans the list of city objects for the first one that is owned by
	 * the given player.
	 *
	 * @param player  The owner of the City to search for.
	 *
	 * @return A pointer to a City object owned by the given player, or
	 *         NULL if the given player doesn't own any city objects.
	 */
        City* getFirstCity(Player* player);

	//! Changes ownership of all cities owned by old owner, to a new owner.
	void changeOwnership(Player *old_owner, Player *new_owner);

	//! Stops vectoring from any city to the specified city.
	/**
	 * Scan through all of the City objects who are vectoring to the given
	 * city, and stop the vectoring.
	 *
	 * @param city  The city to search for vectoring to.
	 */
	void stopVectoringTo(City *city);

	//! Find the center point for all of the given player's cities.
	/**
	 * Scan through the list of City objects for cities being owned by
	 * the given player.  Average all the positions of those cities and
	 * determine a centre point.
	 *
	 * @param player  The player we're trying to calculate the centre
	 *                of territory for.
	 *
	 * @return The position on the map that is the centre of that player's
	 *         cities.  If the player has no cities, this method returns
	 *         a point of (INT_MAX/2, INT_MAX/2).
	 */
	Vector<int> calculateCenterOfTerritory (Player *player);

	//! Return whether or not any cities are vectoring to the given city.
	/**
	 * Scan through all of the city objects for a city that is vectoring
	 * to the given city.
	 *
	 * @param target  The city to check if any other cities are vectoring 
	 *                to.
	 *
	 * @return True if another city is vectoring to the given city.
	 *         Otherwise false.
	 */
	bool isVectoringTarget(City *target);

	//! Return the list of cities vectoring to the given city.
	/**
	 * Scan through all of the city objects for cities that are vectoring
	 * to the given city.  Add pointers to these cities to a list.
	 *
	 * @param target  The city to check if any other cities are vectoring 
	 *                to.
	 *
	 * @return The list of pointers to city objects that are vectoring to
	 *         the given city.
	 */
	std::list<City*> getCitiesVectoringTo(City *target);

	//! Get the nearest city that is farther than a given number tiles.
	/**
	 * Scans through all of the City objects in the list for the nearest
	 * one that isn't razed and is farther away then the given number of
	 * tiles.
	 *
	 * @param pos  The position on the game map to search for the nearest
	 *             City object from.
	 * @param dist The distance that is deemed "too close".
	 *
	 * @return A pointer to the nearest City object that is farther away
	 *         than the given distance.  Returns NULL if there are no 
	 *         cities that are farther away than the given distance.
	 */
	City* getNearestCityPast(const Vector<int>& pos, int dist);

    protected:
        // CREATORS
	//! Default constructor.
        Citylist();
	//! Loading constructor.
	/**
	 * Make a new Citylist object by reading it from an opened saved-game 
	 * file.
	 *
	 * @param helper  The opened saved-game file to load the list of City
	 *                objects from.
	 */
        Citylist(XML_Helper* helper);
	//! Destructor.
        ~Citylist();

    private:
        //! A callback for loading City objects into the list of cities.
        bool load(std::string tag, XML_Helper* helper);

        //! A static pointer for the singleton instance.
        static Citylist* s_instance;
};

#endif // CITYLIST_H

// End of file
