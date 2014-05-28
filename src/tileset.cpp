// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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
#include <iostream>

#include "tileset.h"

#include "defs.h"
#include "File.h"
#include "SmallTile.h"
#include "xmlhelper.h"
#include "gui/image-helpers.h"
#include "GraphicsCache.h"
#include "tilesetlist.h"
#include "tarhelper.h"
#include "Configuration.h"
#include "file-compat.h"
#include "ucompose.hpp"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

#define DEFAULT_TILE_SIZE 40

Glib::ustring Tileset::d_tag = "tileset";
Glib::ustring Tileset::d_road_smallmap_tag = "road_smallmap";
Glib::ustring Tileset::d_ruin_smallmap_tag = "ruin_smallmap";
Glib::ustring Tileset::d_temple_smallmap_tag = "temple_smallmap";
Glib::ustring Tileset::file_extension = TILESET_EXT;

Tileset::Tileset(guint32 id, Glib::ustring name)
	: Set(), d_name(name), d_copyright(""), d_license(""), d_id(id), 
	d_tileSize(DEFAULT_TILE_SIZE), d_basename("")
{
  d_info = "";
  d_large_selector = "";
  d_small_selector = "";
  d_fog = "";
  d_roads = "";
  d_bridges = "";
  d_flags = "";
  d_road_color.set_rgba(0,0,0);
  d_ruin_color.set_rgba(100,100,100);
  d_temple_color.set_rgba(100,100,100);
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    roadpic[i] = NULL;
  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    bridgepic[i] = NULL;
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    flagpic[i] = NULL;
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
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

Tileset::Tileset (const Tileset& t)
  : Set(t), d_name(t.d_name), d_copyright(t.d_copyright), 
    d_license(t.d_license), d_id(t.d_id), d_tileSize(t.d_tileSize), 
    d_basename(t.d_basename)
{
  d_info = t.d_info;
  d_large_selector = t.d_large_selector;
  d_small_selector = t.d_small_selector;
  d_fog = t.d_fog;
  d_roads = t.d_roads;
  d_bridges = t.d_bridges;
  d_flags = t.d_flags;
  d_road_color = t.d_road_color;
  d_ruin_color = t.d_ruin_color;
  d_temple_color = t.d_temple_color;
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    {
      if (t.roadpic[i])
        roadpic[i] = t.roadpic[i]->copy();
      else
        roadpic[i] = NULL;
    }
  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    {
      if (t.bridgepic[i])
        bridgepic[i] = t.bridgepic[i]->copy();
      else
        bridgepic[i] = NULL;
    }
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    {
      if (t.flagpic[i])
        flagpic[i] = t.flagpic[i]->copy();
      else
        flagpic[i] = NULL;
    }
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    {
      if (flagmask[i])
        flagmask[i] = t.flagmask[i]->copy();
      else
        flagmask[i] = NULL;
    }
  number_of_selector_frames = t.number_of_selector_frames;
  std::vector<PixMask*> s1 = std::vector<PixMask*>(number_of_selector_frames);
  for (unsigned int i = 0; i < number_of_selector_frames; i++)
    s1[i] = t.selector[i]->copy();
  selector = s1;
  std::vector<PixMask*> s2 = std::vector<PixMask*>(number_of_selector_frames);
  for (unsigned int i = 0; i < number_of_selector_frames; i++)
    s2[i] = t.selectormask[i]->copy();
  selectormask = s2;

  number_of_small_selector_frames = t.number_of_small_selector_frames;
  std::vector<PixMask*> s3 = std::vector<PixMask*>(number_of_small_selector_frames);
  for (unsigned int i = 0; i < number_of_small_selector_frames; i++)
    s3[i] = t.smallselector[i]->copy();
  smallselector = s3;
  std::vector<PixMask*> s4 = std::vector<PixMask*>(number_of_small_selector_frames);
  for (unsigned int i = 0; i < number_of_small_selector_frames; i++)
    s4[i] = t.smallselectormask[i]->copy();
  smallselectormask = s4;

  if (t.explosion != NULL)
    explosion = t.explosion->copy();
  else
    explosion = NULL;

  for (unsigned int i = 0; i < FOG_TYPES; i++)
    {
      if (t.fogpic[i] != NULL)
        fogpic[i] = t.fogpic[i]->copy();
      else
        fogpic[i] = NULL;
    }

  for (Tileset::const_iterator i = t.begin(); i != t.end(); ++i)
    push_back(new Tile(*(*i)));

  for (Tileset::const_iterator i = begin(); i != end(); ++i)
    {
      for (std::list<TileStyleSet*>::const_iterator j = (*i)->begin(); j != (*i)->end(); j++)
	{
	  for (std::vector<TileStyle*>::const_iterator k = (*j)->begin(); k != (*j)->end(); k++)
            {
      
              d_tilestyles[(*k)->getId()] = *k;
            }
        }
    }
}

Tileset::Tileset(XML_Helper *helper, Glib::ustring directory)
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
  helper->registerTag(Tileset::d_ruin_smallmap_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(Tileset::d_temple_smallmap_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(SmallTile::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(TileStyle::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  helper->registerTag(TileStyleSet::d_tag, sigc::mem_fun((*this), &Tileset::loadTile));
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    roadpic[i] = NULL;
  for (unsigned int i = 0; i < BRIDGE_TYPES; i++)
    bridgepic[i] = NULL;
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    flagpic[i] = NULL;
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
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
  clean_tmp_dir();
}

int Tileset::getIndex(Tile::Type type) const
{
  for (guint32 i = 0; i < size(); i++)
    if (type == (*this)[i]->getType())
      return i;

  // catch errors?
  return -1;
}

bool Tileset::loadTile(Glib::ustring tag, XML_Helper* helper)
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

  if (tag == Tileset::d_ruin_smallmap_tag)
    {
      helper->getData(d_ruin_color, "color");
      return true;
    }

  if (tag == Tileset::d_temple_smallmap_tag)
    {
      helper->getData(d_temple_color, "color");
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
  retval &= helper->openTag(d_ruin_smallmap_tag);
  retval &= helper->saveData("color", d_ruin_color);
  retval &= helper->closeTag();
  retval &= helper->openTag(d_temple_smallmap_tag);
  retval &= helper->saveData("color", d_temple_color);
  retval &= helper->closeTag();
  for (Tileset::const_iterator i = begin(); i != end(); ++i)
    retval &= (*i)->save(helper);
  retval &= helper->closeTag();

  return retval;
}

bool Tileset::save(Glib::ustring filename, Glib::ustring extension) const
{
  bool broken = false;
  Glib::ustring goodfilename = File::add_ext_if_necessary(filename, extension);
  Glib::ustring tmpfile = File::get_tmp_file();
  XML_Helper helper(tmpfile, std::ios::out, Configuration::s_zipfiles);
  helper.begin(LORDSAWAR_TILESET_VERSION);
  broken = !save(&helper);
  helper.close();
  if (broken == true)
    return false;
  Glib::ustring tmptar = tmpfile + ".tar";
  Tar_Helper t(tmptar, std::ios::out, broken);
  if (broken == true)
    return false;
  t.saveFile(tmpfile, File::get_basename(goodfilename, true));
  //now the images, go get 'em from the tarball we were made from.
  std::list<Glib::ustring> delfiles;
  Tar_Helper orig(getConfigurationFile(), std::ios::in, broken);
  if (broken == false)
    {
      std::list<Glib::ustring> files = orig.getFilenamesWithExtension(".png");
      for (std::list<Glib::ustring>::iterator it = files.begin(); 
           it != files.end(); it++)
        {
          Glib::ustring pngfile = orig.getFile(*it, broken);
          if (broken == false)
            {
              t.saveFile(pngfile);
              delfiles.push_back(pngfile);
            }
          else
            break;
        }
      orig.Close();
    }
  else
    {
      FILE *fileptr = fopen (getConfigurationFile().c_str(), "r");
      if (fileptr)
        fclose (fileptr);
      else
        broken = false;
    }
  t.Close();
  for (std::list<Glib::ustring>::iterator it = delfiles.begin(); it != delfiles.end(); it++)
    File::erase(*it);
  File::erase(tmpfile);
  if (broken == false)
    {
      if (File::copy(tmptar, goodfilename) == 0)
        File::erase(tmptar);
    }

  return !broken;
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
	      ids[(*k)->getId()]++;
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

void Tileset::setBaseName(Glib::ustring bname)
{
  d_basename = bname;
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
	  fprintf (stderr, "`%s' tile fails validation in %s\n", 
		   (*i)->getName().c_str(), getConfigurationFile().c_str());
	  return false;
	}
    }
  return true;
}

class TilesetLoader
{
public:
    TilesetLoader(Glib::ustring filename, bool &broken, bool &unsupported)
      {
        unsupported_version = false;
	tileset = NULL;
	dir = File::get_dirname(filename);
        file = File::get_basename(filename);
	if (File::nameEndsWith(filename, Tileset::file_extension) == false)
	  filename += Tileset::file_extension;
        Tar_Helper t(filename, std::ios::in, broken);
        if (broken)
          return;
        Glib::ustring lwtfilename = 
          t.getFirstFile(Tileset::file_extension, broken);
        if (broken)
          return;
	XML_Helper helper(lwtfilename, std::ios::in, false);
	helper.registerTag(Tileset::d_tag, sigc::mem_fun((*this), &TilesetLoader::load));
	if (!helper.parse())
	  {
            unsupported = unsupported_version;
            std::cerr << String::ucompose(_("Error!  can't load tileset `%1'."), filename) << std::endl;
	    if (tileset != NULL)
	      delete tileset;
	    tileset = NULL;
	  }
        File::erase(lwtfilename);
        helper.close();
        t.Close();
      };
    bool load(Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == Tileset::d_tag)
	  {
            if (helper->getVersion() == LORDSAWAR_TILESET_VERSION)
              {
                tileset = new Tileset(helper, dir);
                tileset->setBaseName(file);
                return true;
              }
            else
              {
                unsupported_version = true;
                return false;
              }
	  }
	return false;
      };
    Glib::ustring dir;
    Glib::ustring file;
    Tileset *tileset;
    bool unsupported_version;
};


Tileset *Tileset::create(Glib::ustring file, bool &unsupported_version)
{
  bool broken = false;
  TilesetLoader d(file, broken, unsupported_version);
  if (broken)
    return NULL;
  return d.tileset;
}

void Tileset::getFilenames(std::list<Glib::ustring> &files)
{
  for (iterator it = begin(); it != end(); it++)
    {
      Tile *t = *it;
      for (Tile::iterator sit = t->begin(); sit != t->end(); sit++)
	{
	  Glib::ustring file = (*sit)->getName();
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
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    {
      if (getFlagImage(i) != NULL)
	{
	  delete getFlagImage(i);
	  setFlagImage(i, NULL);
	}
    }
  for (unsigned int i = 0; i < FLAG_TYPES; i++)
    {
      if (getFlagMask(i) != NULL)
	{
	  delete getFlagMask(i);
	  setFlagMask(i, NULL);
	}
    }
}

void Tileset::instantiateImages(Glib::ustring explosion_filename,
				Glib::ustring roads_filename,
				Glib::ustring bridges_filename,
				Glib::ustring fog_filename,
				Glib::ustring flags_filename,
				Glib::ustring selector_filename,
				Glib::ustring small_selector_filename,
                                bool &broken)
{
  if (explosion_filename.empty() == false && !broken)
    setExplosionImage (PixMask::create(explosion_filename, broken));

  if (roads_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > roadpics;
      roadpics = disassemble_row(roads_filename, ROAD_TYPES, broken);
      if (!broken)
        {
          for (unsigned int i = 0; i < ROAD_TYPES ; i++)
            {
              if (roadpics[i]->get_width() != (int)d_tileSize)
                PixMask::scale(roadpics[i], d_tileSize, d_tileSize);
              setRoadImage(i, roadpics[i]);
            }
        }

    }

  if (bridges_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > bridgepics;
      bridgepics = disassemble_row(bridges_filename, BRIDGE_TYPES, broken);
      if (!broken)
        {
          for (unsigned int i = 0; i < BRIDGE_TYPES ; i++)
            {
              if (bridgepics[i]->get_width() != (int)d_tileSize)
                PixMask::scale(bridgepics[i], d_tileSize, d_tileSize);
              setBridgeImage(i, bridgepics[i]);
            }
        }
    }

  if (fog_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > fogpics;
      fogpics = disassemble_row(fog_filename, FOG_TYPES, broken);
      if (!broken)
        {
          for (unsigned int i = 0; i < FOG_TYPES ; i++)
            {
              if (fogpics[i]->get_width() != (int)d_tileSize)
                PixMask::scale(fogpics[i], d_tileSize, d_tileSize);
              setFogImage(i, fogpics[i]);
            }
        }
    }

  if (flags_filename.empty() == false && !broken)
    {
      std::vector<PixMask* > flagpics;
      std::vector<PixMask* > maskpics;
      bool success;
      success = GraphicsCache::loadFlagImages (flags_filename, d_tileSize, 
                                               flagpics, maskpics);
      if (success)
        {
          for (unsigned int i = 0; i < flagpics.size(); i++)
            setFlagImage(i, flagpics[i]);
          for (unsigned int i = 0; i < maskpics.size(); i++)
            setFlagMask(i, maskpics[i]);
        }
      else
        broken = true;
    }

  std::vector<PixMask* > images;
  std::vector<PixMask* > masks;
  if (selector_filename.empty() == false && !broken)
    {
      bool success;
      success = GraphicsCache::loadSelectorImages (selector_filename, 
                                                   d_tileSize, 
                                                   images, masks);
      if (success)
        {
          setNumberOfSelectorFrames(images.size());
          for (unsigned int i = 0; i < images.size(); i++)
            {
              setSelectorImage(i, images[i]);
              setSelectorMask(i, masks[i]);
            }
        }
      else
        broken = true;
    }

  images.clear();
  masks.clear();
  if (small_selector_filename.empty() == false && !broken)
    {
      bool success;
      success = GraphicsCache::loadSelectorImages (small_selector_filename, 
                                                   d_tileSize, images, masks);
      if (success)
        {
          setNumberOfSmallSelectorFrames(images.size());
          for (unsigned int i = 0; i < images.size(); i++)
            {
              setSmallSelectorImage(i, images[i]);
              setSmallSelectorMask(i, masks[i]);
            }
        }
      else
        broken = true;
    }
}

void Tileset::instantiateImages(bool &broken)
{
  int size = getTileSize();
  debug("Loading images for tileset " << getName());
  uninstantiateImages();
  broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return;
  for (iterator it = begin(); it != end(); it++)
    {
      if (!broken)
        (*it)->instantiateImages(size, &t, broken);
    }
  Glib::ustring explosion_filename = "";
  Glib::ustring roads_filename = "";
  Glib::ustring bridges_filename = "";
  Glib::ustring fog_filename = "";
  Glib::ustring flags_filename = "";
  Glib::ustring selector_filename = "";
  Glib::ustring small_selector_filename = "";

  if (getExplosionFilename().empty() == false && !broken)
    explosion_filename = t.getFile(getExplosionFilename() + ".png", broken);
  if (getRoadsFilename().empty() == false && !broken)
    roads_filename = t.getFile(getRoadsFilename() + ".png", broken);
  if (getBridgesFilename().empty() == false && !broken)
    bridges_filename = t.getFile(getBridgesFilename() + ".png", broken);
  if (getFogFilename().empty() == false && !broken)
    fog_filename = t.getFile(getFogFilename() + ".png", broken);
  if (getFlagsFilename().empty() == false && !broken)
    flags_filename = t.getFile(getFlagsFilename() + ".png", broken);
  if (getLargeSelectorFilename().empty() == false && !broken)
    selector_filename = t.getFile(getLargeSelectorFilename() + ".png", broken);
  if (getSmallSelectorFilename().empty() == false && !broken)
    small_selector_filename = 
      t.getFile(getSmallSelectorFilename() + ".png", broken);
  if (!broken)
    instantiateImages(explosion_filename, roads_filename, bridges_filename, 
                      fog_filename, flags_filename, selector_filename, 
                      small_selector_filename, broken);
  if (explosion_filename.empty() == false)
    File::erase(explosion_filename);
  if (roads_filename.empty() == false)
    File::erase(roads_filename);
  if (bridges_filename.empty() == false)
    File::erase(bridges_filename);
  if (fog_filename.empty() == false)
    File::erase(fog_filename);
  if (flags_filename.empty() == false)
    File::erase(flags_filename);
  if (selector_filename.empty() == false)
    File::erase(selector_filename);
  if (small_selector_filename.empty() == false)
    File::erase(small_selector_filename);
  t.Close();
  return;
}

Glib::ustring Tileset::getConfigurationFile() const
{
  return getDirectory() + d_basename + file_extension;
}

std::list<Glib::ustring> Tileset::scanUserCollection()
{
  return File::scanForFiles(File::getUserTilesetDir(), file_extension);
}

std::list<Glib::ustring> Tileset::scanSystemCollection()
{
  std::list<Glib::ustring> retlist = File::scanForFiles(File::getTilesetDir(), 
                                                      file_extension);
  if (retlist.empty())
    {
      //note to translators: %1 is a file extension, %2 is a directory.
      std::cerr << String::ucompose(_("Couldn't find any tilesets (*%1) in `%2'."),file_extension, File::getTilesetDir()) << std::endl;
      std::cerr << _("Please check the path settings in ~/.lordsawarrc") << std::endl;
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

void Tileset::reload(bool &broken)
{
  broken = false;
  bool unsupported_version = false;
  TilesetLoader d(getConfigurationFile(), broken, unsupported_version);
  if (!broken && d.tileset && d.tileset->validate())
    {
      //steal the values from d.tileset and then don't delete it.
      uninstantiateImages();
      for (iterator it = begin(); it != end(); it++)
        delete *it;
      Glib::ustring basename = d_basename;
      *this = *d.tileset;
      instantiateImages(broken);
      d_basename = basename;
    }
}

Glib::ustring Tileset::getFileFromConfigurationFile(Glib::ustring file)
{
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken == false)
    {
      Glib::ustring filename = t.getFile(file, broken);
      t.Close();
  
      if (broken == false)
        return filename;
    }
  return "";
}

bool Tileset::addFileInConfigurationFile(Glib::ustring new_file)
{
  return replaceFileInConfigurationFile("", new_file);
}
bool Tileset::replaceFileInConfigurationFile(Glib::ustring file, Glib::ustring new_file)
{
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken == false)
    {
      broken = t.replaceFile(file, new_file);
      t.Close();
    }
  return broken;
}

guint32 Tileset::calculate_preferred_tile_size() const
{
  guint32 tilesize = 0;
  std::map<guint32, guint32> sizecounts;

  if (roadpic[0])
    sizecounts[roadpic[0]->get_unscaled_width()]++;
  if (bridgepic[0])
    sizecounts[bridgepic[0]->get_unscaled_width()]++;
  if (flagpic[0])
    sizecounts[flagpic[0]->get_unscaled_width()]++;
  if (selector[0])
    sizecounts[selector[0]->get_unscaled_width()]++;
  if (smallselector[0])
    sizecounts[smallselector[0]->get_unscaled_width()]++;
  if (fogpic[0])
    sizecounts[fogpic[0]->get_unscaled_width()]++;
  if (explosion)
    sizecounts[explosion->get_unscaled_width()]++;
  for (const_iterator it = begin(); it != end(); it++)
    {
      Tile *tile = *it;
      for (Tile::const_iterator i = tile->begin(); i != tile->end(); i++)
        {
          TileStyle *tilestyle = (*i)->front();
          if (tilestyle && tilestyle->getImage())
            sizecounts[tilestyle->getImage()->get_unscaled_width()]++;
        }
    }

  guint32 maxcount = 0;
  for (std::map<guint32, guint32>::iterator it = sizecounts.begin(); 
       it != sizecounts.end(); it++)
    {
      if ((*it).second > maxcount)
        {
          maxcount = (*it).second;
          tilesize = (*it).first;
        }
    }
  if (tilesize == 0)
    tilesize = DEFAULT_TILE_SIZE;
  return tilesize;
}

bool Tileset::copy(Glib::ustring src, Glib::ustring dest)
{
  return Tar_Helper::copy(src, dest);
}
        
bool Tileset::addTileStyleSet(Tile *tile, Glib::ustring filename)
{
  bool success = true;
  TileStyle::Type tilestyle_type;
  if (tile->getType() == Tile::GRASS)
    tilestyle_type = TileStyle::LONE;
  else
    tilestyle_type = TileStyle::UNKNOWN;
  TileStyleSet *set = 
    new TileStyleSet(filename, d_tileSize, success, tilestyle_type);
  if (!success)
    {
      delete set;
      return success;
    }
  guint32 first_free_id = getFreeTileStyleId();
  for (TileStyleSet::iterator it = set->begin(); it != set->end(); it++)
    (*it)->setId(first_free_id);

  tile->push_back(set);
  for (TileStyleSet::iterator it = ++set->begin(); it != set->end(); it++)
    (*it)->setId(getFreeTileStyleId());
  return success;
}

bool Tileset::getTileStyle(guint32 id, Tile **tile, TileStyleSet **set, TileStyle ** style) const
{
  for (const_iterator t = begin(); t != end(); t++)
    for (std::list<TileStyleSet*>::const_iterator i = (*t)->begin(); 
         i != (*t)->end(); i++)
      for (std::vector<TileStyle*>::const_iterator j = (*i)->begin(); 
           j != (*i)->end(); j++)
        if ((*j)->getId() == id)
          {
            if (tile)
              *tile = *t;
            if (set)
              *set = *i;
            if (style)
              *style = *j;
            return true;
          }
  return false;
}

void Tileset::clean_tmp_dir() const
{
  return Tar_Helper::clean_tmp_dir(getConfigurationFile());
}

bool Tileset::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version)
{
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::TILESET, d_tag);
}

void Tileset::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type(FileCompat::TILESET, file_extension, 
                                          d_tag, true);
  FileCompat::getInstance()->support_version
    (FileCompat::TILESET, "0.2.0", LORDSAWAR_TILESET_VERSION,
     sigc::ptr_fun(&Tileset::upgrade));
}

Tile *Tileset::getFirstTile(SmallTile::Pattern pattern) const
{
  for (const_iterator i = begin(); i != end(); i++)
    if ((*i)->getSmallTile()->getPattern() == pattern)
      return *i;
    return NULL;
}

Tileset* Tileset::copy(const Tileset *tileset)
{
  if (!tileset)
    return NULL;
  return new Tileset(*tileset);
}
//End of file
