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

        Armyset(XML_Helper* helper);
	bool save(XML_Helper* helper);
        ~Armyset();

        /** Returns the size of this armyset
          * 
          * @return size of the armyset or 0 on error (an armyset should never
          *         have a size of 0)
          */
        Uint32 getSize() const {return size();}
        Uint32 getId() const {return d_id;}

        /** Returns the name of this armyset
          * 
          * @return the name or an empty string on error
          */
        std::string getName() const {return d_name;}
        std::string getSubDir() const {return d_dir;}
        void setSubDir(std::string dir) {d_dir = dir;}

        void instantiatePixmaps();

	//! this is only used for the editor.
	//try to use Armysetlist::getArmyType instead
	Army * lookupArmyByType(Uint32 army_type);
    private:

        //! Callback function for the army tag (see XML_Helper)
        bool loadArmyTemplate(std::string tag, XML_Helper* helper);

	bool instantiatePixmap(Army *a);
        
        Uint32 d_id;
        std::string d_name;
        std::string d_dir;
};

#endif // ARMYSETLIST_H

