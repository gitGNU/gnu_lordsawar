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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef SHIELD_H
#define SHIELD_H

#include <SDL.h>
#include <string>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "defs.h"

class Player;
class XML_Helper;

 /*
  * Description of a single shield type
  * 
  * This class is the atom of every shield. It contains all data related to
  * a single shield type of a shieldset.
  *
  */

class Shield : public sigc::trackable
{
    public:

	enum ShieldColour {WHITE = 0, YELLOW = 1, GREEN = 2, LIGHT_BLUE = 3,
	RED = 4, DARK_BLUE = 5, ORANGE = 6, BLACK = 7, NEUTRAL = 8};
	enum ShieldType {SMALL = 0, MEDIUM = 1, LARGE = 2};
        /** Loading constructor. See XML_Helper for more details.
          *
          * @param helper       the XML_Helper instance of the shieldset xml
          */
        Shield(XML_Helper* helper);
        
        virtual ~Shield();


        // Set functions:
        
        //! Set the basic image of the unit type
        void setPixmap(SDL_Surface* pixmap);

        //! Set the mask of the unit type (for player colors)
        void setMask(SDL_Surface* mask);

        //! Set the shieldset of the shield
        void setShieldset(Uint32 shieldset) {d_shieldset = shieldset;};

	//! Get the shieldset of the shield
        Uint32 getShieldset() const {return d_shieldset;}

        
        // Get functions
        
        //! Get the type of this shield
        Uint32 getType() const {return d_type;}
        //! Get the colour of this shield
	Uint32 getColour() const {return d_colour;}

        //! Get the image of the shield. Internally, this refers to GraphicsCache.
        SDL_Surface* getPixmap() const;

        //! Returns the mask (read-only!!) for player colors
        SDL_Surface* getMask() const {return d_mask;}

	//! Returns the basename of the picture's filename
	std::string getImageName() const {return d_image;}
	void setImageName(std::string image) {d_image = image;}

        //! Saves the shield information (see XML_Helper for further info)
        virtual bool save(XML_Helper* helper) const;
        
    protected:
        bool saveData(XML_Helper* helper) const;

        //! Copies the generic data from the original prototype shield (used for loading)
        
        Uint32 d_type;
	Uint32 d_colour;
        SDL_Surface* d_pixmap;
        SDL_Surface* d_mask;
	std::string d_image;
	Uint32 d_shieldset;
};

#endif // SHIELD_H
