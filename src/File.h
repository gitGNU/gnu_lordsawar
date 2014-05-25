// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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
	//! Get the directory where personal armysets live.
	static std::string getUserArmysetDir();

	//! Get the directory where system armysets live.
	static std::string getArmysetDir();

	//! Get the directory where personal tilesets live.
	static std::string getUserTilesetDir();

	//! Get the directory where system tilesets live.
	static std::string getTilesetDir();

	//! Get the directory where personal shieldsets live.
	static std::string getUserShieldsetDir();

	//! Get the directory where system shieldsets live.
	static std::string getShieldsetDir();

	//! Get the directory where personal citysets live.
	static std::string getUserCitysetDir();

	//! Get the directory where system citysets live.
	static std::string getCitysetDir();

        //! load misc file, e.g. hero names 
        static std::string getMiscFile(std::string filename);

        //! load an xslt file.
        static std::string getXSLTFile(guint32 type, std::string old_version, std::string new_version);
        
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

	//! the location of the system directory that holds scenario terrains.
	static std::string getMapDir();

	//! the location of the system directory that holds personal terrains.
	static std::string getUserMapDir();

	//! get the path of a system scenario file called file.
	static std::string getMapFile(std::string file);

	//! get the path of a personal scenario called file.
	static std::string getUserMapFile(std::string file);

        static std::string getUserProfilesDescription();
        static std::string getUserRecentlyPlayedGamesDescription();
        static std::string getUserRecentlyHostedGamesDescription();
        static std::string getUserRecentlyAdvertisedGamesDescription();
        static std::string getUserRecentlyEditedFilesDescription();

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

	//! does filename end with extension?
	static bool nameEndsWith(std::string filename, std::string extension);

	//! delete a file from the filesystem.
	static void erase(std::string filename);

	//! delete an empty directory from the filesystem.
	static void erase_dir(std::string filename);

        //! delete a directory and the files it contains from the filesystem.
        static void clean_dir(std::string filename);

	static std::string add_slash_if_necessary(std::string dir);

	static std::string getSetConfigurationFilename(std::string dir, std::string subdir, std::string ext);

	static std::string get_dirname(std::string path);

        static std::list<std::string> scanForFiles(std::string dir, std::string extension);

        static std::string add_ext_if_necessary(std::string file, std::string ext);

        static char *sanify(const char *string);

        static std::string get_tmp_file();

        static std::string get_extension(std::string filename);
};

bool case_insensitive (const std::string& first, const std::string& second);

#endif //FILE_H

// End of file
