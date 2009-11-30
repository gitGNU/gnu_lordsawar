// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009 Ben Asselstine
// Copyright (C) 2008 Janek Kozicki
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

#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <sigc++/trackable.h>

#include <vector>

#include <gtkmm.h>
#include "vector.h"
#include "rectangle.h"
#include "maptile.h"
#include "stack.h"
#include "Location.h"
#include "shieldset.h"
#include "cityset.h"

class MapGenerator;
class XML_Helper;
class Port;
class Road;
class City;
class Temple;
class Bridge;
class Ruin;
class Armyset;

/** Class representing the map in the game
  * 
  * GameMap represents a single map. In most cases this will be the map that is
  * currently played, but it might be used to preview a map in a mapeditor too.
  * Notes: GameMap was prefered over Map, because of the potential confusion 
  * with the std::map from the STL.
  * In a previous design this has been a global variable, now it's a singleton.
  * So if you need access to the game map, use one of the getInstance functions
  * to get the singleton instance.
  */

class GameMap: public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static std::string d_tag; 

	//! The xml tag of the itemstack subobject in a saved-game file.
	static std::string d_itemstack_tag; 

        /** Singleton function to get the GameMap instance
          * 
          * @return singleton instance
          */
        static GameMap* getInstance();

        /** Returns singleton instance or creates a new one using the tileset
          * 
          * @param TilesetName      the name of the tileset to be used
          * @return singleton instance
          */
        static GameMap* getInstance(std::string TilesetName,
				    std::string Shieldsetname,
				    std::string Citysetname);

        /** Creates a new singleton instance from a savegame file
          * 
          * @param helper           see XML_Helper for an explanation
          *
          * \note This function deletes an existing instance!
          */
        static GameMap* getInstance(XML_Helper* helper);

        //! Explicitely deletes the singleton instance
        static void deleteInstance();

        //! Set the width of the game map
        static void setWidth(int width){s_width = width;}
 
        //! Set the height of the game map
        static void setHeight(int height){s_height = height;}

        
        //! Returns the width of the map
        static int getWidth() {return s_width;}
 
        //! Returns the height of the map
        static int getHeight() {return s_height;}

	static Vector<int> get_dim() { return Vector<int>(s_width, s_height); }
	
	static Rectangle get_boundary()
	    { return Rectangle(0, 0, s_width, s_height); }

        //! Returns the tileset in use
        Tileset* getTileset() const {return d_tileSet;}

        //! Returns the shieldset in use
        Shieldset* getShieldset() const {return d_shieldSet;}

        //! Returns the tileset in use
        Cityset* getCityset() const {return d_citySet;}

	//! Sets the tile object at position (x, y)
	void setTile(int x, int y, Maptile *tile);
        
        //! Alternative setting
        void setTile(Vector<int> p, Maptile *t) {return setTile(p.x, p.y, t);}

	static City* getCity(Vector<int> pos);
	static City* getEnemyCity(Vector<int> pos);
	static Ruin* getRuin(Vector<int> pos);
	static Temple* getTemple(Vector<int> pos);
	static Port* getPort(Vector<int> pos);
	static Road* getRoad(Vector<int> pos);
	static Bridge* getBridge(Vector<int> pos);
	static Signpost* getSignpost(Vector<int> pos);
	static Stack* getStack(Vector<int> pos);
	static StackTile* getStacks(Vector<int> pos);
	static bool canJoin(Stack *src, Stack *dest);
	static Stack* getFriendlyStack(Vector<int> pos);
	static Stack* getEnemyStack(Vector<int> pos);
	static MapBackpack *getBackpack(Vector<int> pos);

        //! Get the tile object at position (x,y)
        Maptile* getTile(int x, int y) const;

        //! Alternative access
        Maptile* getTile(Vector<int> p) const {return getTile(p.x, p.y);}

	//! Add an army to this location on the map
	Stack* addArmy(Location *l, Army *a);
        Stack* addArmy(Vector<int> pos, Army *a);

	//! Go find a player's planted standard on the map
        Vector<int> findPlantedStandard(Player *p);

	//! go find the player's stack, the slow way.
	Vector<int> findStack(guint32 id);

        /** Fill the map using the data supplied by a map generator
          * 
          * @param generator        the generator which supplies the data
          * @return true on success, false on error
          */
        bool fill(MapGenerator* generator);

        /** Fills the whole map with a single terrain.
          *
          * @param type             the type of the terrain(index in the tileset)
          * @return true on success, false on error
          */
        bool fill(guint32 type);

        /** Save the contents of the map
          * 
          * @param helper           see XML_Helper for more information
          * @return true if saving went well, false otherwise
          */
        bool save(XML_Helper* helper) const;

	//! figure out where a non-flying unit can't go
        void calculateBlockedAvenues();
	void calculateBlockedAvenue(int i, int j);
	void updateStackPositions();

	/** Smooth a portion of the map.
	 *
	 * Give each tile in the prescribed area the preferred picture for 
	 * the underlying terrain tile.
	 */
	void applyTileStyles (Rectangle r, bool smooth_terrain);
	void applyTileStyles (int minx, int miny, int maxx, int maxy,
			      bool smooth_terrain);
	void applyTileStyle (int i, int j);

	void surroundMountains(int minx, int miny, int maxx, int maxy);
	//! Get the positions of all of the items on the game map (in bags).
	std::vector<Vector<int> > getItems();

	Vector<int> findNearestObjectToTheNorth(Vector<int> pos);
	Vector<int> findNearestObjectToTheSouth(Vector<int> pos);
	Vector<int> findNearestObjectToTheEast(Vector<int> pos);
	Vector<int> findNearestObjectToTheWest(Vector<int> pos);

	void switchArmysets(Armyset *armyset);
	void switchCityset(Cityset *cityset);
	void switchShieldset(Shieldset *shieldset);
	void switchTileset(Tileset *tileset);

	bool moveBuilding(Vector<int> from, Vector<int> to);
	bool canPutBuilding(Maptile::Building bldg, guint32 size, Vector<int> to, bool making_islands = true);
	bool canPutStack(guint32 size, Player *p, Vector<int> to);
	bool moveStack(Stack *stack, Vector<int> to);

	void moveBackpack(Vector<int> from, Vector<int> to);
	guint32 getBuildingSize(Vector<int> tile);
	Maptile::Building getBuilding(Vector<int> tile);
	Tile::Type getTerrainType(Vector<int> tile);
	void setBuilding(Vector<int> tile, Maptile::Building building);

	bool putCity(City *c);
	bool removeCity(Vector<int> pos);
	bool putRuin(Ruin *r);
	bool removeRuin(Vector<int> pos);
	bool putTemple(Temple *t);
	bool removeTemple(Vector<int> pos);
	bool putRoad(Road *r);
	bool removeRoad(Vector<int> pos);
	bool putBridge(Bridge *b);
	bool removeBridge(Vector<int> pos);
	bool putSignpost(Signpost *s);
	bool removeSignpost(Vector<int> pos);
	bool putPort(Port *p);
	bool removePort(Vector<int> pos);
	bool putStack(Stack *s);
	void removeStack(Stack *s);

	Location *getLocation(Vector<int> pos);


    protected:
        //! Create the map with the given tileset
        GameMap(std::string TilesetName, std::string ShieldsetName,
		std::string Citysetname);

        //! Load the map using the given XML_Helper
        GameMap(XML_Helper* helper);

        ~GameMap();
        
    private:
        //! Callback for item loading used during loading.
        bool loadItems(std::string tag, XML_Helper* helper);
        Stack* addArmyAtPos(Vector<int> pos, Army *a);
        bool isBlockedAvenue(int x, int y, int destx, int desty);
        bool isDock(Vector<int> pos);
	void close_circles (int minx, int miny, int maxx, int maxy);
	void processStyles(std::string styles, int chars_per_style);
	int determineCharsPerStyle(std::string styles);

	TileStyle *calculatePreferredStyle(int i, int j);
	void demote_lone_tile(int minx, int miny, int maxx, int maxy, 
			      Tile::Type intype, Tile::Type outtype);

	int tile_is_connected_to_other_like_tiles (Tile::Type tile, int i, 
						   int j);
	bool are_those_tiles_similar(Tile::Type outer_tile,Tile::Type inner_tile, bool checking_loneliness);
	Vector<int> findNearestObjectInDir(Vector<int> pos, Vector<int> dir);
	void putBuilding(LocationBox *b, Maptile::Building building);
	void removeBuilding(LocationBox *b);

	void updateShips(Vector<int> pos);

	bool putTerrain(Rectangle r, Tile::Type type);

	bool offmap(int x, int y);

        // Data
        static GameMap* s_instance;
        static int s_width;
        static int s_height;

        Tileset* d_tileSet;
        Shieldset* d_shieldSet;
        Cityset* d_citySet;
        Maptile** d_map;
};

#endif

// End of file
