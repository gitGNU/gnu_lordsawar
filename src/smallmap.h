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

#ifndef SMALLMAP_H
#define SMALLMAP_H

#include <sigc++/signal.h>
#include <sigc++/connection.h>
#include <sigc++/trackable.h>

#include "overviewmap.h"

#include "rectangle.h"
#include "input-events.h"

//! Draw a miniature map graphic and a white box around the selected region.
/**
 * SmallMap is the image at the top right of the game screen which provides an
 * overview of the whole game map.  It draws City objects on top of the graphic
 * that the OverviewMap class provides.  It is interactive in that it handles 
 * mouse clicks, moving of the currently visible portion and changes of the 
 * underlying map.  It draws an animation of the selection rectangle as it 
 * moves to the next selected region.
 */
class SmallMap: public OverviewMap, public sigc::trackable
{
public:
    //! Default constructor.  Make a new SmallMap.
    SmallMap();

    //! Set the portion of the SmallMap that has a white box around it.
    /**
     * @note This method does not perform animation.  The white box disappears
     * from it's previous location and a new one is drawn.
     *
     * @param new_view  The portion of the map graphic to highlight.
     */
    void set_view(Rectangle new_view);

    //! Zip to the selected portion of the SmallMap from the old position.
    /**
     * Move the white box to the specified region in an animated fashion.
     *
     * @param new_view  The portion of the map graphic to highlight.
     */
    void slide_view(Rectangle new_view);

    //! Realize the given mouse button event.
    void mouse_button_event(MouseButtonEvent e);

    //! Realize the given mouse motion event.
    void mouse_motion_event(MouseMotionEvent e);

    //! Set whether or not the map can be clicked on.
    /**
     * @note This is useful when it's not our turn and we want to prevent the
     *       user from clicking on the SmallMap.
     */
    void set_input_locked(bool locked) { input_locked = locked; }

    // Emitted when the white box is redrawn after a call to SmallMap::set_view.
    /**
     * Classes that use SmallMap must catch this signal to display the change
     * in position of the little white box.
     */
    sigc::signal<void, Rectangle> view_changed;

    //! Emitted during sliding animation after a call to Smallmap::slide_view.
    /**
     * Classes that use SmallMap must catch this signal to display the 
     * animation of the little white box.
     */
    sigc::signal<void, Rectangle> view_slid;

    // Emitted after a call to SmallMap::Draw.
    /**
     * Classes that use SmallMap must catch this signal to display the map.
     */
    sigc::signal<void, SDL_Surface *> map_changed;

    //! Center the view on the given map position.
    /**
     * @param pos        The position to move the little white box to.  The
     *                   referenced tile is in the center of the little white
     *                   box.  If the from_value parameter is set to true,
     *                   this value represents the pixel position, instead of
     *                   the tile position.
     * @param slide      Whether or not to animate the movement of the little
     *                   white box from it's current location to it's new
     *                   given location.
     * @param from_tile  When this is set to false, it changes the meaning of
     *                   the pos parameter to mean the position on the small
     *                   map graphic in pixels.  When it is set to true, the
     *                   meaning of the pos parameter is a position of a 
     *                   tile on the game map.
     */
    /*
     * FIXME: separate these into two methods somehow.
     * center_view_on_pixel
     * center_view_on_tile
     */
    void center_view(Vector<int> pos, bool slide, bool from_tile = true);

    //! Make the map go black.
    void blank();
private:

    //! The position and size of the little white box.
    /**
     * This rectangle represents the selected portion of the map.
     */
    Rectangle view;

    //! Whether or not to ignore mouse clicks and movement.
    /**
     * When this is set to true, the SmallMap will ignore mouse clicks and also
     * mouse drags.
     */
    bool input_locked;

    //! Draw the selection rectangle that shows the viewed portion of the map.
    void draw_selection();

    //! Draw the City objects and little white box onto the mini-map graphic.
    /**
     * This method is automatically called by the SmallMap::draw method.
     */
    virtual void after_draw();
};

#endif
