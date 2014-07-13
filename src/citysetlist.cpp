// Copyright (C) 2008, 2010, 2011, 2014 Ben Asselstine
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
#include <assert.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "citysetlist.h"
#include "ucompose.hpp"
#include "File.h"
#include "defs.h"
#include "tarhelper.h"
#include "setlist.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Citysetlist* Citysetlist::s_instance = 0;

Citysetlist* Citysetlist::getInstance()
{
    if (!s_instance)
        s_instance = new Citysetlist();

    return s_instance;
}

void Citysetlist::deleteInstance()
{
    if (s_instance)
      delete s_instance;

    s_instance = 0;
}

void Citysetlist::loadCitysets(std::list<Glib::ustring> citysets)
{
    for (std::list<Glib::ustring>::const_iterator i = citysets.begin(); 
	 i != citysets.end(); i++)
      {
	Cityset *cityset = loadCityset(*i);
	if (!cityset)
	  continue;

	add(cityset, *i);
      }
}

Citysetlist::Citysetlist()
{
    // load all citysets
    std::list<Glib::ustring> citysets = SetList::scan(Cityset::file_extension);
    loadCitysets(citysets);
    citysets = SetList::scan(Cityset::file_extension, false);
    loadCitysets(citysets);

}

Citysetlist::~Citysetlist()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);
}

std::list<Glib::ustring> Citysetlist::getValidNames(guint32 tilesize)
{
  std::list<Glib::ustring> names;
  for (iterator it = begin(); it != end(); it++)
    if ((*it)->getTileSize() == tilesize && (*it)->validate() == true)
      names.push_back((*it)->getName());
  names.sort(case_insensitive);
  return names;
}

Cityset *Citysetlist::loadCityset(Glib::ustring name)
{
  debug("Loading cityset " <<File::get_basename(name));
  bool unsupported_version = false;

  Cityset *cityset = Cityset::create(name, unsupported_version);
  if (!cityset)
    {
      std::cerr << String::ucompose(_("Error!  cityset: `%1' is malformed.  Skipping."), File::get_basename(name, true)) << std::endl;
      return NULL;
    }
    
  if (d_citysets.find(cityset->getBaseName()) != d_citysets.end())
    {
      Cityset *c = (*d_citysets.find(cityset->getBaseName())).second;
      std::cerr << String::ucompose(_("Error!  cityset: `%1' shares a duplicate cityset basename `%2' with `%3'.  Skipping."), cityset->getConfigurationFile(), c->getBaseName(), c->getConfigurationFile()) << std::endl;
      delete cityset;
      return NULL;
    }
    
  if (d_citysetids.find(cityset->getId()) != d_citysetids.end())
    {
      Cityset *c = (*d_citysetids.find(cityset->getId())).second;
      std::cerr << String::ucompose(_("Error!  cityset `%1' shares a duplicate cityset id with `%2'.  Skipping."), cityset->getConfigurationFile(), c->getConfigurationFile()) << std::endl;
      delete cityset;
      return NULL;
    }

  if (d_citysetids.find(cityset->getId()) != d_citysetids.end())
    {
      Cityset *c = (*d_citysetids.find(cityset->getId())).second;
      std::cerr << String::ucompose(_("Error!  cityset: `%1' has a duplicate cityset id with `%2'.  Skipping."), cityset->getName(), c->getConfigurationFile()) << std::endl;
      delete cityset;
      return NULL;
    }
  return cityset;
}

void Citysetlist::add(Cityset *cityset, Glib::ustring file)
{
  Glib::ustring basename = File::get_basename(file);
  push_back(cityset);
  cityset->setBaseName(basename);
  d_dirs[String::ucompose("%1 %2", cityset->getName(), cityset->getTileSize())] = basename;
  d_citysets[basename] = cityset;
  d_citysetids[cityset->getId()] = cityset;
}

void Citysetlist::getSizes(std::list<guint32> &sizes)
{
  for (iterator i = begin(); i != end(); i++)
    {
      if (find (sizes.begin(), sizes.end(), (*i)->getTileSize()) == sizes.end())
	sizes.push_back((*i)->getTileSize());
    }
}

Glib::ustring Citysetlist::getCitysetDir(Glib::ustring name, guint32 tilesize) const
{
  Glib::ustring name_and_size = String::ucompose("%1 %2", name, tilesize);
  DirMap::const_iterator it = d_dirs.find(name_and_size);
  if (it == d_dirs.end())
    return "";
  else
    return (*it).second;
}

void Citysetlist::instantiateImages(bool &broken)
{
  broken = false;
  for (iterator it = begin(); it != end(); it++)
    {
      if (!broken)
        (*it)->instantiateImages(broken);
    }
}

void Citysetlist::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();
}
	
Cityset *Citysetlist::getCityset(guint32 id) const
{ 
  CitysetIdMap::const_iterator it = d_citysetids.find(id);
  if (it == d_citysetids.end())
    return NULL;
  return (*it).second;
}
	
Cityset *Citysetlist::getCityset(Glib::ustring bname) const
{ 
  CitysetMap::const_iterator it = d_citysets.find(bname);
  if (it == d_citysets.end())
    return NULL;
  return (*it).second;
}

Cityset *Citysetlist::import(Tar_Helper *t, Glib::ustring f, bool &broken)
{
  bool unsupported_version = false;
  Glib::ustring filename = t->getFile(f, broken);
  if (broken)
    return NULL;
  Cityset *cityset = Cityset::create(filename, unsupported_version);
  assert (cityset != NULL);
  cityset->setBaseName(File::get_basename(f));

  Glib::ustring basename = "";
  guint32 id = 0;
  addToPersonalCollection(cityset, basename, id);

  return cityset;
}

Glib::ustring Citysetlist::findFreeBaseName(Glib::ustring basename, guint32 max, guint32 &num) const
{
  Glib::ustring new_basename;
  for (unsigned int count = 1; count < max; count++)
    {
      new_basename = String::ucompose("%1%2", basename, count);
      if (getCityset(new_basename) == NULL)
        {
          num = count;
          break;
        }
      else
        new_basename = "";
    }
  return new_basename;
}

bool Citysetlist::addToPersonalCollection(Cityset *cityset, Glib::ustring &new_basename, guint32 &new_id)
{
  //do we already have this one?
      
  if (getCityset(cityset->getBaseName()) == getCityset(cityset->getId()) 
      && getCityset(cityset->getBaseName()) != NULL)
    {
      cityset->setDirectory(getCityset(cityset->getId())->getDirectory());
      return true;
    }

  //if the basename conflicts with any other basename, then change it.
  if (getCityset(cityset->getBaseName()) != NULL)
    {
      if (new_basename != "" && getCityset(new_basename) == NULL)
        ;
      else
        {
          guint32 num = 0;
          Glib::ustring new_basename = findFreeBaseName(cityset->getBaseName(), 100, num);
          if (new_basename == "")
            return false;
        }
    }
  else if (new_basename == "")
    new_basename = cityset->getBaseName();

  //if the id conflicts with any other id, then change it
  if (getCityset(cityset->getId()) != NULL)
    {
      if (new_id != 0 && getCityset(new_id) == NULL)
        cityset->setId(new_id);
      else
        {
          new_id = Citysetlist::getNextAvailableId(cityset->getId());
          cityset->setId(new_id);
        }
    }
  else
    new_id = cityset->getId();

  //make the directory where the cityset is going to live.
  Glib::ustring file = File::getSetDir(Cityset::file_extension, false) + new_basename + Cityset::file_extension;

  cityset->save(file, Cityset::file_extension);

  if (new_basename != cityset->getBaseName())
    cityset->setBaseName(new_basename);
  cityset->setDirectory(File::get_dirname(file));
  add (cityset, file);
  return true;
}

int Citysetlist::getNextAvailableId(int after)
{
  bool unsupported_version = false;
  std::list<guint32> ids;
  std::list<Glib::ustring> citysets = SetList::scan(Cityset::file_extension);
  //there might be IDs in invalid citysets.
  for (std::list<Glib::ustring>::const_iterator i = citysets.begin(); 
       i != citysets.end(); i++)
    {
      Cityset *cityset = Cityset::create(*i, unsupported_version);
      if (cityset != NULL)
	{
	  ids.push_back(cityset->getId());
	  delete cityset;
	}
    }
  citysets = SetList::scan(Cityset::file_extension, false);
  for (std::list<Glib::ustring>::const_iterator i = citysets.begin(); 
       i != citysets.end(); i++)
    {
      Cityset *cityset = Cityset::create(*i, unsupported_version);
      if (cityset != NULL)
	{
	  ids.push_back(cityset->getId());
	  delete cityset;
	}
    }
  for (guint32 i = after + 1; i < 1000000; i++)
    {
      if (find(ids.begin(), ids.end(), i) == ids.end())
	return i;
    }
  return -1;
}

bool Citysetlist::contains(Glib::ustring name) const
{
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->getName() == name)
      return true;
  return false;
}

guint32 Citysetlist::getCitysetId(Glib::ustring basename) const
{
  Cityset *cs = getCityset(basename);
  if (cs)
    return cs->getId();
  else
    return 0;
}

bool Citysetlist::reload(guint32 id) 
{
  Cityset *cityset = getCityset(id);
  if (!cityset)
    return false;
  bool broken = false;
  cityset->reload(broken);
  if (broken)
    return false;
  Glib::ustring basename = cityset->getBaseName();
  d_dirs[String::ucompose("%1 %2", cityset->getName(), cityset->getTileSize())] = basename;
  d_citysets[basename] = cityset;
  d_citysetids[cityset->getId()] = cityset;
  return true;
}
