// Copyright (C) 2008 Ben Asselstine
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
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "citysetlist.h"
#include "ucompose.hpp"
#include "File.h"
#include "defs.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

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

void Citysetlist::loadCitysets(std::list<std::string> citysets)
{
    for (std::list<std::string>::const_iterator i = citysets.begin(); 
	 i != citysets.end(); i++)
      {
	Cityset *cityset = loadCityset(*i);
	if (!cityset)
	  continue;
	    
	add(cityset);
      }
}

Citysetlist::Citysetlist()
{
    // load all citysets
    std::list<std::string> citysets = Cityset::scanSystemCollection();
    loadCitysets(citysets);
    citysets = Cityset::scanUserCollection();
    loadCitysets(citysets);

}

Citysetlist::~Citysetlist()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);
}

std::list<std::string> Citysetlist::getNames()
{
  std::list<std::string> names;
  for (iterator it = begin(); it != end(); it++)
    names.push_back((*it)->getName());
  return names;
}

std::list<std::string> Citysetlist::getNames(guint32 tilesize)
{
  std::list<std::string> names;
  for (iterator it = begin(); it != end(); it++)
    if ((*it)->getTileSize() == tilesize)
      names.push_back((*it)->getName());
  return names;
}

Cityset *Citysetlist::loadCityset(std::string name)
{
  debug("Loading cityset " <<File::get_basename(File::get_dirname(name)));

  Cityset *cityset = Cityset::create(name);
  if (!cityset)
    return NULL;
  if (d_citysetids.find(cityset->getId()) != d_citysetids.end())
    {
      Cityset *c = (*d_citysetids.find(cityset->getId())).second;
      cerr << "Error!  cityset: `" << cityset->getName() << 
	"' shares a duplicate cityset id with `" << c->getConfigurationFile() 
	<< "'.  Skipping." << endl;
      delete cityset;
      return NULL;
    }

  if (d_citysetids.find(cityset->getId()) != d_citysetids.end())
    {
      Cityset *c = (*d_citysetids.find(cityset->getId())).second;
      cerr << "Error!  cityset: `" << cityset->getName() << 
	"' has a duplicate cityset id with `" << c->getConfigurationFile() << 
	"'.  Skipping." << endl;
      delete cityset;
      return NULL;
    }
  return cityset;
}

void Citysetlist::add(Cityset *cityset)
{
  std::string subdir = File::get_basename(cityset->getDirectory());
  push_back(cityset);
  cityset->setSubDir(subdir);
  d_dirs[String::ucompose("%1 %2", cityset->getName(), cityset->getTileSize())] = subdir;
  d_citysets[subdir] = cityset;
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

std::string Citysetlist::getCitysetDir(std::string name, guint32 tilesize)
{
  return d_dirs[String::ucompose("%1 %2", name, tilesize)];
}
void Citysetlist::instantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->instantiateImages();
}
void Citysetlist::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();
}
	
Cityset *Citysetlist::getCityset(guint32 id) 
{ 
  if (d_citysetids.find(id) == d_citysetids.end())
    return NULL;
  return d_citysetids[id];
}
	
Cityset *Citysetlist::getCityset(std::string dir) 
{ 
  if (d_citysets.find(dir) == d_citysets.end())
    return NULL;
  return d_citysets[dir];
}
bool Citysetlist::addToPersonalCollection(Cityset *cityset, std::string &new_subdir, guint32 &new_id)
{
  //do we already have this one?
  if (getCityset(cityset->getSubDir()) == getCityset(cityset->getId()) &&
      getCityset(cityset->getSubDir()) != NULL)
    {
      cityset->setDirectory(getCityset(cityset->getId())->getDirectory());
      return true;
    }

  //if the subdir conflicts with any other subdir, then change it.
  if (getCityset(cityset->getSubDir()) != NULL)
    {
      bool found = false;
      for (int count = 0; count < 100; count++)
	{
	  new_subdir = String::ucompose("%1%2", cityset->getSubDir(), count);
	  if (getCityset(new_subdir) == NULL)
	    {
	      found = true;
	      break;
	    }
	}
      if (found == false)
	return false;
      cityset->setSubDir(new_subdir);
    }
  else
    new_subdir = cityset->getSubDir();

  //if the id conflicts with any other id, then change it
  if (getCityset(cityset->getId()) != NULL)
    {
      new_id = Citysetlist::getNextAvailableId(cityset->getId());
      cityset->setId(new_id);
    }
  else
    new_id = cityset->getId();

  //make the directory where the cityset is going to live.
  std::string directory = 
    File::getUserCitysetDir() + cityset->getSubDir() + "/";

  if (File::create_dir(directory) == false)
    return false;

  //okay now we copy the image files into the new directory 
  std::list<std::string> files;
  cityset->getFilenames(files);
  for (std::list<std::string>::iterator it = files.begin(); it != files.end();
       it++)
    File::copy(cityset->getFile(*it), directory + *it);

  //save out the cityset file
  cityset->setDirectory(directory);
  XML_Helper helper(cityset->getConfigurationFile(), std::ios::out, false);
  cityset->save(&helper);
  helper.close();
  return true;
}

int Citysetlist::getNextAvailableId(int after)
{
  std::list<guint32> ids;
  std::list<std::string> citysets = Cityset::scanSystemCollection();
  //there might be IDs in invalid armysets.
  for (std::list<std::string>::const_iterator i = citysets.begin(); 
       i != citysets.end(); i++)
    {
      Cityset *cityset = Cityset::create(*i);
      if (cityset != NULL)
	{
	  ids.push_back(cityset->getId());
	  delete cityset;
	}
    }
  citysets = Cityset::scanUserCollection();
  for (std::list<std::string>::const_iterator i = citysets.begin(); 
       i != citysets.end(); i++)
    {
      Cityset *cityset = Cityset::create(*i);
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
