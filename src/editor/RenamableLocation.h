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

#ifndef RENAMABLERUIN_H
#define RENAMABLERUIN_H

#include "defs.h"
#include "vector.h"
#include "Location.h"
#include "Renamable.h"
#include <string>

//! Scenario Editor.  A game object that has a position and a changeable name.
class RenamableLocation: public Location, public Renamable
{
 public:
     //! Default constructor.
     RenamableLocation(Vector<int> pos);
     //! Destructor.
    ~RenamableLocation();

    std::string getDescription() const {return d_description;};
    void setDescription(std::string desc) {d_description = desc;};

 private:
    std::string d_description;
};

#endif

