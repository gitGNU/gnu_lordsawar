// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#include "config.h"

#include "GraphicsLoader.h"
#include "GraphicsCache.h"
#include "Configuration.h"
#include "File.h"
#include "armyset.h"
#include "armysetlist.h"
#include "shieldset.h"
#include "shieldsetlist.h"
#include "armyproto.h"
#include "tilesetlist.h"
#include "citysetlist.h"
#include "defs.h"
#include "city.h"
#include "temple.h"
#include "ruin.h"
#include "gui/image-helpers.h"

using namespace std;
#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

void GraphicsLoader::instantiateImages(Shieldset *shieldset)
{
  debug("Loading images for shieldset " << shieldset->getName());
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
	  std::vector<PixMask* > half;
	  half = disassemble_row(File::getShieldsetFile(shieldset, 
							ss->getImageName()), 2);

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
	  PixMask::scale(half[0], xsize, ysize);
	  PixMask::scale(half[1], xsize, ysize);
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
	  if ((*i)->getImage())
	    {
	      delete (*i)->getImage();
	      (*i)->setImage(NULL);
	    }
	  if ((*i)->getMask())
	    {
	      delete (*i)->getMask();
	      (*i)->setMask(NULL);
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
    {
      for (Tile::iterator i = (*it)->begin(); i != (*it)->end(); i++)
	{
	  TileStyleSet *tss = (*i);
	  for (unsigned int j = 0; j < tss->size(); j++)
	    {
	      if ((*tss)[j]->getImage())
		{
		  delete (*tss)[j]->getImage();
		  (*tss)[j]->setImage(NULL);
		}
	    }
	}
    }
  if (ts->getExplosionImage() != NULL)
    {
      delete ts->getExplosionImage();
      ts->setExplosionImage(NULL);
    }
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    {
      if (ts->getRoadImage(i) != NULL)
	{
	  delete ts->getRoadImage(i);
	  ts->setRoadImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    {
      if (ts->getBridgeImage(i) != NULL)
	{
	  delete ts->getBridgeImage(i);
	  ts->setBridgeImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < FOG_TYPES; i++)
    {
      if (ts->getFogImage(i) != NULL)
	{
	  delete ts->getFogImage(i);
	  ts->setFogImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < ts->getNumberOfSelectorFrames(); i++)
    {
      if (ts->getSelectorImage(i) != NULL)
	{
	  delete ts->getSelectorImage(i);
	  ts->setSelectorImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < ts->getNumberOfSelectorFrames(); i++)
    {
      if (ts->getSelectorMask(i) != NULL)
	{
	  delete ts->getSelectorMask(i);
	  ts->setSelectorMask(i, NULL);
	}
    }
  for (unsigned int i = 0; i < ts->getNumberOfSmallSelectorFrames(); i++)
    {
      if (ts->getSmallSelectorImage(i) != NULL)
	{
	  delete ts->getSmallSelectorImage(i);
	  ts->setSmallSelectorImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < ts->getNumberOfSmallSelectorFrames(); i++)
    {
      if (ts->getSmallSelectorMask(i) != NULL)
	{
	  delete ts->getSmallSelectorMask(i);
	  ts->setSmallSelectorMask(i, NULL);
	}
    }
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    {
      if (ts->getFlagImage(i) != NULL)
	{
	  delete ts->getFlagImage(i);
	  ts->setFlagImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    {
      if (ts->getFlagMask(i) != NULL)
	{
	  delete ts->getFlagMask(i);
	  ts->setFlagMask(i, NULL);
	}
    }
}

void GraphicsLoader::instantiateImages(Tileset *ts)
{
  int size = ts->getTileSize();
  debug("Loading images for tileset " << ts->getName());
  uninstantiateImages(ts);
  for (Tileset::iterator it = ts->begin(); it != ts->end(); it++)
    {
      for (Tile::iterator i = (*it)->begin(); i != (*it)->end(); i++)
	instantiateImages(ts, *i, ts->getTileSize());
    }
  ts->setExplosionImage
    (PixMask::create(File::getTilesetFile(ts, ts->getExplosionFilename())));

  std::vector<PixMask* > roadpics;
  roadpics = disassemble_row(File::getTilesetFile(ts, ts->getRoadsFilename()), 
			     ROAD_TYPES);
  for (unsigned int i = 0; i < ROAD_TYPES ; i++)
    {
      if (roadpics[i]->get_width() != size)
	PixMask::scale(roadpics[i], size, size);
      ts->setRoadImage(i, roadpics[i]);
    }

  std::vector<PixMask* > bridgepics;
  bridgepics = 
    disassemble_row(File::getTilesetFile(ts, ts->getBridgesFilename()),
		    BRIDGE_TYPES);
  for (unsigned int i = 0; i < BRIDGE_TYPES ; i++)
    {
      if (bridgepics[i]->get_width() != size)
	PixMask::scale(bridgepics[i], size, size);
      ts->setBridgeImage(i, bridgepics[i]);
    }

  std::vector<PixMask* > fogpics;
  fogpics = disassemble_row(File::getTilesetFile(ts, ts->getFogFilename()),
			    FOG_TYPES);
  for (unsigned int i = 0; i < FOG_TYPES ; i++)
    {
      if (fogpics[i]->get_width() != size)
	PixMask::scale(fogpics[i], size, size);
      ts->setFogImage(i, fogpics[i]);
    }

  std::vector<PixMask* > flagpics;
  std::vector<PixMask* > maskpics;
  GraphicsCache::loadFlagImages
    (File::getTilesetFile(ts, ts->getFlagsFilename()), size, flagpics, maskpics);
  for (unsigned int i = 0; i < flagpics.size(); i++)
    ts->setFlagImage(i, flagpics[i]);
  for (unsigned int i = 0; i < maskpics.size(); i++)
    ts->setFlagMask(i, maskpics[i]);

  std::vector<PixMask* > images;
  std::vector<PixMask* > masks;
  GraphicsCache::loadSelectorImages
    (File::getTilesetFile(ts, ts->getLargeSelectorFilename()), size, 
     images, masks);
  ts->setNumberOfSelectorFrames(images.size());
  for (unsigned int i = 0; i < images.size(); i++)
    {
      ts->setSelectorImage(i, images[i]);
      ts->setSelectorMask(i, masks[i]);
    }

  images.clear();
  masks.clear();
  GraphicsCache::loadSelectorImages
    (File::getTilesetFile(ts, ts->getSmallSelectorFilename()), size, 
     images, masks);
  ts->setNumberOfSmallSelectorFrames(images.size());
  for (unsigned int i = 0; i < images.size(); i++)
    {
      ts->setSmallSelectorImage(i, images[i]);
      ts->setSmallSelectorMask(i, masks[i]);
    }
}

void GraphicsLoader::instantiateImages(Tileset *ts, TileStyleSet *tss, guint32 tsize)
{
  std::vector<PixMask*> styles;
  styles = disassemble_row
    (File::getTilesetFile(ts, tss->getName()), 
     tss->size());

  for (unsigned int i=0; i < tss->size(); i++)
    {
      PixMask::scale(styles[i], tsize, tsize);
      (*tss)[i]->setImage(styles[i]);
    }
}

void GraphicsLoader::loadShipPic(Armyset *armyset)
{
  if (armyset->getShipImageName().empty() == true)
    return;
  debug("Loading images for armyset " << armyset->getName());
  std::vector<PixMask*> half;
  half = disassemble_row(File::getArmysetFile(armyset, 
					      armyset->getShipImageName()), 2);
  int size = armyset->getTileSize();
  PixMask::scale(half[0], size, size);
  PixMask::scale(half[1], size, size);
  armyset->setShipImage(half[0]);
  armyset->setShipMask(half[1]);
}

void GraphicsLoader::loadStandardPic(Armyset *armyset)
{
  if (armyset->getStandardImageName().empty() == true)
    return;
  std::vector<PixMask*> half;
  half = 
    disassemble_row(File::getArmysetFile(armyset, 
					 armyset->getStandardImageName()), 2);
  int size = armyset->getTileSize();
  PixMask::scale(half[0], size, size);
  PixMask::scale(half[1], size, size);
  armyset->setStandardPic(half[0]);
  armyset->setStandardMask(half[1]);
}

bool GraphicsLoader::instantiateImages(Armyset *armyset, ArmyProto *a, Shield::Colour c)
{
  std::string s;

  if (a->getImageName(c) == "")
    return false;
  // load the army picture. This is done here to avoid confusion
  // since the armies are used as prototypes as well as actual units in the
  // game.
  // The army image consists of two halves. On the left is the army image, 
  // on the right the mask.
  std::vector<PixMask*> half;
  half = disassemble_row(File::getArmysetFile(armyset, a->getImageName(c)), 2);
  int size = armyset->getTileSize();
  PixMask::scale(half[0], size, size);
  PixMask::scale(half[1], size, size);

  a->setImage(c, half[0]);
  a->setMask(c, half[1]);

  return true;
}

void GraphicsLoader::instantiateImages(Armyset *armyset)
{
  //have we already instantiated the images in this armyset?
  if (armyset->getShipPic() != NULL)
    return;
  for (Armyset::iterator it = armyset->begin(); it != armyset->end(); it++)
    for (unsigned int c = Shield::WHITE; c <= Shield::NEUTRAL; c++)
      instantiateImages(armyset, *it, Shield::Colour(c));
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
      for (unsigned int c = Shield::WHITE; c <= Shield::NEUTRAL; c++)
	{
	  Shield::Colour col = Shield::Colour(c);
	  if ((*i)->getImage(col))
	    {
	      delete (*i)->getImage(col);
	      (*i)->setImage(col, NULL);
	    }
	  if ((*i)->getMask(col))
	    {
	      delete (*i)->getMask(col);
	      (*i)->setMask(col, NULL);
	    }
	}
    }
  if (armyset->getShipPic())
    {
      delete armyset->getShipPic();
      armyset->setShipImage(NULL);
    }
  if (armyset->getShipMask())
    {
      delete armyset->getShipMask();
      armyset->setShipMask(NULL);
    }
  if (armyset->getStandardPic())
    {
      delete armyset->getStandardPic();
      armyset->setStandardPic(NULL);
    }
  if (armyset->getStandardMask())
    {
      delete armyset->getStandardMask();
      armyset->setStandardMask(NULL);
    }
}

PixMask* GraphicsLoader::loadImage(std::string filename, bool alpha)
{
  return PixMask::create(filename);
}

PixMask* GraphicsLoader::getMiscPicture(std::string picname, bool alpha)
{
  return loadImage(File::getMiscFile("/various/" + picname), alpha);
}

void GraphicsLoader::instantiateImages(Citysetlist *csl)
{
  uninstantiateImages(csl);
  for (Citysetlist::iterator it = csl->begin(); it != csl->end(); it++)
    instantiateImages(*it);
}

void GraphicsLoader::uninstantiateImages(Citysetlist *csl)
{
  for (Citysetlist::iterator cit = csl->begin(); cit != csl->end(); cit++)
    uninstantiateImages(*cit);
}

void GraphicsLoader::uninstantiateImages(Cityset *cs)
{
  if (cs->getPortImage() != NULL)
    {
      delete cs->getPortImage();
      cs->setPortImage(NULL);
    }
  if (cs->getSignpostImage() != NULL)
    {
      delete cs->getSignpostImage();
      cs->setSignpostImage(NULL);
    }
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
      if (cs->getCityImage(i) != NULL)
	{
	  delete cs->getCityImage(i);
	  cs->setCityImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      if (cs->getRazedCityImage(i) != NULL)
	{
	  delete cs->getRazedCityImage(i);
	  cs->setRazedCityImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      if (cs->getTowerImage(i) != NULL)
	{
	  delete cs->getTowerImage(i);
	  cs->setTowerImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < RUIN_TYPES; i++)
    {
      if (cs->getRuinImage(i) != NULL)
	{
	  delete cs->getRuinImage(i);
	  cs->setRuinImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < TEMPLE_TYPES; i++)
    {
      if (cs->getTempleImage(i) != NULL)
	{
	  delete cs->getTempleImage(i);
	  cs->setTempleImage(i, NULL);
	}
    }
}

void GraphicsLoader::instantiateImages(Cityset *cs)
{
  int size = cs->getTileSize();
  debug("Loading images for cityset " << cs->getName());
  uninstantiateImages(cs);
  cs->setPortImage
    (PixMask::create(File::getCitysetFile(cs, cs->getPortFilename())));
  cs->setSignpostImage
    (PixMask::create(File::getCitysetFile(cs, cs->getSignpostFilename())));

  std::vector<PixMask* > citypics;
  citypics = disassemble_row(File::getCitysetFile(cs, cs->getCitiesFilename()), 
			     MAX_PLAYERS + 1);
  int citysize = size * City::getWidth();
  for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
    {
      if (citypics[i]->get_width() != citysize)
	PixMask::scale(citypics[i], citysize, citysize);
      cs->setCityImage(i, citypics[i]);
    }
  std::vector<PixMask* > razedcitypics;
  razedcitypics = disassemble_row(File::getCitysetFile(cs, cs->getRazedCitiesFilename()), 
			     MAX_PLAYERS);
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      if (razedcitypics[i]->get_width() != citysize)
	PixMask::scale(razedcitypics[i], citysize, citysize);
      cs->setRazedCityImage(i, razedcitypics[i]);
    }

  std::vector<PixMask* > towerpics;
  towerpics = 
    disassemble_row(File::getCitysetFile(cs, cs->getTowersFilename()),
		    MAX_PLAYERS);
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      if (towerpics[i]->get_width() != size)
	PixMask::scale(towerpics[i], size, size);
      cs->setTowerImage(i, towerpics[i]);
    }

  std::vector<PixMask* > ruinpics;
  ruinpics = disassemble_row(File::getCitysetFile(cs, cs->getRuinsFilename()),
			    RUIN_TYPES);
  int ruinsize = size * Ruin::getWidth();
  for (unsigned int i = 0; i < RUIN_TYPES ; i++)
    {
      if (ruinpics[i]->get_width() != ruinsize)
	PixMask::scale(ruinpics[i], ruinsize, ruinsize);
      cs->setRuinImage(i, ruinpics[i]);
    }

  std::vector<PixMask* > templepics;
  templepics = disassemble_row(File::getCitysetFile(cs, 
						    cs->getTemplesFilename()),
			    TEMPLE_TYPES);
  int templesize = size * Temple::getWidth();
  for (unsigned int i = 0; i < TEMPLE_TYPES ; i++)
    {
      if (templepics[i]->get_width() != templesize)
	PixMask::scale(templepics[i], templesize, templesize);
      cs->setTempleImage(i, templepics[i]);
    }
}

// End of file
