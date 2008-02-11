// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef TILESET_H
#define TILESET_H

#include <string>
#include <vector>
#include <SDL.h>
#include <sigc++/trackable.h>

#include "Tile.h"

class XML_Helper;

/** Tileset is basically an array of tiles (terrain info objects).
  * 
  * It also contains some functions for loading and some additional items, such
  * as an info string or a name.
  *
  * The image file for a tileset contains of the terrain images. Each terrain
  * has a row in the image, where each column has a special meaning (See
  * MapRenderer for details how smoothing works). Furthermore, there are tiles
  * for special cases of diagonal adjacent water images. I hope to have a
  * documentation about map rendering and tilesets ready soon after the 0.3.5
  * release. If it already exists, see there for further info.
  */

class Tileset : public sigc::trackable, public std::vector<Tile*>
{
    public:
        /** The constructor.
          * 
          */
        Tileset(XML_Helper* helper);
        ~Tileset();

        std::string getSubDir() const {return d_dir;}
        void setSubDir(std::string dir) {d_dir = dir;}
        //! Returns the name of the tileset
        std::string getName() const {return d_name;}

        //! Returns the info string of the tileset
        std::string getInfo() const {return d_info;}

        //! Returns the tilesize of the tileset.
        Uint32 getTileSize() const {return d_tileSize;}

        //! Returns the index to standard terrain type
        Uint32 getIndex(Tile::Type type) const;

	void instantiatePixmaps();

	//! Lookup tilestyle by it's id in this tileset
	TileStyle *getTileStyle(Uint32 id) {return d_tilestyles[id];}

	//! Lookup a random tile style
	TileStyle *getRandomTileStyle(Uint32 index, TileStyle::Type style);

    private:
        //! Callback when the parser finds a tile tag. See XML_Helper for info.
        bool loadTile(std::string, XML_Helper* helper);

        // DATA
        std::string d_name;
        std::string d_info;
        Uint32 d_tileSize;
        std::string d_dir;
        typedef std::map<Uint32, TileStyle*> TileStyleIdMap;
        TileStyleIdMap d_tilestyles;
};

#endif // TILESET_H

// End of file
