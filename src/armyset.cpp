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

#include <iostream>
#include <expat.h>
#include <SDL_image.h>
#include <SDL.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "armyset.h"
#include "File.h"
#include "defs.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

void Armyset::instantiatePixmaps()
{
  iterator a = begin();
  for (std::list<std::string>::iterator it = image_names.begin();
       it != image_names.end(); it++, a++)
    instantiatePixmap(*a, *it);
}
Armyset::Armyset(XML_Helper *helper)
    : d_id(0), d_name(""), d_dir("")
{
  helper->getData(d_id, "id");
  helper->getData(d_name, "name");
  helper->registerTag("army", sigc::mem_fun((*this), 
					      &Armyset::loadArmyTemplate));
}

Armyset::~Armyset()
{
  for (iterator it = begin(); it != end(); it++)
    {
      //fixme: is freeing the surfaces here necessary, or is it done in
      //graphicscache?
      SDL_FreeSurface((*it)->getPixmap());
      SDL_FreeSurface((*it)->getMask());
      delete *it;
    }
}

bool Armyset::instantiatePixmap(Army *a, std::string image)
{
    static int armysize = 54;   // army pic has this size
    std::string s;

    // load the army picture. This is done here to avoid confusion
    // since the armies are used as prototypes as well as actual units in the
    // game.
    // The army image consists of two halves. On the left is the army image, on the
    // right the mask.
    SDL_Surface* pic = File::getArmyPicture(d_dir, image + ".png");
    if (!pic)
    {
        std::cerr <<"Could not load army image: " << s <<std::endl;
	// FIXME: more gentle way of reporting error than just exiting?
        exit(-1);
    }

    // don't use alpha information, just copy the channel! very important
    SDL_SetAlpha(pic, 0, 0);
    SDL_PixelFormat* fmt = pic->format;

    // mask out the army image 
    SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, armysize, armysize,
                fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    SDL_Rect r;
    r.x = r.y = 0;
    r.w = r.h = armysize;
    SDL_BlitSurface(pic, &r, tmp, 0);

    SDL_Surface* pixmap = SDL_DisplayFormatAlpha(tmp);
    a->setPixmap(pixmap);

    SDL_FreeSurface(tmp);

    // now extract the mask; it should have a certain data format since the player
    // colors are applied by modifying the RGB shifts
    tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, armysize, armysize, 32,
                               0xFF000000, 0xFF0000, 0xFF00, 0xFF);

    r.x = armysize;
    SDL_BlitSurface(pic, &r, tmp, 0);
    a->setMask(tmp);

    SDL_FreeSurface(pic);

    return true;
}

bool Armyset::loadArmyTemplate(string tag, XML_Helper* helper)
{
    if (tag == "army")
      {
	std::string s;
	// First step: Load the army data
	Army* a = new Army(helper, true);
	a->setArmyset(d_id, size());
	push_back(a);

	// Second step: remember the image name for later
	helper->getData(s, "image");
	image_names.push_back(s);
      }
    return true;
}

