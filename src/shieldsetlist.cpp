//  Copyright (C) 2008, 2009, 2010, 2011, 2014, 2015 Ben Asselstine
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

#include "shieldsetlist.h"
#include "shieldset.h"
#include "File.h"
#include "defs.h"
#include "ucompose.hpp"
#include "tarhelper.h"
#include "setlist.h"

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

Shieldsetlist::Shieldsetlist()
 : SetList(Shieldset::file_extension)
{
  loadSets(SetList::scan(Shieldset::file_extension));
  loadSets(SetList::scan(Shieldset::file_extension, false));
}

Shieldsetlist::~Shieldsetlist()
{
  uninstantiateImages();
  for (iterator it = begin(); it != end(); it++)
    delete *it;
  clear();
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

Gdk::RGBA Shieldsetlist::getColor(guint32 shieldset, guint32 owner) const
{
  Shieldset *s = get(shieldset);
  if (!s)
    return Gdk::RGBA("black");
  return s->getColor(owner);
}

ShieldStyle *Shieldsetlist::getShield(guint32 shieldset, guint32 type, guint32 colour) const
{
  Shieldset *s = get(shieldset);
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
