//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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
#include "setlist.h"

class Tar_Helper;
class SmallTile;

//! A list of all Tileset objects available to the game.
/**
 * This class contains a list of all Tileset objects available to the game. 
 * Since several classes access this class, it is implemented as a singleton.
 *
 * Tileset objects are usually referenced by the name of the subdirectory
 * in which they reside on disk (inside the tilesets/ directory).
 */
class Tilesetlist : public std::list<Tileset*>, public sigc::trackable, public SetList
{
    public:

	// Methods that operate on class data but do not modify the class.

        //! Returns the names of all tilesets available to the game.
	std::list<std::string> getValidNames() const;

        //! Returns whether the given name is in the list of tilesets.
        bool contains(std::string name) const;

        //! Returns the names of tilesets that have the given tile size.
	std::list<std::string> getValidNames(guint32 tilesize) const;

        //! Returns the different tilesizes present in the tilesetlist.
	void getSizes(std::list<guint32> &sizes) const;

	//! Return the name of the subdirectory for a given tileset.
        /** 
         * @param tileset       The name of the tileset to get the subdir of.
	 * @param tilesize      The size of the tileset to get the subdir of.
	 *
         * @return The name of the directory that holds the tileset.  See 
	 *         Tileset::d_dir for more information about the nature of 
	 *         the return value.
         */
	std::string getTilesetDir(std::string name, guint32 tilesize) const;

	//! Return the Tileset object by the name of the subdir.
	/**
	 * @param dir  The directory where the Tileset resides on disk.
	 *             This value does not contain any slashes, and is
	 *             presumed to be found inside the tilesets/ directory.
	 */
	Tileset *getTileset(std::string dir) const;

	//! Return the Tileset object by the id.
	/**
	 * @param id   A unique numeric identifier that identifies the tileset
	 *             among all tilesets in the tilesetlist.
	 */
	Tileset *getTileset(guint32 id) const;

        //! get the id of the given tileset basename.
        guint32 getTilesetId(std::string basename) const;

        SmallTile *getSmallTile(std::string basename, Tile::Type type) const;

        Gdk::RGBA getColor(std::string basename, Tile::Type type) const;

        std::string findFreeBaseName(std::string basename, guint32 max, guint32 &num) const;

	// Methods that operate on the class data and modify the class.

	//! Add a tileset to the list.  Use this instead of push_back.
	void add(Tileset *tileset, std::string file);

	//! Destroy all of the tileset images in this list.
	void uninstantiateImages();
        
        bool reload(guint32 tileset_id);

	//! Load the images for all tilesets in this list.
	void instantiateImages(bool &broken);


	//! Add the given tileset to the list, and copy files into place.
	/**
	 * This method tries hard to add the tileset to this list.  The subdir
	 * name could be changed, or the id might also be changed so that it
	 * doesn't conflict with any other tilesets in the list.
	 *
	 * @return Returns true if it was added successfully, and the
	 *         new_subdir and new_id parameters updated to reflect the
	 *         changed subdir and id.
	 */
	bool addToPersonalCollection(Tileset *tileset, std::string &new_subdir, guint32 &new_id);
	Tileset *import(Tar_Helper *t, std::string f, bool &broken);

	// Static Methods

        //! Return the singleton instance of this class.
        static Tilesetlist* getInstance();

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

	//! Return an unused tileset number.
	static int getNextAvailableId(int after = 0);

    private:
        //! Default constructor.  Loads all tilesets it can find.
	/**
	 * The tilesets/ directory is scanned for Tileset directories.
	 */
        Tilesetlist();
        
        //! Destructor.
        ~Tilesetlist();

        //! Loads a specific Tileset.
	/**
	 * Load the Tileset from an tileset configuration file and add it to 
	 * this list of Tileset objects.
	 *
	 * @param name  The name of the subdirectory that the Tileset resides 
	 *              in.
	 *
	 * @return the Tileset.  NULL otherwise.
	 */
        Tileset* loadTileset (std::string filename);

	//! Load the given tilesets into the list.
	void loadTilesets(std::list<std::string> tilesets);
        
	std::list<std::string> getNames() const;
	std::list<std::string> getNames(guint32 tilesize) const;
	// DATA

        typedef std::map<std::string, std::string> DirMap;
        typedef std::map<std::string, Tileset*> TilesetMap;
        typedef std::map<guint32, Tileset*> TilesetIdMap;

	//! A map that provides a subdirectory when supplying a Tileset name.
        DirMap d_dirs;

	//! A map that provides a Tileset when supplying a subdirectory name.
        TilesetMap d_tilesets;

	//! A map that provides a Tileset when supplying a tileset id.
        TilesetIdMap d_tilesetids;

        //! A static pointer for the singleton instance.
        static Tilesetlist* s_instance;
};

#endif // TILESETLIST_H

