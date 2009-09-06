// Copyright (C) 2008 Ben Asselstine
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

#ifndef CITYSETLIST_H
#define CITYSETLIST_H

#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"
#include "cityset.h"


//! A list of all Cityset objects available to the game.
/**
 * This class contains a list of all Cityset objects available to the game. 
 * Since several classes access this class, it is implemented as a singleton.
 *
 * Cityset objects are usually referenced by the name of the subdirectory
 * in which they reside on disk (inside the citysets/ directory).
 */
class Citysetlist : public std::list<Cityset*>, public sigc::trackable
{
    public:
        //! Return the singleton instance of this class.
        static Citysetlist* getInstance();

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

        //! Returns the names of all citysets available to the game.
	std::list<std::string> getNames();

        //! Returns the names of citysets that have the given tile size.
	std::list<std::string> getNames(guint32 tilesize);

        //! Returns the different tilesizes present in the citysetlist.
	void getSizes(std::list<guint32> &sizes);

	//! Return the name of the subdirectory for a given cityset.
        /** 
         * @param name          The name of the cityset to get the subdir of.
	 * @param tilesize      The size of the cityset to get the subdir of.
	 *
         * @return The name of the directory that holds the cityset.  See 
	 *         Cityset::d_dir for more information about the nature of 
	 *         the return value.
         */
	std::string getCitysetDir(std::string name, guint32 tilesize);

	//! Return the Cityset object by the name of the subdir.
	/**
	 * @param dir  The directory where the Cityset resides on disk.
	 *             This value does not contain any slashes, and is
	 *             presumed to be found inside the citysets/ directory.
	 */
	Cityset *getCityset(std::string dir) { return d_citysets[dir];}

    private:
        //! Default constructor.  Loads all citysets it can find.
	/**
	 * The citysets/ directory is scanned for Cityset directories.
	 */
        Citysetlist();
        
        //! Destructor.
        ~Citysetlist();

        //! Callback for loading Cityset objects into the list.
	bool load(std::string tag, XML_Helper *helper);

        //! Loads a specific Cityset.
	/**
	 * Load the Cityset from an cityset configuration file and add it to 
	 * this list of Cityset objects.
	 *
	 * @param name  The name of the subdirectory that the Cityset resides 
	 *              in.
	 *
	 * @return True if the Cityset could be loaded.  False otherwise.
	 */
        bool loadCityset (std::string name);
        
        typedef std::map<std::string, std::string> DirMap;
        typedef std::map<std::string, Cityset*> CitysetMap;

	//! A map that provides a subdirectory when supplying a Cityset name.
	/**
	 * the key for this map is actually the city name, a space, and then
	 * the tile size.  e.g. "Default 80".
	 */
        DirMap d_dirs;

	//! A map that provides a Cityset when supplying a subdirectory name.
        CitysetMap d_citysets;

        //! A static pointer for the singleton instance.
        static Citysetlist* s_instance;
};

#endif // CITYSETLIST_H

