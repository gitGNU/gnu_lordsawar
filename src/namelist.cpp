//  Copyright (C) 2009, Ben Asselstine
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
#include <sigc++/functors/mem_fun.h>

#include "namelist.h"
#include "defs.h"
#include "File.h"
#include "ucompose.hpp"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)


NameList::NameList(Glib::ustring filename, Glib::ustring item_tag)
{
  d_item_tag = item_tag;
  XML_Helper helper(File::getMiscFile(filename), std::ios::in, false);

  helper.registerTag(d_item_tag, sigc::mem_fun((*this), &NameList::load));

  if (!helper.parse())
    {
      std::cerr << String::ucompose(_("Error can't load namelist `%1'"), filename) << std::endl;
      exit(-1);
    }

  return;
}

NameList::~NameList()
{
}

bool NameList::load(Glib::ustring tag, XML_Helper *helper)
{
  if (tag == d_item_tag)
    {
      Glib::ustring name;
      helper->getData(name, "name");
      push_back(name); 
    }
  return true;
}

Glib::ustring NameList::popRandomName()
{
  Glib::ustring name;
  if (empty())
    return "";
  int randno = rand() % size();
  name = (*this)[randno];

  NameList::iterator it = std::find(begin(), end(), name);
  if (it != end())
    erase(it);

  return name;
}

