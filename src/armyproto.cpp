// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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
#include "xmlhelper.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)
std::string ArmyProto::d_tag = "armyproto";

ArmyProto::ArmyProto(const ArmyProto& a)
    :ArmyProtoBase(a),
    d_image(0), d_mask(0),
     d_defends_ruins(a.d_defends_ruins), 
     d_awardable(a.d_awardable), d_image_name(a.d_image_name),
     d_hero(a.d_hero)
{
}

ArmyProto::ArmyProto()
  :ArmyProtoBase(), d_image(0), d_mask(0), 
    d_defends_ruins(false), d_awardable(false), d_image_name(""),
    d_hero(false)
{
}

ArmyProto::ArmyProto(XML_Helper* helper)
  :ArmyProtoBase(helper), d_image(0), d_mask(0), 
    d_defends_ruins(false), d_awardable(false), d_image_name(""),
    d_hero(false)
{

  helper->getData(d_image_name, "image");
  helper->getData(d_defends_ruins,"defends_ruins");
  helper->getData(d_awardable,"awardable");
  helper->getData(d_hero, "hero");
}

ArmyProto::~ArmyProto()
{
}

bool ArmyProto::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(d_tag);

  retval &= saveData(helper);

  retval &= helper->closeTag();

  return retval;
}

bool ArmyProto::saveData(XML_Helper* helper) const
{
  bool retval = true;

  retval &= ArmyProtoBase::saveData(helper);
  retval &= helper->saveData("image", d_image_name);
  retval &= helper->saveData("awardable", d_awardable);
  retval &= helper->saveData("defends_ruins", d_defends_ruins);
  retval &= helper->saveData("hero", d_hero);

  return retval;
}
