//  Copyright (C) 2008, 2009 Ben Asselstine
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <iostream>
#include <expat.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "shieldsetlist.h"
#include "armyset.h"
#include "File.h"
#include "defs.h"
#include "rgb_shift.h"

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
	(*it)->setSubDir(*i);
	d_dirs[(*it)->getName()] = *i;
	d_shieldsets[*i] = *it;
      }
}

Shieldsetlist::~Shieldsetlist()
{
  for (iterator it = begin(); it != end(); it++)
    delete (*it);
}

std::list<std::string> Shieldsetlist::getNames()
{
  std::list<std::string> names;
  for (iterator it = begin(); it != end(); it++)
    names.push_back((*it)->getName());
  return names;
}

bool Shieldsetlist::load(std::string tag, XML_Helper *helper)
{
  if (tag == Shieldset::d_tag)
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

  helper.registerTag(Shieldset::d_tag, sigc::mem_fun((*this), &Shieldsetlist::load));

  if (!helper.parse())
    {
      std::cerr << "Error, while loading a shieldset. Shieldset Name: ";
      std::cerr <<name <<std::endl <<std::flush;
      exit(-1);
    }

  return true;
}
        
Gdk::Color Shieldsetlist::getColor(std::string shieldset, guint32 owner)
{
  Shieldset *s = getShieldset(shieldset);
  if (!s)
    return Gdk::Color("black");
  return s->getColor(owner);
}

Gdk::Color Shieldsetlist::getMaskColor(std::string shieldset, guint32 owner)
{
  Shieldset *s = getShieldset(shieldset);
  if (!s)
    return Gdk::Color("black");
  return s->getMaskColor(owner);
}
struct rgb_shift Shieldsetlist::getMaskColorShifts(std::string shieldset, guint32 owner)
{
  struct rgb_shift empty;
  empty.r = 0; empty.g = 0; empty.b = 0;
  Shieldset *s = getShieldset(shieldset);
  if (!s)
    return empty;
  return s->getMaskColorShifts(owner);
}
ShieldStyle *Shieldsetlist::getShield(std::string shieldset, guint32 type, guint32 colour)
{
  Shieldset *s = getShieldset(shieldset);
  if (!s)
    return NULL;
  return s->lookupShieldByTypeAndColour(type, colour);
}
