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

#ifndef SHIELD_H
#define SHIELD_H

#include <gtkmm.h>
#include <string>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "shieldstyle.h"

class Player;
class XML_Helper;
class Shieldset;

//! A single set of shields for a player
/**
 *
 */
class Shield : public std::list<ShieldStyle*>, public sigc::trackable
{
    public:

	//! The xml tag of this object in a shieldset configuration file.
	static std::string d_tag; 

	//! The notional player that the Shield goes with.
	enum Colour {WHITE = 0, GREEN = 1, YELLOW = 2, DARK_BLUE = 3, ORANGE = 4,
	  LIGHT_BLUE = 5, RED = 6, BLACK = 7, NEUTRAL = 8};

	//! Loading constructor.
        /**
	 * Make a new Shield object by reading it in from an opened shieldset
	 * configuration file.
	 *
         * @param helper  The opened shieldset configuration file to read the
	 *                shield object from.
         */
        Shield(XML_Helper* helper);
        
	//! Default constructor.
	Shield(Shield::Colour owner, Gdk::Color color);

	//! Destructor.
        virtual ~Shield();


	// Get Methods

        //! Returns the player that this shield will belong to.
	guint32 getOwner() const {return d_owner;}

        //! Returns the colour of the player shield.
	Gdk::Color getColor() const {return d_color;}


	// Set Methods

	//! Sets the colour of the player shield.
	void setColor(Gdk::Color c) {d_color = c;}


	// Methods that operate on class data and do not modify the class.

	//! Save the shield to an opened shieldset configuration file.
	bool save(XML_Helper *helper) const;

	//! Get the first shieldstyle in the shield with the given type.
	ShieldStyle *getFirstShieldstyle(ShieldStyle::Type type);


	// Methods that operate on class data and modify the class.

	//! Load the images associated with this shield.
	void instantiateImages(Shieldset *s);

	//! Destroy the images associated with this shield.
	void uninstantiateImages();


	// Static Methods

	/**
	 * Get the default colour for the Player with the given Id.
	 *
	 * @note This colour is used to graphically shade Army, Shield, Flags,
	 * and selector pictures.
	 *
	 * @note This is not used to obtain the Neutral player's colour.
	 *
	 * @param player_no  The player's Id for which we want the colour.
	 *
	 * @return The default colour associated with the player.
	 */
	//! Get standard colour for a player.
	static Gdk::Color get_default_color_for_no(int player_no);

	//! Get standard colour for the neutral player.
	static Gdk::Color get_default_color_for_neutral();

	//! Convert the Shield::Colour enumerated value to a string.
	static std::string colourToString(const Shield::Colour c);
    protected:

	//! The player of the shield.
	/**
	 * Equates to the shieldset.shield.d_colour XML entities in the 
	 * shieldset configuration file.
	 * Equates to the Shield::Colour enumeration.
	 */
	guint32 d_owner;

	//! The player's colour.
	/**
	 * Mask portions of images are shaded in this colour.
	 */
	Gdk::Color d_color;
};

#endif // SHIELD_H
