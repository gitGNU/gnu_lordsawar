// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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

#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <sigc++/functors/mem_fun.h>

#include "hero.h"
#include "stacklist.h"
#include "templelist.h"
#include "heroproto.h"
#include "counter.h"
#include "Backpack.h"
#include "xmlhelper.h"

std::string Hero::d_tag = "hero";
using namespace std;

Hero::Hero(const HeroProto& a)
  : Army (dynamic_cast<const ArmyProto&>(a)), d_name(a.getName()),
    d_gender(Gender(a.getGender()))
{
  d_level = 1;
  d_backpack = new Backpack();
}

Hero::Hero(Hero& h)
  : Army(h, h.d_owner), d_name(h.d_name), d_gender(h.d_gender)
{
  d_backpack = new Backpack(*h.d_backpack);
}

Hero::Hero(XML_Helper* helper)
    :Army(helper)
{
  helper->getData(d_name, "name");
  std::string gender_str;
  if (!helper->getData(gender_str, "gender"))
    d_gender = NONE;
  else
    d_gender = genderFromString(gender_str);
  helper->registerTag(Backpack::d_tag, 
		      sigc::mem_fun(*this, &Hero::loadBackpack));
}


Hero::~Hero()
{
  delete d_backpack;
}

bool Hero::save(XML_Helper* helper) const
{
    bool retval = true;
    std::list<Item*>::const_iterator it;

    retval &= helper->openTag(Hero::d_tag);

    retval &= helper->saveData("name", d_name);
    std::string gender_str = genderToString(Hero::Gender(d_gender));
    retval &= helper->saveData("gender", gender_str);
    retval &= saveData(helper);

    // Now save the backpack
    retval &= d_backpack->save(helper);

    retval &= helper->closeTag();

    return retval;
}

bool Hero::loadBackpack(std::string tag, XML_Helper* helper)
{
  if (tag == Backpack::d_tag)
    {
      d_backpack = new Backpack(helper);
      return true;
    }
  return false;
}

guint32 Hero::getStat(Stat stat, bool modified) const
{
    guint32 bonus = 0;
    guint32 value = Army::getStat(stat, modified);

    if (!modified)
        return value;

    // Add item bonuses that affect only this hero
    if (stat == Army::STRENGTH)
      bonus += d_backpack->countStrengthBonuses();

    return value + bonus;
}

guint32 Hero::calculateNaturalCommand()
{
  guint32 command = 0;
  guint32 strength = getStat(Army::STRENGTH, true);
  if (strength == 9)
    command += 3;
  else if (strength > 6)
    command += 2;
  else if (strength > 3)
    command += 1;
  return command;
}


std::string Hero::genderToString(const Hero::Gender gender)
{
  switch (gender)
    {
    case Hero::NONE:
      return "Hero::NONE";
      break;
    case Hero::MALE:
      return "Hero::MALE";
      break;
    case Hero::FEMALE:
      return "Hero::FEMALE";
      break;
    }
  return "Hero::FEMALE";
}

Hero::Gender Hero::genderFromString(const std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Hero::Gender(atoi(str.c_str()));
  if (str == "Hero::MALE")
    return Hero::MALE;
  else if (str == "Hero::NONE")
    return Hero::NONE;
  else if (str == "Hero::FEMALE")
    return Hero::FEMALE;
  return Hero::FEMALE;
}
