// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#ifndef ARMYSETLIST_H
#define ARMYSETLIST_H

#include <gtkmm.h>
#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"
#include "armyproto.h"
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
        ArmyProto* getArmy(guint32 id, guint32 index) const;

	//! Returns an army prototype of a scout from a given armyset.
        /** 
         * @param id       The Id of the armyset.
	 *
         * @return The requested scout or 0 on error.
         */
        ArmyProto* getScout(guint32 id) const;

	//! Get the unshaded ship image for the given Armyset.
	Glib::RefPtr<Gdk::Pixbuf> getShipPic (guint32 id);

	//! Get the ship mask picture for the given Armyset.
	Glib::RefPtr<Gdk::Pixbuf> getShipMask (guint32 id);

	//! Get the unshaded planted standard picture for the given Armyset.
	Glib::RefPtr<Gdk::Pixbuf> getStandardPic (guint32 id);

	//! Get the planted standard mask for the given Armyset.
	Glib::RefPtr<Gdk::Pixbuf> getStandardMask (guint32 id);
        guint32 getTileSize(guint32 id);

	//! Returns the size of a specific armyset.
        /** 
         * @param id       The id of the armyset to get the size of.
	 *
         * @return The number of Army prototype objects in the Armyset.
	 *         Returns 0 on error. 
         */
        guint32 getSize(guint32 id) const;

	//! Return the name of a given armyset.
        /** 
         * @param id       The id of the armyset to get the name of.
	 *
         * @return The name of the Armyset or an empty string on error.
         */
        std::string getName(guint32 id) const;

        //! Returns the different tilesizes present in the armysetlist.
	void getSizes(std::list<guint32> &sizes);

        //! Returns the names of all Armyset objects available to the game.
	std::list<std::string> getNames();

        //! Returns the names of armysets that have the given tile size.
	std::list<std::string> getNames(guint32 tilesize);

        /** Returns the Id of a specific armyset by name
          * 
          * @param armyset       the name of the armyset
	  * @param tilesize      the height and width of tiles in the armyset.
	  *
          * @return the id of the armyset (0 on error)
          */
	guint32 getArmysetId(std::string armyset, guint32 tilesize);
	Armyset *getArmyset(guint32 id);

	//! Return the Armyset object by the name of the subdir.
	/**
	 * @param dir  The directory where the Armyset resides on disk.
	 *             This value does not contain any slashes, and is
	 *             presumed to be found inside the army/ directory.
	 */
	Armyset *getArmyset(std::string dir) { return d_armysets[dir];}

	//! Returns a list of all Armyset objects available to the game.
        std::vector<guint32> getArmysets() const;

	//! Return the name of the subdirectory for a given armyset.
        /** 
         * @param name          The name of the Armyset to get the subdir of.
	 * @param tilesize      The size of the Armyset to get the subdir of.
	 *
         * @return The name of the directory that holds the cityset.  See 
	 *         Armyset::d_dir for more information about the nature of 
	 *         the return value.
         */
	std::string getArmysetDir(std::string name, guint32 tilesize);

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
        
        typedef std::map<guint32, std::vector<ArmyProto*> > ArmyPrototypeMap;
        typedef std::map<guint32, std::string> NameMap;
        typedef std::map<std::string, guint32> IdMap;
        typedef std::map<std::string, Armyset*> ArmysetMap;
        
	//! A map that provides Army objects by their index.
        ArmyPrototypeMap d_armies;

	//! A map that provides Armyset::d_name by supplying a Armyset::d_id.
        NameMap d_names;

	//! A map that provides Armyset:d_id by supplying a Armyset::d_name.
        IdMap d_ids;

	//! A map that provides an Armyset when supplying a subdirectory name.
        ArmysetMap d_armysets;

        //! A static pointer for the singleton instance.
        static Armysetlist* s_instance;
};

#endif // ARMYSETLIST_H

