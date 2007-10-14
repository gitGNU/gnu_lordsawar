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

/** TileSet is basically an array of tiles (terrain info objects).
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

class TileSet : public sigc::trackable, public std::vector<Tile*>
{
    public:
        /** The constructor.
          * 
          */
        TileSet(XML_Helper* helper);
        ~TileSet();

        std::string getSubDir() const {return d_dir;}
        void setSubDir(std::string dir) {d_dir = dir;}
        //! Returns the name of the tileset
        std::string getName() const {return d_name;}

        //! Returns the info string of the tileset
        std::string getInfo() const {return __(d_info);}

        //! Returns the tilesize of the tileset. This feature is yet unused.
        int getTileSize() const {return d_tileSize;}

        //! Returns the index to standard terrain type
        Uint32 getIndex(Tile::Type type) const;

        //! Return special pics; don't change them! pic == 0 for nw, 1 for ne.
        SDL_Surface* getDiagPic(int pic) const;

	void instantiatePixmaps();

    private:
        //! Callback when the parser finds a tile tag. See XML_Helper for info.
        bool loadTile(std::string, XML_Helper* helper);

        /** Fills the surfaces of a tile with data
          * 
          * @param tile     the tile to be processed
          * @param row      the row for the tile images in the image file
          */
        void createCorners(Tile* tile, int row);

        // DATA
        std::string d_name;
        std::string d_info;
        int d_tileSize;
        SDL_Surface* d_surface;
        SDL_Surface* d_nepic, *d_nwpic;
        std::string d_dir;
};

#endif // TILESET_H

// End of file
