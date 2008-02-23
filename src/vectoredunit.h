//  Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef VECTOREDUNIT_H
#define VECTOREDUNIT_H

#include <string>
#include <sigc++/trackable.h>
#include "army.h"
#include "Location.h"

/*
 *  When units are "vectored" to another city, they disappear for 2 turns.
 *  While a unit is "in the air", it is represented in one of these objects.
 *
 */
class VectoredUnit: public Ownable, public Location, public sigc::trackable
{
    public:
        /** Default constructor
          * @param pos          the position of the source of the vectored unit
          * @param dest         destination location for the unit
	  * @param army		the kind of army that is being vectored
          * @param duration     how many turns it takes for the armytype to
	  *                     show up at dest.
          */
        VectoredUnit(Vector<int> pos, Vector<int> dest, Army *army, int duration, Player *p);

        //! Copy constructor
        VectoredUnit(const VectoredUnit&);

        //! Loading constructor. See XML_Helper for a detailed description.
        VectoredUnit(XML_Helper* helper);
        ~VectoredUnit();

	//! Return the location of the destination target for this vectored
	//! unit.
	Vector<int>getDestination() const {return d_destination;};

	//! set the location of the destination target for this vectored unit.
	void setDestination(Vector<int>dest) {d_destination = dest;};

	//! Return the number of turns that it takes for this vectored unit
	//! to show up at the destination.
	int getDuration () const { return d_duration; };

	//! set the number of turns it takes for this vectored unit to show
	//! up at the destination.
	void setDuration(int duration) {d_duration = duration;};

	//! Get the armytype that is being vectored
	Army *getArmy() const { return d_army; };

	//! Set the armytype that is being vectored
	void setArmy(Army *army) {d_army = army;}

        //! Saves the vectored unit data
        bool save(XML_Helper* helper) const;

        //! Do everything neccessary for a new turn
	//! returns true when this vectored unit has shown up in the city
        bool nextTurn();
inline bool operator==(const VectoredUnit &rhs)
{
    return d_pos == rhs.d_pos && d_destination == rhs.d_destination &&
      d_army->getType() == rhs.d_army->getType() && d_duration == rhs.d_duration;
};

    private:

        // DATA
	Vector<int> d_destination;
	Army *d_army; //army prototype to vector
	int d_duration;
};

#endif // VECTOREDUNIT_H

