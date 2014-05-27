// Copyright (C) 2010, 2011, 2014 Ben Asselstine
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

#include "recently-edited-file-list.h"
#include "recently-edited-file.h"
#include <limits.h>
#include <fstream>
#include <iostream>
#include "xmlhelper.h"
#include "Configuration.h"
#include "defs.h"
#include "GameScenario.h"
#include "shieldset.h"
#include "tileset.h"
#include "armyset.h"
#include "cityset.h"
#include "file-compat.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

std::string RecentlyEditedFileList::d_tag = "recentlyeditedfilelist";

RecentlyEditedFileList* RecentlyEditedFileList::s_instance = 0;

RecentlyEditedFileList* RecentlyEditedFileList::getInstance()
{
  if (s_instance == 0)
    s_instance = new RecentlyEditedFileList();

  return s_instance;
}

bool RecentlyEditedFileList::save()
{
  pruneFiles();
  return saveToFile();
}

bool RecentlyEditedFileList::saveToFile(std::string filename) const
{
  if (filename == "")
    filename = File::getSavePath() + "/" + RECENTLY_EDITED_LIST;
  bool retval = true;
  XML_Helper helper(filename, std::ios::out, false);
  retval &= save(&helper);
  helper.close();
  return retval;
}

bool RecentlyEditedFileList::loadFromFile(std::string filename)
{
  if (filename == "")
    filename = File::getSavePath() + "/" + RECENTLY_EDITED_LIST;
  std::ifstream in(filename.c_str());
  if (in)
    {
      XML_Helper helper(filename.c_str(), std::ios::in, false);
      helper.registerTag(RecentlyEditedFile::d_tag, sigc::mem_fun(this, &RecentlyEditedFileList::load));
      bool retval = helper.parse();
      if (retval == false)
	unlink(filename.c_str());
      return retval;
    }
  return true;
}

RecentlyEditedFileList* RecentlyEditedFileList::getInstance(XML_Helper* helper)
{
  if (s_instance)
    deleteInstance();

  s_instance = new RecentlyEditedFileList(helper);
  return s_instance;
}

void RecentlyEditedFileList::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

RecentlyEditedFileList::RecentlyEditedFileList()
{
}

RecentlyEditedFileList::RecentlyEditedFileList(XML_Helper* helper)
{
  helper->registerTag(RecentlyEditedFile::d_tag, sigc::mem_fun(this, &RecentlyEditedFileList::load));
}

RecentlyEditedFileList::~RecentlyEditedFileList()
{
  for (RecentlyEditedFileList::iterator it = begin(); it != end(); it++)
    delete *it;
}

bool RecentlyEditedFileList::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->begin(LORDSAWAR_RECENTLY_EDITED_VERSION);
  retval &= helper->openTag(RecentlyEditedFileList::d_tag);

  for (const_iterator it = begin(); it != end(); it++)
    (*it)->save(helper);

  retval &= helper->closeTag();

  return retval;
}

bool RecentlyEditedFileList::load(std::string tag, XML_Helper* helper)
{
  if (helper->getVersion() != LORDSAWAR_RECENTLY_PLAYED_VERSION)
    {
      return false;
    }
  if (tag == RecentlyEditedFile::d_tag)
    {
      RecentlyEditedFile *g = RecentlyEditedFile::handle_load(helper);
      push_back(g);
      return true;
    }
  return false;
}

bool RecentlyEditedFileList::filename_in_list(std::string filename) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getFileName() == filename)
        return true;
    }
  return false;
}

void RecentlyEditedFileList::addEntry(std::string filename)
{
  bool unsupported_version = false;
  if (Configuration::s_remember_recently_edited_files == false)
    return;
  if (filename_in_list(filename) == true)
    {
      updateEntry(filename);
      return;
    }
  if (File::nameEndsWith(filename, Shieldset::file_extension) == true)
    {
      RecentlyEditedShieldsetFile *g = NULL;
      g = new RecentlyEditedShieldsetFile(filename);
      Shieldset *shieldset = Shieldset::create(filename, unsupported_version);
      g->fillData(shieldset);
      delete shieldset;
      push_back(g);
    }
  else if (File::nameEndsWith(filename, Tileset::file_extension) == true)
    {
      RecentlyEditedTilesetFile *g = NULL;
      g = new RecentlyEditedTilesetFile(filename);
      Tileset *tileset = Tileset::create(filename, unsupported_version);
      g->fillData(tileset);
      delete tileset;
      push_back(g);
    }
  else if (File::nameEndsWith(filename, Armyset::file_extension) == true)
    {
      RecentlyEditedArmysetFile *g = NULL;
      g = new RecentlyEditedArmysetFile(filename);
      Armyset *armyset = Armyset::create(filename, unsupported_version);
      g->fillData(armyset);
      delete armyset;
      push_back(g);
    }
  else if (File::nameEndsWith(filename, Cityset::file_extension) == true)
    {
      RecentlyEditedCitysetFile *g = NULL;
      g = new RecentlyEditedCitysetFile(filename);
      Cityset *cityset = Cityset::create(filename, unsupported_version);
      g->fillData(cityset);
      delete cityset;
      push_back(g);
    }
  else if (File::nameEndsWith(filename, MAP_EXT) == true)
    {
      bool broken = false;
      guint32 players = 0;
      guint32 cities = 0;
      std::string name, comment, id;
      GameScenario::loadDetails(filename, broken, players, cities, name, 
                                comment, id);
      RecentlyEditedMapFile *g = NULL;
      g = new RecentlyEditedMapFile(filename);
      g->fillData(name, players, cities);
      push_back(g);
    }
}

bool RecentlyEditedFileList::orderByTime(RecentlyEditedFile*rhs, RecentlyEditedFile *lhs)
{
  if (rhs->getTimeOfLastEdit().as_double() > lhs->getTimeOfLastEdit().as_double())
    return true;
  else
    return false;
}

void RecentlyEditedFileList::pruneFiles()
{
  sort(orderByTime);
  pruneOldFiles(TWO_WEEKS_OLD);
  pruneTooManyFiles(1);
}

void RecentlyEditedFileList::pruneTooManyFiles(int too_many)
{
  //too many of each kind!
  int count = 0;
  for (iterator it = begin(); it != end();)
    {
      if (File::nameEndsWith((*it)->getFileName(), Shieldset::file_extension))
        {
          count++;
          if (count > too_many)
            {
              delete *it;
              it = erase (it);
              continue;
            }
        }
      it++;
    }
  count = 0;
  for (iterator it = begin(); it != end();)
    {
      if (File::nameEndsWith((*it)->getFileName(), Tileset::file_extension))
        {
          count++;
          if (count > too_many)
            {
              delete *it;
              it = erase (it);
              continue;
            }
        }
      it++;
    }
  count = 0;
  for (iterator it = begin(); it != end();)
    {
      if (File::nameEndsWith((*it)->getFileName(), Armyset::file_extension))
        {
          count++;
          if (count > too_many)
            {
              delete *it;
              it = erase (it);
              continue;
            }
        }
      it++;
    }
  count = 0;
  for (iterator it = begin(); it != end();)
    {
      if (File::nameEndsWith((*it)->getFileName(), Cityset::file_extension))
        {
          count++;
          if (count > too_many)
            {
              delete *it;
              it = erase (it);
              continue;
            }
        }
      it++;
    }
  count = 0;
  for (iterator it = begin(); it != end();)
    {
      if (File::nameEndsWith((*it)->getFileName(), MAP_EXT))
        {
          count++;
          if (count > too_many)
            {
              delete *it;
              it = erase (it);
              continue;
            }
        }
      it++;
    }
}

void RecentlyEditedFileList::pruneOldFiles(int stale)
{
  Glib::TimeVal now;
  now.assign_current_time();
  for (iterator it = begin(); it != end();)
    {
      if ((*it)->getTimeOfLastEdit().as_double() + stale < now.as_double())
	{
	  delete *it;
	  it = erase (it);
	  continue;
	}
      it++;
    }
}

bool RecentlyEditedFileList::removeEntry(std::string filename)
{
  bool found = false;
  for (iterator it = begin(); it != end();)
    {
      if ((*it)->getFileName() == filename)
	{
	  delete *it;
	  it = erase (it);
	  found = true;
	  continue;
	}
      it++;
    }
  return found;
}

void RecentlyEditedFileList::updateEntry(std::string filename)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getFileName() == filename)
        {
          Glib::TimeVal now;
          now.assign_current_time();
          (*it)->setTimeOfLastEdit(now);
        }
    }
}

        
std::list<RecentlyEditedFile*> RecentlyEditedFileList::getFilesWithExtension(std::string ext) const
{
  std::list<RecentlyEditedFile*> files;
  for (const_iterator it = begin(); it != end(); it++)
    {
      if (File::nameEndsWith((*it)->getFileName(), ext) == true)
        files.push_back(*it);
    }
  return files;
}

bool RecentlyEditedFileList::upgrade(std::string filename, std::string old_version, std::string new_version)
{
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::RECENTLYEDITEDFILELIST, 
                                            d_tag);
}

void RecentlyEditedFileList::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type
    (FileCompat::RECENTLYEDITEDFILELIST, 
     File::get_extension(File::getUserRecentlyEditedFilesDescription()), d_tag, 
     false);
  FileCompat::getInstance()->support_version
    (FileCompat::RECENTLYEDITEDFILELIST, "0.2.0", 
     LORDSAWAR_RECENTLY_EDITED_VERSION,
     sigc::ptr_fun(&RecentlyEditedFileList::upgrade));
}
// End of file
