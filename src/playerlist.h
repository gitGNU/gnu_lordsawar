// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#ifndef PLAYERLIST_H
#define PLAYERLIST_H

#include <list>
#include <map>
#include <string>
#include <vector>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include "game-parameters.h"

class History;
class Shieldset;

#include "player.h"

//! A list of all of the Player objects in a scenario.
/** 
 * The Playerlist is implemented as a singleton class. The currently
 * active player is designated, you can access players by name or id and the
 * playerlist can check if there are more than one player remaining alive.
 * This class also holds methods that affect all players.
 */

class Playerlist : public std::list<Player*>, public sigc::trackable
{
    public:

	//! The xml tag of this object in a saved-game file.
	static std::string d_tag; 

	// Set Methods

	//! Set the winning human player.
	void setWinningPlayer(Player *winner);

	//! Set the player who's turn it is.  should only be used by the editor.
	void setActiveplayer(Player *p) {d_activeplayer = p;};

	//! set the player who is looking at the bigmap and smallmap.
	void setViewingplayer(Player *p) {viewingplayer = p;};

	//! Get Methods

        //! Returns the neutral player.
        Player* getNeutral() const {return d_neutral;}

        //! Return the player with the highest score.
        Player *getWinningPlayer() const;


	// Methods that operate on the class data and modify the class.

        //! Sets the active player to the next player in the order.
        void nextPlayer();

        /** 
	 * Checks if a player is alive and has no cities left. If not then 
	 * this method marks the player as killed.
	 *
	 * @param Returns whether or not any players were marked as dead.
         */
	//! Kill any players that don't have cities left.
        bool checkPlayers();

        /** 
	 * Though the neutral player is located in the list of existing 
	 * players, it is handled in a special way in many different cases.
	 *
	 * There can only be one neutral player per Playerlist.
         *
         * @param neutral          The new neutral player.
         */
	//! Set the neutral player.
        void setNeutral(Player* neutral) {d_neutral = neutral;}

	/** 
	 * Swap out a player from the list and replace it with a new one.
	 * Specical care is taken to remove all references to the original
	 * player and replace it with a reference to the new player.
	 *
	 * The purpose of this method is to change a human player into a
	 * computer player and vice-versa.
	 *
	 * @param old_player   A pointer to the player to replace.
	 * @param new_player   A pointer to the new player to replace the 
	 *                     original player.
	 */
	//! Replace a Player in the list with a new Player.
	void swap(Player *old_player, Player *new_player);
        
        /** 
	 * Erase a Player from the list, and free the contents of the Player.
	 *
	 * @param it   The place in the Playerlist to erase.
	 *
	 * @return The place in the list that was erased.
         */
	//! Erase a player from the list.
        iterator flErase(iterator it);

	/**
	 * This method is called when a round starts.
	 * The purpose of this method is to calculate who is winning, and 
	 * it to negotiate diplomacy between players.  This method also 
	 * implements the computer players collectively surrendering to a 
	 * final human player.
	 *
	 * @param diplomacy     Whether or not we should negotiate diplomacy
	 *                      between players.
	 * @param surrender_already_offered Tells the method if surrender
	 *                      has already been offered by the computer
	 *                      players.  This needs to be kept track of
	 *                      because the computer players only offer
	 *                      surrender once.  The method will change this
	 *                      value from false to true if it decided that 
	 *                      the computer players collectively offer 
	 *                      surrender.
	 */
	//! Callback method to process all players at the start of a round.
	void nextRound(bool diplomacy, bool *surrender_already_offered);

	/**
	 * The purpose of randomzing the Playerlist is to implement
	 * random turns.
	 * Note: This method does not set the active player.
	 */
	//! Randomize the order of the players in the list.
	void randomizeOrder();

	/**
	 * This method takes care of giving a player it's diplomatic
	 * ranking among all other players.  The rank is determined by 
	 * adding up all of the diplomatic scores, and then sorting them.
	 * Each rank has a title.  There is always a Player who has the 
	 * title of `Statesman', and there is always a Player who has the 
	 * title of `Running Dog'.  The other titles disappear as the other 
	 * players die off.
	 */
	//! Figure out who's winning diplomatically.
        void calculateDiplomaticRankings();

	//! Sync the playerlist.
	/**
	 * Sync the playerlist with the list of players given.
	 */
	void syncPlayers(std::vector<GameParameters::Player> players);

	//! Sync the given player with the playerlist
	void syncPlayer(GameParameters::Player player);

	//! Converts all of the human players into network players.
	void turnHumansIntoNetworkPlayers();

	//! Converts a given number of the human players into a type of player.
	void turnHumansInto(Player::Type type, int num_players = -1);

	//! Reorder the list according to the given order.
	void reorder(std::list<guint32> order);

	//! Perform the surrender of all computer players.
	void surrender();

	//! Add a player to the list.  Use this instead of push_back.
	void add(Player *player);

        //! Reassign player colours.
        void setNewColours(Shieldset *shieldset);

	// Methods that operate on the class data but do not modify it.

	/**
	 * Scan the list of players for a Player with a given name.
	 *
	 * @param name      The name of the Player to lookup.
	 *
	 * @return A pointer to the Player if it is found, or NULL if it isn't.
	 */
	//! Lookup a Player by it's name.
        Player* getPlayer(std::string name) const;

	/**
	 * Scan the list of players for a Player with a given Id.
	 *
	 * @param id        The id of the Player to lookup.
	 *
	 * @return A pointer to the Player if it is found, or NULL if it isn't.
	 */
        //! Lookup a Player by it's Id.
        Player* getPlayer(guint32 id) const;

        //! Returns the number of living players (neutral player excluded.)
        guint32 getNoOfPlayers() const;

        /** 
	 * Scan the list of players for the first Player that is alive.
	 * This method is used to determine the beginning of a round.
	 *
	 * @return A pointer to the first living Player.
         */
	//! Return the first living Player in the list.
        Player* getFirstLiving() const;

        //! Saves the playerlist to an opened saved-game file.
        bool save(XML_Helper* helper) const;

	//! Return the number of human players left alive in the list.
	guint32 countHumanPlayersAlive() const;

	//! Return the number of players left alive, not including neutral.
	guint32 countPlayersAlive() const;

        //! counts blessings of all army units for all players.
        guint32 countBlessings() const;

	//! Return the list of activities that the given hero has accomplished.
	std::list<History *>getHistoryForHeroId(guint32 id) const;

	/** 
	  \brief Check to see if this is the end of the round or not.
	  */
	bool isEndOfRound() const;


        //! Return true if any of the players use the given armyset.
        bool hasArmyset(guint32 id) const;

	// Signals

	/**
	 * @param player  The player who has died.
	 */
        //! Emitted when a player has died.
        sigc::signal<void, Player*> splayerDead;
    
	/**
	 * Emitted when the computer players collectively offer surrender to 
	 * a single remaining human player.
	 *
	 * @param player  The human player who is being surrendered to.
	 */
        //! Emitted when a surrender is offered.
        sigc::signal<void, Player*> ssurrender;
    

	// Static Methods

        //! Gets the singleton instance or creates a new one.
        static Playerlist* getInstance();

	/**
	 * Load all Players in the Playerlist from a saved-game file.
	 *
	 * @param helper     The opened saved-game file to read from.
	 *
	 * @return The loaded Playerlist.
	 */
        //! Loads the playerlist from a saved-game file.
        static Playerlist* getInstance(XML_Helper* helper);

        //! Explicitly deletes the singleton instance.
        static void deleteInstance();

        //! Returns the active player (the Player whose turn it is).
        static Player* getActiveplayer() {return d_activeplayer;}

        //! Returns the viewing player (the Player who is looking at maps).
	static Player *getViewingplayer() {return viewingplayer;}


        int countFightsThisTurn() const;
        int countMovesThisTurn() const;
    protected:

	//! Default constructor.
        Playerlist();

	//! Loading constructor.
        Playerlist(XML_Helper* helper);

	//! Destructor.
        ~Playerlist();
        
    private:
        //! Callback for loading the playerlist from an opened saved-game file.
        bool load(std::string, XML_Helper* helper);

	//! Comparison function to assist in sorting the list of players.
	static bool randomly(const Player *lhs, const Player *rhs);

	//! Comparison function to assist in sorting the list of players.
	static bool inGivenOrder(const Player *lhs, const Player *rhs);

	//! Comparison function to assist in sorting the list of players.
	static bool inOrderOfId(const Player *lhs, const Player *rhs);

	//! Calculate new scores for all players.
        void calculateWinners();

	//! Calculate new diplomatic states for all players.
	void negotiateDiplomacy();


        Glib::ustring get_title(int rank);
        // DATA

	//! The pointer to the player whose turn it is in the list.
        static Player* d_activeplayer;

	//! The player that the smallmap and bigmap are being viewed as.
	static Player *viewingplayer;

	//! The pointer to the neutral player in the list.
        Player* d_neutral;

	typedef std::map<guint32, Player*> IdMap;
	IdMap d_id;

        //! A static pointer for the singleton instance.
        static Playerlist* s_instance;
        
};

#endif // PLAYERLIST_H
