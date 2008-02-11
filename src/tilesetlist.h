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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef TILESETLIST_H
#define TILESETLIST_H

#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"
#include "Tile.h"
#include "tileset.h"


/** List of all available tilesets
  * 
  * This class contains a list of all tilesets used in the game. 
  * Since several classes access this class, it is implemented as a singleton.
  */

class Tilesetlist : public std::list<Tileset*>, public sigc::trackable
{
    public:
        //! return the singleton instance of this class
        static Tilesetlist* getInstance();

        //! Explicitly delete the singleton instance of this class
        static void deleteInstance();

        //! Returns the names of all tilesets
	std::list<std::string> getNames();

        /** Returns the subdir of a specific tileset by name
          * 
          * @param tileset       the name of the tileset
          * @return the name of the directory that holds the tileset
          */
	std::string getTilesetDir(std::string name) {return d_dirs[name];}

	Tileset *getTileset(std::string dir) { return d_tilesets[dir];}

	/* Reads in the pixmap and mask for every tile of every tileset.
	 * This can only be done after SDL is initialized.
	 */
	void instantiatePixmaps();

    private:
        //! Constructor; loads all tilesets it can find
        Tilesetlist();
        
        //! Destructor; mainly clears the lists
        ~Tilesetlist();

        //! Callback for loading. See XML_Helper for details.
	bool load(std::string tag, XML_Helper *helper);

        //! Loads a specific armyset
        bool loadTileset (std::string name);
        
        typedef std::map<std::string, std::string> DirMap;
        typedef std::map<std::string, Tileset*> TilesetMap;

        DirMap d_dirs;
        TilesetMap d_tilesets;

        static Tilesetlist* s_instance;
};

#endif // TILESETLIST_H

