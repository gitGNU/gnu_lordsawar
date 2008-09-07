// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004, 2005 Bryan Duff
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#ifndef BIGMAP_H
#define BIGMAP_H

#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>

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
class Location;

/** The large map
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
    virtual ~BigMap();

    // draw everything
    void draw(bool redraw_buffer = true);

    // view the rectangle, measured in tiles
    void set_view(Rectangle rect);
    void screen_size_changed();

    // return a good position of a map tip given that it should be close to the
    // tiles in tile_area without covering them
    MapTipPosition map_tip_position(Rectangle tile_area);
    MapTipPosition map_tip_position(Vector<int> tile);

    // emitted when the view has changed because of user interactions
    sigc::signal<void, Rectangle> view_changed;

    void blank();

    //! Save the whole map as one big image (bmp file).
    bool saveAsBitmap(std::string filename);

    //! Save the whole map, but not the game objects on top of it.
    bool saveUnderlyingMapAsBitmap(std::string filename);

    //! Save the current view as an image (bmp file).
    bool saveViewAsBitmap(std::string filename);
 protected:
    MapRenderer* d_renderer;

    Rectangle view;		// approximate view of screen, in tiles
    Vector<int> view_pos; 	// precise position of view in pixels

    SDL_Surface* buffer;	// the buffer we draw things in
    Rectangle buffer_view;	// current view of the buffer, in tiles

    bool input_locked;

    SDL_Surface* d_itempic;

    // helpers
    Vector<int> mouse_pos_to_tile(Vector<int> pos);
    // offset in pixels within tile
    Vector<int> mouse_pos_to_tile_offset(Vector<int> pos); 
    Vector<int> tile_to_buffer_pos(Vector<int> tile);
    Vector<int> get_view_pos_from_view();
    void draw_buffer();  
    bool blit_if_inside_buffer(const Location &obj, SDL_Surface *image,
			       Rectangle &map_view, SDL_Surface *surface);
    void blit_object(const Location &obj, SDL_Surface *image, SDL_Surface *surface);

    virtual void after_draw() { }

 protected:
    void draw_stack(Stack *s);
 private:
    void drawFogTile(int x, int y);
    void draw_buffer(Rectangle map_view, SDL_Surface *surface);
};

#endif
