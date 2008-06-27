// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005  Ulf Lorenz
// Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef ARMYSETLIST_H
#define ARMYSETLIST_H

#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"
#include "army.h"
#include "armyset.h"


//! A list of all Armyset objects available to the game.
/** 
 * This class contains a list of all armyset objects available to the game. 
 * Each armyset has a size, a name and a list of armies. The armysetlist 
 * shields all these from the rest of the program.  Armysets are most often
 * referenced by their id.
 *
 * The Armysetlist is populated with Armyset objects that are loaded from the 
 * army/ directory.
 *
 * Since several classes access this class, it is implemented as a singleton.
 */

class Armysetlist : public std::list<Armyset*>, public sigc::trackable
{
    public:
        //! Return the singleton instance of this class.
        static Armysetlist* getInstance();

        //! Explicitly delete the singleton instance of this class
        static void deleteInstance();

	//! Returns an army prototype from a given armyset.
        /** 
         * @param id       The Id of the armyset.
         * @param index    The index of the army within the set.
	 *                 This value becomes the Army object's type.
	 *
         * @return The requested army or 0 on error.
         */
        Army* getArmy(Uint32 id, Uint32 index) const;

	//! Returns an army prototype of a scout from a given armyset.
        /** 
         * @param id       The Id of the armyset.
	 *
         * @return The requested scout or 0 on error.
         */
        Army* getScout(Uint32 id) const;

	//! Get the unshaded ship image for the given Armyset.
	SDL_Surface * getShipPic (Uint32 id);

	//! Get the ship mask picture for the given Armyset.
	SDL_Surface * getShipMask (Uint32 id);

	//! Get the unshaded planted standard picture for the given Armyset.
	SDL_Surface * getStandardPic (Uint32 id);

	//! Get the planted standard mask for the given Armyset.
	SDL_Surface * getStandardMask (Uint32 id);
        Uint32 getTileSize(Uint32 id);

	//! Returns the size of a specific armyset.
        /** 
         * @param id       The id of the armyset to get the size of.
	 *
         * @return The number of Army prototype objects in the Armyset.
	 *         Returns 0 on error. 
         */
        Uint32 getSize(Uint32 id) const;

	//! Return the name of a given armyset.
        /** 
         * @param id       The id of the armyset to get the name of.
	 *
         * @return The name of the Armyset or an empty string on error.
         */
        std::string getName(Uint32 id) const;

        //! Returns the names of all Armyset objects available to the game.
	std::list<std::string> getNames();

        /** Returns the Id of a specific armyset by name
          * 
          * @param armyset       the name of the armyset
          * @return the id of the armyset (0 on error)
          */
	Uint32 getArmysetId(std::string armyset) {return d_ids[armyset];}

	//! Returns a list of all Armyset objects available to the game.
        std::vector<Uint32> getArmysets() const;

	//! Load the pictures for all Armyset objects available to the game.
	/**
	 * Reads in the pixmap and mask for every army of every armyset.
	 * @note This can only be done after SDL is initialized.
	 */
	void instantiatePixmaps();

    private:
        //! Default Constructor.  Loads all armyset objects it can find.
	/**
	 * The army/ directory is scanned for armyset directories.
	 */
        Armysetlist();
        
        //! Destructor.
        ~Armysetlist();

        //! Callback for loading an armyset.  See XML_Helper for details.
	bool load(std::string tag, XML_Helper *helper);

        //! Loads a specific armyset.
	/**
	 * Load the armyset from an armyset configuration file and add it to 
	 * this list of armysets.
	 *
	 * @param name  The subdirectory name that the Armyset resides in.
	 *
	 * @return True if the Armyset could be loaded.  False otherwise.
	 */
        bool loadArmyset (std::string name);
        
        typedef std::map<Uint32, std::vector<Army*> > ArmyPrototypeMap;
        typedef std::map<Uint32, std::string> NameMap;
        typedef std::map<std::string, Uint32> IdMap;
        
	//! A map that provides Army objects by their index.
        ArmyPrototypeMap d_armies;

	//! A map that provides Armyset::d_name by supplying a Armyset::d_id.
        NameMap d_names;

	//! A map that provides Armyset:d_id by supplying a Armyset::d_name.
        IdMap d_ids;

        //! A static pointer for the singleton instance.
        static Armysetlist* s_instance;
};

#endif // ARMYSETLIST_H

