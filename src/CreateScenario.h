// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
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

#ifndef CREATE_SCENARIO_H
#define CREATE_SCENARIO_H

#include <string>
#include <vector>
#include <list>
#include <SDL.h>
#include "CreateScenarioRandomize.h"
#include "game-parameters.h"
#include "vector.h"

class MapGenerator;
class GameScenario;
class Player;
class City;

/** \brief Creates and dumps (i.e. saves) a scenario.
  * 
  * The purpose of this class is to create a new scenario which can then be
  * "loaded" by the game scenario class. The advantage is that the scenario
  * creation can be concentrated in one specific class instead of being scattered
  * over different classes and all objects (city, player, stack...) only need
  * one "loading" constructor. The disavantage is that all significant changes
  * e.g. to the player class have to be reflected here, too.
  */

class CreateScenario : public CreateScenarioRandomize
{
    public:
        //! This represents the class of the map (hills, islands etc.).
        enum MapType { NORMAL };
    
        /** The Constructor
          * 
          * @param width    the width of the map in the new scenario
          * @param height   the heightof the map in the new scenario
          */
        CreateScenario(int width = 112, int height = 156);
        ~CreateScenario();

        // setters

        //! Set the type of the map to be created
        void setMaptype(MapType type);

        //! Set the terrain distribution; differences to 100% are grass; sum may exceed 100%
        void setPercentages(int pgrass, int pwater, int pforest, int pswamp,
                            int phills, int pmountains);

        //! Set the tileset of the map
        void setMapTiles(std::string tilesname);

	//! Set the shieldset for the map
	void setShieldset(std::string shieldsname);

	//! Set the cityset for the map
	void setCityset(std::string citysetname);

        //! Set the number of cities on the map
        void setNoCities(int number);

        //! Set the number of ruins on the map
        void setNoRuins(int number);

        //! Set the number of signposts on the map
        void setNoSignposts(int number);

        //! Set the number of temples
        void setNoTemples(int number);

        //! Set the width of the map
        void setWidth(int width);

        //! Set the height of the map
        void setHeight(int height);

        /** Set the turn mode
          * 
          * @param turnmode     if set to true, several actions (healing armies
          *                     and producng armies, respectively) take part at
          *                     the beginning of the correpsonding player's turn,
          *                     else they are done when all players have
          *                     finished their round.
          */
        void setTurnmode(bool turnmode) {d_turnmode=turnmode;}
      
        /** Add a player to the scenario
          * 
          * @param name     the name of the player
          * @param armyset  the name of the player's armyset
          * @param color    the color of the player
          * @param type     the type of the player (see class player for more info)
          * @return a pointer to the created player
          */
        Player* addPlayer(std::string name, Uint32 armyset, SDL_Color color,
			  int type);

        /** Almost the same as addPlayer, but performs some additional checks
          * 
          * @param name     the name of the player
          * @param armyset  the name of the player's armyset
          * @param color    the color of the player
          * @param type     the type of the player (see class player for more info)
          * @return false if a neutral player already exists, true otherwise
          */
        bool addNeutral(std::string name, Uint32 armyset, SDL_Color color,
                        int type);

        
        //! Get the number of players already added
        int getNoPlayers() const;

        //! Get the n-th player
        Player* getPlayer(int number) const;

        //! Get the type of the map
        MapType getMaptype() const {return NORMAL;};

        //! Get the number of cities of the map
        int getNoCities() const;

        //! Get the number of ruins on the map
        int getNoRuins() const;

        //! Get the number of signposts on the map
        int getNoSignposts() const;

        //! Get the number of temples
        int getNoTemples() const;

        //! Get the tileset to be used for the map
        std::string getMapTiles() const;
        
        
        /** Creates a map
          * 
          * Calling this function will initiate the creation process itself. The
          * result is a saved map with several distributed players. The created
          * map can be either saved or used further (all lists etc. have already
          * been filled)
          */
        bool create(const GameParameters &g);

        /** Dumps (Saves) the map
          * 
          * @param filename     the full name of the save file
          * 
          * This will do the same as GameScenario::save() does (in fact it calls
          * GameScenario::save)
          */
        bool dump(std::string filename) const;

	MapGenerator *getGenerator() const {return d_generator;};
	static int calculateRoadType (Vector<int> t);
    private:
        //! Creates the map and distributes cities, temples and ruins
        bool createMap();

	void createCapitalCity(Player *player, City *city);
	bool tooNearToOtherCapitalCities(City *c, std::list<City*> capitals, Uint32 distance);

        //! Distributes the players over the map
        bool distributePlayers();

        //! Setup city names and production
        bool setupCities(bool cities_can_produce_allies, 
			 int number_of_armies_factor);

        bool setupCities2(bool quick_start);

        //! Setup temple names
        bool setupTemples();
        
        //! Set up ruin names and keepers
	bool setupRuins(bool strongholds_invisible, int sage_factor, 
			int no_guardian_factor, int stronghold_factor);


	//! Set up the standard set of items
	bool setupItems();

        //! Set up signposts
	//! @param ratio - how many signposts reference nearby cities vs
	//                 signposts that don't.
        bool setupSignposts(int ratio);

        //! Do some setup concerning the players (give them money)
	//! If we're playing with diplomacy then we start out at peace,
	//! and if we're not playing with diplomacy we start out at war.
        bool setupPlayers(bool random_turns, int base_gold);

	bool setupRoads();
	bool setupBridges();

        //! Set up rewards to be given out for quests, ruins and sages
        bool setupRewards();
        bool setupItemRewards();
        bool setupRuinRewards();
	bool setupMapRewards();
	void quickStart();

	//! Given the difficulty, get some characteristics of ruins
	void getRuinDifficulty (int difficulty, int *sage_factor, 
				int *no_guardian_factor, 
				int *stronghold_factor);

	//! Given the difficulty, and whether we're doing a hidden map,
	//see how many signposts should point to cities vs how many
	//do not.
	void getSignpostDifficulty (int difficulty, bool hidden_map, 
				    int *signpost_ratio);

	//! Given the difficulty, see how many army units every city
	//produces by default.  it is thought that more is easier because
	//players do not have to pay for more armies.
	void getCityDifficulty (int difficulty, int *number_of_armies_factor);

	//! Based on the difficulty, get how much gold each player should
	//start with.
	void getBaseGold (int difficulty, int *base_gold);


        //data
        //for map creation
        GameScenario* d_scenario;
        MapGenerator* d_generator;
        std::string d_tilesname;
        std::string d_shieldsname;
        std::string d_citysetname;
        int d_width;
        int d_height;
        bool d_turnmode;
};

#endif  //CREATE_SCENARIO_H
