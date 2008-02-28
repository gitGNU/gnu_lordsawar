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

#ifndef LOCATION_H
#define LOCATION_H

#include "UniquelyIdentified.h"
#include "defs.h"
#include <string>
#include "vector.h"
#include "stack.h"
#include "Immovable.h"
#include "rectangle.h"

class Player;
class Location;
class ::UniquelyIdentified;

/** A Location is a map object with a name. This is the metaclass for
  * cities, ruins and temples.
  */

class Location : public ::UniquelyIdentified, public Immovable
{
 public:
    Location(std::string name, Vector<int> pos, Uint32 size = 1);
    Location(const Location&);
    Location(XML_Helper* helper, Uint32 size = 1);
    ~Location();
    
    std::string getName() const {return d_name;}
    void setName(std::string name) {d_name = name;}

    Stack *addArmy(Army *a) const;
    bool isFogged();
    void deFog();
    void deFog(Player *p);

    //! Get the size of the location
    Uint32 getSize() const {return d_size;}

    //! Does the Location contain this point?
    bool contains(Vector<int> pos) const;

    Rectangle get_area() const
	{ return Rectangle(getPos().x, getPos().y, d_size, d_size); }

 protected:
    Stack* getFreeStack(Player *p) const;
    std::string d_name;
    Uint32 d_size;
};

#endif
