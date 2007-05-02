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

#ifndef RCREATEITEM_H
#define RCREATEITEM_H

#include "Reaction.h"

#include "../vector.h"

class Item;

/** Creates an item at a given location
  * 
  * This reaction just creates one item; to create multiple items, assign
  * multiple reactions to an event.
  */

class RCreateItem : public Reaction
{
    public:
        /** Standard constructor
          * 
          * @param index    the index of the item in the itemlist (see there)
          * @param pos      the position where the item should be created
          *
          * @note Use only valid positions and indices to avoid problems!
          */
        RCreateItem(Uint32 index, Vector<int> pos);
        
        //! Loading constructor
        RCreateItem(XML_Helper* helper);
        ~RCreateItem();

        //! Saves the reaction data
        bool save(XML_Helper* helper) const;

        //! Triggers the reaction
        bool trigger() const;
        
        
        //! Returns the item which the reaction creates
        Item* getItem() const;

        //! Return the item index
        Uint32 getItemIndex() const {return d_item;}

        //! Sets the id of the item
        void setItem(Uint32 item) {d_item = item;}

        //! Returns the position where the item appears
        Vector<int> getPos() const {return d_pos;}

        //! Sets the position where the item appears
        void setPos(Vector<int> pos) {d_pos = pos;}

    private:
        Uint32 d_item;
        Vector<int> d_pos;
};

#endif //RCREATEITEM
