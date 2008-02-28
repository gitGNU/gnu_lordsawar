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

#ifndef TILESTYLE_H
#define TILESTYLE_H

#include <SDL.h>

#include "xmlhelper.h"
#include "defs.h"

//! Describes the look of a Tile.
/**
  * A TileStyle describes a single look of a Tile.  It is responsible for
  * keeping the id, the image, and the kind of style.  The TileStyles for a 
  * given tile are defined in the Tileset configuration file.
  * If the kind of tile is `forest', then the TileStyles are defined in 
  * individual tileset.tile.tilestyleset.tilestyle XML entities.
  *
  * There are many different images that can represent a kind of Tile. 
  * Only certain tiles look right when positioned beside each other.  The
  * correct styles are assigned by the GameMap::calculatePreferredStyle method.
  * The id is important because it is referenced by the saved-game file 
  * (the lordsawar.map.styles XML entity).
  *
  * Every TileStyle belongs to a TileStyleSet, which belongs to a Tile.
  * Every Maptile refers to a TileStyle.
  */
class TileStyle
{
    public:
        //! Describe terrain tile styles.
	  /*
	   * All of the Tilestyle kinds can be described by using the
	   * following template:
	   * @verbatim
+-----+
|#####|
|#+-+#|
|#|o|#|
|#+-+#|
|#####|
+-----+
@endverbatim
           * Picture a terrain feature transitioning to grass on the outside 
	   * of the ring and on the inside of the ring also.
           */
	enum Type { 
	  /*
	   * @verbatim
ooooooo
ooooooo
ooooooo
oooxooo
ooooooo
ooooooo
ooooooo
@endverbatim
           */
	  LONE = 0,

	  /*
	   * @verbatim
xoooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
@endverbatim
           */
	  OUTERTOPLEFT = 1, 
	  /*
	   * @verbatim
oxxxxxo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
@endverbatim
           */
	  OUTERTOPCENTER = 2, 
	  /*
	   * @verbatim
oooooox
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
@endverbatim
           */
	  OUTERTOPRIGHT = 3,
	  /*
	   * @verbatim
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
xoooooo
@endverbatim
           */
	  OUTERBOTTOMLEFT = 4, 
	  /*
	   * @verbatim
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
oxxxxxo
@endverbatim
           */
	  OUTERBOTTOMCENTER = 5, 
	  /*
	   * @verbatim
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
oooooox
@endverbatim
           */
	  OUTERBOTTOMRIGHT = 6,
	  /*
	   * @verbatim
ooooooo
xoooooo
xoooooo
xoooooo
xoooooo
xoooooo
ooooooo
@endverbatim
           */
	  OUTERMIDDLELEFT = 7, 
	  /*
	   * @verbatim
ooooooo
oxxxxxo
oxoooxo
oxoooxo
oxoooxo
oxxxxxo
ooooooo
@endverbatim
           */
	  INNERMIDDLECENTER = 8, 
	  /*
	   * @verbatim
ooooooo
oooooox
oooooox
oooooox
oooooox
oooooox
ooooooo
@endverbatim
           */
	  OUTERMIDDLERIGHT = 9,
	  /*
	   * @verbatim
ooooooo
ooooooo
ooxoooo
ooooooo
ooooooo
ooooooo
ooooooo
@endverbatim
           */
	  INNERTOPLEFT = 10, 
	  /*
	   * @verbatim
ooooooo
ooooooo
ooooxoo
ooooooo
ooooooo
ooooooo
ooooooo
@endverbatim
           */
	  INNERTOPRIGHT = 11, 
	  /*
	   * @verbatim
ooooooo
ooooooo
ooooooo
ooooooo
ooxoooo
ooooooo
ooooooo
@endverbatim
           */
	  INNERBOTTOMLEFT = 12, 
	  /*
	   * @verbatim
ooooooo
ooooooo
ooooooo
ooooooo
ooooxoo
ooooooo
ooooooo
@endverbatim
           */
	  INNERBOTTOMRIGHT = 13,
	  /*
	   * Visually it's the merging of positions 1 and 2.
	   * @verbatim
1oooooo
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
oooooo2
@endverbatim
           */
	  TOPLEFTTOBOTTOMRIGHTDIAGONAL = 14, 
	  /*
	   * Visually it's the merging of positions 1 and 2.
	   * @verbatim
oooooo1
ooooooo
ooooooo
ooooooo
ooooooo
ooooooo
2oooooo
@endverbatim
           */
	  BOTTOMLEFTTOTOPRIGHTDIAGONAL = 15,
	  OTHER = 16,
	};

        //! Loading constructor
        TileStyle(XML_Helper* helper);

        ~TileStyle();

        //! Get the style type of this tile style.
        Type getType() const {return d_type;}
                
        //! Get the picture for tile style.
        SDL_Surface *getPixmap() const {return d_pixmap;}
 
        //! Get the id for this tilestyle.
	/*
	 * The id is unique among all other tilestyles in the Tileset.
	 */
	Uint32 getId() const {return d_id;}
 
	//! Load the picture associated with this tile style.
	/**
	 * Load the image for this tilestyle.
	 * @param tilestyles   The image that contains all of the tilestyles
	 *                     in this tilestyleset.
	 * @param tilesize     How big each tilestyle's picture is.  The
	 *                     width and the height of each tilestyle cell of 
	 *                     tilestyles are equal.
	 * @param index        Load the index-th picture from the 
	 *                     tilestyles image.
	 */
	void instantiatePixmap(SDL_Surface *tilestyles, Uint32 tilesize,
			       int index);
    private:
        // DATA
	//! The image of this tilestyle.
	/**
	 * It tilesize pixels wide and tilesize pixels high.  The tilesize
	 * value is passed into TileStyle::instantiatePixmap.
	 */
        SDL_Surface* d_pixmap; 

	//! The type of the tilestyle.
        Type d_type;

	//! The unique id for this tilestyle.
	/**
	 * It must be unique among all other TileStyle objects in the 
	 * Tileset.
	 * This id shows up in the saved-game file in the lordsawar.map.styles
	 * XML entity as a hexidecimal number.
	 */
	Uint32 d_id;
};

#endif // TILESTYLE_H

// End of file
