// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004 Ulf Lorenz
// Copyright (C) 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#ifndef MAPRENDERER_H
#define MAPRENDERER_H

#include <gtkmm.h>
#include <string>

/** Class which cares about rendering of the map.
  * 
  * This class is initalized with the drawing surface of the BigMap class. It
  * cares for the actual terrain drawing. 
  */

class MapRenderer
{
    public:

        /** Constructor, also does the smoothing of the GameMap.
          * 
          * @param surface      the surface which is rendered with render()
          */
        MapRenderer(Glib::RefPtr<Gdk::Pixmap> surface);
        ~MapRenderer();

        /** Render a portion of the map.
          * 
          * The part of the map which is drawn starts at the tile (tileX,tileY)
          * and goes on till (tileX+columns, tileY+rows). The drawing is done on
          * the surface handed over in the constructor and starts at pixel
          * position (x,y).
          */
        void render(int x, int y, int tileX, int tileY, int columns, int rows);

	void render(int x, int y, int tileStartX, int tileStartY,
		    int columns, int rows, Glib::RefPtr<Gdk::Pixmap> surface,
		    Glib::RefPtr<Gdk::GC> context);

	//! Save the current view of map tiles as an image (bmp file).
	bool saveViewAsBitmap(std::string filename);

	//! Save all of the map tiles as one big image (bmp file).
	bool saveAsBitmap(std::string filename);
    private:
        //Data
	Glib::RefPtr<Gdk::Pixmap> d_surface;
	Glib::RefPtr<Gdk::GC> gc;

};

#endif // MAPRENDERER_H

// End of file
