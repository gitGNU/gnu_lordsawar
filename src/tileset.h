// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008 Ben Asselstine
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
	//! Default constructor.
	/**
	 * Make a new Tileset.
	 *
	 * @param name  The name of the Tileset.  Analagous to Tileset::d_name.
	 */
	Tileset(std::string name);

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
        void setSubDir(std::string dir);

        //! Returns the name of the tileset.
        std::string getName() const {return d_name;}

	//! Set the name of the tileset.
	/**
	 * @note This method is only used in the tileset editor.
	 */
        void setName(std::string name) {d_name = name;}

        //! Returns the description of the tileset.
        std::string getInfo() const {return d_info;}

	//! Set the description of the tileset.
	/**
	 * @note This method is only used in the tileset editor.
	 */
        void setInfo(std::string info) {d_info = info;}

        //! Returns the tilesize of the tileset.
        Uint32 getTileSize() const {return d_tileSize;}

	//! Sets the tilesize of the tileset.
	void setTileSize(Uint32 tileSize) {d_tileSize = tileSize;}

        //! Returns the index to the given terrain type.
        Uint32 getIndex(Tile::Type type) const;

	void setLargeSelectorFilename(std::string p){d_large_selector = p;};
	void setSmallSelectorFilename(std::string p){d_small_selector = p;};
	std::string getLargeSelectorFilename() {return d_large_selector;};
	std::string getSmallSelectorFilename() {return d_small_selector;};

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

	//! Save a Tileset to an opened tile configuration file.
	/**
	 * @param  The opened XML tile configuration file.
	 */
	bool save(XML_Helper *helper);

	Tile *lookupTileByName(std::string name);

	int getFreeTileStyleId();

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

	std::string d_small_selector;
	std::string d_large_selector;

        typedef std::map<Uint32, TileStyle*> TileStyleIdMap;
	//! A map that provides a TileStyle when supplying a TileStyle id.
        TileStyleIdMap d_tilestyles;

};

#endif // TILESET_H

// End of file
