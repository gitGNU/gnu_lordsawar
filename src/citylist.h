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

#ifndef CITYLIST_H
#define CITYLIST_H

#include "ObjectList.h"
#include <sigc++/trackable.h>

class City;
class Player;

/** The list of all cities in the game
  * 
  * This is basically an ObjectList with some extended abilities.
  *
  * Since many different classes require acess to this list, it is implemented
  * as a singleton.
  */

class Citylist : public ObjectList<City>, public sigc::trackable
{
    public:
        //! Returns the singleton instance. Creates a new one if neccessary.
        static Citylist* getInstance();

        //! Loads and returns the singleton instance. See XML_Helper for info.
        static Citylist* getInstance(XML_Helper* helper);

        //! Deletes the singleton instance.
        static void deleteInstance();

        
        /** Process all cities for the next turn.
          * 
          * This function loops through all cities belonging to player p and
          * processes them. Processing means: Adding the income of the city to
          * the player's gold reserves and producing new armies if finished.
          *
          * @param p        the player whose cities are processed
          */
        void nextTurn(Player* p);

        //! Save the city data. See XML_Helper for details.
        bool save(XML_Helper* helper) const;
       
        //! Count the number of cities of player p.
        int countCities(Player* p) const;

        //! Returns the city closest to pos that isn't owned by the ActivePlayer
        City* getNearestEnemyCity(const Vector<int>& pos);

        //! Returns the city closest to pos that is owned by the ActivePlayer
        City* getNearestFriendlyCity(const Vector<int>& pos);

        //! Returns the city closest to pos that is owned by the ActivePlayer
	//! but within N tiles.
        City* getNearestFriendlyCity(const Vector<int>& pos, int dist);

        //! Returns the city closest to pos but within N tiles
        City* getNearestCity(const Vector<int>& pos, int dist);

        //! Returns the city closest to pos
        City* getNearestCity(const Vector<int>& pos);

        //! Returns the first (currently most upper left) city of player p
        City* getFirstCity(Player* p);

    protected:
        // CREATORS
        Citylist();
        Citylist(XML_Helper* helper);
        ~Citylist();

    private:
        //! Callback for loading. See XML_Helper for details.
        bool load(std::string tag, XML_Helper* helper);

        static Citylist* s_instance;
};

#endif // CITYLIST_H

// End of file
