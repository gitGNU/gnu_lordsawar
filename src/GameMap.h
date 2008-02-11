// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <sigc++/trackable.h>

#include "vector.h"
#include "rectangle.h"
#include "maptile.h"
#include "stack.h"
#include "Location.h"
#include "shieldset.h"
#include "cityset.h"

class MapGenerator;
class XML_Helper;

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

        //! Get the tile object at position (x,y)
        Maptile* getTile(int x, int y) const;

        //! Alternative access
        Maptile* getTile(Vector<int> p) const {return getTile(p.x, p.y);}

	//! Add an army to this location on the map
	Stack* addArmy(Location *l, Army *a);
        Stack* addArmy(Vector<int> pos, Army *a);

	//! Go find a player's planted standard on the map
        Vector<int> findPlantedStandard(Player *p);

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
        bool fill(Uint32 type);

        /** Save the contents of the map
          * 
          * @param helper           see XML_Helper for more information
          * @return true if saving went well, false otherwise
          */
        bool save(XML_Helper* helper) const;

	//! figure out where a non-flying unit can't go
        void calculateBlockedAvenues();
	void calculateBlockedAvenue(int i, int j);

	/** Smooth a portion of the map.
	 *
	 * Give each tile in the prescribed area the preferred picture for 
	 * the underlying terrain tile.
	 */
	void applyTileStyles (int minx, int miny, int maxx, int maxy,
			      bool smooth_terrain);

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
        bool isDock(int x, int y);
	void close_circles (int minx, int miny, int maxx, int maxy);

	TileStyle *calculatePreferredStyle(int i, int j);
	void demote_lone_tile(int minx, int miny, int maxx, int maxy, 
			      Tile::Type intype, Tile::Type outtype);
	int tile_is_connected_to_other_like_tiles (Tile::Type tile, int i, 
						   int j);

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
