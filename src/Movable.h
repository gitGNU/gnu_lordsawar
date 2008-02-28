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

#ifndef MOVABLE_H
#define MOVABLE_H

#include "defs.h"
#include "vector.h"
#include "Positionable.h"

class XML_Helper;

//! A game object that has an changing position on the map.
/** 
 * A Movable is a game object on the map that has a position that can change.
 */

class Movable: private Positionable
{
 public:
     //! Default constructor.
     Movable(Vector<int> pos);
     //! Copy constructor.
     Movable(const Movable&);
     //! Loading constructor.
     Movable(XML_Helper* helper);
     //! Destructor.
    ~Movable();
    
    //! Return the position of the object on the game map.
    Vector<int> getPos() const {return d_pos;}

    //! Set the position of the obejct on the game map.
    void setPos(Vector<int> pos) {d_pos = pos;}

};

#endif
