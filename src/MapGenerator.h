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

#ifndef MAPGENERATOR_H
#define MAPGENERATOR_H

#include <string>
#include <vector>

// we need the enums from these classes
#include "Tile.h"
#include "maptile.h"

struct portneeded;
typedef  std::vector<portneeded> vecports;
typedef  std::vector<vecports> vecs2d;

/** This class creates a map including the buildings (cities, ruins, temples).
  * It does NOT care about player setup and such. This is done in a second step
  * by the CreateScenario class. The parameters for map creation are set by
  * GamePreferencesDialog::fillData().
  * GamePreferencesDialog does use the MapConfDialog but this is
  * just a skeleton - GamePreferencesDialog does all the work.
  *
  * The data is stored in two variables:
  *   - the terrain map stores the terrain information
  *   - the building map stores the information about building placement
  *
  * For historical reasons *cough*, the maps are long character arrays. The
  * definition of the single characters can be found further down; missing
  * buildings are denoted by a space. As a normal user, you will propably never
  * need to worry about the format anyway. It is read and "translated" by the
  * CreateScenario class which kind of supervises the scenario creation.
  *
  * Map generators with different styles (e.g. islands), derived from
  * this class are possible and will hpefully be done in the
  * future. 
  * 
  * When designating sites for cities, it takes care to ensure that 
  * the first cities on any land mass will be ports on those seas
  * that connect with other land masses and islands. In doing so, it
  * creates two data structures that may be useful elsewhere.
  * 
  */


class MapGenerator
{
    public :
        MapGenerator();
        ~MapGenerator();

        //! Set the number of cities
        int setNoCities(int nocities);

        //! Set the number of ruins
        int setNoRuins(int noruins);

        //! Set the number of ruins
        int setNoSignposts(int nosignposts);

        //! Set the number of temples
        int setNoTemples(int notemples);

        /** Set terrain distribution. If the sum of the percentages is less than
          * 100, the rest is filled with grass.
          */
        void setPercentages(int pwater, int pforest, int pswamp,
                            int phills, int pmountains);

        
        //! Get number of cities
        int getNoCities() const {return d_nocities;}

        //! Get number of ruins
        int getNoRuins() const {return d_noruins;}

        //! Get number of signposts
        int getNoSignposts() const {return d_nosignposts;}

        //! Get number of temples
        int getNoTemples() const {return d_notemples;}
        
        
        /** Creates a map
          * 
          * Use this function to start off the map generation. This
          * implementation will first distribute the terrain in random patches,
          * normalize it and then distribute the buildings.
          *
          * @param width        the width of the map to be generated
          * @param height       the height of the map to be generated
          */
        void makeMap(int width, int height, bool roads);

        /** Get the array for the terrain map (shallow copy)
          * 
          * @param width        is set to the width of the generated map
          * @param height       is set to the height of the generated map
          * @return char array which represents the terrain map
          */
        const Tile::Type* getMap(int& width, int& height) const;
        
        /** Get the buildings map (shallow copy)
          * 
          * @param width        is set to the width of the generated map
          * @param height       is set to the height of the generated map
          * @return char array which represents the building map
          */
        const Maptile::Building* getBuildings(int& width, int& height) const;

    protected:
        //! Fills the terrain map with grass
        void makePlains();

        /** Spreads terrain over the map
          * 
          * This function randomly places a given terrain type over the map.
          * It only "overwrites" grass tiles, so terrain is not repeatedly 
          * modified.
          *
          * @param t        terrain to be placed
          * @param percent  amount of tiles to be modified (percentage of the
          *                 whole map)
          * @param contin   if set to true (used for water tiles), the algorithm
          *                 tries to create continuous areas instead of leaving
          *                 patches when it hits a dead end.
          */
        void makeTerrain(Tile::Type t, int percent, bool contin);
        void makeStreamer(Tile::Type type, int percent, int width);

        /** Tries to find the nearest grass tile from a given location.
          *
          * This function becomes e.g. neccessary if you want to create 
          * connected water areas, but the algorithm in makeTerrain() gets
          * stuck.
          *
          * @param x    the x position where we are to look
          * @param y    the y position where we are to look
          *
          * @note if the function finds a free place, it modifies x and y.
          * @return true if search succeeded.
          */
        bool seekPlain(int& x,int& y);

        /** Designates the places where the cities will be by setting characters 
          * on the building map.  Creates an island if it
          * finds only water or occupied places.
          *
          * @param cities       the number of cities to distribute
          */
        void makeCities(int cities);
        
        //! Returns true if position (x,y) is free for a city
        bool canPutCity(int x, int y);

        //! Places a city at a certain location
        void putCity(int x, int y, int& city_count);

        /** Designates the location of buildings across the map. Creates an island 
          * if there is only water or occupied places.
          *
          * @param b            the type of the building
          * @param building     the number of buildings to distribute
          */
        void makeBuildings(Maptile::Building b, int building);
        
        //! Returns true if position (x,y) is free for buildings
        bool canPutBuilding(int x, int y);
        
        /** Tries to place a city around a certain position.
          * 
          * This function is used internally for placing ports, that
          * is why some of the parameters may be a bit strange.
          * 
          * @param px           x coordinate of position
          * @param py           y coordinate
          * @param city_count   increased if the city is placed
          */
        bool tryToPlaceCity(int px, int py, int& city_count);

        
        /** Normalizes the terrain
          * 
          * With normalization, we mean that if a tile is surrounded mainly
          * by tiles different from it's own type, then there is a chance
          * that the tile in question is modified itself. This leads to
          * "better" terrain distribution.
          *
          * @note At the moment, only water is normalized
          */
        void normalize();

	bool makeRoad(int src_x, int src_y, int dest_x, int dest_y);
	bool makeRoad(Vector<int> src, Vector<int>dest);
	bool isAccessible(int src_x, int src_y, int dest_x, int dest_y);
	bool isAccessible(Vector<int> src, Vector<int> dest);
	bool makeAccessible(Vector<int> src, Vector<int> dest);
	bool makeAccessible(int src_x, int src_y, int dest_x, int dest_y);
	void makeRoads();
	bool placePort(int x, int y);
	void calculateBlockedAvenue(int x, int y);

        //Data
        int d_xdir[8];
        int d_ydir[8];
        Tile::Type* d_terrain; // the map of the terrain
        Maptile::Building* d_building;
        int d_width;
        int d_height;
        int d_pswamp, d_pwater, d_pforest, d_phills, d_pmountains;
        unsigned int d_nocities, d_notemples, d_noruins, d_nosignposts;
};

#endif

// End of file
