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

#include <SDL.h>
#include <list>
#include <string>
class VectoredUnit;
class Player;
class XML_Helper;

#include "vector.h"

#include <sigc++/trackable.h>

class City;
//! A list of VectoredUnit objects.
/** 
 * This class loads and saves the VectoredUnit objects in the game.  It 
 * facilitates looking up VectoredUnit objects in the list.
 *
 * This class is loaded from, and saved to the lordsawar.vectoredunitlist XML
 * entity in the saved-game file.
 *
 * Implemented as a singleton.
 */
class VectoredUnitlist : public std::list<VectoredUnit*>, public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static std::string d_tag; 

        //! Gets the singleton instance or creates a new one.
        static VectoredUnitlist* getInstance();

        //! Loads the VectoredUnitlist from a saved-game file.
	/**
	 * Load all VectoredUnit objects in the VectoredUnitlist from a 
	 * saved-game file.
	 *
	 * @param helper     The opened saved-game file to read from.
	 *
	 * @return The loaded VectoredUnitlist.
	 */
        static VectoredUnitlist* getInstance(XML_Helper* helper);

        //! Explicitly deletes the singleton instance.
        static void deleteInstance();
        
	//! Processes all VectoredUnit objects belonging to the given Player.
        void nextTurn(Player* p);

        //! Save the list of VectoredUnit objects to a saved-game file.
        bool save(XML_Helper* helper) const;

	//! Cull the list of VectoredUnit objects going to the given position.
	/**
	 * Scan through the VectoredUnitlist for VectoredUnit objects that
	 * have the given destination position on the game map.  When found, 
	 * remove it from the list.
	 *
	 * When a planted standard is picked up by another Player's Hero this
	 * method is called.
	 *
	 * @param pos  Any VectoredUnit object in the list that is being
	 *             vectored to this tile is deleted from the list.
	 */
        bool removeVectoredUnitsGoingTo(Vector<int> pos);

	//! Cull the list of VectoredUnit objects going to the given city.
	/**
	 * Scan through the VectoredUnitlist for VectoredUnit objects that
	 * have a destination position of the given city.  When found, remove
	 * it from the list.
	 *
	 * This method gets called when a destination city is conquered.
	 *
	 * @param city  Any VectoredUnit object in the list that is being
	 *              vectored to one of the tiles in this City are deleted
	 *              from the list.
	 */
        bool removeVectoredUnitsGoingTo(City *city);

	//! Cull the list of VectoredUnit objects being vectored from a place.
	/**
	 * Scan through the VectoredUnitlist for VectoredUnit objects that
	 * have a source tile of the given position on the game map.  When 
	 * found, remove the VectoredUnit object from this list.
	 *
	 * When a source city gets conquered, VectoredUnit objects need to be
	 * deleted from this list.
	 *
	 * @param pos  Any VectoredUnit object in the list that is being
	 *             vectored from this tile is deleted from the list.
	 */
        bool removeVectoredUnitsComingFrom(Vector<int> pos);

	//! Cull the list of VectoredUnit objects coming from the given city.
	/**
	 * Scan through the VectoredUnitlist for VectoredUnit objects that
	 * have a source position of a tile in the given city.  When found, 
	 * remove it from the list.
	 *
	 * This method gets called when a destination city is conquered.
	 *
	 * @param city  Any VectoredUnit object in the list that is being
	 *              vectored to one of the tiles in this City are deleted
	 *              from the list.
	 */
        bool removeVectoredUnitsComingFrom(City *city);

	//! Return the list of VectoredUnit objects with the given destination.
	/**
	 * Scan through the list of VectoredUnit objects for the ones that are
	 * being vectored to the given position.  Return all of the 
	 * VectoredUnit objects that match.
	 *
	 * This method is used for showing who's going where.
	 *
	 * @param pos  Any VectoredUnit object in the list that is being
	 *             vectored to this tile is returned.
	 *
	 * @param vectored  This list is filled with the VectoredUnit objects 
	 *                  being vectored to the given position.
	 */
        void getVectoredUnitsGoingTo(Vector<int> pos, 
				     std::list<VectoredUnit*>& vectored);

	//! Return the list of VectoredUnit objects going to the given city.
	/**
	 * Scan through the list of VectoredUnit objects for the ones that are
	 * being vectored to the tiles on the game map assocaited with the
	 * given city.  Return all of the VectoredUnit objects that match.
	 *
	 * This method is used for showing who's going where.
	 *
	 * @param city  Any VectoredUnit object in the list that is being
	 *              vectored to this city is returned.
	 *
	 * @param vectored  This list is filled with the VectoredUnit objects 
	 *                  being vectored to the given city.
	 */
	void getVectoredUnitsGoingTo(City *city, 
				     std::list<VectoredUnit*>& vectored);

	//! Return the list of VectoredUnit objects with the given source.
	/**
	 * Scan through the list of VectoredUnit objects for the ones that are
	 * being vectored from the given position.  Return all of the 
	 * VectoredUnit objects that match.
	 *
	 * This method is used for showing who's coming from where.
	 *
	 * @param pos  Any VectoredUnit object in the list that is being
	 *             vectored from this tile is returned.
	 *
	 * @param vectored  This list is filled with the VectoredUnit objects 
	 *                  being vectored from the given position.
	 */
        void getVectoredUnitsComingFrom(Vector<int> pos, 
					std::list<VectoredUnit*>& vectored);

	//! Return the number of VectoredUnits being vectored to a given place.
	/**
	 * Scan through all of the VectoredUnit objects in the list for ones
	 * that are being vectored to the given position on the game map.
	 * Count all of the matching VectoredUnit objects, and return the
	 * count.
	 *
	 * @return The number of VectoredUnit objects that are being vectored
	 *         to the given position on the game map.
	 */
        Uint32 getNumberOfVectoredUnitsGoingTo(Vector<int> pos);

	//! Change the destination of vectored units as they are "in the air".
	/**
	 * Scan through all of the VectoredUnit objects in the list for the
	 * ones that are being vectored to the given city.  When found,
	 * change the destination to be the given destination.
	 *
	 * @param city     A pointer to the city to change VectoredUnit objects
	 *                 from going to.
	 * @param new_dest A position on the game map to change where the
	 *                 VectoredUnit objects are going to.
	 */
	void changeDestination(City *city, Vector<int> new_dest);

	void changeOwnership(Player *old_owner, Player *new_owner);

	iterator flErase(iterator object);
    protected:

	//! Default constructor.
        VectoredUnitlist();
	//! Loading constructor.
        VectoredUnitlist(XML_Helper* helper);
	//! Destructor.
        ~VectoredUnitlist();

    private:


        //! Callback for loading the VectoredUnitlist from a saved-game file.
        bool load(std::string tag, XML_Helper* helper);

        //! A static pointer for the singleton instance.
        static VectoredUnitlist* s_instance;
};

#endif
