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

void Citysetlist::loadCitysets(std::list<std::string> citysets, bool p)
{
    for (std::list<std::string>::const_iterator i = citysets.begin(); 
	 i != citysets.end(); i++)
      {
        if (loadCityset(*i, p) == true)
	  {
	    iterator it = end();
	    it--;
	    if (d_citysetids.find((*it)->getId()) != d_citysetids.end())
	      {
		Cityset *c = (*d_citysetids.find((*it)->getId())).second;
		cerr << "Error!  cityset: `" << (*it)->getName() << 
		  "' has a duplicate cityset id with `" << File::getCityset(c) << 
		  "'.  Skipping." << endl;
		continue;
	      }
	    (*it)->setSubDir(*i);
	    d_dirs[String::ucompose("%1 %2", (*it)->getName(), (*it)->getTileSize())] = *i;
	    d_citysets[*i] = *it;
	    d_citysetids[(*it)->getId()] = *it;
	  }
      }
}

Citysetlist::Citysetlist()
{
    // load all citysets
    std::list<std::string> citysets = File::scanCitysets();
    loadCitysets(citysets, false);
    citysets = File::scanUserCitysets();
    loadCitysets(citysets, true);

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

bool Citysetlist::loadCityset(std::string name, bool p)
{
  debug("Loading cityset " <<name);

  Cityset *cityset = Cityset::create(name, p);
  if (!cityset)
    return false;
  if (d_citysetids.find(cityset->getId()) != d_citysetids.end())
    {
      Cityset *c = (*d_citysetids.find(cityset->getId())).second;
      cerr << "Error!  cityset: `" << cityset->getName() << 
	"' shares a duplicate cityset id with `" << File::getCityset(c) << 
	"'.  Skipping." << endl;
      delete cityset;
      return false;
    }
  push_back(cityset);

  return true;
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
