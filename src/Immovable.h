//  Copyright (C) 2008, Ben Asselstine
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

#ifndef IMMOVABLE_H
#define IMMOVABLE_H

#include "defs.h"
#include "vector.h"
#include "Positionable.h"

class XML_Helper;

//! A game object that has an unchanging position on the map.
/** 
 * An Immovable is a game object on the map that doesn't move.
 */
class Immovable: private Positionable
{
 public:
     //! Default constructor.
     /**
      * @note After the position is set in the constructor, it cannot be
      *       altered.
      */
     Immovable(Vector<int> pos);
     //! Copy constructor.
     Immovable(const Immovable&);
     //! Loading constructor.
     Immovable(XML_Helper* helper);
     //! Destructor.
    ~Immovable();
    
    //! Return the position of the object on the game map.
    Vector<int> getPos() const {return d_pos;}

};

#endif
