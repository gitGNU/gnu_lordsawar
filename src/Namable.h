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

#ifndef NAMABLE_H
#define NAMABLE_H

#include "defs.h"
#include "vector.h"
#include "Named.h"

class XML_Helper;

//! A game object that has an unchanging name.
/** 
 * An Namable is a game object that has a name that can't be changed.
 */
class Namable: private Named
{
 public:
     //! Default constructor.
     /**
      * @note After the name is set in the constructor, it cannot be altered.
      */
     Namable(std::string name);
     //! Copy constructor.
     Namable(const Namable&);
     //! Loading constructor.
     Namable(XML_Helper* helper);
     //! Destructor.
    ~Namable();
    
    //! Return the name of the object on the game map.
    std::string getName() const {return d_name;}

};

#endif
