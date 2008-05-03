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

#include "tilestyle.h"
#include "defs.h"
#include <iostream>

using namespace std;

TileStyle::TileStyle(XML_Helper* helper)
{
  int i;
  d_pixmap = NULL;
  char *end = NULL;
  std::string idstr;

  helper->getData(idstr, "id");
  unsigned long int val = 0;
  val = strtoul (idstr.c_str(), &end, 0);
  d_id = (Uint32) val;
  helper->getData(i, "type");
  d_type = static_cast<TileStyle::Type>(i);

}
    
void TileStyle::instantiatePixmap(SDL_Surface *tilestyles, Uint32 tilesize,
				  int index)
{
  SDL_Surface* tmp;
  SDL_PixelFormat* fmt = tilestyles->format;

  tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, tilesize, tilesize, 
			     fmt->BitsPerPixel,
			     fmt->Rmask, fmt->Gmask, 
			     fmt->Bmask, fmt->Amask);

  SDL_Rect r;
  r.x = index * tilesize;
  r.y = 0;
  r.w = r.h = tilesize;
      
  SDL_BlitSurface(tilestyles, &r, tmp, NULL);

  if (d_pixmap)
    free(d_pixmap);

  d_pixmap = SDL_DisplayFormat(tmp);

  SDL_FreeSurface(tmp);
}

TileStyle::~TileStyle()
{
  if (d_pixmap)
    SDL_FreeSurface(d_pixmap);
}

bool TileStyle::save(XML_Helper *helper)
{
  bool retval = true;

  char *idstr = NULL;
  retval &= helper->openTag("tilestyle");
  if (asprintf (&idstr, "0x%02x", d_id) != -1)
    {
      retval &= helper->saveData("id", idstr);
      free (idstr);
    }
  else
    retval &= false;
  retval &= helper->saveData("type", d_type);
  retval &= helper->closeTag();

  return retval;
}
// End of file
