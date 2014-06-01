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

#include <list>
#include <glibmm.h>

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
	static Glib::ustring getUserArmysetDir();

	//! Get the directory where system armysets live.
	static Glib::ustring getArmysetDir();

	//! Get the directory where personal tilesets live.
	static Glib::ustring getUserTilesetDir();

	//! Get the directory where system tilesets live.
	static Glib::ustring getTilesetDir();

	//! Get the directory where personal shieldsets live.
	static Glib::ustring getUserShieldsetDir();

	//! Get the directory where system shieldsets live.
	static Glib::ustring getShieldsetDir();

	//! Get the directory where personal citysets live.
	static Glib::ustring getUserCitysetDir();

	//! Get the directory where system citysets live.
	static Glib::ustring getCitysetDir();

        //! load misc file, e.g. hero names 
        static Glib::ustring getMiscFile(Glib::ustring filename);

        //! load an xslt file.
        static Glib::ustring getXSLTFile(guint32 type, Glib::ustring old_version, Glib::ustring new_version);
        
        //! Load the xml file describing the items
        static Glib::ustring getItemDescription();
        
        //! Get the path to an editor image
	static Glib::ustring getEditorFile(Glib::ustring filename);
    
        // Returns the filename of a music file (description or actual piece)
        static Glib::ustring getMusicFile(Glib::ustring filename);
        
        // get save game path
        static Glib::ustring getSavePath();

	//! get game data path
	static Glib::ustring getDataPath();

	//! the location of the system directory that holds scenario terrains.
	static Glib::ustring getMapDir();

	//! the location of the system directory that holds personal terrains.
	static Glib::ustring getUserMapDir();

	//! get the path of a system scenario file called file.
	static Glib::ustring getMapFile(Glib::ustring file);

	//! get the path of a personal scenario called file.
	static Glib::ustring getUserMapFile(Glib::ustring file);

        static Glib::ustring getUserProfilesDescription();
        static Glib::ustring getUserRecentlyPlayedGamesDescription();
        static Glib::ustring getUserRecentlyHostedGamesDescription();
        static Glib::ustring getUserRecentlyAdvertisedGamesDescription();
        static Glib::ustring getUserRecentlyEditedFilesDescription();

        static Glib::ustring getUIFile(Glib::ustring file);
        static Glib::ustring getEditorUIFile(Glib::ustring file);
        // get the available scenarios
        static std::list<Glib::ustring> scanMaps();

	// get the available scenarios in the user's personal collection
	static std::list<Glib::ustring> scanUserMaps();


	//! Copy a file from one place to another.
	static int copy (Glib::ustring from, Glib::ustring to);

	//! make a directory if it doesn't already exist.
	static bool create_dir(Glib::ustring dir);

	//! simple basename routine, but also strips the file extension.
	static Glib::ustring get_basename(Glib::ustring path, bool keep_ext=false);

	//! is a file writable?
	static bool is_writable(Glib::ustring path);

	//! does a file exist?
	static bool exists(Glib::ustring f);

	//! does filename end with extension?
	static bool nameEndsWith(Glib::ustring filename, Glib::ustring extension);

	//! delete a file from the filesystem.
	static void erase(Glib::ustring filename);

	//! delete an empty directory from the filesystem.
	static void erase_dir(Glib::ustring filename);

        //! delete a directory and the files it contains from the filesystem.
        static void clean_dir(Glib::ustring filename);

	static Glib::ustring add_slash_if_necessary(Glib::ustring dir);

	static Glib::ustring getSetConfigurationFilename(Glib::ustring dir, Glib::ustring subdir, Glib::ustring ext);

	static Glib::ustring get_dirname(Glib::ustring path);

        static std::list<Glib::ustring> scanForFiles(Glib::ustring dir, Glib::ustring extension);

        static Glib::ustring add_ext_if_necessary(Glib::ustring file, Glib::ustring ext);

        static char *sanify(const char *string);

        static Glib::ustring get_tmp_file(Glib::ustring ext = "");

        static Glib::ustring get_extension(Glib::ustring filename);
};

bool case_insensitive (const Glib::ustring& first, const Glib::ustring& second);

#endif //FILE_H

// End of file
