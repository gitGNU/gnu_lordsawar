//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2009, 2011 Ben Asselstine
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

#include "image-helpers.h"

Glib::RefPtr<Gdk::Pixmap> to_pixmap(Glib::RefPtr<Gdk::Pixbuf> pixbuf)
{
  Glib::RefPtr<Gdk::Pixmap> p = Gdk::Pixmap::create(Glib::RefPtr<Gdk::Drawable>(0), pixbuf->get_width(), pixbuf->get_height(), 24);
  pixbuf->render_to_drawable_alpha(p, 0, 0, 0, 0, pixbuf->get_width(),
				 pixbuf->get_height(), Gdk::PIXBUF_ALPHA_BILEVEL,
				 123, Gdk::RGB_DITHER_NONE, 0, 0);
  //p = Gdk::Pixmap::create(Glib::RefPtr<Gdk::Drawable>(0), 
			  //pixbuf->get_width(), pixbuf->get_height(), 24);
  //p->draw_pixbuf(pixbuf, 0, 0, 0, 0, pixbuf->get_width(), pixbuf->get_height(),
		 //Gdk::RGB_DITHER_NORMAL, 0, 0);
  return p;
}

std::vector<PixMask*>
disassemble_row(const std::string &file, int no, bool &broken)
{
  Glib::RefPtr<Gdk::Pixbuf> row;
  try
    {
      if(Gtk::Main::instance())
        row = Gdk::Pixbuf::create_from_file(file);
      else
        broken = true;
    }
  catch (const Glib::Exception &ex)
    {
      broken = true;
    }

  if (broken || !row)
    {
      std::vector<PixMask*> empty;
      return  empty;
    }

    std::vector<Glib::RefPtr<Gdk::Pixbuf> > images;
    images.reserve(no);
  
    int h = row->get_height();
    int w = row->get_width() / no;

    // disassemble row
    for (int x = 0; x < no; ++x) {
	Glib::RefPtr<Gdk::Pixbuf> buf
	    = Gdk::Pixbuf::create(row->get_colorspace(),
				  row->get_has_alpha(),
				  row->get_bits_per_sample(),
				  w, h);

	row->copy_area(x * w, 0, w, h, buf, 0, 0);
    
	images.push_back(buf);
    }
    
    std::vector<PixMask*> pixmasks;
    for (unsigned int i = 0; i < images.size(); i++)
      pixmasks.push_back(PixMask::create(images[i]));

    return pixmasks;
}

std::vector<PixMask*>
disassemble_row(const std::string &file, int no, bool first_half_height, bool &broken)
{
    Glib::RefPtr<Gdk::Pixbuf> row;
    try
      {
        row = Gdk::Pixbuf::create_from_file(file);
      }
  catch (const Glib::Exception &ex)
    {
      broken = true;
    }

  if (broken || !row)
    {
      std::vector<PixMask*> empty;
      return  empty;
    }

    std::vector<Glib::RefPtr<Gdk::Pixbuf> > images;
    images.reserve(no);
  
    int h = row->get_height() / 2;
    int w = row->get_width() / no;

    int s = 0;
    if (first_half_height == false)
      s = h;
    // disassemble row
    for (int x = 0; x < no; ++x) {
	Glib::RefPtr<Gdk::Pixbuf> buf
	    = Gdk::Pixbuf::create(row->get_colorspace(),
				  row->get_has_alpha(),
				  row->get_bits_per_sample(),
				  w, h);

	row->copy_area(x * w, s, w, h, buf, 0, 0);
    
	images.push_back(buf);
    }
    
    std::vector<PixMask*> pixmasks;
    for (unsigned int i = 0; i < images.size(); i++)
      pixmasks.push_back(PixMask::create(images[i]));
    return pixmasks;
}
int get_pwidth(Glib::RefPtr<Gdk::Pixmap> pixmap)
{
  int width = 0, height = 0;
  pixmap->get_size(width, height);
  return width;
}
int get_pheight(Glib::RefPtr<Gdk::Pixmap> pixmap)
{
  int width = 0, height = 0;
  pixmap->get_size(width, height);
  return height;
}
Glib::RefPtr<Gdk::Pixmap> scale (Glib::RefPtr<Gdk::Pixmap> pixmap, int w, int h)
{
  return pixmap;
  //return to_pixmap(to_pixbuf(pixmap)->scale_simple(w, h, Gdk::INTERP_BILINEAR));
}

bool image_width_is_multiple_of_image_height(const std::string file)
{
  Glib::RefPtr<Gdk::Pixbuf> row;
  try
    {
      row = Gdk::Pixbuf::create_from_file(file);
    }
  catch (const Glib::Exception &ex)
    {
      return false;
    }

  guint32 width = row->get_width();
  guint32 height = row->get_height();

  if ((width % height) != 0)
    return false;
  return true;
}

void get_image_width_and_height (const std::string &file, guint32 &width, guint32 &height, bool &broken)
{
  Glib::RefPtr<Gdk::Pixbuf> row;
  try
    {
      row = Gdk::Pixbuf::create_from_file(file);
    }
  catch (const Glib::Exception &ex)
    {
      broken = true;
      return;
    }
  width = row->get_width();
  height = row->get_height();
}
