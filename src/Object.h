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

#ifndef OBJECT_H
#define OBJECT_H

#include <SDL_types.h>
#include "vector.h"
#include "rectangle.h"

class XML_Helper;

/** Base class for map objects.
  *
  * This is a rather straightforward class which simply defines an object
  * with a position and a size. The map objects (city, temple, stack) are
  * derived from this class.
  */

class Object
{
 public:
    /** Constructor
     * 
     * @param pos      the position of the object
     * @param size     the size of the object (we assume a square)
     */
    Object(Vector<int> pos, Uint32 size = 1);
    Object(const Object&);
    Object(XML_Helper* helper, Uint32 size = 1);
    virtual ~Object();

    //! Get the position of the object
    Vector<int> getPos() const {return d_pos;}

    //! Returns the id of the object
    Uint32 getId() const {return d_id;}
        
    //! Get the size of the object
    Uint32 getSize() const {return d_size;}

    //! Does the Object contain this point?
    bool contains(Vector<int> pos) const;

    Rectangle get_area() const
	{ return Rectangle(d_pos.x, d_pos.y, d_size, d_size); }
	
 protected:
    Vector<int> d_pos;
    Uint32 d_id;
    Uint32 d_size;
};

#endif

// End of file
