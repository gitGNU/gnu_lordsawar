//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <iostream>
#include <expat.h>
#include <SDL_image.h>
#include <SDL.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "armysetlist.h"
#include "armyset.h"
#include "File.h"
#include "defs.h"



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

Armysetlist::Armysetlist()
{
    // load all armysets
    std::list<std::string> armysets = File::scanArmysets();

    for (std::list<std::string>::const_iterator i = armysets.begin(); 
	 i != armysets.end(); i++)
      {
        loadArmyset(*i);
	iterator it = end();
	it--;
	for (Armyset::iterator ait = (*it)->begin(); ait != (*it)->end(); ait++)
	  d_armies[(*it)->getId()].push_back(*ait);
	d_names[(*it)->getId()] = (*it)->getName();
	d_ids[(*it)->getName()] = (*it)->getId();
	(*it)->setSubDir(*i);
      }
}

Armysetlist::~Armysetlist()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);

    // remove all army entries
    for (ArmyMap::iterator it = d_armies.begin(); it != d_armies.end(); it++)
        while (!(*it).second.empty())
            delete ((*it).second)[0];
}

Army* Armysetlist::getArmy(Uint32 id, Uint32 index) const
{
    // always use ArmyMap::find for searching, else a default entry is created,
    // which can produce really bad results!!
    ArmyMap::const_iterator it = d_armies.find(id);

    // armyset does not exist
    if (it == d_armies.end())
        return 0;

    // index too large
    if (index >= (*it).second.size())
        return 0;

    return ((*it).second)[index];
}

Uint32 Armysetlist::getSize(Uint32 id) const
{
    ArmyMap::const_iterator it = d_armies.find(id);

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

std::string Armysetlist::getName(Uint32 id) const
{
    NameMap::const_iterator it = d_names.find(id);

    // armyset does not exist
    if (it == d_names.end())
        return 0;

    return (*it).second;
}

std::vector<Uint32> Armysetlist::getArmysets() const
{
    std::vector<Uint32> retlist;
    
    NameMap::const_iterator it;
    for (it = d_names.begin(); it != d_names.end(); it++)
    {
        retlist.push_back((*it).first);
    }

    return retlist;
}

bool Armysetlist::load(std::string tag, XML_Helper *helper)
{
  if (tag == "armyset")
    {
      Armyset *armyset = new Armyset(helper);
      push_back(armyset); 
    }
  return true;
}


bool Armysetlist::loadArmyset(std::string name)
{
  debug("Loading armyset " <<name);

  XML_Helper helper(File::getArmyset(name), ios::in, false);

  helper.registerTag("armyset", sigc::mem_fun((*this), &Armysetlist::load));

  if (!helper.parse())
    {
      std::cerr <<_("Error, while loading an armyset. Armyset Name: ");
      std::cerr <<name <<std::endl <<std::flush;
      exit(-1);
    }

  return true;
}
        
void Armysetlist::instantiatePixmaps()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->instantiatePixmaps();
}
	
SDL_Surface * Armysetlist::getShipPic (Uint32 id)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == id)
	return (*it)->getShipPic();
    }
  return NULL;
}

SDL_Surface * Armysetlist::getShipMask (Uint32 id)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == id)
	return (*it)->getShipMask();
    }
  return NULL;
}

Uint32 Armysetlist::getTileSize(Uint32 id)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == id)
	return (*it)->getTileSize();
    }
  return 0;
}

SDL_Surface * Armysetlist::getStandardPic (Uint32 id)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == id)
	return (*it)->getStandardPic();
    }
  return NULL;
}

SDL_Surface * Armysetlist::getStandardMask (Uint32 id)
{
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == id)
	return (*it)->getStandardMask();
    }
  return NULL;
}

