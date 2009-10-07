// Copyright (C) 2009 Ben Asselstine
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
#ifndef PIXMASK_H
#define PIXMASK_H

#include <gtkmm.h>
#include "vector.h"
#include "rectangle.h"


//! A pixmap and bitmask pair.
/** 
 */
class PixMask
{
 public:
     Glib::RefPtr<Gdk::Pixmap> get_pixmap() {return pixmap;};
     Glib::RefPtr<Gdk::Bitmap> get_mask() {return mask;};
     Glib::RefPtr<Gdk::GC> get_gc() {return gc;};
     int get_width() {return width;};
     int get_height() {return height;};
     int get_depth();

     static PixMask* create(std::string file);
     static PixMask* create(Glib::RefPtr<Gdk::Pixbuf> buf);
     static PixMask* create(Glib::RefPtr<Gdk::Pixmap> pixmap,
					 Glib::RefPtr<Gdk::Bitmap> mask);
     PixMask* copy();

     //! convert this pixmask to a pixbuf.
     Glib::RefPtr<Gdk::Pixbuf> to_pixbuf();

     //! draw a pixbuf onto this pixmask.
     void draw_pixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, int src_x, int src_y, int dest_x, int dest_y, int width, int height);

     //! scale a pixmask in place (alters pixmask)
     static void scale(PixMask*& pixmask, int xsize, int ysize, Gdk::InterpType intper = Gdk::INTERP_NEAREST);

     //! draw this pixmask onto a pixmap.
     void blit(Glib::RefPtr<Gdk::Pixmap> pixmap, int dest_x, int dest_y);
     void blit(Glib::RefPtr<Gdk::Pixmap> pixmap, Vector<int> pos);
     void blit_centered(Glib::RefPtr<Gdk::Pixmap> pixmap, Vector<int> pos);
     // blit a tile's worth of imagery from this pixmask to a pixmap.
     void blit(Vector<int> tile, int ts, Glib::RefPtr<Gdk::Pixmap> pixmap, Vector<int> dest);

     //! Destructor.
    ~PixMask();
 protected:
     //! Default constructor.
     PixMask(Glib::RefPtr<Gdk::Pixbuf> pixbuf);

     //! Alternative constructor.
     PixMask(Glib::RefPtr<Gdk::Pixmap> pixmap, Glib::RefPtr<Gdk::Bitmap> mask);

     //! Copy constructor.
     PixMask(const PixMask&);

     //! Loading constructor.
     /**
      * Load the pixmask from a file.
      *
      */
     PixMask(std::string filename);

    
 private:
    Glib::RefPtr<Gdk::Pixmap> pixmap;
    Glib::RefPtr<Gdk::Bitmap> mask;
    Glib::RefPtr<Gdk::GC> gc;
    int width;
    int height;

     //! return a stretched copy of this pixmask.
     PixMask* scale(int xsize, int ysize, 
		    Gdk::InterpType interp = Gdk::INTERP_NEAREST);
     
     void blit(Rectangle src, Glib::RefPtr<Gdk::Pixmap> pixmap, Vector<int> dest);
};

#endif
