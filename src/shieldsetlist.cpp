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

#include "shieldsetlist.h"
#include "shieldset.h"
#include "File.h"
#include "defs.h"



using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

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

Shieldsetlist::Shieldsetlist()
{
    // load all shieldsets
    std::list<std::string> shieldsets = File::scanShieldsets();

    for (std::list<std::string>::const_iterator i = shieldsets.begin(); 
	 i != shieldsets.end(); i++)
      {
        loadShieldset(*i);
	iterator it = end();
	it--;
	//for (Shieldset::iterator ait = (*it)->begin(); ait != (*it)->end(); ait++)
	d_shieldsets[(*it)->getId()] = *it;
	d_names[(*it)->getId()] = (*it)->getName();
	d_ids[(*it)->getName()] = (*it)->getId();
	(*it)->setSubDir(*i);
      }
}

Shieldsetlist::~Shieldsetlist()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);
}

Shield* Shieldsetlist::getShield(Uint32 id, Uint32 type, Uint32 colour) const
{
    ShieldsetMap::const_iterator it = d_shieldsets.find(id);
	
    if (it == d_shieldsets.end())
        return 0;

    return (*it).second->lookupShieldByTypeAndColour(type, colour);
}

Uint32 Shieldsetlist::getSize(Uint32 id) const
{
    ShieldsetMap::const_iterator it = d_shieldsets.find(id);

    // shieldset does not exist
    if (it == d_shieldsets.end())
        return 0;

    return (*it).second->getSize();
}

std::list<std::string> Shieldsetlist::getNames()
{
  std::list<std::string> names;
  for (iterator it = begin(); it != end(); it++)
    names.push_back((*it)->getName());
  return names;
}

std::string Shieldsetlist::getName(Uint32 id) const
{
    NameMap::const_iterator it = d_names.find(id);

    // shieldset does not exist
    if (it == d_names.end())
        return 0;

    return (*it).second;
}

Shieldset * Shieldsetlist::getShieldset(std::string name)
{
  Uint32 id = getShieldsetId(name);
  if (id == 0)
    return NULL;
  ShieldsetMap::iterator sit = d_shieldsets.find(id);
  if (sit == d_shieldsets.end())
    return 0;
  return (*sit).second;
}

std::vector<Uint32> Shieldsetlist::getShieldsets() const
{
    std::vector<Uint32> retlist;
    
    NameMap::const_iterator it;
    for (it = d_names.begin(); it != d_names.end(); it++)
    {
        retlist.push_back((*it).first);
    }

    return retlist;
}

bool Shieldsetlist::load(std::string tag, XML_Helper *helper)
{
  if (tag == "shieldset")
    {
      Shieldset *shieldset = new Shieldset(helper);
      push_back(shieldset); 
    }
  return true;
}


bool Shieldsetlist::loadShieldset(std::string name)
{
  debug("Loading shieldset " <<name);

  XML_Helper helper(File::getShieldset(name), ios::in, false);

  helper.registerTag("shieldset", sigc::mem_fun((*this), &Shieldsetlist::load));

  if (!helper.parse())
    {
      std::cerr <<_("Error, while loading an shieldset. Shieldset Name: ");
      std::cerr <<name <<std::endl <<std::flush;
      exit(-1);
    }

  return true;
}
        
void Shieldsetlist::instantiatePixmaps()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->instantiatePixmaps();
}
	
