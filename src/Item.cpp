// Copyright (C) 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 Andrea Paternesi
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

#include <sstream>
#include <map>
#include "Item.h"
#include "File.h"
#include "counter.h"
#include "playerlist.h"
#include "ucompose.hpp"

using namespace std;

Item::Item(XML_Helper* helper)
	: Renamable (helper)
{
    
    // Loading of items is a bit complicated, so i'd better loose some words.
    // In general, items can be loaded from the items description file or
    // from a savegame. They both differ a bit, more on that when we encounter
    // such a situation. First, let us deal with the common things.

    std::string bonus_str;
    helper->getData(bonus_str, "bonus");
    d_bonus = bonusFlagsFromString(bonus_str);
    
    helper->getData(d_plantable, "plantable");
    if (d_plantable)
      {
        helper->getData(d_plantable_owner_id, "plantable_owner");
        helper->getData(d_planted, "planted");
      }
    else
      {
	d_plantable_owner_id = MAX_PLAYERS;
	d_planted = false;
      }

    // Now come the differences. Items in the game have an id, other items
    // just the dummy id "0"
    helper->getData(d_id, "id");

}

Item::Item(std::string name, bool plantable, Player *plantable_owner)
	: Renamable(name)
{
  d_bonus = 0;
  d_plantable = plantable;
  d_plantable_owner_id = plantable_owner->getId();
  d_planted = false;
  d_id = fl_counter->getNextId();
  //std::cerr << "item created with id " << d_id << std::endl;
}

Item::Item(const Item& orig)
:Renamable(orig), d_bonus(orig.d_bonus), d_plantable(orig.d_plantable),
    d_plantable_owner_id(orig.d_plantable_owner_id), d_planted(orig.d_planted)
{
  // Some things we don't copy from the template; we rather get an own ID
  d_id = fl_counter->getNextId();
  //std::cerr << "item created with id " << d_id << std::endl;
}

Item::~Item()
{
}

bool Item::save(XML_Helper* helper) const
{
  bool retval = true;

  // A template is never saved, so we assume this class is a real-life item
  retval &= helper->openTag("item");
  retval &= helper->saveData("name", getName());
  retval &= helper->saveData("plantable", d_plantable);
  if (d_plantable)
    {
      retval &= helper->saveData("plantable_owner", d_plantable_owner_id);
      retval &= helper->saveData("planted", d_planted);
    }
  retval &= helper->saveData("id", d_id);

  std::string bonus_str = bonusFlagsToString(d_bonus);
  retval &= helper->saveData("bonus", bonus_str);

  retval &= helper->closeTag();

  return retval;
}

bool Item::getBonus(Item::Bonus bonus) const
{
  return (d_bonus & bonus) == 0 ? false : true;
}

void Item::addBonus(Item::Bonus bonus)
{
  d_bonus |= bonus;
}

void Item::removeBonus(Item::Bonus bonus)
{
  d_bonus ^= bonus;
}

std::string Item::getBonusDescription() const
{
  // the attributes column
  std::vector<Glib::ustring> s;
  if (getBonus(Item::ADD1STR))
    s.push_back(_("+1 Battle"));
  if (getBonus(Item::ADD2STR))
    s.push_back(_("+2 Battle"));
  if (getBonus(Item::ADD3STR))
    s.push_back(_("+3 Battle"));
  if (getBonus(Item::ADD1STACK))
    s.push_back(_("+1 Command"));
  if (getBonus(Item::ADD2STACK))
    s.push_back(_("+2 Command"));
  if (getBonus(Item::ADD3STACK))
    s.push_back(_("+3 Command"));
  if (getBonus(Item::FLYSTACK))
    s.push_back(_("Allows Flight"));
  if (getBonus(Item::DOUBLEMOVESTACK))
    s.push_back(_("Doubles Movement"));
  if (getBonus(Item::ADD2GOLDPERCITY))
    s.push_back(_("+2 gold per city"));
  if (getBonus(Item::ADD3GOLDPERCITY))
    s.push_back(_("+3 gold per city"));
  if (getBonus(Item::ADD4GOLDPERCITY))
    s.push_back(_("+4 gold per city"));
  if (getBonus(Item::ADD5GOLDPERCITY))
    s.push_back(_("+5 gold per city"));

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

std::string Item::bonusFlagToString(Item::Bonus bonus)
{
  switch (bonus)
    {
    case Item::ADD1STR:
      return "Item::ADD1STR";
    case Item::ADD2STR:
      return "Item::ADD2STR";
    case Item::ADD3STR:
      return "Item::ADD3STR";
    case Item::ADD1STACK:
      return "Item::ADD1STACK";
    case Item::ADD2STACK:
      return "Item::ADD2STACK";
    case Item::ADD3STACK:
      return "Item::ADD3STACK";
    case Item::FLYSTACK:
      return "Item::FLYSTACK";
    case Item::DOUBLEMOVESTACK:
      return "Item::DOUBLEMOVESTACK";
    case Item::ADD2GOLDPERCITY:
      return "Item::ADD2GOLDPERCITY";
    case Item::ADD3GOLDPERCITY:
      return "ADD3GOLDPERCITY";
    case Item::ADD4GOLDPERCITY:
      return "Item::ADD4GOLDPERCITY";
    case Item::ADD5GOLDPERCITY:
      return "Item::ADD5GOLDPERCITY";
    }
  return "Item::ADD1STR";
}

std::string Item::bonusFlagsToString(Uint32 bonus)
{
  std::string bonuses;
  if (bonus & Item::ADD1STR)
    bonuses += " " + bonusFlagToString(Item::ADD1STR);
  if (bonus & Item::ADD2STR)
    bonuses += " " + bonusFlagToString(Item::ADD2STR);
  if (bonus & Item::ADD3STR)
    bonuses += " " + bonusFlagToString(Item::ADD3STR);
  if (bonus & Item::ADD1STACK)
    bonuses += " " + bonusFlagToString(Item::ADD1STACK);
  if (bonus & Item::ADD2STACK)
    bonuses += " " + bonusFlagToString(Item::ADD2STACK);
  if (bonus & Item::ADD3STACK)
    bonuses += " " + bonusFlagToString(Item::ADD3STACK);
  if (bonus & Item::FLYSTACK)
    bonuses += " " + bonusFlagToString(Item::FLYSTACK);
  if (bonus & Item::DOUBLEMOVESTACK)
    bonuses += " " + bonusFlagToString(Item::DOUBLEMOVESTACK);
  if (bonus & Item::ADD2GOLDPERCITY)
    bonuses += " " + bonusFlagToString(Item::ADD2GOLDPERCITY);
  if (bonus & Item::ADD3GOLDPERCITY)
    bonuses += " " + bonusFlagToString(Item::ADD3GOLDPERCITY);
  if (bonus & Item::ADD4GOLDPERCITY)
    bonuses += " " + bonusFlagToString(Item::ADD4GOLDPERCITY);
  if (bonus & Item::ADD5GOLDPERCITY)
    bonuses += " " + bonusFlagToString(Item::ADD5GOLDPERCITY);
  return bonuses;
}

Uint32 Item::bonusFlagsFromString(std::string str)
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

Item::Bonus Item::bonusFlagFromString(std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Item::Bonus(atoi(str.c_str()));
  if (str == "Item::ADD1STR")
    return Item::ADD1STR;
  else if (str == "Item::ADD2STR")
    return Item::ADD2STR;
  else if (str == "Item::ADD3STR")
    return Item::ADD3STR;
  else if (str == "Item::ADD1STACK")
    return Item::ADD1STACK;
  else if (str == "Item::ADD2STACK")
    return Item::ADD2STACK;
  else if (str == "Item::ADD3STACK")
    return Item::ADD3STACK;
  else if (str == "Item::FLYSTACK")
    return Item::FLYSTACK;
  else if (str == "Item::DOUBLEMOVESTACK")
    return Item::DOUBLEMOVESTACK;
  else if (str == "Item::ADD2GOLDPERCITY")
    return Item::ADD2GOLDPERCITY;
  else if (str == "Item::ADD3GOLDPERCITY")
    return Item::ADD3GOLDPERCITY;
  else if (str == "Item::ADD4GOLDPERCITY")
    return Item::ADD4GOLDPERCITY;
  else if (str == "Item::ADD5GOLDPERCITY")
    return Item::ADD5GOLDPERCITY;
  return Item::ADD1STR;
}
