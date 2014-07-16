// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "armyproto.h"
#include "armyset.h"
#include "setlist.h"

class Tar_Helper;

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

class Armysetlist : public SetList<Armyset>, public sigc::trackable
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

	//! Returns army prototype of the weakest/quickest from a given armyset.
        /** 
         * @param id       The Id of the armyset.
	 *
         * @return The requested weakest quickest army prototype or 0 on error.
         */
        ArmyProto* lookupWeakestQuickestArmy(guint32 id) const;

	//! Get the unshaded ship image for the given Armyset.
	PixMask* getShipPic (guint32 id);

	//! Get the ship mask picture for the given Armyset.
	PixMask* getShipMask (guint32 id);

	//! Get the unshaded planted standard picture for the given Armyset.
	PixMask* getStandardPic (guint32 id);

	//! Get the bag of oitems picture for the given Armyset.
	PixMask* getBagPic (guint32 id);

	//! Get the planted standard mask for the given Armyset.
	PixMask* getStandardMask (guint32 id);
        guint32 getTileSize(guint32 id);

        //! Returns the different tilesizes present in the armysetlist.
	void getSizes(std::list<guint32> &sizes);

        //! Returns the names of armysets that have the given tile size.
	std::list<Glib::ustring> getValidNames(guint32 tilesize);

	void instantiateImages(bool &broken);
	void uninstantiateImages();

    private:
        //! Default Constructor.  Loads all armyset objects it can find.
	/**
	 * The army/ directory is scanned for armyset directories.
	 */
        Armysetlist();
        
        //! Destructor.
        ~Armysetlist();

        void on_armyset_added(Armyset *armyset);
        void on_armyset_reloaded(Armyset *armyset);
	  
        typedef std::map<guint32, ArmyProto*> IdArmyPrototypeMap;
        typedef std::map<guint32, IdArmyPrototypeMap> ArmyPrototypeMap;
        
	//! A map that provides Army objects by their index.
        ArmyPrototypeMap d_armies;

        //! A static pointer for the singleton instance.
        static Armysetlist* s_instance;
};

#endif // ARMYSETLIST_H

