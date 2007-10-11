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
  * First, it makes a map of all the separate land areas and seas.
  * This is stored in the "array d_l_mass" which has a 
  * similar format to the terain map.  Each location has the id of
  * land/sea to which the tile is part of.  All seas have an
  * id less than zero and all land masses have an id greater than
  * zero.
  * 
  * The other potentially useful structure contains all the 
  * routes between landmasses to each other including the id
  * of the sea that must be crossed to get to the land mass 
  * with the destination id.  This is currently stored in a
  * vector.
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
        void makeMap(int width, int height);

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
        bool tryToPlacePort(int px, int py);
        
        /** Tries to place an arbitrary port at the landmass at (x,y)
          *
          * This function checks which seas bordering the continent don't
          * have a port yet and tries to put one at one of the seas. Its work
          * is derived from the continent function (to find out which continent
          * we are) and findRoutes (to find out which seas exist)
          *
          * @param x            x coordinate of continent
          * @param y            y coordinate of the continent
          * @param city_count   reference which is increased with each placed city
          */
        void placePorts(int x,int y, int& city_count);

        /** Moves along a coast in a clock wise direction
          *
          * This function assumes that it is initially located at a coast with
          * the sea of the appropriate id. It then walks a given number of steps
          * along the coast of that sea. All the time, (x,y) always points to the
          * land part of the coast.
          *
          * @param x            x coordinate; modified as we move along
          * @param y            y coordinate; the same
          * @param sea_id       the id of the sea we move around
          * @param dist         the number of steps to take
          * @param lastwat      indicates the direction of the "last" water tile
          *                     only neccessary to supply for strange cases
          * @return false if we had to abort (e.g. because the map border was met)
          */
        bool walkCoast(int& x,int& y, int sea_id, int dist, int& lastwat);
        
        //! The same as walkCoast, but goes anticlockwise.
        bool walkCoastAntiClock(int& x,int& y, int sea_id, int dist, int& lastwat);
        
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

        /** Populates the d_l_mass array
          *
          * To ensure that all continents are connected by ports, we first need
          * to find out where the continents/seas actually are. This function
          * assigns a positive id to separate continents and a negative to
          * separate seas.
          *
          * @param nmrLands     set to the number of continents found
          * @param nmrSeas      set to the number of seas found
          */
        void continents(int& nmrLands, int&  nmrSeas);
        
        /** Populates the d_portneed vector
          * 
          * To ensure that each continent has at least one port at each sea,
          * we need to specify which continents have to have ports approximately
          * where. This is saved in the d_portneed vector and specified here.
          */
        void findRoutes();

        //Data
        int d_xdir[8];
        int d_ydir[8];
        Tile::Type* d_terrain; // the map of the terrain
        Maptile::Building* d_building;
        int* d_l_mass;//  this maps out seperate seas and land areas
        int d_width;
        int d_height;
        int d_pswamp, d_pwater, d_pforest, d_phills, d_pmountains;
        unsigned int d_nocities, d_notemples, d_noruins, d_nosignposts;
        vecs2d d_portneed;//the seas a landmass needs to have a port on
};

#endif

// End of file
