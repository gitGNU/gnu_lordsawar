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

#include <sstream>
#include <vector>
#include "ItemProto.h"
#include "ucompose.hpp"
#include "defs.h"

std::string ItemProto::d_tag = "itemproto";
using namespace std;

ItemProto::ItemProto(XML_Helper* helper)
	: Renamable (helper)
{
    
    // Loading of items is a bit complicated, so i'd better loose some words.
    // In general, items can be loaded from the items description file or
    // from a savegame. They both differ a bit, more on that when we encounter
    // such a situation. First, let us deal with the common things.

    std::string bonus_str;
    helper->getData(bonus_str, "bonus");
    d_bonus = bonusFlagsFromString(bonus_str);

}

ItemProto::ItemProto(std::string name, Uint32 id)
	: Renamable(name)
{
  d_bonus = 0;
  d_type_id = id;
}

ItemProto::ItemProto(const ItemProto& orig)
:Renamable(orig), d_bonus(orig.d_bonus), d_type_id(orig.d_type_id)
{
}

ItemProto::~ItemProto()
{
}

bool ItemProto::save(XML_Helper* helper) const
{
  bool retval = true;

  // A template is never saved, so we assume this class is a real-life item
  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("name", getName(false));
  std::string bonus_str = bonusFlagsToString(d_bonus);
  retval &= helper->saveData("bonus", bonus_str);

  retval &= helper->closeTag();

  return retval;
}

bool ItemProto::getBonus(ItemProto::Bonus bonus) const
{
  return (d_bonus & bonus) == 0 ? false : true;
}

void ItemProto::addBonus(ItemProto::Bonus bonus)
{
  d_bonus |= bonus;
}

void ItemProto::removeBonus(ItemProto::Bonus bonus)
{
  d_bonus ^= bonus;
}

std::string ItemProto::getBonusDescription() const
{
  Uint32 battle = 0;
  Uint32 command = 0;
  Uint32 goldpercity = 0;
  // the attributes column
  std::vector<Glib::ustring> s;
  if (getBonus(ItemProto::ADD1STR))
    battle++;
  if (getBonus(ItemProto::ADD2STR))
    battle+=2;
  if (getBonus(ItemProto::ADD3STR))
    battle+=3;
  if (getBonus(ItemProto::ADD1STACK))
    command++;
  if (getBonus(ItemProto::ADD2STACK))
    command+=2;
  if (getBonus(ItemProto::ADD3STACK))
    command+=3;
  if (getBonus(ItemProto::FLYSTACK))
    s.push_back(_("Allows Flight"));
  if (getBonus(ItemProto::DOUBLEMOVESTACK))
    s.push_back(_("Doubles Movement"));
  if (getBonus(ItemProto::ADD2GOLDPERCITY))
    goldpercity+=2;
  if (getBonus(ItemProto::ADD3GOLDPERCITY))
    goldpercity+=3;
  if (getBonus(ItemProto::ADD4GOLDPERCITY))
    goldpercity+=4;
  if (getBonus(ItemProto::ADD5GOLDPERCITY))
    goldpercity+=5;

  if (battle > 0)
    s.push_back(String::ucompose(_("+%1 Battle"), battle));
  if (command > 0)
    s.push_back(String::ucompose(_("+%1 Command"), command));
  if (goldpercity > 0)
    s.push_back(String::ucompose(_("+%1 gold per city"), goldpercity));

  Glib::ustring str;
  bool first = true;
  for (std::vector<Glib::ustring>::iterator i = s.begin(), end = s.end();
       i != end; ++i)
    {
      if (first)
	first = false;
      else
	str += "\n";
      str += *i;
    }
  return str;
}

std::string ItemProto::bonusFlagToString(ItemProto::Bonus bonus)
{
  switch (bonus)
    {
    case ItemProto::ADD1STR:
      return "ItemProto::ADD1STR";
    case ItemProto::ADD2STR:
      return "ItemProto::ADD2STR";
    case ItemProto::ADD3STR:
      return "ItemProto::ADD3STR";
    case ItemProto::ADD1STACK:
      return "ItemProto::ADD1STACK";
    case ItemProto::ADD2STACK:
      return "ItemProto::ADD2STACK";
    case ItemProto::ADD3STACK:
      return "ItemProto::ADD3STACK";
    case ItemProto::FLYSTACK:
      return "ItemProto::FLYSTACK";
    case ItemProto::DOUBLEMOVESTACK:
      return "ItemProto::DOUBLEMOVESTACK";
    case ItemProto::ADD2GOLDPERCITY:
      return "ItemProto::ADD2GOLDPERCITY";
    case ItemProto::ADD3GOLDPERCITY:
      return "ADD3GOLDPERCITY";
    case ItemProto::ADD4GOLDPERCITY:
      return "ItemProto::ADD4GOLDPERCITY";
    case ItemProto::ADD5GOLDPERCITY:
      return "ItemProto::ADD5GOLDPERCITY";
    }
  return "ItemProto::ADD1STR";
}

std::string ItemProto::bonusFlagsToString(Uint32 bonus)
{
  std::string bonuses;
  if (bonus & ItemProto::ADD1STR)
    bonuses += " " + bonusFlagToString(ItemProto::ADD1STR);
  if (bonus & ItemProto::ADD2STR)
    bonuses += " " + bonusFlagToString(ItemProto::ADD2STR);
  if (bonus & ItemProto::ADD3STR)
    bonuses += " " + bonusFlagToString(ItemProto::ADD3STR);
  if (bonus & ItemProto::ADD1STACK)
    bonuses += " " + bonusFlagToString(ItemProto::ADD1STACK);
  if (bonus & ItemProto::ADD2STACK)
    bonuses += " " + bonusFlagToString(ItemProto::ADD2STACK);
  if (bonus & ItemProto::ADD3STACK)
    bonuses += " " + bonusFlagToString(ItemProto::ADD3STACK);
  if (bonus & ItemProto::FLYSTACK)
    bonuses += " " + bonusFlagToString(ItemProto::FLYSTACK);
  if (bonus & ItemProto::DOUBLEMOVESTACK)
    bonuses += " " + bonusFlagToString(ItemProto::DOUBLEMOVESTACK);
  if (bonus & ItemProto::ADD2GOLDPERCITY)
    bonuses += " " + bonusFlagToString(ItemProto::ADD2GOLDPERCITY);
  if (bonus & ItemProto::ADD3GOLDPERCITY)
    bonuses += " " + bonusFlagToString(ItemProto::ADD3GOLDPERCITY);
  if (bonus & ItemProto::ADD4GOLDPERCITY)
    bonuses += " " + bonusFlagToString(ItemProto::ADD4GOLDPERCITY);
  if (bonus & ItemProto::ADD5GOLDPERCITY)
    bonuses += " " + bonusFlagToString(ItemProto::ADD5GOLDPERCITY);
  return bonuses;
}

Uint32 ItemProto::bonusFlagsFromString(std::string str)
{
  Uint32 total = 0;
  std::stringstream bonuses;
  bonuses.str(str);

  while (bonuses.eof() == false)
    {
      std::string bonus;
      bonuses >> bonus;
      if (bonus.size() == 0)
	break;
      total += bonusFlagFromString(bonus);
    }
  return total;
}

ItemProto::Bonus ItemProto::bonusFlagFromString(std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return ItemProto::Bonus(atoi(str.c_str()));
  if (str == "ItemProto::ADD1STR")
    return ItemProto::ADD1STR;
  else if (str == "ItemProto::ADD2STR")
    return ItemProto::ADD2STR;
  else if (str == "ItemProto::ADD3STR")
    return ItemProto::ADD3STR;
  else if (str == "ItemProto::ADD1STACK")
    return ItemProto::ADD1STACK;
  else if (str == "ItemProto::ADD2STACK")
    return ItemProto::ADD2STACK;
  else if (str == "ItemProto::ADD3STACK")
    return ItemProto::ADD3STACK;
  else if (str == "ItemProto::FLYSTACK")
    return ItemProto::FLYSTACK;
  else if (str == "ItemProto::DOUBLEMOVESTACK")
    return ItemProto::DOUBLEMOVESTACK;
  else if (str == "ItemProto::ADD2GOLDPERCITY")
    return ItemProto::ADD2GOLDPERCITY;
  else if (str == "ItemProto::ADD3GOLDPERCITY")
    return ItemProto::ADD3GOLDPERCITY;
  else if (str == "ItemProto::ADD4GOLDPERCITY")
    return ItemProto::ADD4GOLDPERCITY;
  else if (str == "ItemProto::ADD5GOLDPERCITY")
    return ItemProto::ADD5GOLDPERCITY;
  return ItemProto::ADD1STR;
}
