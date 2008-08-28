// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#include "config.h"

#include "GraphicsLoader.h"
#include "Configuration.h"
#include "File.h"
#include "defs.h"
#include "armyset.h"
#include "armysetlist.h"
#include "shieldset.h"
#include "shieldsetlist.h"
#include "armyproto.h"
#include "tilesetlist.h"

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

void GraphicsLoader::instantiatePixmaps(Shieldset *shieldset)
{
  uninstantiatePixmaps(shieldset);
  for (Shieldset::iterator sit = shieldset->begin(); sit != shieldset->end(); 
       sit++)
    {
      std::string s;
      Shield *shield = *sit;
      for (Shield::iterator it = shield->begin(); it != shield->end(); it++)
	{
	  ShieldStyle *ss = *it;
	  // The shield image consists of two halves. On the left is the shield 
	  // image, on the right the mask.
	  SDL_Surface* pic = getShieldsetPicture(shieldset->getSubDir(), 
						 ss->getImageName() + ".png");
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
	  switch (ss->getType())
	    {
	    case ShieldStyle::SMALL:
	      xsize = shieldset->getSmallWidth(); ysize = shieldset->getSmallHeight(); break;
	    case ShieldStyle::MEDIUM:
	      xsize = shieldset->getMediumWidth(); ysize = shieldset->getMediumHeight(); break;
	    case ShieldStyle::LARGE:
	      xsize = shieldset->getLargeWidth(); ysize = shieldset->getLargeHeight(); break;
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
	  ss->setPixmap(pixmap);

	  SDL_FreeSurface(tmp);

	  // now extract the mask; it should have a certain data format since the 
	  // player colors are applied by modifying the RGB shifts
	  tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, xsize, ysize, 32,
				     0xFF000000, 0xFF0000, 0xFF00, 0xFF);

	  r.x = xsize;
	  SDL_BlitSurface(pic, &r, tmp, 0);
	  ss->setMask(tmp);

	  SDL_FreeSurface(pic);
	}
    }
}

void GraphicsLoader::instantiatePixmaps(Shieldsetlist *ssl)
{
  uninstantiatePixmaps(ssl);
  for (Shieldsetlist::iterator it = ssl->begin(); it != ssl->end(); it++)
    instantiatePixmaps(*it);
}

void GraphicsLoader::uninstantiatePixmaps(Shieldsetlist *ssl)
{
  for (Shieldsetlist::iterator it = ssl->begin(); it != ssl->end(); it++)
    uninstantiatePixmaps(*it);
}

void GraphicsLoader::uninstantiatePixmaps(Shieldset *shieldset)
{
  for (Shieldset::iterator sit = shieldset->begin(); sit != shieldset->end(); 
       sit++)
    {
      Shield *shield = *sit;
      for (Shield::iterator i = shield->begin(); i != shield->end(); i++)
	{
	  if ((*i)->getPixmap())
	    {
	      SDL_FreeSurface((*i)->getPixmap());
	      (*i)->setPixmap(NULL);
	    }
	  if ((*i)->getMask())
	    {
	      SDL_FreeSurface((*i)->getMask());
	      (*i)->setMask(NULL);
	    }
	}
    }
}

void GraphicsLoader::instantiatePixmaps(Shieldsetlist *ssl, std::string subdir)
{
  for (Shieldsetlist::iterator it = ssl->begin(); it != ssl->end(); it++)
    {
      if ((*it)->getSubDir() == subdir)
	instantiatePixmaps(*it);
    }
}

void GraphicsLoader::instantiatePixmaps(Tilesetlist *tsl)
{
  uninstantiatePixmaps(tsl);
  for (Tilesetlist::iterator it = tsl->begin(); it != tsl->end(); it++)
    instantiatePixmaps(*it);
}

void GraphicsLoader::uninstantiatePixmaps(Tilesetlist *tsl)
{
  for (Tilesetlist::iterator tit = tsl->begin(); tit != tsl->end(); tit++)
    uninstantiatePixmaps(*tit);
}

void GraphicsLoader::uninstantiatePixmaps(Tileset *ts)
{
  for (Tileset::iterator it = ts->begin(); it != ts->end(); it++)
    for (Tile::iterator i = (*it)->begin(); i != (*it)->end(); i++)
      {
	TileStyleSet *tss = (*i);
	for (unsigned int j = 0; j < tss->size(); j++)
	  {
	    if ((*tss)[j]->getPixmap())
	      {
		SDL_FreeSurface((*tss)[j]->getPixmap());
		(*tss)[j]->setPixmap(NULL);
	      }
	  }
      }
}

void GraphicsLoader::instantiatePixmaps(Tileset *ts)
{
  uninstantiatePixmaps(ts);
  for (Tileset::iterator it = ts->begin(); it != ts->end(); it++)
    {
      for (Tile::iterator i = (*it)->begin(); i != (*it)->end(); i++)
	instantiatePixmaps(*i, ts->getTileSize());
    }
}

void GraphicsLoader::instantiatePixmaps(TileStyleSet *tss, Uint32 tilesize)
{
  SDL_Surface* pixmaps = getTilesetPicture(tss->getSubDir(), 
					   tss->getName() + ".png");
  if (pixmaps)
    {
      for (unsigned int i=0; i < tss->size(); i++)
	{
	  //(*this)[i]->instantiatePixmap(pixmaps, tilesize, i);
	  SDL_Surface* tmp;
	  SDL_PixelFormat* fmt = pixmaps->format;

	  tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, tilesize, tilesize, 
				     fmt->BitsPerPixel,
				     fmt->Rmask, fmt->Gmask, 
				     fmt->Bmask, fmt->Amask);

	  SDL_Rect r;
	  r.x = i * tilesize;
	  r.y = 0;
	  r.w = r.h = tilesize;

	  SDL_BlitSurface(pixmaps, &r, tmp, NULL);

	  if ((*tss)[i]->getPixmap())
	    {
	      SDL_FreeSurface((*tss)[i]->getPixmap());
	      (*tss)[i]->setPixmap(NULL);
	    }
	  (*tss)[i]->setPixmap(SDL_DisplayFormat(tmp));

	  SDL_FreeSurface(tmp);
	}
      SDL_FreeSurface (pixmaps);
    }
}

void GraphicsLoader::loadShipPic(Armyset *armyset)
{
  //load the ship picture and it's mask
  SDL_Rect shiprect;

  SDL_Surface* shippic = getArmyPicture(armyset->getSubDir(), "stackship.png");
  // copy alpha values, don't use them
  SDL_SetAlpha(shippic, 0, 0);
  SDL_PixelFormat* fmt = shippic->format;
  Uint32 tilesize = armyset->getTileSize();
  SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, 
					  tilesize, tilesize, 
					  fmt->BitsPerPixel, fmt->Rmask, 
					  fmt->Gmask, fmt->Bmask, 
					  fmt->Amask);
  shiprect.x = 0;
  shiprect.y = 0;
  shiprect.w = shiprect.h = tilesize;
  SDL_BlitSurface(shippic, &shiprect, tmp, 0);
  SDL_Surface *ship = SDL_DisplayFormatAlpha(tmp);
  SDL_FreeSurface(tmp);

  SDL_Surface *shipmask;
  shipmask = SDL_CreateRGBSurface(SDL_SWSURFACE, tilesize, tilesize, 32,
				  0xFF000000, 0xFF0000, 0xFF00, 
				  0xFF);
  shiprect.x = tilesize;
  shiprect.y = 0;
  shiprect.w = shiprect.h = tilesize;
  SDL_BlitSurface(shippic, &shiprect, shipmask, 0);

  armyset->setShipImage(ship);
  armyset->setShipMask(shipmask);
  SDL_FreeSurface(shippic);
}

void GraphicsLoader::loadStandardPic(Armyset *armyset)
{
  //load the planted standard picture and it's mask
  SDL_Rect standrect;
  SDL_Surface* standpic = getArmyPicture(armyset->getSubDir(), "plantedstandard.png");
  // copy alpha values, don't use them
  SDL_SetAlpha(standpic, 0, 0);
  SDL_PixelFormat* fmt = standpic->format;
  int size = armyset->getTileSize();
  SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, 
					  fmt->BitsPerPixel, fmt->Rmask, 
					  fmt->Gmask, fmt->Bmask, 
					  fmt->Amask);
  standrect.x = 0;
  standrect.y = 0;
  standrect.w = standrect.h = size;
  SDL_BlitSurface(standpic, &standrect, tmp, 0);
  SDL_Surface *standard = SDL_DisplayFormatAlpha(tmp);
  SDL_FreeSurface(tmp);

  SDL_Surface *standard_mask;
  standard_mask = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, 32, 
				       0xFF000000, 0xFF0000, 0xFF00, 0xFF);
  standrect.x = size;
  standrect.y = 0;
  standrect.w = standrect.h = size;
  SDL_BlitSurface(standpic, &standrect, standard_mask, 0);

  armyset->setStandardImage(standard);
  armyset->setStandardMask(standard_mask);
  SDL_FreeSurface(standpic);
}

bool GraphicsLoader::instantiatePixmaps(Armyset *set, ArmyProto *a)
{
  std::string s;

  if (a->getImageName() == "")
    return false;
  // load the army picture. This is done here to avoid confusion
  // since the armies are used as prototypes as well as actual units in the
  // game.
  // The army image consists of two halves. On the left is the army image, 
  // on the right the mask.
  SDL_Surface* pic = getArmyPicture(set->getSubDir(), a->getImageName() + ".png");
  if (!pic)
    {
      std::cerr <<"Could not load army image: " << s <<std::endl;
      exit(-1);
    }

  Uint32 size = set->getTileSize();
  // don't use alpha information, just copy the channel! very important
  SDL_SetAlpha(pic, 0, 0);
  SDL_PixelFormat* fmt = pic->format;

  // mask out the army image 
  SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, 
					  size, size,
					  fmt->BitsPerPixel, fmt->Rmask, 
					  fmt->Gmask, fmt->Bmask, fmt->Amask);
  SDL_Rect r;
  r.x = r.y = 0;
  r.w = r.h = size;
  SDL_BlitSurface(pic, &r, tmp, 0);

  SDL_Surface* pixmap = SDL_DisplayFormatAlpha(tmp);
  if (a->getPixmap())
    {
      SDL_FreeSurface(a->getPixmap());
      a->setPixmap(NULL);
    }
  a->setPixmap(pixmap);

  SDL_FreeSurface(tmp);

  // now extract the mask; it should have a certain data format since the 
  // player colors are applied by modifying the RGB shifts
  tmp = SDL_CreateRGBSurface(SDL_SWSURFACE, size, size, 32,
			     0xFF000000, 0xFF0000, 0xFF00, 0xFF);

  r.x = size;
  SDL_BlitSurface(pic, &r, tmp, 0);
  a->setMask(tmp);

  SDL_FreeSurface(pic);

  return true;
}

void GraphicsLoader::instantiatePixmaps(Armyset *armyset)
{
  for (Armyset::iterator it = armyset->begin(); it != armyset->end(); it++)
    instantiatePixmaps(armyset, *it);
  loadShipPic(armyset);
  loadStandardPic(armyset);
}

void GraphicsLoader::instantiatePixmaps(Armysetlist *asl)
{
  for (Armysetlist::iterator it = asl->begin(); it != asl->end(); it++)
    instantiatePixmaps(*it);
}

void GraphicsLoader::uninstantiatePixmaps(Armysetlist *asl)
{
  for (Armysetlist::iterator it = asl->begin(); it != asl->end(); it++)
    uninstantiatePixmaps(*it);
}

void GraphicsLoader::uninstantiatePixmaps(Armyset *armyset)
{
  for (Armyset::iterator i = armyset->begin(); i != armyset->end(); i++)
    {
      if ((*i)->getPixmap())
	{
	  SDL_FreeSurface((*i)->getPixmap());
	  (*i)->setPixmap(NULL);
	}
      if ((*i)->getMask())
	{
	  SDL_FreeSurface((*i)->getMask());
	  (*i)->setMask(NULL);
	}
    }
  if (armyset->getShipPic())
    {
      SDL_FreeSurface(armyset->getShipPic());
      armyset->setShipImage(NULL);
    }
  if (armyset->getShipMask())
    {
      SDL_FreeSurface(armyset->getShipMask());
      armyset->setShipMask(NULL);
    }
  if (armyset->getStandardPic())
    {
      SDL_FreeSurface(armyset->getStandardPic());
      armyset->setStandardImage(NULL);
    }
  if (armyset->getStandardMask())
    {
      SDL_FreeSurface(armyset->getStandardMask());
      armyset->setStandardMask(NULL);
    }
}

SDL_Surface* GraphicsLoader::loadImage(std::string filename, bool alpha)
{
  SDL_Surface* tmp = IMG_Load(filename.c_str());
  if (!tmp)
    {
      std::cerr << _("ERROR: Couldn't load image ") << filename << std::endl;
      return 0;
    }

  SDL_Surface* convertedImage;
  // 1:1 copy
  if (alpha)
    convertedImage = SDL_DisplayFormatAlpha(tmp);
  // strip alpha channel if any
  else
    convertedImage = SDL_DisplayFormat(tmp);
  SDL_FreeSurface(tmp);

  return convertedImage;

}

SDL_Surface* GraphicsLoader::getArmyPicture(std::string armysetdir, std::string pic)
{
  return loadImage(Configuration::s_dataPath + "/army/" + armysetdir + "/" + pic);
}

SDL_Surface* GraphicsLoader::getTilesetPicture(std::string tilesetdir, std::string picname)
{
  return loadImage(File::getTilesetFile(tilesetdir, picname));
}

SDL_Surface* GraphicsLoader::getMiscPicture(std::string picname, bool alpha)
{
  return loadImage(Configuration::s_dataPath + "/various/" + picname,alpha);
}

SDL_Surface* GraphicsLoader::getShieldsetPicture(std::string shieldsetdir, std::string picname)
{
  return loadImage(File::getShieldsetFile(shieldsetdir, picname));
}

SDL_Surface* GraphicsLoader::getCitysetPicture(std::string citysetdir, std::string picname)
{
  return loadImage(File::getCitysetFile(citysetdir, picname));
}

// End of file
