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

#ifndef SHIELDSET_H
#define SHIELDSET_H

#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"
#include "shield.h"


class Shieldset: public std::list<Shield *>, public sigc::trackable
{
    public:

        Shieldset(XML_Helper* helper);
        ~Shieldset();

	Uint32 getSmallHeight() const {return d_small_height;}
	Uint32 getSmallWidth() const {return d_small_width;}
	Uint32 getMediumHeight() const {return d_medium_height;}
	Uint32 getMediumWidth() const {return d_medium_width;}
	Uint32 getLargeHeight() const {return d_large_height;}
	Uint32 getLargeWidth() const {return d_large_width;}

        /** Returns the number of shields in this shieldset
          */
        Uint32 getSize() const {return size();}

        /** Returns the name of this shieldset
          * 
          * @return the name or an empty string on error
          */
        std::string getName() const {return d_name;}
        void setName(std::string name) {d_name = name;}
        std::string getSubDir() const {return d_dir;}
        void setSubDir(std::string dir) {d_dir = dir;}

        void instantiatePixmaps();

	//! this is only used for the editor.
	//try to use Shieldsetlist::getShieldType instead
	Shield * lookupShieldByTypeAndColour(Uint32 type, Uint32 colour);
    private:

        //! Callback function for the shield tag (see XML_Helper)
        bool loadShield(std::string tag, XML_Helper* helper);

	bool instantiatePixmap(Shield *a);
        
        std::string d_name;
        std::string d_dir;
	Uint32 d_small_height;
	Uint32 d_small_width;
	Uint32 d_medium_height;
	Uint32 d_medium_width;
	Uint32 d_large_height;
	Uint32 d_large_width;
};

#endif // SHIELDSET_H

