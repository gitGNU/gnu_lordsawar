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

#ifndef ARMYSETLIST_H
#define ARMYSETLIST_H

#include <string>
#include <map>
#include <vector>
#include <sigc++/object_slot.h>

#include "xmlhelper.h"
#include "army.h"


/** List of all available armysets
  * 
  * This class contains a list of all armysetsused in the game. Each armyset has
  * a size, a name and a list of armies. The armysetlist shields all these from
  * the evil rest of the program. Armysets are in general referenced by their
  * id.
  *
  * @note Throughout this class, it is assumed that the standard armyset usable
  * by all playes ha sthe index 1, the heroes armyset index 2. However, to avoid
  * this assumption being spread over the whole code, we separate the access to
  * these two armysets from the rest via special functions.
  *
  * Since several classes access this class, it is implemented as a singleton.
  */

class Armysetlist : public SigC::Object
{
    public:
        //! return the singleton instance of this class
        static Armysetlist* getInstance();

        //! Explicitely delete the singleton instance of this class
        static void deleteInstance();


        /** Returns an army prototype
          *
          * @param id       the id of the armyset
          * @param index    the index of the army within the set
          * @return the requested army or 0 on error
          */
        const Army* getArmy(Uint32 id, Uint32 index) const;

        /** Returns the size of a specific armyset
          * 
          * @param id       the id of the armyset
          * @return size of the armyset or 0 on error (an armyset should never
          *         have a size of 0)
          */
        Uint32 getSize(Uint32 id) const;

        /** Returns the name of a specific armyset
          * 
          * @param id       the id of the armyset
          * @return the name or an empty string on error
          */
        std::string getName(Uint32 id) const;

        /** Returns the index of the standard armyset
          * 
          * Since the standard armyset is special, we try to avoid access to it.
          * You have to call this function explicitely to get to know its id.
          */
        Uint32 getStandardId() const {return d_standard;}

        /** Returns the index of the heroes
          * 
          * The same as for the standar armyset applies here.
          */
        Uint32 getHeroId() const {return d_heroes;}


        /** Returns a list of all existing armies
          * 
          * @param force_all    if set to true, all armysets are returned, else
          *                     the standard and the hero armyset are excluded.
          */
        std::vector<Uint32> getArmysets(bool force_all = false) const;
    
    private:
        //! Constructor; loads all armysets it can find
        Armysetlist();
        
        //! Destructor; mainly clears the lists
        ~Armysetlist();


        //! Loads a specific armyset
        bool loadArmyset (std::string name);
        
        //! Callback function for the armyset tag (see XML_Helper)
        bool loadGlobalStuff(std::string tag, XML_Helper* helper);

        //! Callback function for the army tag (see XML_Helper)
        bool loadArmy(std::string tag, XML_Helper* helper);

        
        typedef std::map<Uint32, std::vector<Army*> > ArmyMap;
        typedef std::map<Uint32, std::string> NameMap;
        
        ArmyMap d_armies;
        NameMap d_names;

        Uint32 d_standard;
        Uint32 d_heroes;

        // temporary variables for game loading
        Uint32 d_loading;
        std::string d_file;

        static Armysetlist* s_instance;
};

#endif // ARMYSETLIST_H

