//  Copyright (C) 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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

class Tar_Helper;

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

	// Methods that operate on the class data but do not modify the class.

        //! Returns the names of all Shieldset objects available to the game.
	std::list<std::string> getValidNames() const;

        //! Returns whether the given name is our list of shieldset objects.
        bool contains(std::string name) const;

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
	std::string getShieldsetDir(std::string name) const;

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
	ShieldStyle *getShield(guint32 shieldset, guint32 type, guint32 colour) const;

        guint32 getShieldsetId(std::string basename) const;

	//! Return the Shieldset object that is in the given directory.
	Shieldset *getShieldset(std::string dir) const;

	Gdk::RGBA getColor(guint32 shieldset, guint32 owner) const;

	//! Return the Shieldset object by the id.
	/**
	 * @param id   A unique numeric identifier that identifies the 
	 *             shieldset among all shieldsets in the shieldsetlist.
	 */
	Shieldset *getShieldset(guint32 id) const;


	// Methods that operate on the class data and modify the class.

	//! Add a shieldset to the list.  Use this instead of push_back.
	void add(Shieldset *shieldset, std::string filename);

	//! Destroy all of the images associated with shieldsets in this list.
	void uninstantiateImages();
        
        bool reload(guint32 shieldset_id);

	//! Load all of the images associated with all of the shieldsets.
	void instantiateImages(bool &broken);

	//! Add the given shieldset to the list, and copy files into place.
	/**
	 * This method tries hard to add the shieldset to this list.  The 
	 * basename could be changed, or the id might also be changed so 
	 * that it doesn't conflict with any other shieldsets in the list.
	 *
	 * @return Returns true if it was added successfully, and the
	 *         new_basename and new_id parameters updated to reflect the
	 *         changed basename and id.
	 */
	bool addToPersonalCollection(Shieldset *shieldset, std::string &new_basename, guint32 &new_id);
	Shieldset *import(Tar_Helper *t, std::string f, bool &broken);


	// Static Methods

        //! Return the singleton instance of this class.
        static Shieldsetlist* getInstance();

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

	//! Return a unique id for a shieldset.
	static int getNextAvailableId(int after);

        std::string findFreeBaseName(std::string basename, guint32 max, guint32 &num) const;
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

	//! Loads a bunch of shieldsets and puts them in this list.
	void loadShieldsets(std::list<std::string> shieldsets);
	
        std::list<std::string> getNames() const;
        
	// DATA

        typedef std::map<std::string, std::string> DirMap;
        typedef std::map<std::string, Shieldset*> ShieldsetMap;
        typedef std::map<guint32, Shieldset*> ShieldsetIdMap;

	//! A map that provides a basename when supplying a Shieldset name.
        DirMap d_dirs;

	//! A map that provides a Shieldset when supplying a basename name.
        ShieldsetMap d_shieldsets;

	//! A map that provides a Shieldset when supplying a shieldset id.
        ShieldsetIdMap d_shieldsetids;

        //! A static pointer for the singleton instance.
        static Shieldsetlist* s_instance;
};

#endif // SHIELDSETLIST_H

