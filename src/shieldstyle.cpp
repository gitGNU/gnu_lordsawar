//  Copyright (C) 2008, Ben Asselstine
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

#include <iostream>
#include <sstream>
#include "shieldstyle.h"
#include "GraphicsCache.h"
#include "xmlhelper.h"
#include "File.h"
#include "shieldset.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

ShieldStyle::ShieldStyle(XML_Helper* helper)
  :d_pixmap(0), d_mask(0)
{
  std::string type_str;
  helper->getData(type_str, "type");
  d_type = shieldStyleTypeFromString(type_str);
  helper->getData(d_image, "image");
}

ShieldStyle::~ShieldStyle()
{
    if (d_pixmap)
        SDL_FreeSurface(d_pixmap);
    if (d_mask)
        SDL_FreeSurface(d_mask);
}

SDL_Surface* ShieldStyle::getPixmap() const
{
    // if we already have a pixmap return it
    if (d_pixmap)
        return d_pixmap;
    else
      return NULL;
}

void ShieldStyle::setPixmap(SDL_Surface* pixmap)
{
  if (d_pixmap)
    SDL_FreeSurface(d_pixmap);
  d_pixmap = pixmap;
}
        
void ShieldStyle::setMask(SDL_Surface* mask)
{
  if (d_mask)
    SDL_FreeSurface(d_mask);
  d_mask = mask;
}

bool ShieldStyle::instantiatePixmap(Shieldset *sh)
{
    std::string s;

    // The shield image consists of two halves. On the left is the shield 
    // image, on the right the mask.
    SDL_Surface* pic = File::getShieldPicture(sh->getSubDir(), 
					      getImageName() + ".png");
    if (!pic)
    {
        std::cerr <<"Could not load shield image: " << s <<std::endl;
        exit(-1);
    }

    // don't use alpha information, just copy the channel! very important
    SDL_SetAlpha(pic, 0, 0);
    SDL_PixelFormat* fmt = pic->format;

    int xsize = 0;
    int ysize = 0;
    switch (getType())
      {
      case ShieldStyle::SMALL:
	xsize = sh->getSmallWidth(); ysize = sh->getSmallHeight(); break;
      case ShieldStyle::MEDIUM:
	xsize = sh->getMediumWidth(); ysize = sh->getMediumHeight(); break;
      case ShieldStyle::LARGE:
	xsize = sh->getLargeWidth(); ysize = sh->getLargeHeight(); break;
      }

    // mask out the shield image 
    SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, 
					    xsize, ysize,
					    fmt->BitsPerPixel, fmt->Rmask, 
					    fmt->Gmask, fmt->Bmask, fmt->Amask);
    SDL_Rect r;
    r.x = r.y = 0;
    r.w = xsize;
    r.h = ysize;
    SDL_BlitSurface(pic, &r, tmp, 0);

    SDL_Surface* pixmap = SDL_DisplayFormatAlpha(tmp);
    setPixmap(pixmap);

    SDL_FreeSurface(tmp);

    // now extract the mask; it should have a certain data format since the 
    // player colors are applied by modifying the RGB shifts
    tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, xsize, ysize, 32,
                               0xFF000000, 0xFF0000, 0xFF00, 0xFF);

    r.x = xsize;
    SDL_BlitSurface(pic, &r, tmp, 0);
    setMask(tmp);

    SDL_FreeSurface(pic);

    return true;
}

std::string ShieldStyle::shieldStyleTypeToString(const ShieldStyle::Type type)
{
  switch (type)
    {
      case ShieldStyle::SMALL:
	return "ShieldStyle::SMALL";
	break;
      case ShieldStyle::MEDIUM:
	return "ShieldStyle::MEDIUM";
	break;
      case ShieldStyle::LARGE:
	return "ShieldStyle::LARGE";
	break;
    }
  return "ShieldStyle::SMALL";
}

ShieldStyle::Type ShieldStyle::shieldStyleTypeFromString(const std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return ShieldStyle::Type(atoi(str.c_str()));
  if (str == "ShieldStyle::SMALL")
    return ShieldStyle::SMALL;
  else if (str == "ShieldStyle::MEDIUM")
    return ShieldStyle::MEDIUM;
  else if (str == "ShieldStyle::LARGE")
    return ShieldStyle::LARGE;
  return ShieldStyle::SMALL;
}
