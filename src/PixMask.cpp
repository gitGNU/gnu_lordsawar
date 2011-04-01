// Copyright (C) 2009, 2010, 2011 Ben Asselstine
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

#include <iostream>
#include "defs.h"
#include "PixMask.h"
#include <string.h>

PixMask::PixMask(Glib::RefPtr<Gdk::Pixbuf> pixbuf)
     : width(0), height(0), unscaled_width(0), unscaled_height(0)
{
  pixbuf->render_pixmap_and_mask(pixmap, mask, 1);
  pixmap->get_size(width, height);
  gc = Gdk::GC::create(pixmap);
}

PixMask::PixMask(Glib::RefPtr<Gdk::Pixmap> p, Glib::RefPtr<Gdk::Bitmap> m)
    : width(0), height(0)
{
  pixmap = p;
  mask = m;
  if (pixmap == true)
    {
      pixmap->get_size(width, height);
      gc = Gdk::GC::create(pixmap);
    }
  else if (mask == true)
    {
      mask->get_size(width, height);
      gc = Gdk::GC::create(mask);
    }
  unscaled_width = width;
  unscaled_height = height;
}

PixMask::PixMask(const PixMask&p)
{
  pixmap = 
    Gdk::Pixmap::create(Glib::RefPtr<Gdk::Drawable>(0), p.width, p.height, 24);
  gc = Gdk::GC::create(pixmap);
  pixmap->draw_drawable(gc, p.pixmap, 0, 0, 0, 0, p.width, p.height);
  int size = p.width * p.height / 8;
  char *data = (char*)malloc(size);
  memset(data, 0, size);
  mask = 
    Gdk::Bitmap::create(data, p.width, p.height);
  mask->draw_drawable(Gdk::GC::create(p.mask), p.mask, 0, 0, 0, 0, p.width, p.height);
  free(data);

  width = p.width;
  height = p.height;
  unscaled_width = p.unscaled_width;
  unscaled_height = p.unscaled_height;
}

PixMask::PixMask(std::string filename, bool &broken)
     : width(0), height(0)
{
  if (Gtk::Main::instance() == NULL)
    {
      broken = true;
      return;
    }
  Glib::RefPtr<Gdk::Pixbuf> pixbuf;
  try
    {
      pixbuf = Gdk::Pixbuf::create_from_file(filename);
    }
  catch (const Glib::Exception &ex)
    {
      std::cerr << _("Couldn't load image file ") << filename << std::endl;
      broken = true;
      return;
    }
  pixbuf->render_pixmap_and_mask(pixmap, mask, 1);
  pixmap->get_size(width, height);
  gc = Gdk::GC::create(pixmap);
  unscaled_width = width;
  unscaled_height = height;
}

PixMask* PixMask::create(std::string filename, bool &broken)
{
  return new PixMask(filename, broken);
}

PixMask* PixMask::create(Glib::RefPtr<Gdk::Pixbuf> pixbuf)
{
  return new PixMask(pixbuf);
}

PixMask* PixMask::create(Glib::RefPtr<Gdk::Pixmap> pixmap, Glib::RefPtr<Gdk::Bitmap> mask)
{
  return new PixMask(pixmap, mask);
}

PixMask* PixMask::copy()
{
  return new PixMask(*this);
}

void PixMask::blit_centered(Glib::RefPtr<Gdk::Pixmap> dest, Vector<int> pos)
{
  blit (dest, pos.x - (width/2), pos.y - (height/2));
  return;
}

void PixMask::blit(Glib::RefPtr<Gdk::Pixmap> dest, Vector<int> pos)
{
  blit (dest, pos.x, pos.y);
  return;
}

void PixMask::blit(Glib::RefPtr<Gdk::Pixmap> dest, int dest_x, int dest_y)
{
  gc->set_clip_origin(dest_x,dest_y);
  gc->set_clip_mask(mask);
  dest->draw_drawable(gc, pixmap, 0, 0, dest_x, dest_y, width, height);
  gc->set_clip_mask(Glib::RefPtr<Gdk::Bitmap>(0));
}
     
void PixMask::blit(Rectangle src, Glib::RefPtr<Gdk::Pixmap> p, Vector<int> dest)
{
  if (src.x + src.w > get_width())
    return;
  if (src.y + src.h > get_height())
    return;
  //here we cleverly set the clip origin
  gc->set_clip_origin(dest.x - src.x,dest.y - src.y);
  gc->set_clip_mask(mask);
  p->draw_drawable(gc, pixmap, src.x, src.y, dest.x, dest.y, src.w, src.h);
  gc->set_clip_mask(Glib::RefPtr<Gdk::Bitmap>(0));
}

void PixMask::blit(Vector<int> tile, int ts, Glib::RefPtr<Gdk::Pixmap> pixmap, Vector<int> dest)
{
  Vector<int> src = tile * ts;
  blit (Rectangle(src.x, src.y, ts, ts), pixmap, dest);
}

void PixMask::scale(PixMask*& p, int xsize, int ysize, Gdk::InterpType interp)
{
  PixMask *scaled = p->scale(xsize, ysize, interp);
  delete p;
  p = scaled;
  p->set_unscaled_width(p->get_unscaled_width());
  p->set_unscaled_height(p->get_unscaled_height());
  return;
}

PixMask * PixMask::scale(int xsize, int ysize, Gdk::InterpType interp)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = to_pixbuf();
  PixMask *pix = PixMask::create(pixbuf->scale_simple(xsize, ysize, interp));
  pix->set_unscaled_width(get_unscaled_width());
  pix->set_unscaled_height(get_unscaled_height());
  return pix;
}

Glib::RefPtr<Gdk::Pixbuf> PixMask::to_pixbuf()
{
  Glib::RefPtr<Gdk::Pixmap> result = Gdk::Pixmap::create(Glib::RefPtr<Gdk::Drawable>(0), width, height, 24);
  Gdk::Color transparent = Gdk::Color();
  //fixme: check to see if this colour is already present somehow
  transparent.set_rgb_p(255.0 /255.0 , 87.0 / 255.0, 204.0/255.0);
  gc->set_rgb_fg_color(transparent);
  result->draw_rectangle(gc, true, 0, 0, width, height);
  blit(result, 0, 0);
  Glib::RefPtr<Gdk::Pixbuf> buf = Gdk::Pixbuf::create(Glib::RefPtr<Gdk::Drawable>(result), 0, 0, width, height);
  Glib::RefPtr<Gdk::Pixbuf> alphabuf = buf->add_alpha(true, 255, 87, 204);
  return alphabuf;
}
      
void PixMask::draw_pixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, int src_x, int src_y, int dest_x, int dest_y, int width, int height)
{
  pixmap->draw_pixbuf(gc, pixbuf, src_x, src_y, dest_x, dest_y, width, height,
		      Gdk::RGB_DITHER_NONE, 0, 0);
}

int PixMask::get_depth()
{
  if (pixmap == true)
    return pixmap->get_depth();
  else
    return 0;
}
PixMask::~PixMask()
{
  pixmap.clear();
  mask.clear();
  gc.clear();
}
