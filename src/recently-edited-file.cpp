//  Copyright (C) 2010 Ben Asselstine
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

//#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "recently-edited-file.h"
#include "xmlhelper.h"
#include "shieldset.h"
#include "tileset.h"
#include "armyset.h"
#include "cityset.h"
#include "GameScenario.h"

std::string RecentlyEditedFile::d_tag = "recentlyeditedfile";

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

RecentlyEditedFile::RecentlyEditedFile(std::string filename)
{
  d_last_edit.assign_current_time();
  d_filename = filename;
}

RecentlyEditedFile::RecentlyEditedFile(XML_Helper* helper)
{
  std::string s;
  helper->getData(s, "last_edited_on");
  d_last_edit.assign_from_iso8601(s);
  helper->getData(d_filename, "filename");
}

RecentlyEditedFile::~RecentlyEditedFile()
{
}

bool RecentlyEditedFile::saveContents(XML_Helper *helper) const
{
  bool retval = true;
  std::string s = d_last_edit.as_iso8601();
  retval &= helper->saveData("last_edited_on", s);
  retval &= helper->saveData("filename", d_filename);
  retval &= doSave(helper);
  return retval;
}

RecentlyEditedFile* RecentlyEditedFile::handle_load(XML_Helper *helper)
{
  std::string filename;
  helper->getData(filename, "filename");
  if (File::nameEndsWith(filename, Shieldset::file_extension) == true)
    return new RecentlyEditedShieldsetFile(helper);
  else if (File::nameEndsWith(filename, Tileset::file_extension) == true)
    return new RecentlyEditedTilesetFile(helper);
  else if (File::nameEndsWith(filename, Armyset::file_extension) == true)
    return new RecentlyEditedArmysetFile(helper);
  else if (File::nameEndsWith(filename, Cityset::file_extension) == true)
    return new RecentlyEditedCitysetFile(helper);
  else if (File::nameEndsWith(filename, MAP_EXT) == true)
    return new RecentlyEditedMapFile(helper);
  return NULL;
}

bool RecentlyEditedFile::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag(RecentlyEditedFile::d_tag);
  retval &= saveContents(helper);
  retval &= helper->closeTag();
  return retval;
}

//-----------------------------------------------------------------------------
//RecentlyEditedShieldsetFile

RecentlyEditedShieldsetFile::RecentlyEditedShieldsetFile(std::string filename)
	:RecentlyEditedFile(filename)
{
}

RecentlyEditedShieldsetFile::RecentlyEditedShieldsetFile(XML_Helper *helper)
	:RecentlyEditedFile(helper)
{
  helper->getData(d_name, "name");
  helper->getData(d_images_needed, "images_needed");
}

RecentlyEditedShieldsetFile::~RecentlyEditedShieldsetFile()
{
}

bool RecentlyEditedShieldsetFile::doSave(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("images_needed", d_images_needed);
  return retval;
}

bool RecentlyEditedShieldsetFile::fillData(Shieldset *shieldset)
{
  d_name = shieldset->getName();
  d_images_needed = shieldset->countEmptyImageNames();
  return true;
}

//-----------------------------------------------------------------------------
//RecentlyEditedTilesetFile

RecentlyEditedTilesetFile::RecentlyEditedTilesetFile(std::string filename)
	:RecentlyEditedFile(filename)
{
}

RecentlyEditedTilesetFile::RecentlyEditedTilesetFile(XML_Helper *helper)
	:RecentlyEditedFile(helper)
{
  helper->getData(d_name, "name");
  helper->getData(d_num_tiles, "num_tiles");
}

RecentlyEditedTilesetFile::~RecentlyEditedTilesetFile()
{
}

bool RecentlyEditedTilesetFile::doSave(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("num_tiles", d_num_tiles);
  return retval;
}

bool RecentlyEditedTilesetFile::fillData(Tileset *tileset)
{
  d_name = tileset->getName();
  d_num_tiles = tileset->size();
  return true;
}

//-----------------------------------------------------------------------------
//RecentlyEditedArmysetFile

RecentlyEditedArmysetFile::RecentlyEditedArmysetFile(std::string filename)
	:RecentlyEditedFile(filename)
{
}

RecentlyEditedArmysetFile::RecentlyEditedArmysetFile(XML_Helper *helper)
	:RecentlyEditedFile(helper)
{
  helper->getData(d_name, "name");
  helper->getData(d_num_armies, "num_armies");
}

RecentlyEditedArmysetFile::~RecentlyEditedArmysetFile()
{
}

bool RecentlyEditedArmysetFile::doSave(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("num_armies", d_num_armies);
  return retval;
}

bool RecentlyEditedArmysetFile::fillData(Armyset *armyset)
{
  d_name = armyset->getName();
  d_num_armies = armyset->size();
  return true;
}

//-----------------------------------------------------------------------------
//RecentlyEditedCitysetFile

RecentlyEditedCitysetFile::RecentlyEditedCitysetFile(std::string filename)
	:RecentlyEditedFile(filename)
{
}

RecentlyEditedCitysetFile::RecentlyEditedCitysetFile(XML_Helper *helper)
	:RecentlyEditedFile(helper)
{
  helper->getData(d_name, "name");
  helper->getData(d_images_needed, "images_needed");
}

RecentlyEditedCitysetFile::~RecentlyEditedCitysetFile()
{
}

bool RecentlyEditedCitysetFile::doSave(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("images_needed", d_images_needed);
  return retval;
}

bool RecentlyEditedCitysetFile::fillData(Cityset *cityset)
{
  d_name = cityset->getName();
  d_images_needed = cityset->countEmptyImageNames();
  return true;
}

//-----------------------------------------------------------------------------
//RecentlyEditedMapFile

RecentlyEditedMapFile::RecentlyEditedMapFile(std::string filename)
	:RecentlyEditedFile(filename)
{
}

RecentlyEditedMapFile::RecentlyEditedMapFile(XML_Helper *helper)
	:RecentlyEditedFile(helper)
{
  helper->getData(d_name, "name");
  helper->getData(d_num_players, "num_players");
  helper->getData(d_num_cities, "num_cities");
}

RecentlyEditedMapFile::~RecentlyEditedMapFile()
{
}

bool RecentlyEditedMapFile::doSave(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->saveData("name", d_name);
  retval &= helper->saveData("num_players", d_num_players);
  retval &= helper->saveData("num_cities", d_num_cities);
  return retval;
}

bool RecentlyEditedMapFile::fillData(std::string name, guint32 players, guint32 cities)
{
  d_name = name;
  d_num_players = players;
  d_num_cities = cities;
  return true;
}

