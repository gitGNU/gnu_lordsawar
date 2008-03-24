// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2004, 2005 Ulf Lorenz
// Copyright (C) 2006 Andrea Patton
// Copyright (C) 2006, 2007, 2008 Ben Asselstine

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

//! A feature constructed on the game map.
/** 
 * A Location is a map feature with a location, and a size. 
 * City, Ruin, Temple, Signpost and more classes are derived from Location.
 */
class Location : public ::UniquelyIdentified, public Immovable
{
 public:
     //! Default constructor.
     /**
      * @param pos     The top-right corner of the feature is located at this
      *                position on the game map.
      * @param size    The number of tiles wide and high the feature is.
      */
     Location(Vector<int> pos, Uint32 size = 1);
     //! Copy constructor.
     Location(const Location&);
     //! Loading constructor.
     /**
      * Load the location from an opened saved-game file.
      *
      * @param helper  The opened saved-game file to read the location from.
      * @param size    The size of the feature.  This value is not read in
      *                from the saved-game file.
      */
     Location(XML_Helper* helper, Uint32 size = 1);
     //! Destructor.
    ~Location();
    
    //! Add an army to a tile that is included in this location.
    /**
     * @param army    The army instance to add to a tile in the location.
     *
     * @return A pointer to the stack where the Army was added.  Returns NULL
     *         when the Army couldn't be added because the location is full.
     */
    Stack *addArmy(Army *army) const;

    //! Returns whether or not this location obscured from view on a hidden map.
    /**
     * @note This method relies on Playerlist::getActiveplayer to know
     *       which FogMap to query.
     */
    bool isFogged();

    //! Unobscures the view of this location in the active player's FogMap.
    void deFog();

    //! Unobscures the view of this location in the given player's FogMap.
    void deFog(Player *p);

    //! Return the size of the location.
    Uint32 getSize() const {return d_size;}

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
    Uint32 d_size;
};

#endif
