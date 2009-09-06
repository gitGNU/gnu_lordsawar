// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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

#ifndef ARMY_PROTO_H
#define ARMY_PROTO_H

#include <gtkmm.h>
#include <SDL.h>
#include <string>

class XML_Helper;

#include "armyprotobase.h"

class ArmyProto : public ArmyProtoBase
{
    public:
	//! The xml tag of this object in an armyset configuration file.
	static std::string d_tag; 

	//! Copy constructor.
        ArmyProto(const ArmyProto& armyproto);

	//! Loading constructor.
        ArmyProto(XML_Helper* helper);
        
	//! Create an empty army prototype.
	ArmyProto();

	//! Destructor.
        virtual ~ArmyProto();

        // Set functions:
        
	//! Sets the filename of the image.
	void setImageName(std::string image) {d_image = image;}

        //! Set the basic image of the Army.
        void setPixmap(SDL_Surface* pixmap) {d_pixmap = pixmap;};

        //! Set the image mask of the unit type (for player colours).
        void setMask(SDL_Surface* mask) {d_mask = mask;};

	//! Sets whether or not this Army prototype can found in a ruin.
	void setDefendsRuins(bool defends) {d_defends_ruins = defends; }

	/**
	 * Sets whether or not this Army prototype can be a reward for
	 * Quest, or if Army units of this kind can accompany a new
	 * Hero when one emerges in a City.
	 */
	//! Sets the awardable state of an Army prototype.
	void setAwardable (bool awardable) {d_awardable = awardable; }

        // Get functions
        
        
	//! Returns the basename of the picture's filename
	/**
	 * Returns the filename that holds the image for this Army.
	 * The filename does not have a path, and the filename does
	 * not have an extension (e.g. .png).
	 */
	std::string getImageName() const {return d_image;}

        //! Get the image of the army prototype. 
        SDL_Surface* getPixmap() const {return d_pixmap;};

        //! Returns the mask (read-only) for player colors.
        SDL_Surface* getMask() const {return d_mask;}

	//! Gets whether or not this army type can found in a ruin.
	bool getDefendsRuins() const {return d_defends_ruins; }

	/**
	 * Gets whether or not this army can be a reward for completing a 
	 * Quest, or if an Army unit of this type can accompany a new
	 * Hero when one emerges in a City.
	 */
	//! Gets the awardable state of the Army.
	bool getAwardable() const {return d_awardable; }

        //! Saves the Army prototype to an opened armyset file.
        virtual bool save(XML_Helper* helper) const;
        
	//! Set whether or not this is a hero army prototype.
	void setHero(bool hero) {d_hero = hero;};

	//! Returns whether or not the Army prototype is a Hero.
	bool isHero() const {return d_hero;};

	
    protected:
	bool saveData(XML_Helper* helper) const;
    private:

	//! The picture of the Army prototype.
        SDL_Surface* d_pixmap;

	//! The mask portion of the Army prototype picture.
        SDL_Surface* d_mask;
        
	//! Whether or not the Army prototype can defend a Ruin.
	/**
	 * Some Army unit can be the guardian of a Ruin.  Hero units fight
	 * a single Army unit of this kind when they search a Ruin.  
	 * d_defends_ruin indicates whether this Army unit can defend a Ruin 
	 * or not.
	 *
	 * This value does not change during gameplay.
	 */
	bool d_defends_ruins;

	//! The awardable status of the Army prototype.
	/**
	 * Whether or not this Army prototype can be a reward for a Quest, 
	 * or if Army units of this kind can accompany a new Hero when one 
	 * emerges in a City.
	 *
	 * This value does not change during gameplay.
	 */
	bool d_awardable;

	//! The basename of the file containing the image for this Army proto.
	/**
	 * This value does not contain a path, and does not contain an
	 * extension (e.g. .png).
	 */
	std::string d_image;

	bool d_hero;
};

#endif // ARMY_PROTO_H
