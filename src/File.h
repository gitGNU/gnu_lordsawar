// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#ifndef FILE_H
#define FILE_H

#include <string>
#include <list>

#include "xmlhelper.h"
class Armyset;
class Tileset;

/** \brief Miscellaneous functions for file access
  * 
  * These functions should be the sole way to access any files. They will
  * automatically prepend the correct directory, extract the correct file etc.
  * This enables us to install and package LordsAWar (there is no fixed
  * directory structure). To use these functions, you issue e.g. an armyset name
  * and have the full path returned, which is a file you can then load.
  */

class File
{
    public:
        /** Scan the system data directories for armysets
          * 
          * @return a list of available armysets
          */
        static std::list<std::string> scanArmysets();

	// get the available armysets in the user's personal collection
	static std::list<std::string> scanUserArmysets();

        /** Get the armyset description file
          *
          * @param armysetsubdir    the name of the armyset.
          * @return the full name of the description file
          */
        static std::string getArmyset(std::string armysetsubdir);

        /** Get the armyset description file from the user's personal collection
          *
          * @param armysetsubdir    the name of the armyset.
          * @return the full name of the description file
          */
	static std::string getUserArmyset(std::string armysetsubdir);

	//! Get the directory where personal armysets live.
	static std::string getUserArmysetDir();

	//! Get the directory where system armysets live.
	static std::string getArmysetDir();

	//! Get the director where the given armyset lives.
	static std::string getArmysetDir(Armyset *armyset);

	//! Gets the description file for the given armyset.
	static std::string getArmyset(Armyset *armyset);

	// get an path for a file belonging to the given armyset.
	static std::string getArmysetFile(Armyset *armyset, std::string pic);


        /** Scan the system data directories for tilesets
          * 
          * @return a list of available tilesets
          */
        static std::list<std::string> scanTilesets();

	// get the available tilesets in the user's personal collection
	static std::list<std::string> scanUserTilesets();

        /** Get the tileset description file
          *
          * @param tilesetsubdir    the dir name of the tileset.
          * @return the full name of the description file
          */
        static std::string getTileset(std::string tilesetsubdir);

        /** Get the tileset description file from the user's personal collection
          *
          * @param tilesetsubdir    the name of the tileset.
          * @return the full name of the description file
          */
	static std::string getUserTileset(std::string tilesetsubdir);

	//! Get the directory where personal tilesets live.
	static std::string getUserTilesetDir();

	//! Get the directory where system tilesets live.
	static std::string getTilesetDir();

	//! Get the director where the given tileset lives.
	static std::string getTilesetDir(Tileset *tileset);

	//! Gets the description file for the given armyset.
	static std::string getTileset(Tileset *tileset);

	// get a path of a file in the given Tileset.
	static std::string getTilesetFile(Tileset *tileset, std::string pic);

        /** Scan the system data directories for shieldsets 
          * 
          * @return a list of available shieldsets
          */
	static std::list<std::string> scanShieldsets();

        /** Get the shieldset description file
          *
          * @param shieldsetsubdir    the name of the shieldset
          * @return the full name of the shield description file
          */
        static std::string getShieldset(std::string shieldsetsubdir);

	//get a shieldset path
	static std::string getShieldsetFile(std::string shieldsetsubdir, std::string picname);
	
        /** Get the description file for the cityset
          * 
          * @param citysetsubdir     the name of the cityset
          * @return the full name of the description file
          */
        static std::string getCityset(std::string citysetsubdir);

	// get a cityset path
	static std::string getCitysetFile(std::string citysetsubdir, std::string picname);

	
        //! load misc file, e.g. hero names 
        static std::string getMiscFile(std::string filename);
        
        //! Load the xml file describing the items
        static std::string getItemDescription();
        
        //! Get the path to an editor image
	static std::string getEditorFile(std::string filename);
    
        // Returns the filename of a music file (description or actual piece)
        static std::string getMusicFile(std::string filename);
        
        // get save game path
        static std::string getSavePath();

	//! get game data path
	static std::string getDataPath();

        // get the available citysets
        static std::list<std::string> scanCitysets();

	//! the location of the system directory that holds scenario terrains.
	static std::string getMapDir();

	//! the location of the system directory that holds personal terrains.
	static std::string getUserMapDir();

	//! get the path of a system scenario file called file.
	static std::string getMapFile(std::string file);

	//! get the path of a personal scenario called file.
	static std::string getUserMapFile(std::string file);

        // get the available scenarios
        static std::list<std::string> scanMaps();

	// get the available scenarios in the user's personal collection
	static std::list<std::string> scanUserMaps();


	//! Copy a file from one place to another.
	static int copy (Glib::ustring from, Glib::ustring to);

	//! make a directory if it doesn't already exist.
	static bool create_dir(std::string dir);

	//! simple basename routine, but also strips the file extension.
	static std::string get_basename(std::string path, bool keep_ext=false);

	//! is a file writable?
	static bool is_writable(std::string path);

	//! does a file exist?
	static bool exists(std::string f);

};

#endif //FILE_H

// End of file
