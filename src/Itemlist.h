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

#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <map>
#include <sigc++/trackable.h>

#include "Item.h"

/** The Itemlist holds all item templates (i.e. types of items) together.
  * 
  * It is implemented as a singleton. Upon creation, it reads the item
  * description file and initialises an internal list.
  *
  * For easier access, the Itemlist is derived from map. Given an item index,
  * you can get the item belonging to the index by the []-operator using the
  * item index as index.
  */

class Itemlist : public std::map<Uint32, Item*>, public sigc::trackable
{
    public:

        //! Returns the singleton instance. Returns 0 if none exists.
        static Itemlist* getInstance();

        //! Creates a new singleton instance. Deletes an existing one.
        static void createInstance();

        //! Explicitely deletes the singleton instance.
        static void deleteInstance();
        
    private:
        //! The only constructor loads the items from a description file.
        Itemlist(XML_Helper* helper);

        //! The destructor clears the list of items.
        ~Itemlist();

        //! Callback for the initialisation. See XML_Helper why using this.
        bool loadItem(std::string tag, XML_Helper* helper);

        //! Erases an item and frees its memory
        void fl_erase(iterator it);

        //! Clears the itemlist
        void fl_clear();

        static Itemlist* d_instance;
};

#endif //ITEMLIST_H
