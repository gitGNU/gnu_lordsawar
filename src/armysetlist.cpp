// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
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

#include <iostream>
#include <algorithm>
#include <expat.h>
#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>
#include <assert.h>

#include "rectangle.h"
#include "armysetlist.h"
#include "armyset.h"
#include "File.h"
#include "defs.h"
#include "ucompose.hpp"
#include "PixMask.h"
#include "tarhelper.h"


using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

Armysetlist* Armysetlist::s_instance = 0;

Armysetlist* Armysetlist::getInstance()
{
  if (!s_instance)
    s_instance = new Armysetlist();

  return s_instance;
}

void Armysetlist::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

void Armysetlist::add(Armyset *armyset)
{
  std::string subdir = File::get_basename(armyset->getDirectory());
  push_back(armyset); 
  for (Armyset::iterator ait = armyset->begin(); ait != armyset->end(); ait++)
    d_armies[armyset->getId()].push_back(*ait);
  d_names[armyset->getId()] = armyset->getName();
  d_ids[String::ucompose("%1 %2", armyset->getName(), armyset->getTileSize())] = armyset->getId();
  armyset->setSubDir(subdir);
  d_armysets[subdir] = armyset;
  d_armysetids[armyset->getId()] = armyset;
}

void Armysetlist::loadArmysets(std::list<std::string> armysets)
{
  for (std::list<std::string>::const_iterator i = armysets.begin(); 
       i != armysets.end(); i++)
    {
      Armyset *armyset = loadArmyset(*i);
      if (!armyset)
	continue;

      add(armyset);
    }
}

Armysetlist::Armysetlist()
{
  // load all armysets
  std::list<std::string> armysets = Armyset::scanSystemCollection();
  loadArmysets(armysets);
  armysets = Armyset::scanUserCollection();
  loadArmysets(armysets);

}

Armysetlist::~Armysetlist()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);

  // remove all army entries
  for (ArmyPrototypeMap::iterator it = d_armies.begin(); 
       it != d_armies.end(); it++)
    while (!(*it).second.empty())
      delete ((*it).second)[0];
}

ArmyProto* Armysetlist::getArmy(guint32 id, guint32 index) const
{
  // always use ArmyProtoMap::find for searching, else a default entry is 
  // created, which can produce really bad results!!
  ArmyPrototypeMap::const_iterator it = d_armies.find(id);

  // armyset does not exist
  if (it == d_armies.end())
    return 0;

  // index too large
  if (index >= (*it).second.size())
    return 0;

  return ((*it).second)[index];
}

ArmyProto* Armysetlist::getScout(guint32 id) const
{
  // always use ArmyProtoMap::find for searching, else a default entry is 
  // created, which can produce really bad results!!
  ArmyPrototypeMap::const_iterator it = d_armies.find(id);

  // armyset does not exist
  if (it == d_armies.end())
    return 0;

  return ((*it).second)[0];
}

guint32 Armysetlist::getSize(guint32 id) const
{
  ArmyPrototypeMap::const_iterator it = d_armies.find(id);

  // armyset does not exist
  if (it == d_armies.end())
    return 0;

  return (*it).second.size();
}

std::list<std::string> Armysetlist::getNames()
{
  std::list<std::string> names;
  for (iterator it = begin(); it != end(); it++)
    names.push_back((*it)->getName());
  return names;
}

std::list<std::string> Armysetlist::getNames(guint32 tilesize)
{
  std::list<std::string> names;
  for (iterator it = begin(); it != end(); it++)
    if ((*it)->getTileSize() == tilesize)
      names.push_back((*it)->getName());
  return names;
}

std::string Armysetlist::getName(guint32 id) const
{
  NameMap::const_iterator it = d_names.find(id);

  // armyset does not exist
  if (it == d_names.end())
    return 0;

  return (*it).second;
}

Armyset *Armysetlist::loadArmyset(std::string name)
{
  debug("Loading armyset " <<name);
  Armyset *armyset = Armyset::create(name);
  if (armyset == NULL)
    return NULL;
  if (armyset->validate() == false)
    {
      cerr << "Error!  armyset: `" << armyset->getName() << 
	"' is invalid.  Skipping." << endl;
      delete armyset;
      return NULL;
    }
  if (d_armysetids.find(armyset->getId()) != d_armysetids.end())
    {
      Armyset *a = (*d_armysetids.find(armyset->getId())).second;
      cerr << "Error!  armyset: `" << armyset->getName() << 
	"' shares a duplicate armyset id with `" << 
	a->getConfigurationFile() << "'.  Skipping." << endl;
      delete armyset;
      return NULL;
      return false;
    }
  return armyset;
}

PixMask* Armysetlist::getShipPic (guint32 id)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == id)
	return (*it)->getShipPic();
    }
  return NULL;
}

PixMask* Armysetlist::getShipMask (guint32 id)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == id)
	return (*it)->getShipMask();
    }
  return NULL;
}

guint32 Armysetlist::getTileSize(guint32 id)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == id)
	return (*it)->getTileSize();
    }
  return 0;
}

PixMask* Armysetlist::getBagPic (guint32 id)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == id)
	return (*it)->getBagPic();
    }
  return NULL;
}

PixMask* Armysetlist::getStandardPic (guint32 id)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == id)
	return (*it)->getStandardPic();
    }
  return NULL;
}

PixMask* Armysetlist::getStandardMask (guint32 id)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == id)
	return (*it)->getStandardMask();
    }
  return NULL;
}

void Armysetlist::getSizes(std::list<guint32> &sizes)
{
  for (iterator i = begin(); i != end(); i++)
    {
      if (find (sizes.begin(), sizes.end(), (*i)->getTileSize()) == sizes.end())
	sizes.push_back((*i)->getTileSize());
    }
}

guint32 Armysetlist::getArmysetId(std::string armyset, guint32 tilesize)
{
  return d_ids[String::ucompose("%1 %2", armyset, tilesize)];
}

std::string Armysetlist::getArmysetDir(std::string name, guint32 tilesize)
{
  return getArmyset(getArmysetId(name, tilesize))->getSubDir();
}

int Armysetlist::getNextAvailableId(int after)
{
  std::list<guint32> ids;
  std::list<std::string> armysets = Armyset::scanSystemCollection();
  //there might be IDs in invalid armysets.
  for (std::list<std::string>::const_iterator i = armysets.begin(); 
       i != armysets.end(); i++)
    {
      Armyset *armyset = Armyset::create(*i);
      if (armyset != NULL)
	{
	  ids.push_back(armyset->getId());
	  delete armyset;
	}
    }
  armysets = Armyset::scanUserCollection();
  for (std::list<std::string>::const_iterator i = armysets.begin(); 
       i != armysets.end(); i++)
    {
      Armyset *armyset = Armyset::create(*i);
      if (armyset != NULL)
	{
	  ids.push_back(armyset->getId());
	  delete armyset;
	}
    }
  for (guint32 i = after + 1; i < 1000000; i++)
    {
      if (find(ids.begin(), ids.end(), i) == ids.end())
	return i;
    }
  return -1;
}

void Armysetlist::instantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->instantiateImages();
}

void Armysetlist::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();
}

Armyset *Armysetlist::getArmyset(guint32 id) 
{
  if (d_armysetids.find(id) == d_armysetids.end())
    return NULL;
  return d_armysetids[id];
}

Armyset *Armysetlist::getArmyset(std::string dir) 
{ 
  if (d_armysets.find(dir) == d_armysets.end())
    return NULL;
  return d_armysets[dir];
}
bool Armysetlist::addToPersonalCollection(Armyset *armyset, std::string &new_subdir, guint32 &new_id)
{
  //do we already have this one?

  if (getArmyset(armyset->getSubDir()) == getArmyset(armyset->getId()) &&
      getArmyset(armyset->getSubDir()) != NULL)
    {
      armyset->setDirectory(getArmyset(armyset->getId())->getDirectory());
      return true;
    }

  //if the subdir conflicts with any other subdir, then change it.
  if (getArmyset(armyset->getSubDir()) != NULL)
    {
      if (new_subdir != "" && getArmyset(new_subdir) == NULL)
        armyset->setSubDir(new_subdir);
      else
        {
          bool found = false;
          for (int count = 0; count < 100; count++)
            {
              new_subdir = String::ucompose("%1%2", armyset->getSubDir(), 
                                            count);
              if (getArmyset(new_subdir) == NULL)
                {
                  found = true;
                  break;
                }
            }
          if (found == false)
            return false;
          armyset->setSubDir(new_subdir);
        }
    }
  else
    new_subdir = armyset->getSubDir();

  //if the id conflicts with any other id, then change it
  if (getArmyset(armyset->getId()) != NULL)
    {
      if (new_id != 0 && getArmyset(new_id) == NULL)
        armyset->setId(new_id);
      else
        {
          new_id = Armysetlist::getNextAvailableId(armyset->getId());
          armyset->setId(new_id);
        }
    }
  else
    new_id = armyset->getId();

  //make the directory where the armyset is going to live.
  std::string directory = 
    File::getUserArmysetDir() + armyset->getSubDir() + "/";

  if (File::create_dir(directory) == false)
    return false;

  //okay now we copy the image files into the new directory 
  std::list<std::string> files;
  armyset->getFilenames(files);
  for (std::list<std::string>::iterator it = files.begin(); it != files.end();
       it++)
    File::copy(armyset->getFile(*it), directory + *it);

  //save out the armyset file
  armyset->setDirectory(directory);
  XML_Helper helper(armyset->getConfigurationFile(), std::ios::out, false);
  armyset->save(&helper);
  helper.close();
      
  add(armyset);
  return true;
}

Armyset *Armysetlist::import(Tar_Helper *t, std::string f, bool &broken)
{
  std::string filename = t->getFile(f, broken);
  Armyset *armyset = Armyset::create(filename);
  assert (armyset != NULL);
  armyset->setSubDir(File::get_basename(f));

  //extract all the files and remember where we extracted them
  std::list<std::string> delfiles;
  delfiles.push_back(filename);
  std::list<std::string> files;
  armyset->getFilenames(files);
  for (std::list<std::string>::iterator i = files.begin(); i != files.end(); i++)
    {
      std::string b = *i + ".png";
      std::string file = t->getFile(*i + ".png", broken);
    delfiles.push_back (file);
    }

  std::string subdir = "";
  guint32 id = 0;
  addToPersonalCollection(armyset, subdir, id);

  for (std::list<std::string>::iterator it = delfiles.begin(); it != delfiles.end(); it++)
    File::erase(*it);
  return armyset;

}

