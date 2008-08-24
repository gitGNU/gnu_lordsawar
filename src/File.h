// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#ifndef FILE_H
#define FILE_H

#include <string>
#include <list>
#include <SDL.h>

/** \brief Miscellaneous functions for unified file access
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
        /** Scan the data directories for armysets
          * 
          * @return a list of available armysets
          */
        static std::list<std::string> scanArmysets();


        /** Get the armyset description file
          *
          * @param armysetname      the name of the armyset
          * @return the full name of the description file
          */
        static std::string getArmyset(std::string armysetname);

        /** Scan the data directories for shieldsets
          * 
          * @return a list of available shieldsets
          */
        static std::list<std::string> scanShieldsets();

        /** Get the shieldset description file
          *
          * @param shieldsetname      the name of the shieldset
          * @return the full name of the shield description file
          */
        static std::string getShieldset(std::string shieldsetname);

        
        

        /** Get the description file for the tileset
          * 
          * @param tilesetname       the name of the tileset
          * @return the full name of the description file
          */
        static std::string getTileset(std::string tilesetname);

	// get a tileset path
	static std::string getTilesetFile(std::string tilesetname, std::string picname);
	
        /** Get the description file for the cityset
          * 
          * @param citysetname       the name of the cityset
          * @return the full name of the description file
          */
        static std::string getCityset(std::string citysetname);

	// get a cityset path
	static std::string getCitysetFile(std::string citysetname, std::string picname);
	
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

        // get the available tilesets
        static std::list<std::string> scanTilesets();

        // get the available citysets
        static std::list<std::string> scanCitysets();

        // get the available maps
        static std::list<std::string> scanMaps();

    private:
};

#endif //FILE_H

// End of file
