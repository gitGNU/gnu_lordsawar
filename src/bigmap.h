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

#ifndef BIGMAP_H
#define BIGMAP_H

#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>
#include "rectangle.h"

#include "vector.h"
#include "input-events.h"
#include "map-tip-position.h"
#include "rectangle.h"

class Stack;
class MapRenderer;

class City;
class Ruin;
class Signpost;
class Temple;
class Object;

/** The large game map
  * 
  * The detailed map in which all the action takes place. Handles scrolling
  * and mouse clicks via signals.
  *
  * Draws everything to a buffer to simplify scrolling, the buffer is then
  * blitted to the screen. The current view of the map is kept track of both
  * approximately (in tiles) and more precisely in pixels.
  */
class BigMap: public sigc::trackable
{
 public:
    BigMap();
    ~BigMap();

    // draw everything
    void draw(bool redraw_buffer = true);

    // will center the bigmap on the stack
    void select_active_stack();
	
    void unselect_active_stack();

    // view the rectangle, measured in tiles
    void set_view(Rectangle rect);

    void mouse_button_event(MouseButtonEvent e);
    void mouse_motion_event(MouseMotionEvent e);
    
    /** Callback if a stack has moved
     * 
     * This centers the bigmap around the stack.
     */
    void stackMoved(Stack* s);

    //! Center the bigmap around point p
    void centerView(const Vector<int> p);

    //! Enable/Disable the processing of mouse clicks
    void setEnable (bool enable){d_enable=enable;}


    // emitted when the view has changed because of user interactions
    sigc::signal<void, Rectangle> view_changed;

    // emitted when a stack is selected, a NULL parameter signifies that no
    // stack is selected 
    sigc::signal<void, Stack*> stack_selected;

    // emitted when a path for a stack is set
    sigc::signal<void> path_set;
	
    // signals for mouse clicks
    sigc::signal<void, City*, MapTipPosition> city_selected;
    sigc::signal<void, Ruin*, MapTipPosition> ruin_selected;
    sigc::signal<void, Signpost*, MapTipPosition> signpost_selected;
    sigc::signal<void, Temple*, MapTipPosition> temple_selected;

    // emitted whenever the user moves the mouse to a new tile
    sigc::signal<void, Vector<int> > mouse_on_tile;

 private:
    MapRenderer* d_renderer;
	
    Rectangle view;		// approximate view of screen, in tiles
    Vector<int> view_pos; 	// precise position of view in pixels
        
    SDL_Surface* buffer;	// the buffer we draw things in
    Rectangle buffer_view;	// current view of the buffer, in tiles
	
    bool d_enable;
	
    SDL_Surface* d_arrows;
    SDL_Surface* d_ruinpic;
    SDL_Surface* d_signpostpic;
    SDL_Surface* d_itempic;
    SDL_Surface* d_fogpic;
        
    Vector<int> current_tile;

    enum {
	NONE, DRAGGING, SHOWING_CITY, SHOWING_RUIN,
	SHOWING_TEMPLE, SHOWING_SIGNPOST
    } mouse_state;
	

    // for the marching ants around selected stack
    sigc::connection selection_timeout_handler;
    bool on_selection_timeout();

    // helpers
    Vector<int> mouse_pos_to_tile(Vector<int> pos);
    // return a good position of the map tip given that it should be close
    // to the tiles in tile_area without covering them
    MapTipPosition map_tip_position(Rectangle tile_area);

    const Vector<int> getRelativeXPos(const Vector<int>& absolutePos);
    void draw_buffer();  // does the actual hard work of drawing the buffer
     // helper for draw_buffer
    void blit_if_inside_buffer(const Object &obj, SDL_Surface *image);
};

#endif
