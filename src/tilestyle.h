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

/** Class which describes the look of a terrain tile
  * 
  * A tilestyle describes a single look of a tile
  * It keeps the image, and the kind of style.
  */

class TileStyle
{
    public:
        //! Describe terrain tile styles 
        
	enum Type { 
	  LONE = 0,
	  OUTERTOPLEFT = 1, 
	  OUTERTOPCENTER = 2, 
	  OUTERTOPRIGHT = 3,
	  OUTERBOTTOMLEFT = 4, 
	  OUTERBOTTOMCENTER = 5, 
	  OUTERBOTTOMRIGHT = 6,
	  OUTERMIDDLELEFT = 7, 
	  INNERMIDDLECENTER = 8, 
	  OUTERMIDDLERIGHT = 9,
	  INNERTOPLEFT = 10, 
	  INNERTOPRIGHT = 11, 
	  INNERBOTTOMLEFT = 12, 
	  INNERBOTTOMRIGHT = 13,
	  TOPLEFTTOBOTTOMRIGHTDIAGONAL = 14, 
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
 
        //! Get the index for this tile style, among other tile styles in a set
	Uint32 getId() const {return d_id;}
 
        //! Get the index for this tile style, among other tile styles in a set
	void setIndex(Uint32 index) {d_index = index;}
 
	//! Load the picture associated with this tile style.
	//! Load the INDEXth picture from the TILESTYLES picture.
	//! The TILESIZE parameter says how big each tile's picture is.
	void instantiatePixmap(SDL_Surface *tilestyles, Uint32 tilesize);
    private:
        // DATA
        SDL_Surface* d_pixmap;      // picture
        Type d_type;
	Uint32 d_id;
	Uint32 d_index;
};

#endif // TILESTYLE_H

// End of file
