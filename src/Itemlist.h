// Copyright (C) 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <map>
#include <sigc++/trackable.h>

#include "Item.h"

//! A list of Item objects.
/** 
 * The Itemlist holds all item templates (i.e. types of items) together.
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

        //! Returns the singleton instance.
	static Itemlist* getInstance();

	//! Reads in the itemlist from a file
        static Itemlist* getInstance(XML_Helper *helper);

        //! Creates a new singleton instance. Deletes an existing one.
	//This list of items comes from loading the items.xml file.
        static void createStandardInstance();

        //! Explicitely deletes the singleton instance.
        static void deleteInstance();
        
	//! Save the item data.  See XML_Helper for details.
	bool save(XML_Helper* helper) const;

	void remove(Item *item);

    protected:
	//! Default constructor.
	Itemlist();
	//! Loading constructor.
        Itemlist(XML_Helper* helper);
	//! Destructor.
        ~Itemlist();

    private:

        //! Callback for loading an Item from an opened saved-game file.
        bool loadItem(std::string tag, XML_Helper* helper);

        //! Erases an Item from the list and frees the Item too.
        void flErase(iterator it);

        //! Clears the itemlist of all it's items.
        void flClear();

        static Itemlist* d_instance;
};

#endif //ITEMLIST_H
