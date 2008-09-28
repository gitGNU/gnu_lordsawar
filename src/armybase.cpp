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
#include "ucompose.hpp"
#include "armybase.h"
#include "xmlhelper.h"
#include "Tile.h"
#include "defs.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

ArmyBase::ArmyBase(const ArmyBase& a)
: d_upkeep(a.d_upkeep), d_strength(a.d_strength), d_max_moves(a.d_max_moves), 
    d_sight(a.d_sight), d_move_bonus(a.d_move_bonus),
    d_army_bonus(a.d_army_bonus), d_xp_value(a.d_xp_value)
{
}

ArmyBase::ArmyBase()
  : d_upkeep(0), 
    d_strength(0), d_max_moves(0), d_sight(0), 
    d_move_bonus(0), d_army_bonus(0), d_xp_value(0.0)
{
}

ArmyBase::ArmyBase(XML_Helper* helper)
{
  helper->getData(d_upkeep, "upkeep");
  std::string move_bonus_str;
  helper->getData(move_bonus_str, "move_bonus");
  d_move_bonus = moveFlagsFromString(move_bonus_str);
  std::string army_bonus_str;
  helper->getData(army_bonus_str, "army_bonus");
  d_army_bonus = bonusFlagsFromString(army_bonus_str);

  helper->getData(d_max_moves, "max_moves");
  helper->getData(d_strength, "strength");
  helper->getData(d_sight, "sight");
  helper->getData(d_xp_value, "expvalue");
}

ArmyBase::~ArmyBase()
{
}

bool ArmyBase::saveData(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->saveData("upkeep", d_upkeep);
  std::string move_bonus_str = moveFlagsToString(d_move_bonus);
  retval &= helper->saveData("move_bonus", move_bonus_str);
  std::string army_bonus_str = bonusFlagsToString(d_army_bonus);
  retval &= helper->saveData("army_bonus", army_bonus_str);
  retval &= helper->saveData("max_moves", d_max_moves);
  retval &= helper->saveData("strength", d_strength);
  retval &= helper->saveData("sight", d_sight);
  retval &= helper->saveData("expvalue", d_xp_value);
  return retval;
}

std::string ArmyBase::getArmyBonusDescription() const
{
  Uint32 bonus = d_army_bonus;
  Glib::ustring s = "";
  if (bonus & ArmyBase::ADD1STRINOPEN)
    s += String::ucompose(_("%1%2"), s == "" ? " " : "& ", 
			  _("+1 str in open"));
  if (bonus & ArmyBase::ADD2STRINOPEN)
    s += String::ucompose(_("%1%2"), s == "" ? " " : "& ", 
			  _("+2 str in open"));
  if (bonus & ArmyBase::ADD1STRINFOREST)
    s += String::ucompose(_("%1%2"), s == "" ? " " : "& ", 
			  _("+1 str in woods"));
  if (bonus & ArmyBase::ADD1STRINHILLS)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("+1 str in hills"));
  if (bonus & ArmyBase::ADD1STRINCITY)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("+1 str in city"));
  if (bonus & ArmyBase::ADD2STRINCITY)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("+2 str in city"));
  if (bonus & ArmyBase::ADD1STACKINHILLS)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("+1 stack in hills"));
  if (bonus & ArmyBase::SUBALLCITYBONUS)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("Cancel city bonus"));
  if (bonus & ArmyBase::SUB1ENEMYSTACK)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("-1 enemy stack"));
  if (bonus & ArmyBase::ADD1STACK)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", _("+1 stack"));
  if (bonus & ArmyBase::ADD2STACK)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", _("+2 stack"));
  if (bonus & ArmyBase::SUBALLNONHEROBONUS)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("cancel non-hero"));
  if (bonus & ArmyBase::SUBALLHEROBONUS)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("cancel hero"));
  return s;
}

std::string ArmyBase::moveFlagsToString(const Uint32 bonus)
{
  std::string move_bonuses;
  //we don't add grass, because it's always implied.
  if (bonus & Tile::WATER)
    move_bonuses += " " + Tile::tileTypeToString(Tile::WATER);
  if (bonus & Tile::FOREST)
    move_bonuses += " " + Tile::tileTypeToString(Tile::FOREST);
  if (bonus & Tile::HILLS)
    move_bonuses += " " + Tile::tileTypeToString(Tile::HILLS);
  if (bonus & Tile::MOUNTAIN)
    move_bonuses += " " + Tile::tileTypeToString(Tile::MOUNTAIN);
  if (bonus & Tile::SWAMP)
    move_bonuses += " " + Tile::tileTypeToString(Tile::SWAMP);
  if (bonus & Tile::VOID)
    move_bonuses += " " + Tile::tileTypeToString(Tile::VOID);
  return move_bonuses;
}

Uint32 ArmyBase::moveFlagsFromString(const std::string str)
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
      total += Tile::tileTypeFromString(bonus);
    }
  return total;
}

std::string ArmyBase::bonusFlagToString(const ArmyBase::Bonus bonus)
{
  switch (bonus)
    {
    case ArmyBase::ADD1STRINOPEN:
      return "ArmyBase::ADD1STRINOPEN";
    case ArmyBase::ADD2STRINOPEN:
      return "ArmyBase::ADD2STRINOPEN";
    case ArmyBase::ADD1STRINFOREST:
      return "ArmyBase::ADD1STRINFOREST";
    case ArmyBase::ADD1STRINHILLS:
      return "ArmyBase::ADD1STRINHILLS";
    case ArmyBase::ADD1STRINCITY:
      return "ArmyBase::ADD1STRINCITY";
    case ArmyBase::ADD2STRINCITY:
      return "ArmyBase::ADD2STRINCITY";
    case ArmyBase::ADD1STACKINHILLS:
      return "ArmyBase::ADD1STACKINHILLS";
    case ArmyBase::SUBALLCITYBONUS:
      return "ArmyBase::SUBALLCITYBONUS";
    case ArmyBase::SUB1ENEMYSTACK:
      return "ArmyBase::SUB1ENEMYSTACK";
    case ArmyBase::ADD1STACK:
      return "ArmyBase::ADD1STACK";
    case ArmyBase::ADD2STACK:
      return "ArmyBase::ADD2STACK";
    case ArmyBase::SUBALLNONHEROBONUS:
      return "ArmyBase::SUBALLNONHEROBONUS";
    case ArmyBase::SUBALLHEROBONUS:
      return "ArmyBase::SUBALLHEROBONUS";
    case ArmyBase::FORTIFY:
      return "ArmyBase::FORTIFY";
    }
  return "";
}

std::string ArmyBase::bonusFlagsToString(const Uint32 bonus)
{
  std::string bonuses;
  if (bonus & ArmyBase::ADD1STRINOPEN)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD1STRINOPEN);
  if (bonus & ArmyBase::ADD2STRINOPEN)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD2STRINOPEN);
  if (bonus & ArmyBase::ADD1STRINFOREST)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD1STRINFOREST);
  if (bonus & ArmyBase::ADD1STRINHILLS)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD1STRINHILLS);
  if (bonus & ArmyBase::ADD1STRINCITY)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD1STRINCITY);
  if (bonus & ArmyBase::ADD2STRINCITY)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD2STRINCITY);
  if (bonus & ArmyBase::ADD1STACKINHILLS)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD1STACKINHILLS);
  if (bonus & ArmyBase::SUBALLCITYBONUS)
    bonuses += " " + bonusFlagToString(ArmyBase::SUBALLCITYBONUS);
  if (bonus & ArmyBase::SUB1ENEMYSTACK)
    bonuses += " " + bonusFlagToString(ArmyBase::SUB1ENEMYSTACK);
  if (bonus & ArmyBase::ADD1STACK)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD1STACK);
  if (bonus & ArmyBase::ADD2STACK)
    bonuses += " " + bonusFlagToString(ArmyBase::ADD2STACK);
  if (bonus & ArmyBase::SUBALLNONHEROBONUS)
    bonuses += " " + bonusFlagToString(ArmyBase::SUBALLNONHEROBONUS);
  if (bonus & ArmyBase::SUBALLHEROBONUS)
    bonuses += " " + bonusFlagToString(ArmyBase::SUBALLHEROBONUS);
  if (bonus & ArmyBase::FORTIFY)
    bonuses += " " + bonusFlagToString(ArmyBase::FORTIFY);
  return bonuses;
}

Uint32 ArmyBase::bonusFlagsFromString(const std::string str)
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

ArmyBase::Bonus ArmyBase::bonusFlagFromString(const std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return ArmyBase::Bonus(atoi(str.c_str()));
  if (str == "ArmyBase::ADD1STRINOPEN")
    return ArmyBase::ADD1STRINOPEN;
  else if (str == "ArmyBase::ADD2STRINOPEN")
    return ArmyBase::ADD2STRINOPEN;
  else if (str == "ArmyBase::ADD1STRINFOREST")
    return ArmyBase::ADD1STRINFOREST;
  else if (str == "ArmyBase::ADD1STRINHILLS")
    return ArmyBase::ADD1STRINHILLS;
  else if (str == "ArmyBase::ADD1STRINCITY")
    return ArmyBase::ADD1STRINCITY;
  else if (str == "ArmyBase::ADD2STRINCITY")
    return ArmyBase::ADD2STRINCITY;
  else if (str == "ArmyBase::ADD1STACKINHILLS")
    return ArmyBase::ADD1STACKINHILLS;
  else if (str == "ArmyBase::SUBALLCITYBONUS")
    return ArmyBase::SUBALLCITYBONUS;
  else if (str == "ArmyBase::ADD2GOLDPERCITY")
    return ArmyBase::SUB1ENEMYSTACK;
  else if (str == "ArmyBase::SUB1ENEMYSTACK")
    return ArmyBase::ADD1STACK;
  else if (str == "ArmyBase::ADD1STACK")
    return ArmyBase::ADD2STACK;
  else if (str == "ArmyBase::ADD2STACK")
    return ArmyBase::ADD2STACK;
  else if (str == "ArmyBase::SUBALLNONHEROBONUS")
    return ArmyBase::SUBALLNONHEROBONUS;
  else if (str == "ArmyBase::SUBALLHEROBONUS")
    return ArmyBase::SUBALLHEROBONUS;
  return ArmyBase::ADD1STRINOPEN;
}
