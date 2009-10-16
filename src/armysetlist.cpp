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
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "armysetlist.h"
#include "armyset.h"
#include "File.h"
#include "defs.h"
#include "ucompose.hpp"
#include "PixMask.h"


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

void Armysetlist::loadArmysets(std::list<std::string> armysets, 
			  bool private_collection)
{
    for (std::list<std::string>::const_iterator i = armysets.begin(); 
	 i != armysets.end(); i++)
      {
        bool valid = loadArmyset(*i, private_collection);
	if (!valid)
	  continue;

	iterator it = end();
	it--;
	//fill out the maps
	for (Armyset::iterator ait = (*it)->begin(); ait != (*it)->end(); ait++)
	  d_armies[(*it)->getId()].push_back(*ait);
	d_names[(*it)->getId()] = (*it)->getName();
	d_ids[String::ucompose("%1 %2", (*it)->getName(), (*it)->getTileSize())] = (*it)->getId();
	(*it)->setSubDir(*i);
	d_armysets[*i] = *it;
	d_armysetids[(*it)->getId()] = *it;
      }
}

Armysetlist::Armysetlist()
{
    // load all armysets
    std::list<std::string> armysets = File::scanArmysets();
    loadArmysets(armysets, false);
    armysets = File::scanUserArmysets();
    loadArmysets(armysets, true);

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

std::vector<guint32> Armysetlist::getArmysets() const
{
    std::vector<guint32> retlist;
    
    NameMap::const_iterator it;
    for (it = d_names.begin(); it != d_names.end(); it++)
    {
        retlist.push_back((*it).first);
    }

    return retlist;
}

bool Armysetlist::load(std::string tag, XML_Helper *helper)
{
  if (tag == Armyset::d_tag)
    {
      Armyset *armyset = new Armyset(helper);
      push_back(armyset); 
    }
  return true;
}


bool Armysetlist::loadArmyset(std::string name, bool private_collection)
{
  debug("Loading armyset " <<name);
  Armyset *armyset = Armyset::create(name, private_collection);
  if (armyset == NULL)
    return false;
  if (armyset->validate() == false)
    {
      cerr << "Error!  armyset: `" << armyset->getName() << 
	      "' is invalid." << endl;
      delete armyset;
      return false;
    }
  if (d_armysetids.find(armyset->getId()) != d_armysetids.end())
    {
      Armyset *a = (*d_armysetids.find(armyset->getId())).second;
      cerr << "Error!  armyset: `" << armyset->getName() << 
        "' shares a duplicate armyset id with `" << File::getArmyset(a) << 
        "'.  Skipping." << endl;
      delete armyset;
      return false;
    }
  push_back(armyset); 
  return true;
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
  std::list<std::string> armysets = File::scanArmysets();
  //there might be IDs in invalid armysets.
  for (std::list<std::string>::const_iterator i = armysets.begin(); 
       i != armysets.end(); i++)
    {
      Armyset *armyset = Armyset::create(*i, false);
      if (armyset != NULL)
	{
	  ids.push_back(armyset->getId());
	  delete armyset;
	}
    }
  armysets = File::scanUserArmysets();
  for (std::list<std::string>::const_iterator i = armysets.begin(); 
       i != armysets.end(); i++)
    {
      Armyset *armyset = Armyset::create(*i, true);
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
