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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef CREATE_SCENARIO_H
#define CREATE_SCENARIO_H

#include <string>
#include <vector>
#include <SDL.h>

class MapGenerator;
class GameScenario;
class Player;

/** \brief Creates and dumps (i.e. saves) a scenario.
  * 
  * The purpose of this class is to create a new scenario which can then be
  * "loaded" by the game scenario class. The advantage is that the scenario
  * creation can be concentrated in one specific class instead of being scattered
  * over different classes and all objects (city, player, stack...) only need
  * one "loading" constructor. The disavantage is that all significant changes
  * e.g. to the player class have to be reflected here, too.
  */

class CreateScenario
{
    public:
        //! This represents the class of the map (hills, islands etc.).
        enum MapType { NORMAL };
    
        /** The Constructor
          * 
          * @param uncle    the progress bar which displays the advance
          */
        CreateScenario();
        ~CreateScenario();

        // setters

        //! Set the type of the map to be created
        void setMaptype(MapType type);

        //! Set the terrain distribution; differences to 100% are grass; sum may exceed 100%
        void setPercentages(int pgrass, int pwater, int pforest, int pswamp,
                            int phills, int pmountains);

        //! Set the tileset of the map
        void setMapTiles(std::string tilesname);

        //! Set the number of cities on the map
        void setNoCities(int number);

        //! Set the number of ruins on the map
        void setNoRuins(int number);

        //! Set the number of signposts on the map
        void setNoSignposts(int number);

        //! Set the number of stones on the map
        void setNoStones (int number);

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

        //! Get the number of stones on the map
        int getNoStones () const;

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
        bool create();

        /** Dumps (Saves) the map
          * 
          * @param filename     the full name of the save file
          * 
          * This will do the same as GameScenario::save() does (in fact it calls
          * GameScenario::save)
          */
        bool dump(std::string filename) const;

    private:
        //! Creates the map and distributes cities, temples and ruins
        bool createMap();

        //! Distributes the players over the map
        bool distributePlayers();

        //! Setup city names and production
        bool setupCities();

        //! Setup temple names
        bool setupTemples();
        
        //! Set up ruin names and keepers
        bool setupRuins();

        //! Set up signposts
        bool setupSignposts();

        //! Do some setup concerning the players (grant each a hero etc.)
        bool setupPlayers();

        /** Loads a list of possible city, ruin or temple names
          * 
          * @param list     the list to be filled with the names
          * @param namefile an ifstream of the file containing the names
          */
        bool loadNames(std::vector<std::string>& list, std::ifstream& namefile);

        //! Sets up some basic events to satisfy victory conditions
        bool setupEvents();


        //data
        //for map creation
        GameScenario* d_scenario;
        MapGenerator* d_generator;
        std::string d_tilesname;
        int d_width;
        int d_height;
        bool d_turnmode;

        //the namelists
        std::vector<std::string> d_citynames, d_signposts;
        std::vector<std::string> d_templenames, d_ruinnames;
};

#endif  //CREATE_SCENARIO_H
