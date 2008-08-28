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

#ifndef ARMYSET_H
#define ARMYSET_H

#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"
#include "armyproto.h"


//! A collection of Army prototype objects.
/**
 * An Armyset is a complete set of Army prototype objects.  An Army prototype
 * is a kind of Army, as opposed to an Army unit instance (e.g. on the game
 * map).  See the Army class for more information about what an Army prototype
 * is.  The Armyset describes the size of the graphic tiles that each Army 
 * graphic occupies on the screen (Army::d_tilesize).
 * Two special images are kept with the Armyset:  the ship picture, and the
 * planted standard picture.
 *
 * The ship picture is what the Stack looks like when it is in a boat.
 * The planted standard is what the player's standard looks like when it has
 * been planted in the ground.
 *
 * Armysets are most often referred to by their Id (Armyset::d_id), but may 
 * sometimes be referred to by their name (Armyset::d_name) or subdirectory 
 * name (Armyset::d_dir).
 *
 * Armyset objects are loaded from an armyset configuration file.
 *
 * Armyset objects are created by the armyset editor.
 *
 * Every Player has an Armyset that dictates the characteristics of the
 * player's forces, but in practise there is only one Armyset per scenario.
 *
 * The armyset configuration file is a same named XML file inside the Armyset's
 * directory.  E.g. army/${Armyset::d_dir}/${Armyset::d_dir}.xml.
 */
class Armyset: public std::list<ArmyProto *>, public sigc::trackable
{
    public:

	//! Default constructor.
	/**
	 * Make a new Armyset.
	 *
	 * @param id    The unique Id of this Armyset among all other Armyset
	 *              objects.  Must be more than 0.  
	 * @param name  The name of the Armyset.  Analagous to Armyset::d_name.
	 */
	Armyset(Uint32 id, std::string name);
	//! Loading constructor.
	/**
	 * Load armyset XML entities from armyset configuration files.
	 */
        Armyset(XML_Helper* helper);
	//! Destructor.
        ~Armyset();

	/**
	 * @param helper  An opened armyset configuration file.
	 */
	//! Save the Armyset to an Armyset configuration file.
	bool save(XML_Helper* helper);

	//! Returns the size of this armyset.
        /** 
         * @return The number of Army prototype objects in the armyset or 0 
	 *         on error (an armyset should never have a size of 0).
         */
        Uint32 getSize() const {return size();}

	//! Get the tile size of the Armyset.
	/**
	 * The width and height of the Army graphic images as they appear
	 * on the screen.
	 * Analagous to the tileset.d_tilesize XML entity in the armyset 
	 * configuration file.
	 */
        Uint32 getTileSize() const {return d_tilesize;}

	//! Get the unique identifier for this armyset.
	/**
	 * Analagous to the tileset.d_id XML entity in the armyset 
	 * configuration file.
	 */
        Uint32 getId() const {return d_id;}

	//! Set the unique identifier for this armyset.
	/**
	 * @note This method is only used in the armyset editor.  
	 */
        void setId(Uint32 id) {d_id = id;}

	//! Returns the name of the armyset.
        /** 
	 * Analagous to the tileset.d_name XML entity in the armyset 
	 * configuration file.
	 *
         * @return The name or an empty string on error.
         */
        std::string getName() const {return d_name;}

	//! Set the name of the armyset.
	/**
	 * @note This method is only used in the armyset editor.
	 */
        void setName(std::string name) {d_name = name;}

	//! Get the subdirectory name of the armyset.
	/**
	 * This value does not contain a path (e.g. no slashes).  It is the
	 * name of an armyset directory inside army/.
	 *
	 * @return The name of the subdirectory the Armyset is held in.
	 */
        std::string getSubDir() const {return d_dir;}

	//! Set the subdirectory that the armyset is in.
        void setSubDir(std::string dir) {d_dir = dir;}

	//! Get the image of the stack in a ship (minus the mask).
	SDL_Surface *getShipPic() const {return d_ship;}

	//! Set the image of the stack in a ship
	void setShipImage(SDL_Surface *ship) {d_ship = ship;};

	//! Get the mask portion of the image of the stack in a ship.
	SDL_Surface *getShipMask() const {return d_shipmask;}

	//! Set the mask portion of the image of the stack in a ship.
	void setShipMask(SDL_Surface *shipmask) {d_shipmask = shipmask;};

	//! Get the image of the planted standard (minus the mask).
	SDL_Surface *getStandardPic() const {return d_standard;}

	//! Set the image of the planted standard (minus the mask).
	void setStandardImage(SDL_Surface *s) {d_standard = s;};

	//! Get the mask portion of the image of the planted standard.
	SDL_Surface *getStandardMask() const {return d_standard_mask;}

	//! Set the mask portion of the image of the planted standard.
	void setStandardMask(SDL_Surface *s) {d_standard_mask = s;};

	//! Find an army with a type in this armyset.
	/**
	 * Scan the Army prototype objects in this Armyset and return it.
	 *
	 * @note This is only used for the editor.  Most callers should use 
	 * Armysetlist::getArmy instead.
	 *
	 * @param army_type  The army type id of the Army prototype object
	 *                   to search for in this Armyset.
	 *
	 * @return The Army with the given army type id, or NULL if none
	 *         could be found.
	 */
	ArmyProto * lookupArmyByType(Uint32 army_type);
    private:

        //! Callback function for the army tag (see XML_Helper)
        bool loadArmyProto(std::string tag, XML_Helper* helper);

	//! Load the image and mask of the stack's ship picture.
	void loadShipPic();

	//! Load the image and mask of the planted standard.
	void loadStandardPic();
        
	//! The unique Id of this armyset.
	/**
	 * This Id is unique among all other armysets.
	 * It is analgous to tileset.d_id in the armyset configuration files.
	 */
        Uint32 d_id;

	//! The name of the Armyset.
	/**
	 * This value appears in game configuration dialogs.
	 * It is analgous to tileset.d_name in the armyset configuration files.
	 */
        std::string d_name;

	//! The subdirectory of the Armyset.
	/**
	 * This is the name of the subdirectory that the Armyset files are
	 * residing in.  It does not contain a path (e.g. no slashes).
	 * Armyset files sit in the army/ directory.
	 */
        std::string d_dir;

	//! The size of each army tile as rendered in the game.
	/**
	 * The tile size represents the height and width in pixels of the 
	 * army picture.
	 * The actual army picture holds the unshaded image, and then the
	 * mask portion of the image to it's right.
	 * The tilesize is the height of the army image file, and it is also
	 * precisely equal to half of the image's width.
	 */
	Uint32 d_tilesize;

	//! The unshaded picture of the stack when it's in a boat.
	SDL_Surface *d_ship;

	//! The mask of what to shade with the player's colour on the boat.
	SDL_Surface *d_shipmask;

	//! The unshaded picture of the planted standard.
	SDL_Surface *d_standard;

	//! The mask of what to shade with the player's colour on the standard.
	SDL_Surface *d_standard_mask;
};

#endif // ARMYSET_H

