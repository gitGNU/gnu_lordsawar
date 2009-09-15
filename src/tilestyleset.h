//  Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef TILESTYLESET_H
#define TILESTYLESET_H

#include <string>
#include <vector>
#include <gtkmm.h>
#include <sigc++/trackable.h>

#include "tilestyle.h"

class XML_Helper;

/** 
 * TileStyleSet is an array of tilestyles (the look of terrain tile objects).
 * All of the TileStyles describe one of the many looks of a particular kind
 * of Tile.  e.g. `Forest'.
 * Every TileStyleSet belongs to a Tile.
 * The TileStyleSet images are located in on disk in the Tileset's directory.
 */
//! This class manages a set of TileStyle objects.
class TileStyleSet : public sigc::trackable, public std::vector<TileStyle*>
{
    public:

	//! The xml tag of this object in a tileset configuration file.
	static std::string d_tag; 

	//! The default constructor.
        TileStyleSet();

	//! The loading constuctor loads the TileStyleSet from the config file.
	/**
	 * Read the tileset.tile.tilestyleset XML entities in the tileset
	 * configuration file.
	 *
	 * @param helper  The opened tileset configuration file.
	 */
        TileStyleSet(XML_Helper* helper);

	//! Destructor.
        ~TileStyleSet();

	//! Get the name of this tilestyleset.
	/**
	 * Returns the text loaded from a tileset.tile.tilestyles.d_name
	 * XML entity of the tileset configuration flie.
	 */
	std::string getName() const {return d_name;}

	//! Set the name of this tilestyleset.
	void setName(std::string name) {d_name = name;}

	//! Save a TileStyleSet to an opened tile configuration file.
	/**
	 * @param  The opened XML tile configuration file.
	 */
	bool save(XML_Helper *helper);

	//! Return the subdirectory of this Tilestyleset.
        std::string getSubDir() const {return d_dir;};

	//! Set the subdirectory of where this Tilestyleset resides on disk.
        void setSubDir(std::string dir) {d_dir = dir;};

	void getUniqueTileStyleTypes(std::list<TileStyle::Type> &types);
	bool validate();
    private:

	//! The name of the tilestyleset.
	/**
	 * This is the basename of the image that contains a row of
	 * cells where each cell is tilesize pixels high, and tilesize
	 * pixels wide.  Each cell is another image of a tilestyle.  There is
	 * one cell per TileStyle in this TileStyleSet.
	 * The tilesize comes from the TileStyleSet::instantiateImages
	 * method.
	 * The name does not contain a path, and does not contain an
	 * extension (e.g. .png).  It must refer to a PNG file.
	 */
        std::string d_name;

	//! The directory of where the image file lives.
	/**
	 * @param This is a hack used for the tileset editor.
	 */
	std::string d_dir;
};

#endif // TILESTYLESET_H

// End of file
