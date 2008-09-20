// Copyright (C) 2008 Ben Asselstine
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

#ifndef CITYSET_H
#define CITYSET_H

#include <string>
#include <vector>
#include <SDL.h>
#include <sigc++/trackable.h>

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
        Cityset(XML_Helper* helper);
	//! Destructor.
        ~Cityset();

	//! Get the directory in which the cityset configuration file resides.
        std::string getSubDir() const {return d_dir;}

	//! Set the direction where the shieldset configuration file resides.
        void setSubDir(std::string dir) {d_dir = dir;}

        //! Returns the name of the cityset.
        std::string getName() const {return d_name;}

        //! Returns the description of the cityset.
        std::string getInfo() const {return d_info;}

        //! Returns the width and height in pixels of the city images.
        Uint32 getTileSize() const {return d_tileSize;}

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
        Uint32 d_tileSize;

	//! The subdirectory of the cityset.
	/**
	 * This is the name of the subdirectory that the Cityset files are
	 * residing in.  It does not contain a path (e.g. no slashes).
	 * Cityset directories sit in the citysets/ directory.
	 */
        std::string d_dir;
};

#endif // CITYSET_H

// End of file
