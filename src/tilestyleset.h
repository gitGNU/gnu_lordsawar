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

#ifndef TILESTYLESET_H
#define TILESTYLESET_H

#include <string>
#include <vector>
#include <SDL.h>
#include <sigc++/trackable.h>

#include "tilestyle.h"

class XML_Helper;

/** TileStyleSet is an array of tile styles (the look of terrain info objects).
  * 
  */

class TileStyleSet : public sigc::trackable, public std::vector<TileStyle*>
{
    public:
        /** The constructor.
          * 
          */
        TileStyleSet(XML_Helper* helper);
        ~TileStyleSet();

	//! Get the name of this tilestyleset
	std::string getName() const {return d_name;}

	void instantiatePixmaps(std::string tileset, Uint32 tilesize);

    private:

        std::string d_name;
};

#endif // TILESTYLESET_H

// End of file
