// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>
#include <string.h>

#include "tileset.h"

#include "defs.h"
#include "File.h"
#include "SmallTile.h"
#include "xmlhelper.h"
#include "gui/image-helpers.h"
#include "GraphicsCache.h"
#include "tilesetlist.h"

using namespace std;

#include <iostream>
//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

#define DEFAULT_TILE_SIZE 40

std::string Tileset::d_tag = "tileset";
std::string Tileset::d_road_smallmap_tag = "road_smallmap";
std::string Tileset::file_extension = TILESET_EXT;

Tileset::Tileset(guint32 id, std::string name)
	: Set(), d_name(name), d_copyright(""), d_license(""), d_id(id), 
	d_tileSize(DEFAULT_TILE_SIZE), d_subdir("")
{
  d_info = "";
  d_large_selector = "";
  d_small_selector = "";
  d_fog = "";
  d_roads = "";
  d_bridges = "";
  d_flags = "";
  d_road_color.set_rgb_p(0,0,0);
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    roadpic[i] = NULL;
  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    bridgepic[i] = NULL;
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    flagpic[i] = NULL;
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    flagmask[i] = NULL;
  number_of_selector_frames = 0;
  selector.clear();
  selectormask.clear();
  number_of_small_selector_frames = 0;
  smallselector.clear();
  smallselectormask.clear();
  explosion = NULL;
  for (unsigned int i = 0; i < FOG_TYPES; i++)
    fogpic[i] = NULL;
}

Tileset::Tileset(XML_Helper *helper, std::string directory)
	:Set()
{
  setDirectory(directory);
  helper->getData(d_id, "id"); 
  helper->getData(d_name, "name"); 
  helper->getData(d_copyright, "copyright"); 
  helper->getData(d_license, "license"); 
  helper->getData(d_info, "info");
  helper->getData(d_tileSize, "tilesize");
  helper->getData(d_large_selector, "large_selector");
  helper->getData(d_small_selector, "small_selector");
  helper->getData(d_explosion, "explosion");
  helper->getData(d_roads, "roads");
  helper->getData(d_bridges, "bridges");
  helper->getData(d_fog, "fog");
  helper->getData(d_flags, "flags");
  helper->registerTag(Tile::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(Tileset::d_road_smallmap_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(SmallTile::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(TileStyle::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(TileStyleSet::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    roadpic[i] = NULL;
  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    bridgepic[i] = NULL;
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    flagpic[i] = NULL;
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    flagmask[i] = NULL;
  number_of_selector_frames = 0;
  selector.clear();
  selectormask.clear();
  number_of_small_selector_frames = 0;
  smallselector.clear();
  smallselectormask.clear();
  explosion = NULL;
  for (unsigned int i = 0; i < FOG_TYPES; i++)
    fogpic[i] = NULL;
}

Tileset::~Tileset()
{
  uninstantiateImages();
  for (unsigned int i=0; i < size(); i++)
    delete (*this)[i];
}

int Tileset::getIndex(Tile::Type type) const
{
  for (guint32 i = 0; i < size(); i++)
    if (type == (*this)[i]->getType())
      return i;

  // catch errors?
  return -1;
}

bool Tileset::loadTile(string tag, XML_Helper* helper)
{
  debug("loadTile()")

    if (tag == Tile::d_tag)
      {
	// create a new tile with the information we got
	Tile* tile = new Tile(helper);
	this->push_back(tile);

	return true;
      }

  if (tag == Tileset::d_road_smallmap_tag)
    {
      helper->getData(d_road_color, "color");
      return true;
    }

  if (tag == SmallTile::d_tag)
    {
      Tile *tile = this->back();
      SmallTile* smalltile = new SmallTile(helper);
      tile->setSmallTile(smalltile);
      return true;
    }

  if (tag == TileStyle::d_tag)
    {
      Tile *tile = this->back();
      TileStyleSet *tilestyleset = tile->back();
      // create a new tile style with the information we got
      // put it on the latest tilestyleset
      TileStyle* tilestyle = new TileStyle(helper);
      tilestyleset->push_back(tilestyle);
      d_tilestyles[tilestyle->getId()] = tilestyle;

      return true;
    }

  if (tag == TileStyleSet::d_tag)
    {
      Tile *tile = this->back();
      // create a new tile style set with the information we got
      // put it on the latest tile
      TileStyleSet* tilestyleset = new TileStyleSet(helper);
      tilestyleset->setSubDir(getSubDir());
      tile->push_back(tilestyleset);
      return true;
    }

  return false;
}

TileStyle *Tileset::getRandomTileStyle(guint32 index, TileStyle::Type style) const
{
  Tile *tile = (*this)[index];
  if (tile)
    return tile->getRandomTileStyle (style);
  else
    return NULL;
}

bool Tileset::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("id", d_id);
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("copyright", d_copyright);
  retval &= helper->saveData("license", d_license);
  retval &= helper->saveData("info", d_info);
  retval &= helper->saveData("tilesize", d_tileSize);
  retval &= helper->saveData("large_selector", d_large_selector);
  retval &= helper->saveData("small_selector", d_small_selector);
  retval &= helper->saveData("explosion", d_explosion);
  retval &= helper->saveData("roads", d_roads);
  retval &= helper->saveData("bridges", d_bridges);
  retval &= helper->saveData("fog", d_fog);
  retval &= helper->saveData("flags", d_flags);
  retval &= helper->openTag(d_road_smallmap_tag);
  retval &= helper->saveData("color", d_road_color);
  retval &= helper->closeTag();
  for (Tileset::const_iterator i = begin(); i != end(); ++i)
    retval &= (*i)->save(helper);
  retval &= helper->closeTag();

  return retval;
}

int Tileset::getFreeTileStyleId() const
{
  int ids[65535];
  memset (ids, 0, sizeof (ids));
  for (Tileset::const_iterator i = begin(); i != end(); ++i)
    {
      for (std::list<TileStyleSet*>::const_iterator j = (*i)->begin(); j != (*i)->end(); j++)
	{
	  for (std::vector<TileStyle*>::const_iterator k = (*j)->begin(); k != (*j)->end(); k++)
	    {
	      ids[(*k)->getId()] = 1;
	    }
	}
    }
  //these ids range from 0 to 65535.
  for (unsigned int i = 0; i <= 65535; i++)
    {
      if (ids[i] == 0)
	return i;
    }
  return -1;
}

int Tileset::getLargestTileStyleId() const
{
  unsigned int largest = 0;
  for (Tileset::const_iterator i = begin(); i != end(); ++i)
    {
      for (std::list<TileStyleSet*>::const_iterator j = (*i)->begin(); j != (*i)->end(); j++)
	{
	  for (std::vector<TileStyle*>::const_iterator k = (*j)->begin(); k != (*j)->end(); k++)
	    {
	      if ((*k)->getId() > largest)
		largest = (*k)->getId();
	    }
	}
    }
  return largest;
}

void Tileset::setSubDir(std::string dir)
{
  d_subdir = dir;
  for (Tileset::iterator i = begin(); i != end(); ++i)
    for (Tile::iterator j = (*i)->begin(); j != (*i)->end(); j++)
      (*j)->setSubDir(dir);
}

guint32 Tileset::getDefaultTileSize()
{
  return DEFAULT_TILE_SIZE;
}

bool Tileset::validate() const
{
  if (size() == 0)
    return false;
  for (Tileset::const_iterator i = begin(); i != end(); i++)
    {
      if ((*i)->validate() == false)
	{
	  fprintf (stderr, "`%s' fails validation\n", 
		   (*i)->getName().c_str());
	  return false;
	}
    }
  return true;
}

class TilesetLoader
{
public:
    TilesetLoader(std::string filename) 
      {
	tileset = NULL;
	dir = File::get_dirname(filename);
	if (File::nameEndsWith(filename, Tileset::file_extension) == false)
	  filename += Tileset::file_extension;
	XML_Helper helper(filename, ios::in, false);
	helper.registerTag(Tileset::d_tag, sigc::mem_fun((*this), &TilesetLoader::load));
	if (!helper.parse())
	  {
	    std::cerr << "Error, while loading an tileset. Tileset Name: ";
	    std::cerr <<dir<<std::endl <<std::flush;
	    if (tileset != NULL)
	      delete tileset;
	    tileset = NULL;
	  }
      };
    bool load(std::string tag, XML_Helper* helper)
      {
	if (tag == Tileset::d_tag)
	  {
	    tileset = new Tileset(helper, dir);
	    return true;
	  }
	return false;
      };
    std::string dir;
    Tileset *tileset;
};
Tileset *Tileset::create(std::string file)
{
  TilesetLoader d(file);
  return d.tileset;
}
void Tileset::getFilenames(std::list<std::string> &files)
{
  for (iterator it = begin(); it != end(); it++)
    {
      Tile *t = *it;
      for (Tile::iterator sit = t->begin(); sit != t->end(); sit++)
	{
	  std::string file = (*sit)->getName();
	  if (std::find(files.begin(), files.end(), file) == files.end())
	    files.push_back(file);
	}
    }
  files.push_back(d_small_selector);
  files.push_back(d_large_selector);
  files.push_back(d_explosion);
  files.push_back(d_fog);
  files.push_back(d_roads);
  files.push_back(d_bridges);
  files.push_back(d_flags);
}

void Tileset::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();

  if (getExplosionImage() != NULL)
    {
      delete getExplosionImage();
      setExplosionImage(NULL);
    }
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    {
      if (getRoadImage(i) != NULL)
	{
	  delete getRoadImage(i);
	  setRoadImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    {
      if (getBridgeImage(i) != NULL)
	{
	  delete getBridgeImage(i);
	  setBridgeImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < FOG_TYPES; i++)
    {
      if (getFogImage(i) != NULL)
	{
	  delete getFogImage(i);
	  setFogImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < getNumberOfSelectorFrames(); i++)
    {
      if (getSelectorImage(i) != NULL)
	{
	  delete getSelectorImage(i);
	  setSelectorImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < getNumberOfSmallSelectorFrames(); i++)
    {
      if (getSmallSelectorImage(i) != NULL)
	{
	  delete getSmallSelectorImage(i);
	  setSmallSelectorImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < getNumberOfSmallSelectorFrames(); i++)
    {
      if (getSmallSelectorMask(i) != NULL)
	{
	  delete getSmallSelectorMask(i);
	  setSmallSelectorMask(i, NULL);
	}
    }
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    {
      if (getFlagImage(i) != NULL)
	{
	  delete getFlagImage(i);
	  setFlagImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    {
      if (getFlagMask(i) != NULL)
	{
	  delete getFlagMask(i);
	  setFlagMask(i, NULL);
	}
    }
}

void Tileset::instantiateImages(std::string explosion_filename,
				std::string roads_filename,
				std::string bridges_filename,
				std::string fog_filename,
				std::string flags_filename,
				std::string selector_filename,
				std::string small_selector_filename)
{
  if (explosion_filename.empty() == false)
    setExplosionImage (PixMask::create(explosion_filename));

  if (roads_filename.empty() == false)
    {
      std::vector<PixMask* > roadpics;
      roadpics = disassemble_row(roads_filename, ROAD_TYPES);
      for (unsigned int i = 0; i < ROAD_TYPES ; i++)
	{
	  if (roadpics[i]->get_width() != (int)d_tileSize)
	    PixMask::scale(roadpics[i], d_tileSize, d_tileSize);
	  setRoadImage(i, roadpics[i]);
	}
    }

  if (bridges_filename.empty() == false)
    {
      std::vector<PixMask* > bridgepics;
      bridgepics = disassemble_row(bridges_filename, BRIDGE_TYPES);
      for (unsigned int i = 0; i < BRIDGE_TYPES ; i++)
	{
	  if (bridgepics[i]->get_width() != (int)d_tileSize)
	    PixMask::scale(bridgepics[i], d_tileSize, d_tileSize);
	  setBridgeImage(i, bridgepics[i]);
	}
    }

  if (fog_filename.empty() == false)
    {
      std::vector<PixMask* > fogpics;
      fogpics = disassemble_row(fog_filename, FOG_TYPES);
      for (unsigned int i = 0; i < FOG_TYPES ; i++)
	{
	  if (fogpics[i]->get_width() != (int)d_tileSize)
	    PixMask::scale(fogpics[i], d_tileSize, d_tileSize);
	  setFogImage(i, fogpics[i]);
	}
    }

  if (flags_filename.empty() == false)
    {
      std::vector<PixMask* > flagpics;
      std::vector<PixMask* > maskpics;
      GraphicsCache::loadFlagImages (flags_filename, d_tileSize, 
				     flagpics, maskpics);
      for (unsigned int i = 0; i < flagpics.size(); i++)
	setFlagImage(i, flagpics[i]);
      for (unsigned int i = 0; i < maskpics.size(); i++)
	setFlagMask(i, maskpics[i]);
    }

      
  std::vector<PixMask* > images;
  std::vector<PixMask* > masks;
  if (selector_filename.empty() == false)
    {
      GraphicsCache::loadSelectorImages (selector_filename, d_tileSize, 
					 images, masks);
      setNumberOfSelectorFrames(images.size());
      for (unsigned int i = 0; i < images.size(); i++)
	{
	  setSelectorImage(i, images[i]);
	  setSelectorMask(i, masks[i]);
	}
    }

  images.clear();
  masks.clear();
  if (small_selector_filename.empty() == false)
    {
      GraphicsCache::loadSelectorImages (small_selector_filename, d_tileSize,
					 images, masks);
      setNumberOfSmallSelectorFrames(images.size());
      for (unsigned int i = 0; i < images.size(); i++)
	{
	  setSmallSelectorImage(i, images[i]);
	  setSmallSelectorMask(i, masks[i]);
	}
    }
}

void Tileset::instantiateImages()
{
  int size = getTileSize();
  debug("Loading images for tileset " << getName());
  uninstantiateImages();
  for (iterator it = begin(); it != end(); it++)
    (*it)->instantiateImages(size, this);
  std::string explosion_filename = "";
  std::string roads_filename = "";
  std::string bridges_filename = "";
  std::string fog_filename = "";
  std::string flags_filename = "";
  std::string selector_filename = "";
  std::string small_selector_filename = "";

  if (getExplosionFilename().empty() == false)
    explosion_filename = getFile(getExplosionFilename());
  if (getRoadsFilename().empty() == false)
    roads_filename = getFile(getRoadsFilename());
  if (getBridgesFilename().empty() == false)
    bridges_filename = getFile(getBridgesFilename());
  if (getFogFilename().empty() == false)
    fog_filename = getFile(getFogFilename());
  if (getFlagsFilename().empty() == false)
    flags_filename = getFile(getFlagsFilename());
  if (getLargeSelectorFilename().empty() == false)
    selector_filename = getFile(getLargeSelectorFilename());
  if (getSmallSelectorFilename().empty() == false)
    small_selector_filename = getFile(getSmallSelectorFilename());
  instantiateImages(explosion_filename, roads_filename, bridges_filename, 
		    fog_filename, flags_filename, selector_filename, 
		    small_selector_filename);
}

std::string Tileset::getConfigurationFile() const
{
  return getDirectory() + d_subdir + file_extension;
}

std::list<std::string> Tileset::scanUserCollection()
{
  return File::scanFiles(File::getUserTilesetDir(), file_extension);
}

std::list<std::string> Tileset::scanSystemCollection()
{
  std::list<std::string> retlist = File::scanFiles(File::getTilesetDir(), 
						   file_extension);
  if (retlist.empty())
    {
      std::cerr << "Couldn't find any tilesets!" << std::endl;
      std::cerr << "Please check the path settings in /etc/lordsawarrc or ~/.lordsawarrc" << std::endl;
      std::cerr << "Exiting!" << std::endl;
      exit(-1);
    }

  return retlist;
}

TileStyle *Tileset::getTileStyle(guint32 id) const
{
  TileStyleIdMap::const_iterator it = d_tilestyles.find(id);
  if (it == d_tilestyles.end())
    return NULL;
  else
    return (*it).second;
}
//End of file
