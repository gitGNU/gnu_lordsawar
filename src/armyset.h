//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2015 Ben Asselstine
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

#ifndef ARMYSET_H
#define ARMYSET_H

#include <gtkmm.h>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "armyproto.h"
#include "set.h"
#include "hero.h"

//! A collection of Army prototype objects.
/**
 * An Armyset is a complete set of Army prototype objects.  An Army prototype
 * is a kind of Army, as opposed to an Army unit instance (e.g. on the game
 * map).  See the Army class for more information about what an Army prototype
 * is.  The Armyset describes the size of the graphic tiles that each Army 
 * graphic occupies on the screen (Army::d_tilesize).
 * There special images are kept with the Armyset:  the ship picture, the
 * planted standard picture, and the bag of items picture.
 *
 * The ship picture is what the Stack looks like when it is in a boat.
 * The planted standard is what the player's standard looks like when it has
 * been planted in the ground.  The bag of items picture is what it looks like
 * when a hero has dropped an item on the ground.
 *
 * Armysets are most often referred to by their Id (Armyset::d_id), but may 
 * sometimes be referred to by their name (Armyset::d_name) or basename 
 * name (Armyset::d_basename).
 *
 * Armyset objects are loaded from an armyset configuration file.
 *
 * Armyset objects are created by the armyset editor.
 *
 * Every Player has an Armyset that dictates the characteristics of the
 * player's forces, but in practise there is only one Armyset per scenario.
 *
 * The armyset configuration file is a tar file that contains an XML file, 
 * and a set of png files.  Filenames have the following form:
 * army/${Armyset::d_basename}.lwa.
 */
class Armyset: public std::list<ArmyProto *>, public sigc::trackable, public Set
{
    public:

	//! The xml tag of this object in an armyset configuration file.
	static Glib::ustring d_tag; 
	static Glib::ustring file_extension; 

	//! Default constructor.
	/**
	 * Make a new Armyset.
	 *
	 * @param id    The unique Id of this Armyset among all other Armyset
	 *              objects.  Must be more than 0.  
	 * @param name  The name of the Armyset.  Analagous to Armyset::d_name.
	 */
	Armyset(guint32 id, Glib::ustring name);
	//! Loading constructor.
	/**
	 * Load armyset XML entities from armyset configuration files.
	 */
        Armyset(XML_Helper* helper, Glib::ustring directory);

        //! Copy constructor.
        Armyset(const Armyset& armyset);

	static Armyset *create(Glib::ustring filename, bool &unsupported);

        static Armyset *copy (const Armyset *orig);

	//! Destructor.
        ~Armyset();

	/**
	 * @param helper  An opened armyset configuration file.
	 */
	//! Save the Armyset to an Armyset configuration file.
	bool save(XML_Helper* helper) const;
        
        bool save(Glib::ustring filename, Glib::ustring ext) const;

	//! Get the image of the stack in a ship (minus the mask).
	PixMask* getShipPic() const {return d_ship;}

	//! Set the image of the stack in a ship
	void setShipImage(PixMask* ship) {d_ship = ship;};

	//! Get the mask portion of the image of the stack in a ship.
	PixMask* getShipMask() const {return d_shipmask;}

	//! Set the mask portion of the image of the stack in a ship.
	void setShipMask(PixMask* shipmask) {d_shipmask = shipmask;};

	//! Get the image of the bag.
	PixMask* getBagPic() const {return d_bag;}

	//! Set the image of the bag.
	void setBagPic(PixMask* s) {d_bag = s;};

	//! Get the image of the planted standard (minus the mask).
	PixMask* getStandardPic() const {return d_standard;}

	//! Set the image of the planted standard (minus the mask).
	void setStandardPic(PixMask* s) {d_standard = s;};

	//! Get the mask portion of the image of the planted standard.
	PixMask* getStandardMask() const {return d_standard_mask;}

	//! Set the mask portion of the image of the planted standard.
	void setStandardMask(PixMask* s) {d_standard_mask = s;};

	//! Set the name of the file holding the image of the stack in a boat.
	void setShipImageName(Glib::ustring n) {d_stackship_name = n;};

	//! Get the name of the file holding the image of the stack in a boat.
	Glib::ustring getShipImageName() {return d_stackship_name;};

	//! Set the name of the file holding the image of the hero's flag.
	void setStandardImageName(Glib::ustring n) {d_standard_name = n;};

	//! Get the name of the file holding the image of the hero's flag.
	Glib::ustring getStandardImageName() {return d_standard_name;};

	//! Set the name of the file holding the image of the bag.
	void setBagImageName(Glib::ustring n) {d_bag_name = n;};

	//! Get the name of the file holding the image of the bag.
	Glib::ustring getBagImageName() {return d_bag_name;};

        //! Find the type id with the highest value and return it.
        guint32 getMaxId() const;

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
	ArmyProto * lookupArmyByType(guint32 army_type) const;

	ArmyProto * lookupArmyByName(Glib::ustring name) const;

	ArmyProto * lookupArmyByStrengthAndTurns(guint32 str, guint32 turns) const;

	ArmyProto * lookupArmyByGender(Hero::Gender gender) const;

	ArmyProto * lookupSimilarArmy(ArmyProto *army) const;
        
        ArmyProto * lookupWeakestQuickestArmy() const;

	//! can this armyset be used within the game?
	bool validate();
	bool validateHero();
	bool validatePurchasables();
	bool validateRuinDefenders();
	bool validateAwardables();
	bool validateShip();
	bool validateStandard();
	bool validateBag();
	bool validateArmyUnitImages();
	bool validateArmyUnitImage(ArmyProto *a, Shield::Colour &c);
	bool validateArmyUnitNames();
	bool validateArmyUnitName(ArmyProto *a);
	bool validateArmyTypeIds();
	//! get filenames in this armyset, excepting the configuration file.
	void getFilenames(std::list<Glib::ustring> &files);

	void instantiateImages(bool &broken);
	void uninstantiateImages();
	void loadStandardPic(Glib::ustring image_filename, bool &broken);
	void loadShipPic(Glib::ustring image_filename, bool &broken);
	void loadBagPic(Glib::ustring image_filename, bool &broken);

	static void switchArmyset(Army *army, const Armyset *armyset);
	static void switchArmyset(ArmyProdBase *army, const Armyset *armyset);
	static void switchArmysetForRuinKeeper(Army *army, const Armyset *armyset);
	const ArmyProto * getRandomRuinKeeper() const;
	const ArmyProto *getRandomAwardableAlly() const;

        //! Load the armyset again.
        void reload(bool &broken);
        guint32 calculate_preferred_tile_size() const;

        //! callback to upgrade old files.
        static bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version);
        static void support_backward_compatibility();

    private:

        //! Callback function for the army tag (see XML_Helper)
        bool loadArmyProto(Glib::ustring tag, XML_Helper* helper);
        
	//! The unshaded picture of the stack when it's in a boat.
	PixMask* d_ship;

	//! The mask of what to shade with the player's colour on the boat.
	PixMask* d_shipmask;

	//! The unshaded picture of the planted standard.
	PixMask* d_standard;

	//! The mask of what to shade with the player's colour on the standard.
	PixMask* d_standard_mask;

	//! The picture of an item when it's lying on the ground.
	PixMask *d_bag;

	//! The name of the file that holds the picture of the hero's flag.
	Glib::ustring d_standard_name;

	//! The name of the file that holds the picture of stack on water.
	Glib::ustring d_stackship_name;

	//! The name of the file that holds the picture of the sack of items.
	Glib::ustring d_bag_name;
};

bool weakest_quickest (const ArmyProto* first, const ArmyProto* second);
#endif // ARMYSET_H

