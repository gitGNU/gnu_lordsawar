// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2004, 2005 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008 Ben Asselstine

#ifndef LOCATION_H
#define LOCATION_H

#include "UniquelyIdentified.h"
#include "defs.h"
#include <string>
#include "vector.h"
#include "stack.h"
#include "LocationBox.h"
#include "rectangle.h"

class Player;
class Location;
class UniquelyIdentified;

//! A feature constructed on the game map.
/** 
 * A Location is a map feature with a location, and a size. 
 * City, Ruin, Temple, Signpost and more classes are derived from Location.
 */
class Location : public UniquelyIdentified, public LocationBox
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
};

#endif
