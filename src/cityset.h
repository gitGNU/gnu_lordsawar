// Copyright (C) 2008, 2009 Ben Asselstine
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

#ifndef CITYSET_H
#define CITYSET_H

#include <string>
#include <vector>
#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include "PixMask.h"

#include "defs.h"

class XML_Helper;

//! A list of city graphic objects in a city theme.
/** 
 * Every scenario has a city set; it is the theme of the city graphics 
 * within the game. 
 *
 * The Cityset dictates the size of city images.
 *
 * Citysets are referred to by their subdirectory name.
 *
 * The cityset configuration file is a same named XML file inside the 
 * cityset's directory.  E.g. cityset/${Cityset::d_dir}/${Cityset::d_dir}.xml.
 */
class Cityset : public sigc::trackable
{
    public:
	//! The xml tag of this object in a cityset configuration file.
	static std::string d_tag; 

	//! Default constructor.
	/**
	 * Make a new Cityset object by reading it in from the cityset
	 * configuration file.
	 *
	 * @param helper  The opened cityset configuration file to load the
	 *                Cityset from.
	 */
        Cityset(XML_Helper* helper, bool private_collection = false);

	static Cityset *create(std::string file, bool private_collection = false);
	//! Destructor.
        ~Cityset();

	//! Get the directory in which the cityset configuration file resides.
        std::string getSubDir() const {return d_dir;}

	//! Set the direction where the shieldset configuration file resides.
        void setSubDir(std::string dir) {d_dir = dir;}

        //! Returns the name of the cityset.
        std::string getName() const {return _(d_name.c_str());}

	/**
	 * Analagous to the cityset.d_id XML entity in the cityset
	 * configuration file.
	 */
        guint32 getId() const {return d_id;}

	//! Set the unique identifier for this cityset.
        void setId(guint32 id) {d_id = id;}

        //! Returns the description of the cityset.
        std::string getInfo() const {return _(d_info.c_str());}

        //! Returns the width and height in pixels of the city images.
        guint32 getTileSize() const {return d_tileSize;}

	void setCitiesFilename(std::string s) {d_cities_filename = s;};
	std::string getCitiesFilename() {return d_cities_filename;};
	void setRazedCitiesFilename(std::string s) {d_razedcities_filename = s;};
	std::string getRazedCitiesFilename() {return d_razedcities_filename;};
	void setPortFilename(std::string s) {d_port_filename = s;};
	std::string getPortFilename() {return d_port_filename;};
	void setSignpostFilename(std::string s) {d_signpost_filename = s;};
	std::string getSignpostFilename() {return d_signpost_filename;};
	void setRuinsFilename(std::string s) {d_ruins_filename = s;};
	std::string getRuinsFilename() {return d_ruins_filename;};
	void setTemplesFilename(std::string s) {d_temples_filename = s;};
	std::string getTemplesFilename() {return d_temples_filename;};
	void setTowersFilename(std::string s) {d_towers_filename = s;};
	std::string getTowersFilename() {return d_towers_filename;};

	void setCityImage(guint32 i, PixMask *p) {citypics[i] = p;};
	PixMask *getCityImage(guint32 i) {return citypics[i];};
	void setRazedCityImage(guint32 i, PixMask *p) {razedcitypics[i] = p;};
	PixMask *getRazedCityImage(guint32 i) {return razedcitypics[i];};
	PixMask *getPortImage() {return port;};
	void setPortImage(PixMask *p) {port = p;};
	PixMask *getSignpostImage() {return signpost;};
	void setSignpostImage(PixMask *p) {signpost = p;};
	void setRuinImage(guint32 i, PixMask *p) {ruinpics[i] = p;};
	PixMask *getRuinImage(guint32 i) {return ruinpics[i];};
	void setTempleImage(guint32 i, PixMask *p) {templepics[i] = p;};
	PixMask *getTempleImage(guint32 i) {return templepics[i];};
	void setTowerImage(guint32 i, PixMask *p) {towerpics[i] = p;};
	PixMask *getTowerImage(guint32 i) {return towerpics[i];};

	//! Return whether this is a cityset in the user's personal collection.
	bool fromPrivateCollection() {return private_collection;};

	//! get filenames in this cityset, excepting the configuration file.
	void getFilenames(std::list<std::string> &files);
    private:

        // DATA
	//! The name of the cityset.
	/**
	 * This equates to the cityset.d_name XML entity in the cityset
	 * configuration file.
	 * This name appears in the dialogs where the user is asked to 
	 * select a particular Cityset.
	 */
        std::string d_name;

	//! A unique numeric identifier among all citysets.
	guint32 d_id;

	//! The description of the cityset.
	/**
	 * Equates to the cityset.d_info XML entity in the cityset 
	 * configuration file.
	 * This value is not used.
	 */
        std::string d_info;

	//! The size of each city image onscreen.
	/**
	 * Equates to the cityset.d_tilesize XML entity in the cityset
	 * configuration file.
	 * It represents the size in pixels of the width and height of city
	 * imagery onscreen.
	 */
        guint32 d_tileSize;

	//! The subdirectory of the cityset.
	/**
	 * This is the name of the subdirectory that the Cityset files are
	 * residing in.  It does not contain a path (e.g. no slashes).
	 * Cityset directories sit in the citysets/ directory.
	 */
        std::string d_dir;


	std::string d_cities_filename;
	std::string d_razedcities_filename;
	std::string d_port_filename;
	std::string d_signpost_filename;
	std::string d_ruins_filename;
	std::string d_temples_filename;
	std::string d_towers_filename;
	PixMask *citypics[MAX_PLAYERS + 1];
	PixMask *razedcitypics[MAX_PLAYERS];
	PixMask *port;
	PixMask *signpost;
	PixMask *ruinpics[RUIN_TYPES];
	PixMask *templepics[TEMPLE_TYPES];
	PixMask *towerpics[MAX_PLAYERS];

	//! Whether this is a system cityset, or one that the user made.
	bool private_collection;
};

#endif // CITYSET_H

// End of file
