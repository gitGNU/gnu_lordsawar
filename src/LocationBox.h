// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2004, 2005 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009 Ben Asselstine
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
#ifndef LOCATION_BOX_H
#define LOCATION_BOX_H

#include <gtkmm.h>
#include <string>
#include "vector.h"
#include "Immovable.h"
#include "rectangle.h"


class Player;
class Stack;
class Army;
//! A reference to a rectangular place on the game map.
/** 
 * A LocationBox is a place on the map that has a size.   The size is how many
 * tiles the place is tall and wide.
 */
class LocationBox : public Immovable
{
 public:
     //! Default constructor.
     /**
      * @param pos     The top-right corner of the feature is located at this
      *                position on the game map.
      * @param size    The number of tiles wide and high the feature is.
      */
     LocationBox(Vector<int> pos, guint32 size = 1);

     //! Non-Standard constructor.
     /**
      * Make a LocationBox from two points.
      */
     LocationBox(Vector<int> src, Vector<int> dest);

     //! Copy constructor.
     LocationBox(const LocationBox&);
     //! Loading constructor.
     /**
      * Load the location box from an opened saved-game file.
      *
      * @param helper  The opened saved-game file to read the location from.
      * @param size    The size of the place.  This value is not read in
      *                from the saved-game file.
      */
     LocationBox(XML_Helper* helper, guint32 size = 1);
     //! Destructor.
    ~LocationBox();
    
    //! Add an army to a tile that is included in this location.
    /**
     * @param army    The army instance to add to a tile in the location.
     *
     * @return A pointer to the stack where the Army was added.  Returns NULL
     *         when the Army couldn't be added because the location is full.
     */
    Stack *addArmy(Army *army) const;

    //! Returns whether this location is at least partially viewable.
    /**
     * This method returns true if the location has parts that are completely
     * defogged.
     * If the location is completely fogged or partially fogged then this 
     * method returns false.
     *
     * @param player The player whose map to query.
     */
    bool isVisible(Player *player) const;

    bool isCompletelyObscuredByFog(Player *player) const;

    //! Unobscures the view of this location in the active player's FogMap.
    void deFog();

    //! Unobscures the view of this location in the given player's FogMap.
    void deFog(Player *p);

    //! Return the size of the location.
    guint32 getSize() const {return d_size;}

    //! Returns whether or not the Location contains the given point?
    bool contains(Vector<int> pos) const;

    //! Returns a rectangle that describes the location.
    Rectangle get_area() const
	{ return Rectangle(getPos().x, getPos().y, d_size, d_size); }

 protected:

    //! Obtains a stack in the location to put an Army unit in.
    /**
     * This method scans the tiles of the location for a place to put a new
     * Army unit.  If the position is empty it makes a new stack in that
     * position and returns that stack.  If a stack containing fewer than
     * eight Army units is found, that stack is returned.  If no open spots 
     * could be found, it returns NULL.
     *
     * @param owner  The player to own the new stack if one needs to be created.
     *
     * @return The stack that has room for one Army unit in the Location.  If
     *         an available stack could not be found this method returns NULL.
     */
    Stack* getFreeStack(Player *owner) const;

    //! The size of the location.
    /**
     * This size is the number tiles high and wide the location is.
     * This value is always 1, except for City objects which are always 2.
     */
    guint32 d_size;
};

#endif
