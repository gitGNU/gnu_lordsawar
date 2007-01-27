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

#ifndef ITEMDIALOG_H
#define ITEMDIALOG_H

#include <pgwindow.h>
#include <pgrichedit.h>
#include "hero.h"
#include "action.h"

/** A dialog which allows the player to take and exchange items.
  * 
  * The purpose of the dialog is to have the player exchange the items of his 
  * heroes. Each modification takes place immediately.
  *
  * Since the change to a layout-based description, there are some
  * pecularities regarding the correct id/names of the buttons/labels.
  * They can be found in the private section. Note that in the layout file,
  * some buttons (those for the items) need special id's which we rely on here!
  *
  * TODO: - remove workaround for no icon bug of paragui when upgrading to unstable paragui
  */

class ItemDialog : public PG_Window
{
    public:
        //! some information where an item is/was located
        enum Location {NONE = 0, GROUND = 1, BACK = 2, BODY = 3}; 


        /** The constructor. We load the layout from a data file.
          * 
          * @param hero     the hero whose items are arranged
          * @param pos      the position of the hero on the map
          * @param parent   the parent widget (taken from paragui)
          * @param rect     the widgets drawing area (taken from paragui)
          */
        ItemDialog(Hero* hero, PG_Widget* parent, PG_Rect rect);
        ~ItemDialog();

    private:
        // ok button has been pressed; close the dialog
        bool okClicked(PG_Button* btn);

        // one of the scrolling buttons has been pressed. Redraw the items
        bool scrollClicked(PG_Button* btn);


        //! The name is a bit misleading: replace the icons of all buttons.
        void drawItems();

        //! initializes d_description with the data of the item
        void createDescription(Item* item);


        /** Returns the item that belongs to the button that the mouse
          * cursor currently is over. If the parameters are != 0, they
          * will be filled with the data where the item is located.
          * If the cursor is currently on a slot that is (yet) blank,
          * the location is still stored in the parameters.
          *
          * @param location         one of BACKPACK, GROUND, BODY
          * @param index            backpack/ground =>index in the item list
          *                         else the index of the body location
          * @return the item or 0 if the mouse cursor is outside any
          * item button.
          */
        Item* getItem(Action_Equip::Slot* location = 0, int* index = 0);

        /** Get the item by position
          * 
          * This function returns an item by specifying its location
          * instead of using the mouse cursor.
          *
          * @param location     where the item is located (backpack/ground/body)
          * @param index        e.g. the n-th item lying on the ground
          * @return the item we found or 0 if there is none there.
          */
        Item* getItem(Action_Equip::Slot location, int index);

        /** Removes an item at the specified position
          * 
          * @param location     where the item is located (body/...)
          * @param index        further specification
          * @return false on error, otherwise true
          */
        bool removeItem(Action_Equip::Slot location, int index);

        /** Adds an item to the given location
          * 
          * @param location     where to add it
          * @param index        at which position to add it
          * @param item         the item to add
          * @return false on error, true otherwise
          */
        bool addItem(Action_Equip::Slot location, int index, Item* item);

        /** catch all mouse events
          * 
          * I want to implement some draw and drop for the items and
          * highlighting for a right-mouse click. Since the buttons do not
          * handle this properly (they just emit a sigClick _after_ being
          * clicked), we need to use the dialog's properties.
          */
        bool eventMouseButtonDown(const SDL_MouseButtonEvent* ev);

        bool eventMouseButtonUp(const SDL_MouseButtonEvent* ev);


        Hero* d_hero;
        SDL_Surface* d_tempsurf;

        int d_ground, d_back;
        int d_nground, d_nback;

        // for displaying of item statistics
        PG_RichEdit* d_description;
        bool d_showdesc;

        // for memorizing when the user has pressed down a button
        Action_Equip::Slot d_location;
        int d_index;
};

#endif //ITEMDIALOG_H
