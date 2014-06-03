// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
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

#include <iostream>
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


//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

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

void Armysetlist::add(Armyset *armyset, Glib::ustring file)
{
  Glib::ustring basename = File::get_basename(file);
  push_back(armyset); 
  for (Armyset::iterator ait = armyset->begin(); ait != armyset->end(); ait++)
    d_armies[armyset->getId()][(*ait)->getId()] = (*ait);
  d_names[armyset->getId()] = armyset->getName();
  d_ids[String::ucompose("%1 %2", armyset->getName(), armyset->getTileSize())] = armyset->getId();
  armyset->setBaseName(basename);
  d_armysets[basename] = armyset;
  d_armysetids[armyset->getId()] = armyset;
}

void Armysetlist::loadArmysets(std::list<Glib::ustring> armysets)
{
  for (std::list<Glib::ustring>::const_iterator i = armysets.begin(); 
       i != armysets.end(); i++)
    {
      Armyset *armyset = loadArmyset(*i);
      if (!armyset)
	continue;

      add(armyset, *i);
    }
}

Armysetlist::Armysetlist()
{
  // load all armysets
  std::list<Glib::ustring> armysets = Armyset::scanSystemCollection();
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
      delete (*((*it).second).begin()).second;
}

ArmyProto* Armysetlist::getArmy(guint32 id, guint32 type_id) const
{
  // always use ArmyProtoMap::find for searching, else a default entry is 
  // created, which can produce really bad results!!
  ArmyPrototypeMap::const_iterator it = d_armies.find(id);

  // armyset does not exist
  if (it == d_armies.end())
    return NULL;

  IdArmyPrototypeMap::const_iterator j = (*it).second.find(type_id);
  if (j == (*it).second.end())
    return NULL;
  return (*j).second;
}

ArmyProto* Armysetlist::lookupWeakestQuickestArmy(guint32 id) const
{
  Armyset *a = getArmyset(id);
  if (a)
    return a->lookupWeakestQuickestArmy();
  return NULL;
}

std::list<Glib::ustring> Armysetlist::getValidNames() const
{
  std::list<Glib::ustring> names;
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->validate() == true)
      names.push_back((*it)->getName());
  names.sort(case_insensitive);
  return names;
}

std::list<Glib::ustring> Armysetlist::getValidNames(guint32 tilesize)
{
  std::list<Glib::ustring> names;
  for (iterator it = begin(); it != end(); it++)
    if ((*it)->getTileSize() == tilesize && (*it)->validate() == true)
      names.push_back((*it)->getName());
  names.sort(case_insensitive);
  return names;
}

std::list<Glib::ustring> Armysetlist::getNames() const
{
  std::list<Glib::ustring> names;
  for (const_iterator it = begin(); it != end(); it++)
    names.push_back((*it)->getName());
  names.sort(case_insensitive);
  return names;
}

std::list<Glib::ustring> Armysetlist::getNames(guint32 tilesize)
{
  std::list<Glib::ustring> names;
  for (iterator it = begin(); it != end(); it++)
    if ((*it)->getTileSize() == tilesize)
      names.push_back((*it)->getName());
  names.sort(case_insensitive);
  return names;
}

Glib::ustring Armysetlist::getName(guint32 id) const
{
  NameMap::const_iterator it = d_names.find(id);

  // armyset does not exist
  if (it == d_names.end())
    return 0;

  return (*it).second;
}

Armyset *Armysetlist::loadArmyset(Glib::ustring name)
{
  debug("Loading armyset " <<File::get_basename(name));
  bool unsupported_version;
  Armyset *armyset = Armyset::create(name, unsupported_version);
  if (armyset == NULL)
    {
      std::cerr << String::ucompose (_("Error!  armyset: `%1' is malformed.  Skipping."),File::get_basename(name, true)) << std::endl;
      return NULL;
    }
  if (d_armysets.find(armyset->getBaseName()) != d_armysets.end())
    {
      Armyset *a = (*d_armysets.find(armyset->getBaseName())).second;
      std::cerr << String::ucompose(_("Error!  armyset: `%1' shares a duplicate armyset basename `%2' with `%3'.  Skipping."), armyset->getConfigurationFile(), a->getBaseName(), a->getConfigurationFile()) << std::endl;
      delete armyset;
      return NULL;
    }
    
  if (d_armysetids.find(armyset->getId()) != d_armysetids.end())
    {
      Armyset *a = (*d_armysetids.find(armyset->getId())).second;
      std::cerr << String::ucompose(_("Error!  armyset: `%1' sahres a duplicate armyset id with `%2'.  Skipping."), armyset->getConfigurationFile(), a->getConfigurationFile()) << std::endl;
      delete armyset;
      return NULL;
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

int Armysetlist::getArmysetId(Glib::ustring armyset, guint32 tilesize) const
{
  IdMap::const_iterator it = 
    d_ids.find(String::ucompose("%1 %2", armyset, tilesize));
  if (it == d_ids.end())
    return -1;
  return (*it).second;
}

Glib::ustring Armysetlist::getArmysetDir(Glib::ustring name, guint32 tilesize) const
{
  int id = getArmysetId(name, tilesize);
  if (id == -1)
    return "";
  return getArmyset(id)->getBaseName();
}

int Armysetlist::getNextAvailableId(int after)
{
  bool unsupported_version;
  std::list<guint32> ids;
  std::list<Glib::ustring> armysets = Armyset::scanSystemCollection();
  //there might be IDs in invalid armysets.
  for (std::list<Glib::ustring>::const_iterator i = armysets.begin(); 
       i != armysets.end(); i++)
    {
      Armyset *armyset = Armyset::create(*i, unsupported_version);
      if (armyset != NULL)
	{
	  ids.push_back(armyset->getId());
	  delete armyset;
	}
    }
  armysets = Armyset::scanUserCollection();
  for (std::list<Glib::ustring>::const_iterator i = armysets.begin(); 
       i != armysets.end(); i++)
    {
      Armyset *armyset = Armyset::create(*i, unsupported_version);
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

void Armysetlist::instantiateImages(bool &broken)
{
  broken = false;
  for (iterator it = begin(); it != end(); it++)
    {
      if (!broken)
        (*it)->instantiateImages(broken);
    }
}

void Armysetlist::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();
}

Armyset *Armysetlist::getArmyset(guint32 id) const
{
  ArmysetIdMap::const_iterator it = d_armysetids.find(id);
  if (it == d_armysetids.end())
    return NULL;
  return (*it).second;
}

Armyset *Armysetlist::getArmyset(Glib::ustring bname) const
{ 
  ArmysetMap::const_iterator it = d_armysets.find(bname);
  if (it == d_armysets.end())
    return NULL;
  return (*it).second;
}

Glib::ustring Armysetlist::findFreeBaseName(Glib::ustring basename, guint32 max, guint32 &num) const
{
  Glib::ustring new_basename;
  for (unsigned int count = 1; count < max; count++)
    {
      new_basename = String::ucompose("%1%2", basename, count);
      if (getArmyset(new_basename) == NULL)
        {
          num = count;
          break;
        }
      else
        new_basename = "";
    }
  return new_basename;
}

bool Armysetlist::addToPersonalCollection(Armyset *armyset, Glib::ustring &new_basename, guint32 &new_id)
{
  //do we already have this one?
      
  if (getArmyset(armyset->getBaseName()) == getArmyset(armyset->getId())
      && getArmyset(armyset->getBaseName()) != NULL)
    {
      armyset->setDirectory(getArmyset(armyset->getId())->getDirectory());
      return true;
    }

  //if the basename conflicts with any other basename, then change it.
  if (getArmyset(armyset->getBaseName()) != NULL)
    {
      if (new_basename != "" && getArmyset(new_basename) == NULL)
        ;
      else
        {
          guint32 num = 0;
          Glib::ustring new_basename = findFreeBaseName(armyset->getBaseName(), 100, num);
          if (new_basename == "")
            return false;
        }
    }
  else if (new_basename == "")
    new_basename = armyset->getBaseName();

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
  Glib::ustring file = File::getUserArmysetDir() + new_basename + Armyset::file_extension;

  armyset->save(file, Armyset::file_extension);

  if (new_basename != armyset->getBaseName())
    armyset->setBaseName(new_basename);
  armyset->setDirectory(File::get_dirname(file));
  add (armyset, file);
  return true;
}

Armyset *Armysetlist::import(Tar_Helper *t, Glib::ustring f, bool &broken)
{
  bool unsupported_version;
  Glib::ustring filename = t->getFile(f, broken);
  if (broken)
    return NULL;
  Armyset *armyset = Armyset::create(filename, unsupported_version);
  assert (armyset != NULL);
  armyset->setBaseName(File::get_basename(f));

  Glib::ustring basename = "";
  guint32 id = 0;
  addToPersonalCollection(armyset, basename, id);

  return armyset;
}

bool Armysetlist::contains(Glib::ustring name) const
{
  std::list<Glib::ustring> n = getNames();
  for (std::list<Glib::ustring>::iterator it = n.begin(); it != n.end(); it++)
    {
      if (*it == name)
        return true;
    }
  return false;
}

bool Armysetlist::reload(guint32 id) 
{
  Armyset *armyset = getArmyset(id);
  if (!armyset)
    return false;
  bool broken = false;
  armyset->reload(broken);
  if (broken)
    return false;
  d_armies[armyset->getId()].clear();
  for (Armyset::iterator ait = armyset->begin(); ait != armyset->end(); ait++)
    d_armies[armyset->getId()][(*ait)->getId()] = (*ait);
  d_names[armyset->getId()] = armyset->getName();
  d_ids[String::ucompose("%1 %2", armyset->getName(), armyset->getTileSize())] = armyset->getId();
  d_armysetids[armyset->getId()] = armyset;
  d_armysets[armyset->getBaseName()] = armyset;
  return true;
}
