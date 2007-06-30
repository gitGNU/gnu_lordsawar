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

#ifndef RUIN_H
#define RUIN_H

#include <string>
#include <sigc++/trackable.h>
#include "Location.h"
#include "stack.h"

class Stack;

/** A ruin is a simple object on the map which contains an id, a flag whether it
  * has already been searched and optionally an occupant (called "keeper").
  * If a ruin is searched, the player starts a fight with the keeper. If he
  * wins, the ruin becomes search and the player gets some reward.
  */

class Ruin : public Location, public sigc::trackable
{
    public:
        /** Default constructor
          * @param pos          the location of the ruin
          * @param name         the name of the ruin
          * @param occupant     the monsters occupying the ruin
          * @param searched     sets the searched flag of the ruin
	  * @param hidden       sets the hidden flag of the ruin
	  * @param owner        who can see this hidden ruin
	  * @param sage         if this ruin contains a sage or not
          */
        Ruin(Vector<int> pos, std::string name = "", Stack* occupant = 0, 
	     bool searched = false, bool hidden = false, Player *owner = 0,
	     bool sage = false);

        //! Copy constructor
        Ruin(const Ruin&);

        //! Loading constructor. See XML_Helper for a detailed description.
        Ruin(XML_Helper* helper);
        ~Ruin();

        //! Change the "searched" flag of the ruin
        void setSearched(bool searched) {d_searched = searched;}
        
        //! Set the keeper of the ruin
        void setOccupant(Stack* occupant) {d_occupant = occupant;}

        
        //! Gets the status of the ruin
        bool isSearched() const {return d_searched;}

        //! Returns the keeper
        Stack* getOccupant() const {return d_occupant;}

	//! Returns wether or not this is a "hidden" ruin
	bool isHidden() const {return d_hidden;}

	//! Returns wether or not this is ruin has a sage
	bool hasSage() const {return d_sage;}

	//! Returns the player that owns this hidden ruin
	Player *getOwner() const {return d_owner;}

        //! Callback for loading the ruin data
        bool load(std::string tag, XML_Helper* helper);

        //! Saves the ruin data
        bool save(XML_Helper* helper) const;

    private:
        // DATA
        bool d_searched;    // has ruin already been searched for treasure?
        Stack* d_occupant;
	bool d_hidden;      // is this a "hidden" ruin that can only be seen
	Player *d_owner;    // by an "owner"?
	bool d_sage;        // does this ruin have a sage?
};

#endif // RUIN_H

