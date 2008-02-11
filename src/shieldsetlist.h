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

#ifndef SHIELDSETLIST_H
#define SHIELDSETLIST_H

#include <string>
#include <map>
#include <vector>
#include <sigc++/trackable.h>

#include "xmlhelper.h"
#include "shield.h"
#include "shieldset.h"


/** List of all available shieldsets
  * 
  * This class contains a list of all shieldsets used in the game. 
  * Since several classes access this class, it is implemented as a singleton.
  */

class Shieldsetlist : public std::list<Shieldset*>, public sigc::trackable
{
    public:
        //! return the singleton instance of this class
        static Shieldsetlist* getInstance();

        //! Explicitly delete the singleton instance of this class
        static void deleteInstance();

        //! Returns the names of all shieldsets
	std::list<std::string> getNames();

        /** Returns the subdir of a specific shieldset by name
          * 
          * @param shieldset       the name of the shieldset
          * @return the name of the directory that holds the shieldset
          */
	std::string getShieldsetDir(std::string name) {return d_dirs[name];}

	Shield *getShield(std::string shieldset, Uint32 type, Uint32 colour);

	Shieldset *getShieldset(std::string dir) { return d_shieldsets[dir];}

	/* Reads in the pixmap and mask for every shield of every shieldset.
	 * This can only be done after SDL is initialized.
	 */
	void instantiatePixmaps();

    private:
        //! Constructor; loads all shieldsets it can find
        Shieldsetlist();
        
        //! Destructor; mainly clears the lists
        ~Shieldsetlist();

        //! Callback for loading. See XML_Helper for details.
	bool load(std::string tag, XML_Helper *helper);

        //! Loads a specific armyset
        bool loadShieldset (std::string name);
        
        typedef std::map<std::string, std::string> DirMap;
        typedef std::map<std::string, Shieldset*> ShieldsetMap;

        DirMap d_dirs;
        ShieldsetMap d_shieldsets;

        static Shieldsetlist* s_instance;
};

#endif // SHIELDSETLIST_H

