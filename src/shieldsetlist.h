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
  * Each shieldset has a size, a name and a list of shields. 
  * The shieldsetlist abstracts all these from the evil rest of the program. 
  * Shieldsets are in general referenced by their id.
  *
  * Since several classes access this class, it is implemented as a singleton.
  */

class Shieldsetlist : public std::list<Shieldset*>, public sigc::trackable
{
    public:
        //! return the singleton instance of this class
        static Shieldsetlist* getInstance();

        //! Explicitly delete the singleton instance of this class
        static void deleteInstance();

        /** Returns a shield 
          *
          * @param id       the id of the shieldset
	  * @param type     the size of the shield: 0=small, 1=medium, 2=large
          * @param colour   which player.  white, yellow, etc.
          * @return the requested shield or 0 on error
          */
        Shield* getShield(Uint32 id, Uint32 type, Uint32 colour) const;

        /** Returns the size of a specific shieldset
          * 
          * @param id       the id of the shieldset
          * @return size of the shieldset or 0 on error (a shieldset should never
          *         have a size of 0)
          */
        Uint32 getSize(Uint32 id) const;

        /** Returns the name of a specific shieldset
          * 
          * @param id       the id of the shieldset
          * @return the name or an empty string on error
          */
        std::string getName(Uint32 id) const;

        //! Returns the names of all shieldsets
	std::list<std::string> getNames();

	Shieldset* getShieldset(std::string name);

        /** Returns the Id of a specific shieldset by name
          * 
          * @param shieldset       the name of the shieldset
          * @return the id of the shieldset (0 on error)
          */
	Uint32 getShieldsetId(std::string shieldset) {return d_ids[shieldset];} 

        /** Returns a list of all existing shield sets
          */
        std::vector<Uint32> getShieldsets() const;

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

        //! Loads a specific shieldset
        bool loadShieldset (std::string name);
        
        typedef std::map<Uint32, Shieldset*> ShieldsetMap;
        typedef std::map<Uint32, std::string> NameMap;
        typedef std::map<std::string, Uint32> IdMap;
        
	ShieldsetMap d_shieldsets;
        NameMap d_names;
        IdMap d_ids;

        static Shieldsetlist* s_instance;
};

#endif // SHIELDSETLIST_H

