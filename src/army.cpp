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
#include "army.h"
#include "armyprodbase.h"
#include "armyproto.h"
#include "armysetlist.h"
#include "counter.h"
#include "xmlhelper.h"
#include "stacklist.h"
#include "templelist.h"
#include "ucompose.hpp"
#include "Tile.h"
#include "player.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

sigc::signal<void, Army*> Army::sdying;

Army::Army(const Army& a, Player* p)
    :ArmyBase(a), UniquelyIdentified(a), Ownable(p), d_type_id(a.d_type_id), 
    d_armyset(a.d_armyset), d_max_hp(a.d_max_hp),
     d_max_moves_multiplier(a.d_max_moves_multiplier),
     d_max_moves_rest_bonus(a.d_max_moves_rest_bonus),
     d_ship(a.d_ship), d_hp(a.d_hp), d_moves(a.d_moves), d_xp(a.d_xp),
     d_level(a.d_level), d_grouped(a.d_grouped),
     d_battles_number(a.d_battles_number), d_number_hashit(a.d_number_hashit),
     d_number_hasbeenhit(a.d_number_hasbeenhit), 
     d_visitedTemples(a.d_visitedTemples)
{
  for(int i = 0; i < 3; i++)
    d_medal_bonus[i] = a.d_medal_bonus[i];
}

Army::Army(const ArmyProto& a, Player* p)
    :ArmyBase(a), UniquelyIdentified(), Ownable(p), 
    d_type_id(a.getTypeId()), d_armyset(a.getArmyset()), 
    d_max_hp(2), d_max_moves_multiplier(1), d_max_moves_rest_bonus(0),
    d_ship(false), d_hp(2), d_moves(a.getMaxMoves()), d_xp(0), d_level(0),
    d_grouped(false), d_battles_number(0), d_number_hashit(0), 
    d_number_hasbeenhit(0)
{
  for(int i = 0; i < 3; i++)
    d_medal_bonus[i] = 0;
  d_visitedTemples.clear();
}

Army::Army(const ArmyProto& a, Uint32 id, Player *p)
    :ArmyBase(a), UniquelyIdentified(id), Ownable(p), 
    d_type_id(a.getTypeId()), d_armyset(a.getArmyset()), 
    d_max_hp(2), d_max_moves_multiplier(1), d_max_moves_rest_bonus(0),
    d_ship(false), d_hp(2), d_moves(a.getMaxMoves()), d_xp(0), d_level(0),
    d_grouped(false), d_battles_number(0), d_number_hashit(0), 
    d_number_hasbeenhit(0)
{
  for(int i = 0; i < 3; i++)
    d_medal_bonus[i] = 0;
  d_visitedTemples.clear();
}

Army::Army(const ArmyProdBase& a, Uint32 id, Player *p)
    :ArmyBase(a), UniquelyIdentified(id), Ownable(p), 
    d_type_id(a.getTypeId()), d_armyset(a.getArmyset()), 
    d_max_hp(2), d_max_moves_multiplier(1), d_max_moves_rest_bonus(0),
    d_ship(false), d_hp(2), d_moves(a.getMaxMoves()), d_xp(0), d_level(0),
    d_grouped(false), d_battles_number(0), d_number_hashit(0), 
    d_number_hasbeenhit(0)
{
  for(int i = 0; i < 3; i++)
    d_medal_bonus[i] = 0;
  d_visitedTemples.clear();
}

Army* Army::createNonUniqueArmy(const ArmyProto& a, Player *player)
{
  return new Army(a, (Uint32) 0, player);
}

Army* Army::createNonUniqueArmy(const ArmyProdBase& a, Player *player)
{
  return new Army(a, (Uint32) 0, player);
}

Army::Army(const ArmyProdBase& a, Player* p)
    :ArmyBase(a), UniquelyIdentified(), Ownable(p), 
    d_type_id(a.getTypeId()), d_armyset(a.getArmyset()), 
    d_max_hp(2), d_max_moves_multiplier(1), d_max_moves_rest_bonus(0),
    d_ship(false), d_hp(2), d_moves(a.getMaxMoves()), d_xp(0), d_level(0),
    d_grouped(false), d_battles_number(0), d_number_hashit(0), 
    d_number_hasbeenhit(0)
{
  for(int i = 0; i < 3; i++)
    d_medal_bonus[i] = 0;
  d_visitedTemples.clear();
}

Army::Army()
  :ArmyBase(), UniquelyIdentified(), Ownable((Player *)0),
    d_type_id(0), d_armyset(0), d_max_hp(2), d_max_moves_multiplier(1), 
    d_max_moves_rest_bonus(0), d_ship(false), d_hp(2), d_moves(0), d_xp(0),
    d_level(0), d_grouped(false), d_battles_number(0), d_number_hashit(0), 
    d_number_hasbeenhit(0)
{
  d_visitedTemples.clear();
}

Army::Army(XML_Helper* helper)
  :ArmyBase(helper), UniquelyIdentified(helper), Ownable((XML_Helper*) 0),
    d_type_id(0), d_armyset(0), d_max_hp(2), d_max_moves_multiplier(1), 
    d_max_moves_rest_bonus(0), d_ship(false), d_hp(2), d_moves(0), d_xp(0),
    d_level(0), d_grouped(false), d_battles_number(0), d_number_hashit(0), 
    d_number_hasbeenhit(0)
{
  d_visitedTemples.clear();

  int ival = -1;
  //get the information which army we are
  helper->getData(d_type_id, "type");
  helper->getData(d_armyset, "armyset");

  helper->getData(d_hp, "hp");
  helper->getData(d_ship, "ship");
  helper->getData(d_moves, "moves");
  helper->getData(d_max_moves_multiplier, "max_moves_multiplier");
  helper->getData(d_xp, "xp");
  helper->getData(d_level, "level");

  std::string medals;
  std::stringstream smedals;
  bool val;

  helper->getData(medals, "medals");
  smedals.str(medals);

  for(int i=0;i<3;i++)
    {
      smedals >> val;
      d_medal_bonus[i]=val;
      debug("ARMY-XML-CONSTRUCTOR medalsbonus[" << i << "]=" << d_medal_bonus[i])
    }

  helper->getData(d_battles_number, "battlesnumber");    

  std::string temples;
  std::stringstream stemples;
  helper->getData(temples, "visited_temples");
  stemples.str(temples);

  while (stemples.eof() == false)
    {
      ival = -1;
      stemples >> ival;
      if (ival != -1)
	d_visitedTemples.push_front(ival);
    }
}

Army::~Army()
{
  sdying.emit(this);
}

void Army::setStat(Army::Stat stat, Uint32 value)
{
  switch (stat)
    {
    case STRENGTH:  
      d_strength = value;
      if (d_strength > 9)
	d_strength = 9;
      break;
    case HP:        
      d_max_hp = value;
      if (d_hp > d_max_hp)
	d_hp = value;
      break;
    case MOVES:     
      d_max_moves = value;
      if (d_moves > d_max_moves)
	d_moves = value;
      break;
    case MOVES_MULTIPLIER:
      d_max_moves_multiplier = value;
      break;
    case MOVE_BONUS:    d_move_bonus = value;
			break;
    case ARMY_BONUS:    d_army_bonus = value;
			break;
    case SIGHT:         d_sight = value;
			break;
    case SHIP:          value == 0 ? d_ship = false : d_ship = true;
			break;
    }
}

Uint32 Army::getStat(Stat stat, bool modified) const
{
  switch (stat)
    {
    case STRENGTH:
      return d_strength;
    case HP:
      return d_max_hp;
    case MOVES:
	{
	  if (modified)
	    return (d_max_moves + d_max_moves_rest_bonus) * d_max_moves_multiplier;
	  else
	    return d_max_moves;
	}
    case MOVE_BONUS:
      return d_move_bonus;
    case ARMY_BONUS:
      return d_army_bonus;
    case SIGHT:
      return d_sight;
    case SHIP:
      return d_ship;
    case MOVES_MULTIPLIER:
      return d_max_moves_multiplier;
    }

  // should never come to this
  return 0;
}

void Army::resetMoves()
{
  switch (d_moves)
    {
    case 0: d_max_moves_rest_bonus = 0; break;
    case 1: d_max_moves_rest_bonus = 1; break;
    case 2: d_max_moves_rest_bonus = 2; break;
    default: d_max_moves_rest_bonus = 2; break;
    }
  if (d_ship)
    d_moves = MAX_BOAT_MOVES;
  else
    d_moves = getStat(MOVES);
}

/* is this temple one we've already visited? */
bool Army::bless()
{
  bool visited = false;
  Stack *stack = d_owner->getStacklist()->getActivestack();
  Temple* temple = Templelist::getInstance()->getObjectAt(stack->getPos());

  if (!temple)
    return false;

  Uint32 templeId = temple->getId();
  std::list<unsigned int>::const_iterator tit = d_visitedTemples.begin();
  std::list<unsigned int>::const_iterator tend = d_visitedTemples.end();
  for(;tit != tend;++tit)
    {
      if ((*tit) == templeId)
	{
	  visited = true;
	  break;
	}
    }

  if (visited == false)  /* no?  increase strength */
    {
      d_visitedTemples.push_back(templeId);
      setStat(STRENGTH, d_strength + 1);
    }
  return !visited;
}


void Army::heal(Uint32 hp)
{
  if (hp == 0)
    {
      // if no hp are specified, we assume that the healing at the end of
      // the turn takes place. In this case the algorithm is: Heal 10%
      // plus 1HP for each point of vitality above 5 (or one less for each
      // point below 5), heal a minimum of 1 HP per turn
      hp = getStat(HP)/10;
      if (hp <= 5)
	hp = 1;
      else
	hp += 5;
    }

  d_hp += hp;
  if (d_hp > getStat(HP))
    d_hp = getStat(HP);
}

bool Army::damage(Uint32 damageDone)
{
  if (damageDone >= d_hp)
    d_hp = 0;
  else
    d_hp -= damageDone;
  return (d_hp == 0);
}

void Army::decrementMoves(Uint32 moves)
{
  if (moves >= d_moves)
    d_moves = 0;
  else
    d_moves -= moves;
}

void Army::gainXp(double n)
{
  d_xp += n;
}

bool Army::canGainLevel() const
{
  return getXP() >= getXpNeededForNextLevel();
}

Uint32 Army::getXpNeededForNextLevel() const
{
  return xp_per_level * getLevel();
}

int Army::computeLevelGain(Stat stat)
{
  if (stat == MOVE_BONUS || stat == ARMY_BONUS || stat == SHIP)
    return -1;

  switch (stat)
    {
    case STRENGTH:
    case SIGHT:
      return 1;
    case HP:
    case MOVES:
      return 4;
    default:
      return -1;
    }
}

int Army::gainLevel(Stat stat)
{
  if (!canGainLevel())
    return -1;

  if (stat == MOVE_BONUS || stat == ARMY_BONUS || stat == SHIP ||
      stat == MOVES_MULTIPLIER)
    return -1;

  d_level++;
  d_xp_value *= 1.2;

  int delta = computeLevelGain(stat);
  switch (stat)
    {
    case STRENGTH:
      d_strength += delta;
      if (d_strength > 9)
	d_strength = 9;
      break;
    case HP:
      d_max_hp += delta;
      break;
    case MOVES:
      d_max_moves += delta;
      break;
    case SIGHT:
      d_sight += delta;
      break;
    default:
      break;
    }

  return delta;
}

bool Army::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("army");
  retval &= saveData(helper);
  retval &= helper->closeTag();

  return retval;
}

bool Army::saveData(XML_Helper* helper) const
{
  bool retval = true;

  retval &= ArmyBase::saveData(helper);
  retval &= helper->saveData("id", d_id);
  retval &= helper->saveData("armyset", d_armyset);
  retval &= helper->saveData("type", d_type_id);
  retval &= helper->saveData("hp", d_hp);
  retval &= helper->saveData("ship", d_ship);
  retval &= helper->saveData("moves", d_moves);
  retval &= helper->saveData("xp", d_xp);
  retval &= helper->saveData("max_moves_multiplier", 
			     d_max_moves_multiplier);
  retval &= helper->saveData("level", d_level);

  std::stringstream medals;
  for (int i=0;i<3;i++)
    {
      medals << d_medal_bonus[i] << " ";
    }
  retval &= helper->saveData("medals", medals.str());
  retval &= helper->saveData("battlesnumber",d_battles_number);    

  std::stringstream temples;
  std::list<unsigned int>::const_iterator tit = d_visitedTemples.begin();
  std::list<unsigned int>::const_iterator tend = d_visitedTemples.end();
  for(;tit != tend;++tit)
    temples << (*tit) << " ";
  retval &= helper->saveData("visited_temples", temples.str());

  return retval;
}

void  Army::printAllDebugInfo() const
{
  std::cerr << "name = " << getName() << std::endl;
  std::cerr << "max_hp = " << d_max_hp << std::endl;
  std::cerr << "xp_value = " << d_xp_value << std::endl;
  std::cerr << "strength = " << d_strength << std::endl;
  std::cerr << "max_moves = " << d_max_moves << std::endl;
  std::cerr << "max_moves_multiplier = " 
    << d_max_moves_multiplier << std::endl;
  std::cerr << "max_moves_rest_bonus = " 
    << d_max_moves_rest_bonus << std::endl;
  std::cerr << "upkeep = " << d_upkeep << std::endl;
  std::cerr << "production = " << d_production << std::endl;
  std::cerr << "move_bonus = " << d_move_bonus << std::endl;
  std::cerr << "ship = " << d_ship << std::endl;
  std::cerr << "army_bonus = " << d_army_bonus << std::endl;

  std::cerr << "type = "    << d_type_id    << std::endl;
  std::cerr << "level = "   << d_level   << std::endl;
  std::cerr << "xp = "      << d_xp      << std::endl;
  std::cerr << "grouped = " << d_grouped << std::endl;

  std::cerr << "medal[0] = " << d_medal_bonus[0] << std::endl;
  std::cerr << "medal[1] = " << d_medal_bonus[1] << std::endl;
  std::cerr << "medal[2] = " << d_medal_bonus[2] << std::endl;

  std::cerr << "battle number = "     << d_battles_number    << std::endl;
  std::cerr << "has hit = "           << d_number_hashit     << std::endl;
  std::cerr << "has been hit = "      << d_number_hasbeenhit << std::endl;
  std::stringstream temples;
  std::list<unsigned int>::const_iterator tit = d_visitedTemples.begin();
  std::list<unsigned int>::const_iterator tend = d_visitedTemples.end();
  for(;tit != tend;++tit)
    temples << (*tit) << " ";
  std::cerr << "visited temples with ids = " << temples << std::endl;
}

void Army::setInShip (bool s)
{
  d_ship = s;
}

//! Sets this army as being fortified (+1 to stack)
void Army::setFortified (bool f)
{
  if (getFortified() == true && f == true)
    ; // do nothing
  else if (getFortified() == true && f == false)
    d_army_bonus ^= Army::FORTIFY;
  else if (getFortified() == false && f == true)
    d_army_bonus |= Army::FORTIFY;
  else if (getFortified() == false && f == false)
    ; // do nothing
}

//! get the fortify flag for this army
bool Army::getFortified ()
{
  return (d_army_bonus & Army::FORTIFY) == Army::FORTIFY;
}

bool Army::blessedAtTemple(Uint32 temple_id)
{
  unsigned int id = temple_id;
  if (std::find (d_visitedTemples.begin(), d_visitedTemples.end(), id) ==
      d_visitedTemples.end())
    return false;

  return true;
}

bool Army::getDefendsRuins() const
{
  ArmyProto *a = Armysetlist::getInstance()->getArmy(d_armyset, d_type_id);
  if (a)
    return a->getDefendsRuins();
  else
    return false;
}

bool Army::getAwardable() const
{
  ArmyProto *a = Armysetlist::getInstance()->getArmy(d_armyset, d_type_id);
  if (a)
    return a->getAwardable();
  else
    return false;
}

std::string Army::getName() const
{
  ArmyProto *a = Armysetlist::getInstance()->getArmy(d_armyset, d_type_id);
  if (a)
    return a->getName();
  else
    return "";
}

