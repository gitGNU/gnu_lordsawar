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

#define DEFAULT_ARMY_TILE_SIZE 40
Armyset::Armyset(Uint32 id, std::string name)
	: d_id(id), d_name(name), d_dir(""), d_tilesize(DEFAULT_ARMY_TILE_SIZE)
{
  d_ship = NULL;
  d_shipmask = NULL;
}

Armyset::Armyset(XML_Helper *helper)
    : d_id(0), d_name(""), d_dir(""), d_tilesize(DEFAULT_ARMY_TILE_SIZE)
{
  d_ship = NULL;
  d_shipmask = NULL;
  helper->getData(d_id, "id");
  helper->getData(d_name, "name");
  helper->getData(d_tilesize, "tilesize");
  helper->registerTag("army", sigc::mem_fun((*this), 
					      &Armyset::loadArmyTemplate));
}

Armyset::~Armyset()
{
  for (iterator it = begin(); it != end(); it++)
      delete *it;
}

void Armyset::instantiatePixmaps()
{
  iterator a = begin();
  for (iterator it = begin(); it != end(); it++)
    instantiatePixmap(*it);
  loadShipPic();
  loadStandardPic();
}

bool Armyset::instantiatePixmap(Army *a)
{
    std::string s;

    if (a->getImageName() == "")
      return false;
    // load the army picture. This is done here to avoid confusion
    // since the armies are used as prototypes as well as actual units in the
    // game.
    // The army image consists of two halves. On the left is the army image, on the
    // right the mask.
    SDL_Surface* pic = File::getArmyPicture(d_dir, a->getImageName() + ".png");
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
    SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, 
					    d_tilesize, d_tilesize,
					    fmt->BitsPerPixel, fmt->Rmask, 
					    fmt->Gmask, fmt->Bmask, fmt->Amask);
    SDL_Rect r;
    r.x = r.y = 0;
    r.w = r.h = d_tilesize;
    SDL_BlitSurface(pic, &r, tmp, 0);

    SDL_Surface* pixmap = SDL_DisplayFormatAlpha(tmp);
    a->setPixmap(pixmap);

    SDL_FreeSurface(tmp);

    // now extract the mask; it should have a certain data format since the 
    // player colors are applied by modifying the RGB shifts
    tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, d_tilesize, d_tilesize, 32,
                               0xFF000000, 0xFF0000, 0xFF00, 0xFF);

    r.x = d_tilesize;
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
	Army* a = new Army(helper, Army::TYPE);
	a->setArmyset(d_id, size());
	push_back(a);
      }
    return true;
}

bool Armyset::save(XML_Helper* helper)
{
    bool retval = true;

    retval &= helper->openTag("armyset");

    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("tilesize", d_tilesize);

    for (const_iterator it = begin(); it != end(); it++)
        (*it)->save(helper, Army::TYPE);
    
    retval &= helper->closeTag();

    return retval;
}

Army * Armyset::lookupArmyByType(Uint32 army_type)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getType() == army_type)
	return *it;
    }
  return NULL;
}

void Armyset::loadShipPic()
{
  //load the ship picture and it's mask
  SDL_Rect shiprect;
    
  SDL_Surface* shippic = File::getArmyPicture(d_dir, "stackship.png");
  // copy alpha values, don't use them
  SDL_SetAlpha(shippic, 0, 0);
  SDL_PixelFormat* fmt = shippic->format;
  SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, 
					  d_tilesize, d_tilesize, 
					  fmt->BitsPerPixel, fmt->Rmask, 
					  fmt->Gmask, fmt->Bmask, 
					  fmt->Amask);
  shiprect.x = 0;
  shiprect.y = 0;
  shiprect.w = shiprect.h = d_tilesize;
  SDL_BlitSurface(shippic, &shiprect, tmp, 0);
  if (d_ship)
    SDL_FreeSurface(d_ship);
  d_ship = SDL_DisplayFormatAlpha(tmp);
  SDL_FreeSurface(tmp);

  if (d_shipmask)
    SDL_FreeSurface(d_shipmask);
  d_shipmask =  SDL_CreateRGBSurface(SDL_SWSURFACE, d_tilesize, d_tilesize, 32,
				     0xFF000000, 0xFF0000, 0xFF00, 
				     0xFF);
  shiprect.x = d_tilesize;
  shiprect.y = 0;
  shiprect.w = shiprect.h = d_tilesize;
  SDL_BlitSurface(shippic, &shiprect, d_shipmask, 0);

  SDL_FreeSurface(shippic);
}

void Armyset::loadStandardPic()
{
  //load the planted standard picture and it's mask
  SDL_Rect standrect;
  SDL_Surface* standpic = File::getArmyPicture(d_dir, "plantedstandard.png");
  // copy alpha values, don't use them
  SDL_SetAlpha(standpic, 0, 0);
  SDL_PixelFormat* fmt = standpic->format;
  int size = d_tilesize;
  SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, 
					  fmt->BitsPerPixel, fmt->Rmask, 
					  fmt->Gmask, fmt->Bmask, 
					  fmt->Amask);
  standrect.x = 0;
  standrect.y = 0;
  standrect.w = standrect.h = size;
  SDL_BlitSurface(standpic, &standrect, tmp, 0);
  d_standard = SDL_DisplayFormatAlpha(tmp);
  SDL_FreeSurface(tmp);

  d_standard_mask = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, 32, 
					 0xFF000000, 0xFF0000, 0xFF00, 0xFF);
  standrect.x = size;
  standrect.y = 0;
  standrect.w = standrect.h = size;
  SDL_BlitSurface(standpic, &standrect, d_standard_mask, 0);

  SDL_FreeSurface(standpic);
}
