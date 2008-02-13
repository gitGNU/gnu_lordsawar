//  Copyright (C) 2007, Ole Laursen
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

#include "image-helpers.h"

Glib::RefPtr<Gdk::Pixbuf> to_pixbuf(SDL_Surface *surface)
{
    // copy the surface into a new SDL_Surface with the right format, then
    // use the raw data from the surfce in the pixbuf
    if (surface->format->Amask != 0)
    {
	// we need a SDL_PixelFormat struct with the right format, the
	// easiest way to get that is through SDL_CreateRGBSurface
	static SDL_Surface *dummy = SDL_CreateRGBSurface(
	    SDL_SWSURFACE, 1, 1,
	    32, 0xFFu, 0xFFu << 8, 0xFFu << 16, 0xFFu << 24);

	SDL_Surface *tmp
	    = SDL_ConvertSurface(surface, dummy->format, SDL_SWSURFACE);

	Glib::RefPtr<Gdk::Pixbuf> pixbuf =
	    Gdk::Pixbuf::create_from_data(
		static_cast<guint8 *>(tmp->pixels), Gdk::COLORSPACE_RGB,
		true, 8, tmp->w, tmp->h, tmp->pitch,
		sigc::hide(sigc::bind(sigc::ptr_fun(SDL_FreeSurface), tmp)));
	return pixbuf;
    }
    else
    {
	// we need a SDL_PixelFormat struct with the right format, the
	// easiest way to get that is through SDL_CreateRGBSurface
	static SDL_Surface *dummy = SDL_CreateRGBSurface(
	    SDL_SWSURFACE, 1, 1, 24, 0xFFu, 0xFFu << 8, 0xFFu << 16, 0);

	SDL_Surface *tmp
	    = SDL_ConvertSurface(surface, dummy->format, SDL_SWSURFACE);

	Glib::RefPtr<Gdk::Pixbuf> pixbuf =
	    Gdk::Pixbuf::create_from_data(
		static_cast<guint8 *>(tmp->pixels), Gdk::COLORSPACE_RGB,
		false, 8, tmp->w, tmp->h, tmp->pitch,
		sigc::hide(sigc::bind(sigc::ptr_fun(SDL_FreeSurface), tmp)));
	return pixbuf;
    }
}

std::vector<Glib::RefPtr<Gdk::Pixbuf> >
disassemble_row(const std::string &file, int no)
{
    Glib::RefPtr<Gdk::Pixbuf> row = Gdk::Pixbuf::create_from_file(file);

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
    
    return images;
}
