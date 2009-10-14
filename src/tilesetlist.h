//  Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef TILESETLIST_H
#define TILESETLIST_H

#include <gtkmm.h>
#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"
#include "Tile.h"
#include "tileset.h"


//! A list of all Tileset objects available to the game.
/**
 * This class contains a list of all Tileset objects available to the game. 
 * Since several classes access this class, it is implemented as a singleton.
 *
 * Tileset objects are usually referenced by the name of the subdirectory
 * in which they reside on disk (inside the tilesets/ directory).
 */
class Tilesetlist : public std::list<Tileset*>, public sigc::trackable
{
    public:
        //! Return the singleton instance of this class.
        static Tilesetlist* getInstance();

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

        //! Returns the names of all tilesets available to the game.
	std::list<std::string> getNames();

        //! Returns the names of tilesets that have the given tile size.
	std::list<std::string> getNames(guint32 tilesize);

        //! Returns the different tilesizes present in the tilesetlist.
	void getSizes(std::list<guint32> &sizes);

	//! Return the name of the subdirectory for a given tileset.
        /** 
         * @param tileset       The name of the tileset to get the subdir of.
	 * @param tilesize      The size of the tileset to get the subdir of.
	 *
         * @return The name of the directory that holds the tileset.  See 
	 *         Tileset::d_dir for more information about the nature of 
	 *         the return value.
         */
	std::string getTilesetDir(std::string name, guint32 tilesize);

	//! Return the Tileset object by the name of the subdir.
	/**
	 * @param dir  The directory where the Tileset resides on disk.
	 *             This value does not contain any slashes, and is
	 *             presumed to be found inside the tilesets/ directory.
	 */
	Tileset *getTileset(std::string dir) { return d_tilesets[dir];}

    private:
        //! Default constructor.  Loads all tilesets it can find.
	/**
	 * The tilesets/ directory is scanned for Tileset directories.
	 */
        Tilesetlist();
        
        //! Destructor.
        ~Tilesetlist();

        //! Callback for loading Tileset objects into the Tilesetlist.
	bool load(std::string tag, XML_Helper *helper);

        //! Loads a specific Tileset.
	/**
	 * Load the Tileset from an tileset configuration file and add it to 
	 * this list of Tileset objects.
	 *
	 * @param name  The name of the subdirectory that the Tileset resides 
	 *              in.
	 *
	 * @return True if the Tileset could be loaded.  False otherwise.
	 */
        bool loadTileset (std::string name, bool from_private_collection);

	void loadTilesets(std::list<std::string> tilesets, bool priv);
        
        typedef std::map<std::string, std::string> DirMap;
        typedef std::map<std::string, Tileset*> TilesetMap;

	//! A map that provides a subdirectory when supplying a Tileset name.
        DirMap d_dirs;

	//! A map that provides a Tileset when supplying a subdirectory name.
        TilesetMap d_tilesets;

        //! A static pointer for the singleton instance.
        static Tilesetlist* s_instance;
};

#endif // TILESETLIST_H

