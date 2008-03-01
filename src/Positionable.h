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

#ifndef POSITIONABLE_H
#define POSITIONABLE_H

#include "defs.h"
#include "vector.h"

class XML_Helper;

//! A game object that can be positioned at least once on the game map.
/** 
 * An Positionable is an object on the map.
 */

class Positionable
{
 public:
     //! Default constructor.
     Positionable(Vector<int> pos);
     //! Copy constructor.
     Positionable(const Positionable&);
     //! Loading constructor.
     Positionable(XML_Helper* helper);
     //! Destructor.
    ~Positionable();
    
 protected:
    //! The position of the object on the game map.
    Vector<int> d_pos;
};

#endif
