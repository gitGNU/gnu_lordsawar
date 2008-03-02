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

#ifndef TILESET_H
#define TILESET_H

#include <string>
#include <vector>
#include <SDL.h>
#include <sigc++/trackable.h>

#include "Tile.h"

class XML_Helper;

//! A list of Tile objects in a terrain theme.
/** 
 * Tileset is a list of Tile objects.  It acts as the themeing mechanism for
 * the look (and partially the behaviour) of terrain objects in the game.
 * The Tileset dictates the pixel size of the tiles, and is used to lookup
 * Tile and TileStyle objects.  It is implemented as a singleton because many 
 * classes use it for looking up Tile and TileStyle objects.
 * 
 * Tileset objects are often referred to by their subdirectory (Tileset::d_dir).
 *
 * Tileset objects reside on disk in the tilesets/ directory, each of which is
 * inside it's own directory.
 *
 * The tileset configuration file is a same named XML file inside the Tileset's
 * directory.  E.g. tilesets/${Tileset::d_dir}/${Tileset::d_dir}.xml.
 */
class Tileset : public sigc::trackable, public std::vector<Tile*>
{
    public:
	//! Loading constructor.
	/**
	 * Make a new Tileset object by loading the data from a tileset
	 * configuration file.
	 *
	 * @param helper  The opened tileset configuration file to load the
	 *                tileset from.
	 */
        Tileset(XML_Helper* helper);
	//! Destructor.
        ~Tileset();

	//! Return the subdirectory of this Tileset.
        std::string getSubDir() const {return d_dir;}

	//! Set the subdirectory of where this Tileset resides on disk.
        void setSubDir(std::string dir) {d_dir = dir;}

        //! Returns the name of the tileset.
        std::string getName() const {return d_name;}

        //! Returns the description of the tileset.
        std::string getInfo() const {return d_info;}

        //! Returns the tilesize of the tileset.
        Uint32 getTileSize() const {return d_tileSize;}

        //! Returns the index to the given terrain type.
        Uint32 getIndex(Tile::Type type) const;

	//! Load the graphics for every Tile in the Tileset.
	/**
	 * Loads the images associated with every TileStyle that is associated 
	 * with every Tile of this Tileset.
	 *
	 * @note SDL must be initialized before this method is called.
	 */
	void instantiatePixmaps();

	//! Lookup tilestyle by it's id in this tileset.
	TileStyle *getTileStyle(Uint32 id) {return d_tilestyles[id];}

	//! Lookup a random tile style.
	/**
	 * Scan the TileStyles for the given Tile (given by index) for a
	 * TileStyle that matches the given style.  When there is more than
	 * one TileStyle to choose from, randomly pick one from all of the 
	 * matching TileStyle objects.
	 *
	 * @param index  The index of the Tile in this set to operate on.
	 * @param style  The kind of style we're looking for.
	 *
	 * @return A pointer to the matching TileStyle object, or NULL if no 
	 *         TileStyle could be found with that given style.
	 */
	TileStyle *getRandomTileStyle(Uint32 index, TileStyle::Type style);

    private:
        //! Callback to load Tile objects into the Tileset.
        bool loadTile(std::string, XML_Helper* helper);

        // DATA
	//! The name of the Tileset.
	/**
	 * Equates to the tileset.d_name XML entity in the tileset 
	 * configuration file.
	 * This value appears in dialogs where the user is asked to select
	 * a Tileset among all other Tileset objects available to the game.
	 */
        std::string d_name;

	//! The description of the Tileset.
	/**
	 * Equates to the tileset.d_info XML entity in the tileset
	 * configuration file.
	 * This value is not used.
	 */
        std::string d_info;

	//! The size of the graphic tiles in the Tileset.
	/**
	 * Equates to the tileset.d_tilesize XML entity in the tileset
	 * configuration file.
	 * It represents the size in pixels of the width and height of tile
	 * imagery onscreen.
	 */
        Uint32 d_tileSize;

	//! The subdirectory of the Tileset.
	/**
	 * This is the name of the subdirectory that the Tileset files are
	 * residing in.  It does not contain a path (e.g. no slashes).
	 * Tileset directories sit in the tileset/ directory.
	 */
        std::string d_dir;

        typedef std::map<Uint32, TileStyle*> TileStyleIdMap;
	//! A map that provides a TileStyle when supplying a TileStyle id.
        TileStyleIdMap d_tilestyles;
};

#endif // TILESET_H

// End of file
