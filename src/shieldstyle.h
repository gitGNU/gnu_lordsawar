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

#ifndef SHIELDSTYLE_H
#define SHIELDSTYLE_H

#include <gtkmm.h>
#include <string>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include "PixMask.h"


class Player;
class XML_Helper;
class Shieldset;

//! A graphic of a shield.
/**
 * This class is the atom of every shield. It contains all data related to
 * a single ShieldStyle type of a Shield.  ShieldStyles come in three sizes: 
 * small, medium and large (ShieldStyle::Type).
 *
 * Every ShieldStyle object has an image and a mask.  The mask identifies the
 * portion of the ShieldStyle to shade in the Player's colour (Player::d_color).
 * The mask appears on the right side the shield image file.
 *
 */
class ShieldStyle : public sigc::trackable
{
    public:

	//! The xml tag of this object in a shieldset configuration file.
	static std::string d_tag; 

	//! The size of the shield.
	enum Type {
	  //! Small shields are shown on the OverviewMap object.
	  SMALL = 0, 
	  //! Medium shields are shown in the top right of the GameWindow.
	  MEDIUM = 1, 
	  //! Large shields are shown in the DiplomacyDialog and FightWindow.
	  LARGE = 2
	};
	static std::string shieldStyleTypeToString(const ShieldStyle::Type type);
	static ShieldStyle::Type shieldStyleTypeFromString(const std::string str);

	//! Loading constructor.
        /**
	 * Make a new ShieldStyle object by readiang it in from an opened shieldset
	 * configuration file.
	 *
         * @param helper  The opened shieldset configuration file to read the
	 *                shield object from.
         */
        ShieldStyle(XML_Helper* helper);
        
	//! Destructor.
        virtual ~ShieldStyle();

	bool save(XML_Helper *helper);
        // Set functions:
        
        //! Set the basic image of the shield.
        void setImage(PixMask* image) {d_image = image;};

        //! Set the mask of the shield.
        void setMask(PixMask* mask) {d_mask = mask;}

	//! Set the basename of the shield picture's filename.
	void setImageName(std::string name) {d_image_name = name;}
        
        // Get functions
        
        //! Get the size of this shield.
        guint32 getType() const {return d_type;}

        //! Get the image of the shield.
	PixMask* getImage() {return d_image;}

        //! Returns the mask of the shield.
	PixMask* getMask() {return d_mask;}

	//! Returns the basename of the picture's filename.
	std::string getImageName() const {return d_image_name;}

	void instantiateImages(std::string filename, Shieldset *s);
	void uninstantiateImages();
    protected:

	//! The size of the shield. (small, medium, or large)
	/**
	 * Equates to the shieldset.shield.d_type XML entities in the shieldset
	 * configuration file.
	 * Equates to the ShieldStyle::Type enumeration.
	 */
        guint32 d_type;

	//! The unshaded image portion of the shield's picture.
	PixMask* d_image;

	//! The portion of the shield's image to shade in the player's colour.
	/**
	 * The mask appears to the right of the image in the shield's picture.
	 * The colour that shades the mask is dictated by Player::d_colour.
	 */
	PixMask* d_mask;

	//! The basename of the shield's picture file.
	/**
	 * Returns the filename that holds the image for this ShieldStyle.
	 * The filename does not have a path, and the filename does
	 * not have an extension (e.g. .png).
	 */
	std::string d_image_name;
};

#endif // SHIELDSTYLE_H
