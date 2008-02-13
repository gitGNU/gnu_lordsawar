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

#ifndef VECTOREDUNITLIST_H
#define VECTOREDUNITLIST_H

#include "vectoredunit.h"
#include "ObjectList.h"
#include <sigc++/trackable.h>

class City;
/** An object list which keeps track of all vectoredunits. It cannot do much more than
  * saving and loading the elements. Implemented as a singleton again.
  */

class VectoredUnitlist : public std::list<VectoredUnit*>, public sigc::trackable
{
    public:
        //! Returns the singleton instance. Creates a new one if required.
        static VectoredUnitlist* getInstance();

        //! Loads the singleton instance with a savegame.
        static VectoredUnitlist* getInstance(XML_Helper* helper);

        //! Explicitely deletes the singleton instance.
        static void deleteInstance();
        
	//! updates the vectored units belonging to the player
        void nextTurn(Player* p);

        //! Save function. See XML_Helper for details.
        bool save(XML_Helper* helper) const;

	//! When destination cities get conquered, this list needs to be 
	//! cleaned up.
        void removeVectoredUnitsGoingTo(Vector<int> pos);
        void removeVectoredUnitsGoingTo(City *c);

	//! When source cities get conquered, this list needs to be 
	//! cleaned up.
        void removeVectoredUnitsComingFrom(Vector<int> pos);
        void removeVectoredUnitsComingFrom(City *c);

	//! When showing info we need to know who's going to where.
	//! vectored is filled up with the results.
        void getVectoredUnitsGoingTo(Vector<int> pos, std::list<VectoredUnit*>& vectored);
	void getVectoredUnitsGoingTo(City *c, std::list<VectoredUnit*>& vectored);

	//! When showing info we need to know who's coming from where.
	//! vectored is filled up with the results.
        void getVectoredUnitsComingFrom(Vector<int> pos, std::list<VectoredUnit*>& vectored);

	//! Instead of returning the the vector, just return how many units
	//! are going to a particular destination.
        Uint32 getNumberOfVectoredUnitsGoingTo(Vector<int> pos);

	//! Change the destination of vectored units as they are "in the air".
	void changeDestination(City *c, Vector<int> new_dest);

    protected:
        VectoredUnitlist();
        ~VectoredUnitlist();
        VectoredUnitlist(XML_Helper* helper);

    private:
        //! Loading callback. See XML_Helper as well.
        bool load(std::string tag, XML_Helper* helper);

        static VectoredUnitlist* s_instance;
};

#endif
