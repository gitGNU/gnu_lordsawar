//  Copyright (C) 2008, Ben Asselstine
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

#ifndef SHIELDSETLIST_H
#define SHIELDSETLIST_H

#include <gtkmm.h>
#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"
#include "shield.h"
#include "shieldset.h"


//! A list of Shieldset objects available to the game.
/** 
 * This class holds all of the shield themes that are located in shield/.
 * It is implemented as a singleton.
 *
 * Other classes use it to lookup Shield and Shieldset objects.
 */
class Shieldsetlist : public std::list<Shieldset*>, public sigc::trackable
{
    public:
        //! return the singleton instance of this class.
        static Shieldsetlist* getInstance();

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

        //! Returns the names of all Shieldset objects available to the game.
	std::list<std::string> getNames();

	//! Return the directory of a specific Shieldset by name.
        /**
	 * Scan all of the Shieldset objects in the list for one with the 
	 * given name.
	 *
         * @param name   The name of the shieldset to search for.
	 *
         * @return The name of the directory that holds the shieldset, or an
	 *         empty string if a shieldset by that name could not be found.
	 *         This value relates to Shieldset::d_dir.
         */
	std::string getShieldsetDir(std::string name) {return d_dirs[name];}

	//! Return a particular Shield object from a given shieldset.
	/**
	 * Scan the given shieldset for a Shield of the given type and colour.
	 *
	 * @param shieldset  The id the shieldset.  This value relates to the 
	 *                   Shieldset::d_id member.
	 * @param type       The size of the shield to search for.  This value
	 *                   relates to the Shield::ShieldType enumeration.
	 * @param colour     The player of the shield.  This value relates to
	 *                   the Shield::ShieldColour enumeration.
	 *
	 * @return A pointer to a Shield in the given shieldset that matches
	 *         the given parameters.  If no shield could be found, NULL
	 *         is returned.
	 */
	ShieldStyle *getShield(guint32 shieldset, guint32 type, guint32 colour);

	//! Return the Shieldset object that is in the given directory.
	Shieldset *getShieldset(std::string dir);

	Gdk::Color getColor(guint32 shieldset, guint32 owner);

	//! Return the Shieldset object by the id.
	/**
	 * @param id   A unique numeric identifier that identifies the 
	 *             shieldset among all shieldsets in the shieldsetlist.
	 */
	Shieldset *getShieldset(guint32 id);


	void add(Shieldset *shieldset);

	void uninstantiateImages();
	void instantiateImages();

	bool addToPersonalCollection(Shieldset *shieldset, std::string &new_subdir, guint32 &new_id);

	static int getNextAvailableId(int after);
    private:
        //! Default Constructor.
	/**
	 * Loads all shieldsets it can find in the shield/ directory, and
	 * makes a new Shieldsetlist object from what it finds.
	 */
        Shieldsetlist();
        
        //! Destructor.
        ~Shieldsetlist();

        //! Loads a specific shieldset.
	Shieldset *loadShieldset(std::string name);
	void loadShieldsets(std::list<std::string> shieldsets);
        
        typedef std::map<std::string, std::string> DirMap;
        typedef std::map<std::string, Shieldset*> ShieldsetMap;
        typedef std::map<guint32, Shieldset*> ShieldsetIdMap;

	//! A map that provides a subdirectory when supplying a Shieldset name.
        DirMap d_dirs;

	//! A map that provides a Shieldset when supplying a subdirectory name.
        ShieldsetMap d_shieldsets;

	//! A map that provides a Shieldset when supplying a shieldset id.
        ShieldsetIdMap d_shieldsetids;

        //! A static pointer for the singleton instance.
        static Shieldsetlist* s_instance;
};

#endif // SHIELDSETLIST_H

