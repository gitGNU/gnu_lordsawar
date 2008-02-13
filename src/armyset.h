//  Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef ARMYSET_H
#define ARMYSET_H

#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"
#include "army.h"


class Armyset: public std::list<Army *>, public sigc::trackable
{
    public:

	Armyset(Uint32 id, std::string name);
        Armyset(XML_Helper* helper);
	bool save(XML_Helper* helper);
        ~Armyset();

        /** Returns the size of this armyset
          * 
          * @return size of the armyset or 0 on error (an armyset should never
          *         have a size of 0)
          */
        Uint32 getSize() const {return size();}
        Uint32 getTileSize() const {return d_tilesize;}
        Uint32 getId() const {return d_id;}
        void setId(Uint32 id) {d_id = id;} //for editor only

        /** Returns the name of this armyset
          * 
          * @return the name or an empty string on error
          */
        std::string getName() const {return d_name;}
        void setName(std::string name) {d_name = name;}
        std::string getSubDir() const {return d_dir;}
        void setSubDir(std::string dir) {d_dir = dir;}

        void instantiatePixmaps();

	SDL_Surface *getShipPic() const {return d_ship;}
	SDL_Surface *getShipMask() const {return d_shipmask;}

	SDL_Surface *getStandardPic() const {return d_standard;}
	SDL_Surface *getStandardMask() const {return d_standard_mask;}

	//! this is only used for the editor.
	//try to use Armysetlist::getArmyType instead
	Army * lookupArmyByType(Uint32 army_type);
    private:

        //! Callback function for the army tag (see XML_Helper)
        bool loadArmyTemplate(std::string tag, XML_Helper* helper);

	bool instantiatePixmap(Army *a);
	void loadShipPic();
	void loadStandardPic();
        
        Uint32 d_id;
        std::string d_name;
        std::string d_dir;
	Uint32 d_tilesize;
	SDL_Surface *d_ship; //what the stack looks like when in the water
	SDL_Surface *d_shipmask;
	SDL_Surface *d_standard; //what the standard looks like when planted
	SDL_Surface *d_standard_mask;
};

#endif // ARMYSETLIST_H

