//  Copyright (C) 2008, Ben Asselstine
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

#ifndef SHIELD_H
#define SHIELD_H

#include <SDL.h>
#include <string>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "defs.h"

class Player;
class XML_Helper;

//! A graphic of a shield.
/**
 * This class is the atom of every shield. It contains all data related to
 * a single Shield type of a Shieldset.
 * Shields come in three sizes: small, medium and large (Shield::ShieldType).
 * Shields are associated with one of 9 players (Shield::ShieldColours).
 * These so-called players are not Player objects; instead they are notional.
 *
 * Every Shield object has an image and a mask.  The mask identifies the
 * portion of the Shield to shade in the Player's colour (Player::d_color).
 * The mask appears on the right side the shield image file.
 *
 */
class Shield : public sigc::trackable
{
    public:

	//! The notional player that the Shield goes with.
	enum ShieldColour {WHITE = 0, GREEN = 1, YELLOW = 2, LIGHT_BLUE = 3,
	RED = 4, DARK_BLUE = 5, ORANGE = 6, BLACK = 7, NEUTRAL = 8};

	//! The size of the shield.
	enum ShieldType {
	  //! Small shields are shown on the OverviewMap object.
	  SMALL = 0, 
	  //! Medium shields are shown in the top right of the GameWindow.
	  MEDIUM = 1, 
	  //! Large shields are shown in the DiplomacyDialog and FightWindow.
	  LARGE = 2
	};

	//! Loading constructor.
        /**
	 * Make a new Shield object by readiang it in from an opened shieldset
	 * configuration file.
	 *
         * @param helper  The opened shieldset configuration file to read the
	 *                shield object from.
         */
        Shield(XML_Helper* helper);
        
	//! Destructor.
        virtual ~Shield();

        // Set functions:
        
        //! Set the basic image of the shield.
        void setPixmap(SDL_Surface* pixmap);

        //! Set the mask of the shield.
        void setMask(SDL_Surface* mask);

        //! Set the shieldset of the shield.
        void setShieldset(std::string shieldset) {d_shieldset = shieldset;};

	//! Set the basename of the shield picture's filename.
	void setImageName(std::string image) {d_image = image;}
        
        // Get functions
        
        //! Get the size of this shield.
        Uint32 getType() const {return d_type;}

        //! Get the player that this shield will belong to.
	Uint32 getColour() const {return d_colour;}

        //! Get the image of the shield.
        SDL_Surface* getPixmap() const;

        //! Returns the mask of the shield.
        SDL_Surface* getMask() const {return d_mask;}

	//! Returns the basename of the picture's filename.
	std::string getImageName() const {return d_image;}

	//! Get the shieldset of the shield.
	std::string getShieldset() const {return d_shieldset;}

    protected:

	//! The size of the shield.
	/**
	 * Equates to the shieldset.shield.d_type XML entities in the shieldset
	 * configuration file.
	 * Equates to the Shield::ShieldType enumeration.
	 */
        Uint32 d_type;

	//! The player of the shield.
	/**
	 * Equates to the shieldset.shield.d_colour XML entities in the 
	 * shieldset configuration file.
	 * Equates to the Shield::ShieldColour enumeration.
	 */
	Uint32 d_colour;

	//! The unshaded image portion of the shield's picture.
        SDL_Surface* d_pixmap;

	//! The portion of the shield's image to shade in the player's colour.
	/**
	 * The mask appears to the right of the image in the shield's picture.
	 * The colour that shades the mask is dictated by Player::d_colour.
	 */
        SDL_Surface* d_mask;

	//! The basename of the shield's picture file.
	/**
	 * Returns the filename that holds the image for this Shield.
	 * The filename does not have a path, and the filename does
	 * not have an extension (e.g. .png).
	 */
	std::string d_image;

	//! The Shieldset to which this Shield belongs.
	std::string d_shieldset;
};

#endif // SHIELD_H
