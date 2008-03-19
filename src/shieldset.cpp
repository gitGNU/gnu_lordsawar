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
#include <expat.h>
#include <SDL_image.h>
#include <SDL.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "shieldset.h"
#include "File.h"
#include "defs.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

Shieldset::Shieldset(XML_Helper *helper)
    : d_dir("")
{
  helper->getData(d_name, "name");
  helper->getData(d_small_width, "small_width");
  helper->getData(d_small_height, "small_height");
  helper->getData(d_medium_width, "medium_width");
  helper->getData(d_medium_height, "medium_height");
  helper->getData(d_large_width, "large_width");
  helper->getData(d_large_height, "large_height");
  helper->registerTag("shield", sigc::mem_fun((*this), 
					      &Shieldset::loadShield));
}

Shieldset::~Shieldset()
{
  for (iterator it = begin(); it != end(); it++)
      delete *it;
}

void Shieldset::instantiatePixmaps()
{
  iterator a = begin();
  for (iterator it = begin(); it != end(); it++)
    instantiatePixmap(*it);
}

bool Shieldset::instantiatePixmap(Shield *sh)
{
    std::string s;

    // The shield image consists of two halves. On the left is the shield 
    // image, on the right the mask.
    SDL_Surface* pic = File::getShieldPicture(d_dir, sh->getImageName() + ".png");
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
    switch (sh->getType())
      {
      case Shield::SMALL:
	xsize = getSmallWidth(); ysize = getSmallHeight(); break;
      case Shield::MEDIUM:
	xsize = getMediumWidth(); ysize = getMediumHeight(); break;
      case Shield::LARGE:
	xsize = getLargeWidth(); ysize = getLargeHeight(); break;
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
    sh->setPixmap(pixmap);

    SDL_FreeSurface(tmp);

    // now extract the mask; it should have a certain data format since the 
    // player colors are applied by modifying the RGB shifts
    tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, xsize, ysize, 32,
                               0xFF000000, 0xFF0000, 0xFF00, 0xFF);

    r.x = xsize;
    SDL_BlitSurface(pic, &r, tmp, 0);
    sh->setMask(tmp);

    SDL_FreeSurface(pic);

    return true;
}

bool Shieldset::loadShield(string tag, XML_Helper* helper)
{
    if (tag == "shield")
      {
	std::string s;
	// First step: Load the shield data
	Shield* sh = new Shield(helper);
	sh->setShieldset(d_dir);
	push_back(sh);
      }
    return true;
}

Shield * Shieldset::lookupShieldByTypeAndColour(Uint32 type, Uint32 colour)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getType() == type && (*it)->getColour() == colour)
	return *it;
    }
  return NULL;
}

    
