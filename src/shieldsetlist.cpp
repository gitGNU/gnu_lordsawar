//  Copyright (C) 2008, 2009, 2010, 2011, 2014 Ben Asselstine
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
#include <expat.h>
#include <assert.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "shieldsetlist.h"
#include "shieldset.h"
#include "File.h"
#include "defs.h"
#include "ucompose.hpp"
#include "tarhelper.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Shieldsetlist* Shieldsetlist::s_instance = 0;

Shieldsetlist* Shieldsetlist::getInstance()
{
    if (!s_instance)
        s_instance = new Shieldsetlist();

    return s_instance;
}

void Shieldsetlist::deleteInstance()
{
    if (s_instance)
      delete s_instance;

    s_instance = 0;
}


void Shieldsetlist::loadShieldsets(std::list<Glib::ustring> shieldsets)
{
    for (std::list<Glib::ustring>::const_iterator i = shieldsets.begin(); 
	 i != shieldsets.end(); i++)
      {
        Shieldset *shieldset = loadShieldset(*i);
	if (!shieldset)
	  continue;
	add(shieldset, *i);
      }
}

Shieldsetlist::Shieldsetlist()
{
    // load all shieldsets
    std::list<Glib::ustring> shieldsets = Shieldset::scanSystemCollection();
    loadShieldsets(shieldsets);
    shieldsets = Shieldset::scanUserCollection();
    loadShieldsets(shieldsets);

}

Shieldsetlist::~Shieldsetlist()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);
}

std::list<Glib::ustring> Shieldsetlist::getNames() const
{
  std::list<Glib::ustring> names;
  for (const_iterator it = begin(); it != end(); it++)
    names.push_back((*it)->getName());
  names.sort(case_insensitive);
  return names;
}

std::list<Glib::ustring> Shieldsetlist::getValidNames() const
{
  std::list<Glib::ustring> names;
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->validate() == true)
        names.push_back((*it)->getName());
    }
  names.sort(case_insensitive);
  return names;
}

Glib::ustring Shieldsetlist::getShieldsetDir(Glib::ustring name) const
{
  DirMap::const_iterator it = d_dirs.find(name);
  if (it == d_dirs.end())
    return "";
  else
    return (*it).second;
}

Shieldset* Shieldsetlist::loadShieldset(Glib::ustring name)
{
  bool unsupported_version = false;
  debug("Loading shieldset " <<File::get_basename(name));

  Shieldset *shieldset = Shieldset::create(name, unsupported_version);
  if (!shieldset)
    {
      std::cerr << String::ucompose(_("Error!  shieldset: `%1' is malformed.  Skipping."), File::get_basename(name, true)) << std::endl;
      return NULL;
    }

  if (d_shieldsets.find(shieldset->getBaseName()) != d_shieldsets.end())
    {
      Shieldset *s = (*d_shieldsets.find(shieldset->getBaseName())).second;
      std::cerr << String::ucompose(_("Error!  shieldset: `%1' shares a duplicate shieldset basename `%2' with `%3'.  Skipping."), shieldset->getConfigurationFile(), s->getBaseName(), s->getConfigurationFile()) << std::endl;
      delete shieldset;
      return NULL;
    }
    
  if (d_dirs.find(shieldset->getName()) != d_dirs.end())
    {
      Glib::ustring basename = (*d_dirs.find(shieldset->getName())).second;
      if (basename != "")
        {
          Shieldset *s = (*d_shieldsets.find(basename)).second;
          std::cerr << String::ucompose(_("Error!  shieldset: `%1' shares a duplicate shieldset name `%2' with `%3'.  Skipping."), shieldset->getConfigurationFile(), s->getName(), s->getConfigurationFile()) << std::endl;
          delete shieldset;
        }
      return NULL;
    }
    
  if (d_shieldsetids.find(shieldset->getId()) != d_shieldsetids.end())
    {
      Shieldset *s = (*d_shieldsetids.find(shieldset->getId())).second;
      std::cerr << String::ucompose(_("Error!  shieldset: `%1' shares a duplicate shieldset id with `%2'.  Skipping."), shieldset->getConfigurationFile(), s->getConfigurationFile()) << std::endl;
      delete shieldset;
      return NULL;
    }
    
  return shieldset;
}

void Shieldsetlist::add(Shieldset *shieldset, Glib::ustring file)
{
  Glib::ustring basename = File::get_basename(file);
  push_back(shieldset);
  shieldset->setBaseName(basename);
  d_dirs[shieldset->getName()] = basename;
  d_shieldsets[basename] = shieldset;
  d_shieldsetids[shieldset->getId()] = shieldset;
}
        
Gdk::RGBA Shieldsetlist::getColor(guint32 shieldset, guint32 owner) const
{
  Shieldset *s = getShieldset(shieldset);
  if (!s)
    return Gdk::RGBA("black");
  return s->getColor(owner);
}

ShieldStyle *Shieldsetlist::getShield(guint32 shieldset, guint32 type, guint32 colour) const
{
  Shieldset *s = getShieldset(shieldset);
  if (!s)
    return NULL;
  return s->lookupShieldByTypeAndColour(type, colour);
}

void Shieldsetlist::instantiateImages(bool &broken)
{
  broken = false;
  for (iterator it = begin(); it != end(); it++)
    {
      if (!broken)
        (*it)->instantiateImages(broken);
    }
}

void Shieldsetlist::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();
}
	
Shieldset *Shieldsetlist::getShieldset(guint32 id)  const
{ 
  ShieldsetIdMap::const_iterator it = d_shieldsetids.find(id);
  if (it == d_shieldsetids.end())
    return NULL;
  return (*it).second;
}

Shieldset *Shieldsetlist::getShieldset(Glib::ustring bname) const
{ 
  ShieldsetMap::const_iterator it = d_shieldsets.find(bname);
  if (it == d_shieldsets.end())
    return NULL;
  return (*it).second;
}

Shieldset *Shieldsetlist::import(Tar_Helper *t, Glib::ustring f, bool &broken)
{
  bool unsupported_version = false;
  Glib::ustring filename = t->getFile(f, broken);
  Shieldset *shieldset = Shieldset::create(filename, unsupported_version);
  assert (shieldset != NULL);
  shieldset->setBaseName(File::get_basename(f));

  Glib::ustring basename = "";
  guint32 id = 0;
  addToPersonalCollection(shieldset, basename, id);

  return shieldset;

}

Glib::ustring Shieldsetlist::findFreeBaseName(Glib::ustring basename, guint32 max, guint32 &num) const
{
  Glib::ustring new_basename;
  for (unsigned int count = 1; count < max; count++)
    {
      new_basename = String::ucompose("%1%2", basename, count);
      if (getShieldset(new_basename) == NULL)
        {
          num = count;
          break;
        }
      else
        new_basename = "";
    }
  return new_basename;
}

bool Shieldsetlist::addToPersonalCollection(Shieldset *shieldset, Glib::ustring &new_basename, guint32 &new_id)
{
  //do we already have this one?
      
  if (getShieldset(shieldset->getBaseName()) == getShieldset(shieldset->getId()) 
      && getShieldset(shieldset->getBaseName()) != NULL)
    {
      shieldset->setDirectory(getShieldset(shieldset->getId())->getDirectory());
      return true;
    }

  //if the basename conflicts with any other basename, then change it.
  if (getShieldset(shieldset->getBaseName()) != NULL)
    {
      if (new_basename != "" && getShieldset(new_basename) == NULL)
        ;
      else
        {
          guint32 num = 0;
          Glib::ustring new_basename = findFreeBaseName(shieldset->getBaseName(), 100, num);
          if (new_basename == "")
            return false;
        }
    }
  else if (new_basename == "")
    new_basename = shieldset->getBaseName();

  //if the id conflicts with any other id, then change it
  if (getShieldset(shieldset->getId()) != NULL)
    {
      if (new_id != 0 && getShieldset(new_id) == NULL)
        shieldset->setId(new_id);
      else
        {
          new_id = Shieldsetlist::getNextAvailableId(shieldset->getId());
          shieldset->setId(new_id);
        }
    }
  else
    new_id = shieldset->getId();

  //make the directory where the shieldset is going to live.
  Glib::ustring file = File::getUserShieldsetDir() + new_basename + Shieldset::file_extension;

  shieldset->save(file, Shieldset::file_extension);

  if (new_basename != shieldset->getBaseName())
    shieldset->setBaseName(new_basename);
  shieldset->setDirectory(File::get_dirname(file));
  add (shieldset, file);
  return true;
}

int Shieldsetlist::getNextAvailableId(int after)
{
  bool unsupported_version = false;
  std::list<guint32> ids;
  std::list<Glib::ustring> shieldsets = Shieldset::scanSystemCollection();
  for (std::list<Glib::ustring>::const_iterator i = shieldsets.begin(); 
       i != shieldsets.end(); i++)
    {
      Shieldset *shieldset = Shieldset::create(*i, unsupported_version);
      if (shieldset != NULL)
	{
	  ids.push_back(shieldset->getId());
	  delete shieldset;
	}
    }
  shieldsets = Shieldset::scanUserCollection();
  for (std::list<Glib::ustring>::const_iterator i = shieldsets.begin(); 
       i != shieldsets.end(); i++)
    {
      Shieldset *shieldset = Shieldset::create(*i, unsupported_version);
      if (shieldset != NULL)
	{
	  ids.push_back(shieldset->getId());
	  delete shieldset;
	}
    }
  for (guint32 i = after + 1; i < 1000000; i++)
    {
      if (find(ids.begin(), ids.end(), i) == ids.end())
	return i;
    }
  return -1;
}

bool Shieldsetlist::contains(Glib::ustring name) const
{
  std::list<Glib::ustring> n = getNames();
  for (std::list<Glib::ustring>::iterator it = n.begin(); it != n.end(); it++)
    {
      if (*it == name)
        return true;
    }
  return false;
}

guint32 Shieldsetlist::getShieldsetId(Glib::ustring basename) const
{
  Shieldset *ss = getShieldset(basename);
  if (ss)
    return ss->getId();
  else
    return 0;
}

bool Shieldsetlist::reload(guint32 id) 
{
  Shieldset *shieldset = getShieldset(id);
  if (!shieldset)
    return false;
  bool broken = false;
  shieldset->reload(broken);
  if (broken)
    return false;
  Glib::ustring basename = shieldset->getBaseName();
  d_dirs[shieldset->getName()] = basename;
  d_shieldsets[basename] = shieldset;
  d_shieldsetids[shieldset->getId()] = shieldset;
  return true;
}
