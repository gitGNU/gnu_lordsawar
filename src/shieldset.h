//  Copyright (C) 2008, 2009 Ben Asselstine
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

#ifndef SHIELDSET_H
#define SHIELDSET_H

#include <gtkmm.h>
#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"
#include "shield.h"
#include "set.h"

#include "defs.h"

struct rgbshift;
//! A list of Shield graphic objects in a shield theme.
/**
 * Every scenario has a shield set; it is the theme of the shield graphics 
 * within the game.  Shields come in three sizes -- small, medium and large.  
 * Small shields appear on the OverviewMap.  Medium shields appear in the turn 
 * indicator in the top right of the GameWindow.  Large shields appear in many 
 * dialogs, chiefly the FightWindow, and DiplomacyDialog.
 * Every shield belongs to one of 9 players (the ninth is the Neutral player).
 * The players aren't Player objects in this case; instead it refers to a 
 * Shield::ShieldColour.  e.g. Not `The Sirians' but rather the `White player'
 * of the scenario.
 *
 * The Shieldset dictates the dimensions of these three sizes of shields.
 *
 * Shieldsets are referred to by their subdirectory name.
 *
 * The shieldset configuration file is a same named XML file inside the 
 * Shieldset's directory.  
 * E.g. shield/${Shieldset::d_subdir}/${Shieldset::d_subdir}.xml.
 */
class Shieldset: public std::list<Shield *>, public sigc::trackable, public Set
{
    public:

	//! The xml tag of this object in a shieldset configuration file.
	static std::string d_tag; 
	static std::string file_extension;

	//! Load a Shieldset from a shieldset configuration file.
	/**
	 * Make a new Shieldset object by reading it in from the shieldset
	 * configuration file.
	 *
	 * @param helper  The opened shieldset configuration file to load the
	 *                Shieldset from.
	 */
        Shieldset(XML_Helper* helper, std::string directory);

	static Shieldset *create(std::string filename);
	//! Destructor.
        ~Shieldset();

	bool save(XML_Helper *helper);

	//! Return the number of pixels high the small shields are.
	guint32 getSmallHeight() const {return d_small_height;}

	//! Return the number of pixels wide the small shields are.
	guint32 getSmallWidth() const {return d_small_width;}

	//! Return the number of pixels high the medium shields are.
	guint32 getMediumHeight() const {return d_medium_height;}

	//! Return the number of pixels wide the medium shields are.
	guint32 getMediumWidth() const {return d_medium_width;}

	//! Return the number of pixels the large shields are.
	guint32 getLargeHeight() const {return d_large_height;}

	//! Return the number of pixels wide the large shields are.
	guint32 getLargeWidth() const {return d_large_width;}

	//! Return the total number of shields in this shieldset.
        guint32 getSize() const {return size();}

	//! Return the name of the Shieldset.
        std::string getName() const {return _(d_name.c_str());}

	//! Return the copyright holders of the shieldset.
	std::string getCopyright() const {return d_copyright;};

	//! Return the license of the shieldset.
	std::string getLicense() const {return d_license;};

	//! Get the unique identifier for this shieldset.
	/**
	 * Analagous to the shieldset.d_id XML entity in the shieldset
	 * configuration file.
	 */
        guint32 getId() const {return d_id;}

	//! Set the unique identifier for this shieldset.
        void setId(guint32 id) {d_id = id;}

	//! Set the name of the Shieldset.
        void setName(std::string name) {d_name = name;}

	//! Set the copyright holders of the shieldset.
	void setCopyright(std::string copy) {d_copyright = copy;};

	//! Set the license of this shieldset.
	void setLicense(std::string license) {d_license = license;};

	//! Get the directory in which the shieldset configuration file resides.
        std::string getSubDir() const {return d_subdir;}

	//! Set the direction where the shieldset configuration file resides.
        void setSubDir(std::string dir) {d_subdir = dir;}

	//! Find the shield of a given size and colour in this Shieldset.
	/**
	 * Scan through all Shield objects in this set for first one that is 
	 * the desired size, and for the desired player.
	 *
	 * @param type    One of the values in Shield::ShieldType.
	 * @param colour  One of the values in Shield::ShieldColour.
	 *
	 * @return A pointer to the shield that matches the size and player.
	 *         If no Shield object could be found that matches the given
	 *         parameters, NULL is returned.
	 */
	ShieldStyle * lookupShieldByTypeAndColour(guint32 type, guint32 colour);

	Gdk::Color getColor(guint32 owner);
	struct rgb_shift getMaskColorShifts(guint32 owner);

	//! get filenames in this shieldset, excepting the configuration file.
	void getFilenames(std::list<std::string> &filenames);

	void instantiateImages();
	void uninstantiateImages();
	std::string getConfigurationFile();

	static std::list<std::string> scanSystemCollection();
	static std::list<std::string> scanUserCollection();
    private:

	//! Callback function to load Shield objects into the Shieldset.
	bool loadShield(std::string tag, XML_Helper* helper);

	//! The name of the Shieldset.
	/**
	 * This equates to the shieldset.d_name XML entity in the shieldset
	 * configuration file.
	 * This name appears in the dialogs where the user is asked to 
	 * select a particular Shieldset.
	 */
        std::string d_name;

	//! The copyright holders of the shieldset.
	std::string d_copyright;

	//! The license of the shieldset.
	std::string d_license;

	//! A unique numeric identifier among all shieldset.
	guint32 d_id;

	//! The subdirectory of the Shieldset.
	/**
	 * This is the name of the subdirectory that the Shieldset files are
	 * residing in.  It does not contain a path (e.g. no slashes).
	 * Shieldset directories sit in the shield/ directory.
	 */
        std::string d_subdir;

	//! The number of pixels high the small shield occupies onscreen.
	/**
	 * Equates to the shieldset.d_small_height XML entity in the shieldset 
	 * configuration file.
	 */
	guint32 d_small_height;

	//! The number of pixels wide the small shield occupies onscreen.
	/**
	 * Equates to the shieldset.d_small_width XML entity in the shieldset 
	 * configuration file.
	 */
	guint32 d_small_width;

	//! The number of pixels high the medium shield occupies onscreen.
	/**
	 * Equates to the shieldset.d_medium_height XML entity in the shieldset 
	 * configuration file.
	 */
	guint32 d_medium_height;

	//! The number of pixels wide the medium shield occupies onscreen.
	/**
	 * Equates to the shieldset.d_medium_width XML entity in the shieldset 
	 * configuration file.
	 */
	guint32 d_medium_width;

	//! The number of pixels high the large shield occupies onscreen.
	/**
	 * Equates to the shieldset.d_large_height XML entity in the shieldset 
	 * configuration file.
	 */
	guint32 d_large_height;

	//! The number of pixels wide the large shield occupies onscreen.
	/**
	 * Equates to the shieldset.d_large_width XML entity in the shieldset 
	 * configuration file.
	 */
	guint32 d_large_width;
};

#endif // SHIELDSET_H

