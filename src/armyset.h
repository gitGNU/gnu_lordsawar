//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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
#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"
#include "armyproto.h"
#include "defs.h"
#include "shield.h"
#include "File.h"
#include "set.h"
#include "hero.h"

using namespace std;

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
	static std::string d_tag; 
	static std::string file_extension; 

	//! Default constructor.
	/**
	 * Make a new Armyset.
	 *
	 * @param id    The unique Id of this Armyset among all other Armyset
	 *              objects.  Must be more than 0.  
	 * @param name  The name of the Armyset.  Analagous to Armyset::d_name.
	 */
	Armyset(guint32 id, std::string name);
	//! Loading constructor.
	/**
	 * Load armyset XML entities from armyset configuration files.
	 */
        Armyset(XML_Helper* helper, std::string directory);

        //! Copy constructor.
        Armyset(const Armyset& armyset);

	static Armyset *create(std::string filename, bool &unsupported);

        static Armyset *copy (const Armyset *orig);

	//! Destructor.
        ~Armyset();

	/**
	 * @param helper  An opened armyset configuration file.
	 */
	//! Save the Armyset to an Armyset configuration file.
	bool save(XML_Helper* helper) const;
        
        bool save(std::string filename, std::string extension) const;

	//! Get the tile size of the Armyset.
	/**
	 * The width and height of the Army graphic images as they appear
	 * on the screen.
	 * Analagous to the armyset.d_tilesize XML entity in the armyset 
	 * configuration file.
	 */
        guint32 getTileSize() const {return d_tilesize;}

        void setTileSize(guint32 tile_size) {d_tilesize = tile_size;}

	//! Get the unique identifier for this armyset.
	/**
	 * Analagous to the armyset.d_id XML entity in the armyset 
	 * configuration file.
	 */
        guint32 getId() const {return d_id;}

	//! Set the unique identifier for this armyset.
        void setId(guint32 id) {d_id = id;}

	//! Returns the name of the armyset.
        /** 
	 * Analagous to the armyset.d_name XML entity in the armyset 
	 * configuration file.
	 *
         * @return The name or an empty string on error.
         */
        std::string getName() const {return _(d_name.c_str());}

	//! Set the name of the armyset.
	/**
	 * @note This method is only used in the armyset editor.
	 */
        void setName(std::string name) {d_name = name;}

	//! Get the copyright holders for this armyset.
	std::string getCopyright() const {return d_copyright;};

	//! Set the copyright holders on the armyset.
	void setCopyright(std::string copy) {d_copyright = copy;};

	//! Get the license of this armyset.
	std::string getLicense() const {return d_license;};

        //! Returns the description of the armyset.
        std::string getInfo() const {return _(d_info.c_str());}

	//! Set the license for this armyset.
	void setLicense(std::string license) {d_license = license;};

	//! Set the description of the armyset.
	/**
	 * @note This method is only used in the armyset editor.
	 */
        void setInfo(std::string info) {d_info = info;}

	//! Get the base name of the armyset.
	/**
	 * This value does not contain a path (e.g. no slashes).  It is the
	 * name of an armyset directory inside army/.
	 *
	 * @return The basename of the file that the Armyset is held in.
	 */
        std::string getBaseName() const {return d_basename;}

	//! Set the base name of the file that the armyset is in.
        void setBaseName(std::string bname) {d_basename = bname;}

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
	void setShipImageName(std::string n) {d_stackship_name = n;};

	//! Get the name of the file holding the image of the stack in a boat.
	std::string getShipImageName() {return d_stackship_name;};

	//! Set the name of the file holding the image of the hero's flag.
	void setStandardImageName(std::string n) {d_standard_name = n;};

	//! Get the name of the file holding the image of the hero's flag.
	std::string getStandardImageName() {return d_standard_name;};

	//! Set the name of the file holding the image of the bag.
	void setBagImageName(std::string n) {d_bag_name = n;};

	//! Get the name of the file holding the image of the bag.
	std::string getBagImageName() {return d_bag_name;};

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

	ArmyProto * lookupArmyByName(std::string name) const;

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
	void getFilenames(std::list<std::string> &files);
        //! Delete the armyset's temporary directory.
        void clean_tmp_dir() const;

	void instantiateImages(bool &broken);
	void uninstantiateImages();
	void loadStandardPic(std::string image_filename, bool &broken);
	void loadShipPic(std::string image_filename, bool &broken);
	void loadBagPic(std::string image_filename, bool &broken);

	std::string getConfigurationFile() const;
	static std::list<std::string> scanUserCollection();
	static std::list<std::string> scanSystemCollection();
	static void switchArmyset(Army *army, const Armyset *armyset);
	static void switchArmyset(ArmyProdBase *army, const Armyset *armyset);
	static void switchArmysetForRuinKeeper(Army *army, const Armyset *armyset);
        static bool copy(std::string src, std::string dest);

	const ArmyProto * getRandomRuinKeeper() const;
	const ArmyProto *getRandomAwardableAlly() const;

        std::string getFileFromConfigurationFile(std::string file);
        bool replaceFileInConfigurationFile(std::string file, std::string new_file);
        //! Load the armyset again.
        void reload(bool &broken);
        guint32 calculate_preferred_tile_size() const;

        //! callback to upgrade old files.
        static bool upgrade(std::string filename, std::string old_version, std::string new_version);
        static void support_backward_compatibility();

    private:

        //! Callback function for the army tag (see XML_Helper)
        bool loadArmyProto(std::string tag, XML_Helper* helper);
        
	//! The unique Id of this armyset.
	/**
	 * This Id is unique among all other armysets.
	 * It is analgous to armyset.d_id in the armyset configuration files.
	 */
        guint32 d_id;

	//! The name of the Armyset.
	/**
	 * This value appears in game configuration dialogs.
	 * It is analgous to armyset.d_name in the armyset configuration files.
	 */
        std::string d_name;

	//! The armyset has these copyright holders.
	std::string d_copyright;

	//! The license of the armyset.
	std::string d_license;

	//! The basename of the Armyset.
	/**
	 * This is the base name of the file that the Armyset files are
	 * residing in.  It does not contain a path (e.g. no slashes).
	 * Armyset files sit in the army/ directory.
	 */
        std::string d_basename;

	//! The description of the Armyset.
	/**
	 * Equates to the armyset.d_info XML entity in the armyset
	 * configuration file.
	 * This value is not used.
	 */
        std::string d_info;

	//! The size of each army tile as rendered in the game.
	/**
	 * The tile size represents the height and width in pixels of the 
	 * army picture.
	 * The actual army picture holds the unshaded image, and then the
	 * mask portion of the image to it's right.
	 * The tilesize is the height of the army image file, and it is also
	 * precisely equal to half of the image's width.
	 */
	guint32 d_tilesize;

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
	std::string d_standard_name;

	//! The name of the file that holds the picture of stack on water.
	std::string d_stackship_name;

	//! The name of the file that holds the picture of the sack of items.
	std::string d_bag_name;
};

#endif // ARMYSET_H

