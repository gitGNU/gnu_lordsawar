// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009 Ben Asselstine
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
#include "armyset.h"
#include "armysetlist.h"
#include "shieldset.h"
#include "shieldsetlist.h"
#include "armyproto.h"
#include "tilesetlist.h"
#include "defs.h"
#include "gui/image-helpers.h"

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

void GraphicsLoader::instantiateImages(Shieldset *shieldset)
{
  uninstantiateImages(shieldset);
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
	  std::vector<Glib::RefPtr<Gdk::Pixbuf> > half;
	  half = disassemble_row(File::getShieldsetFile(shieldset->getSubDir(), 
							ss->getImageName() 
							+ ".png"), 2);

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
	  half[0] = half[0]->scale_simple (xsize, ysize, Gdk::INTERP_BILINEAR);
	  half[1] = half[1]->scale_simple (xsize, ysize, Gdk::INTERP_BILINEAR);
	  ss->setImage(half[0]);
	  ss->setMask(half[1]);
	}
    }
}

void GraphicsLoader::instantiateImages(Shieldsetlist *ssl)
{
  uninstantiateImages(ssl);
  for (Shieldsetlist::iterator it = ssl->begin(); it != ssl->end(); it++)
    instantiateImages(*it);
}

void GraphicsLoader::uninstantiateImages(Shieldsetlist *ssl)
{
  for (Shieldsetlist::iterator it = ssl->begin(); it != ssl->end(); it++)
    uninstantiateImages(*it);
}

void GraphicsLoader::uninstantiateImages(Shieldset *shieldset)
{
  for (Shieldset::iterator sit = shieldset->begin(); sit != shieldset->end(); 
       sit++)
    {
      Shield *shield = *sit;
      for (Shield::iterator i = shield->begin(); i != shield->end(); i++)
	{
	  if ((*i)->getImage() == true)
	    {
	      (*i)->getImage().clear();
	      (*i)->setImage(Glib::RefPtr<Gdk::Pixbuf>(0));
	    }
	  if ((*i)->getMask() == true)
	    {
	      (*i)->getMask().clear();
	      (*i)->setMask(Glib::RefPtr<Gdk::Pixbuf>(0));
	    }
	}
    }
}

void GraphicsLoader::instantiateImages(Shieldsetlist *ssl, std::string subdir)
{
  for (Shieldsetlist::iterator it = ssl->begin(); it != ssl->end(); it++)
    {
      if ((*it)->getSubDir() == subdir)
	instantiateImages(*it);
    }
}

void GraphicsLoader::instantiateImages(Tilesetlist *tsl)
{
  uninstantiateImages(tsl);
  for (Tilesetlist::iterator it = tsl->begin(); it != tsl->end(); it++)
    instantiateImages(*it);
}

void GraphicsLoader::uninstantiateImages(Tilesetlist *tsl)
{
  for (Tilesetlist::iterator tit = tsl->begin(); tit != tsl->end(); tit++)
    uninstantiateImages(*tit);
}

void GraphicsLoader::uninstantiateImages(Tileset *ts)
{
  for (Tileset::iterator it = ts->begin(); it != ts->end(); it++)
    for (Tile::iterator i = (*it)->begin(); i != (*it)->end(); i++)
      {
	TileStyleSet *tss = (*i);
	for (unsigned int j = 0; j < tss->size(); j++)
	  {
	    if ((*tss)[j]->getImage() == true)
	      {
		(*tss)[j]->getImage().clear();
		(*tss)[j]->setImage(Glib::RefPtr<Gdk::Pixbuf>(0));
	      }
	  }
      }
}

void GraphicsLoader::instantiateImages(Tileset *ts)
{
  uninstantiateImages(ts);
  for (Tileset::iterator it = ts->begin(); it != ts->end(); it++)
    {
      for (Tile::iterator i = (*it)->begin(); i != (*it)->end(); i++)
	instantiateImages(*i, ts->getTileSize());
    }
}

void GraphicsLoader::instantiateImages(TileStyleSet *tss, guint32 tsize)
{
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > styles;
  styles = disassemble_row
    (File::getTilesetFile(tss->getSubDir(), tss->getName() + ".png"), 
     tss->size());

  for (unsigned int i=0; i < tss->size(); i++)
    {
      styles[i] = styles[i]->scale_simple(tsize, tsize, Gdk::INTERP_BILINEAR);
      (*tss)[i]->setImage(styles[i]);
    }
}

void GraphicsLoader::loadShipPic(Armyset *armyset)
{
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > half;
  half = disassemble_row(File::getArmysetFile(armyset->getSubDir(), "stackship.png"), 2);
  int size = armyset->getTileSize();
  half[0] = half[0]->scale_simple(size, size, Gdk::INTERP_BILINEAR);
  half[1] = half[1]->scale_simple(size, size, Gdk::INTERP_BILINEAR);
  armyset->setShipImage(half[0]);
  armyset->setShipMask(half[1]);
}

void GraphicsLoader::loadStandardPic(Armyset *armyset)
{
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > half;
  half = disassemble_row(File::getArmysetFile(armyset->getSubDir(), "plantedstandard.png"), 2);
  int size = armyset->getTileSize();
  half[0] = half[0]->scale_simple(size, size, Gdk::INTERP_BILINEAR);
  half[1] = half[1]->scale_simple(size, size, Gdk::INTERP_BILINEAR);
  armyset->setStandardPic(half[0]);
  armyset->setStandardMask(half[1]);
}

bool GraphicsLoader::instantiateImages(Armyset *armyset, ArmyProto *a)
{
  std::string s;

  if (a->getImageName() == "")
    return false;
  // load the army picture. This is done here to avoid confusion
  // since the armies are used as prototypes as well as actual units in the
  // game.
  // The army image consists of two halves. On the left is the army image, 
  // on the right the mask.
  std::vector<Glib::RefPtr<Gdk::Pixbuf> > half;
  half = disassemble_row(File::getArmysetFile(armyset->getSubDir(), a->getImageName() + ".png"), 2);
  int size = armyset->getTileSize();
  half[0] = half[0]->scale_simple(size, size, Gdk::INTERP_BILINEAR);
  half[1] = half[1]->scale_simple(size, size, Gdk::INTERP_BILINEAR);

  a->setImage(half[0]);
  a->setMask(half[1]);

  return true;
}

void GraphicsLoader::instantiateImages(Armyset *armyset)
{
  for (Armyset::iterator it = armyset->begin(); it != armyset->end(); it++)
    instantiateImages(armyset, *it);
  loadShipPic(armyset);
  loadStandardPic(armyset);
}

void GraphicsLoader::instantiateImages(Armysetlist *asl)
{
  for (Armysetlist::iterator it = asl->begin(); it != asl->end(); it++)
    instantiateImages(*it);
}

void GraphicsLoader::uninstantiateImages(Armysetlist *asl)
{
  for (Armysetlist::iterator it = asl->begin(); it != asl->end(); it++)
    uninstantiateImages(*it);
}

void GraphicsLoader::uninstantiateImages(Armyset *armyset)
{
  for (Armyset::iterator i = armyset->begin(); i != armyset->end(); i++)
    {
      if ((*i)->getImage() == true)
	{
	  (*i)->getImage().clear();
	  (*i)->setImage(Glib::RefPtr<Gdk::Pixbuf>(0));
	}
      if ((*i)->getMask() == true)
	{
	  (*i)->getMask().clear();
	  (*i)->setMask(Glib::RefPtr<Gdk::Pixbuf>(0));
	}
    }
  if (armyset->getShipPic() == true)
    {
      armyset->getShipPic().clear();
      armyset->setShipImage(Glib::RefPtr<Gdk::Pixbuf>(0));
    }
  if (armyset->getShipMask() == true)
    {
      armyset->getShipMask().clear();
      armyset->setShipMask(Glib::RefPtr<Gdk::Pixbuf>(0));
    }
  if (armyset->getStandardPic() == true)
    {
      armyset->getStandardPic().clear();
      armyset->setStandardPic(Glib::RefPtr<Gdk::Pixbuf>(0));
    }
  if (armyset->getStandardMask())
    {
      armyset->getStandardMask().clear();
      armyset->setStandardMask(Glib::RefPtr<Gdk::Pixbuf>(0));
    }
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsLoader::loadImage(std::string filename, bool alpha)
{
  return Gdk::Pixbuf::create_from_file(filename);

}

Glib::RefPtr<Gdk::Pixbuf> GraphicsLoader::getArmyPicture(std::string armysetdir, std::string pic)
{
  return loadImage(Configuration::s_dataPath + "/army/" + armysetdir + "/" + pic);
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsLoader::getTilesetPicture(std::string tilesetdir, std::string picname)
{
  return loadImage(File::getTilesetFile(tilesetdir, picname));
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsLoader::getMiscPicture(std::string picname, bool alpha)
{
  return loadImage(Configuration::s_dataPath + "/various/" + picname,alpha);
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsLoader::getShieldsetPicture(std::string shieldsetdir, std::string picname)
{
  return loadImage(File::getShieldsetFile(shieldsetdir, picname));
}

Glib::RefPtr<Gdk::Pixbuf> GraphicsLoader::getCitysetPicture(std::string citysetdir, std::string picname)
{
  return loadImage(File::getCitysetFile(citysetdir, picname));
}

// End of file
