// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2004, 2005 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#ifndef UNIQUELYIDENTIFIED_H
#define UNIQUELYIDENTIFIED_H

#include <SDL_types.h>
#include "vector.h"
#include "rectangle.h"

class XML_Helper;

//! A game object that has a unique numeric identifier.
class UniquelyIdentified
{
 public:
    //! Default constructor.
    UniquelyIdentified();
    //! Copy constructor.
    UniquelyIdentified(const UniquelyIdentified&);

    //! non-default constructor. 
    UniquelyIdentified(Uint32 id);

    //! Loading constructor.
    UniquelyIdentified(XML_Helper* helper);
    virtual ~UniquelyIdentified();

    //! Returns the id of the object.
    Uint32 getId() const {return d_id;}

    void syncNewId();

    void assignNewId();
        
 protected:
    Uint32 d_id;
    bool d_unique;
};

#endif

// End of file
