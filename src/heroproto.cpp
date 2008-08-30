// Copyright (C) 2008 Ben Asselstine
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
#include <sstream>
#include <algorithm>
#include "armyproto.h"
#include "heroproto.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

HeroProto::HeroProto(const HeroProto& a)
    :ArmyProto(a), d_gender(a.d_gender)
{
}

HeroProto::HeroProto(const ArmyProto& a)
    :ArmyProto(a), d_gender(Hero::NONE)
{
}

HeroProto::HeroProto()
  :ArmyProto(), d_gender(Hero::NONE)
{
}

HeroProto::~HeroProto()
{
}

HeroProto::HeroProto(XML_Helper* helper)
  :ArmyProto(helper)
{
  std::string gender_str;
  if (!helper->getData(gender_str, "gender"))
    d_gender = Hero::NONE;
  else
    d_gender = Hero::genderFromString(gender_str);
}

bool HeroProto::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("heroproto");

  retval &= ArmyProto::saveData(helper);
  std::string gender_str = Hero::genderToString(Hero::Gender(d_gender));
  retval &= helper->saveData("gender", gender_str);

  retval &= helper->closeTag();

  return retval;
}

