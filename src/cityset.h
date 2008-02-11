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

#ifndef CITYSET_H
#define CITYSET_H

#include <string>
#include <vector>
#include <SDL.h>
#include <sigc++/trackable.h>

class XML_Helper;

/** Cityset is basically an array of citys (terrain info objects).
  * 
  * It also contains some functions for loading and some additional items, such
  * as an info string or a name.
  *
  * The image file for a cityset contains of the terrain images. Each terrain
  * has a row in the image, where each column has a special meaning (See
  * MapRenderer for details how smoothing works). Furthermore, there are citys
  * for special cases of diagonal adjacent water images. I hope to have a
  * documentation about map rendering and citysets ready soon after the 0.3.5
  * release. If it already exists, see there for further info.
  */

class Cityset : public sigc::trackable
{
    public:
        /** The constructor.
          * 
          */
        Cityset(XML_Helper* helper);
        ~Cityset();

        std::string getSubDir() const {return d_dir;}
        void setSubDir(std::string dir) {d_dir = dir;}
        //! Returns the name of the cityset
        std::string getName() const {return d_name;}

        //! Returns the info string of the cityset
        std::string getInfo() const {return d_info;}

        //! Returns the citysize of the cityset.
        Uint32 getTileSize() const {return d_tileSize;}

    private:

        // DATA
        std::string d_name;
        std::string d_info;
        Uint32 d_tileSize;
        std::string d_dir;
};

#endif // CITYSET_H

// End of file
