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
#include "File.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)


NameList::NameList(std::string filename, std::string item_tag)
{
  d_item_tag = item_tag;
  XML_Helper helper(File::getMiscFile(filename), ios::in, false);

  helper.registerTag(d_item_tag, sigc::mem_fun((*this), &NameList::load));

  if (!helper.parse())
    {
      std::cerr << "Error, while loading a name from  " << filename <<std::endl <<std::flush;
      exit(-1);
    }

  return;
}

NameList::~NameList()
{
}

bool NameList::load(std::string tag, XML_Helper *helper)
{
  if (tag == d_item_tag)
    {
      std::string name;
      helper->getData(name, "name");
      push_back(name); 
    }
  return true;
}

std::string NameList::popRandomName()
{
  std::string name;
  if (empty())
    return "";
  int randno = rand() % size();
  name = (*this)[randno];
  return name;
}

