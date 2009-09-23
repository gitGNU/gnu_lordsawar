//  Copyright (C) 2008, Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#ifndef NAMEDLOCATION_H
#define NAMEDLOCATION_H

#include <string>
#include "vector.h"
#include "Location.h"
#include "Namable.h"

class XML_Helper;

//! A game object that has a position and a name.
/** 
 * A NamedLocation is a game object on the map that has a position and a name.
 * It also has a description.
 */

class NamedLocation: public Location, public Namable
{
 public:
     //! Default constructor.
     NamedLocation(Vector<int> pos, guint32 size, std::string name, 
		   std::string desc);
     //! Copy constructor.
     NamedLocation(const NamedLocation&);
     //! Loading constructor.
     NamedLocation(XML_Helper* helper, guint32 size);
     //! Destructor.
    ~NamedLocation();

    std::string getDescription() const {return d_description;};
 private:
    std::string d_description;
};

#endif
