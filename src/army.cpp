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
#include "army.h"
#include "armysetlist.h"
#include "counter.h"
#include "GraphicsCache.h"
#include "xmlhelper.h"
#include "stacklist.h"
#include "templelist.h"
#include "ucompose.hpp"
#include "Tile.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

sigc::signal<void, Army*> Army::sdying;

Army::Army(const Army& a, Player* p, bool for_template)
    :Ownable(p), d_type(a.d_type), d_armyset(a.d_armyset), d_pixmap(0),
     d_mask(0), d_name(a.d_name), d_description(a.d_description), 
     d_production(a.d_production),
     d_production_cost(a.d_production_cost), d_upkeep(a.d_upkeep),
     d_strength(a.d_strength),
     d_max_hp(a.d_max_hp),
     d_max_moves(a.d_max_moves), 
     d_max_moves_multiplier(a.d_max_moves_multiplier),
     d_max_moves_rest_bonus(a.d_max_moves_rest_bonus),
     d_sight(a.d_sight),
     d_xp_value(a.d_xp_value), d_move_bonus(a.d_move_bonus),
     d_army_bonus(a.d_army_bonus), d_ship(a.d_ship), d_gender(a.d_gender), 
     d_id(a.d_id), d_hp(a.d_hp), d_moves(a.d_moves), d_xp(a.d_xp),
     d_level(a.d_level), d_grouped(a.d_grouped),
     d_battles_number(a.d_battles_number), d_number_hashit(a.d_number_hashit),
     d_number_hasbeenhit(a.d_number_hasbeenhit), 
     d_defends_ruins(a.d_defends_ruins), d_awardable(a.d_awardable),
     d_visitedTemples(a.d_visitedTemples), d_hero(a.d_hero)
{
    // if we have been copied from an army prototype, initialise several values
    if (d_id == 0 && !for_template)
    {
        d_id = fl_counter->getNextId();
        debug("army created with id " << d_id);
        d_hp = d_max_hp;
        d_moves = d_max_moves;
	d_max_moves_multiplier = 1;
	d_max_moves_rest_bonus = 0;
    }
        
    for(int i=0;i<3;i++)
    {
        d_medal_bonus[i] = a.d_medal_bonus[i];
    }  
}

Army::Army()
  :Ownable((Player *)0), d_pixmap(0), d_mask(0), d_name("Untitled"), d_description(""),
    d_production(0), d_production_cost(0), d_upkeep(0), d_strength(0),
    d_max_hp(0), d_max_moves(0), d_max_moves_multiplier(1), 
    d_max_moves_rest_bonus(0), d_sight(0), 
    d_gender(NONE), d_level(1), d_defends_ruins(false), d_awardable(false), 
    d_hero(false), d_image("")
{
}

Army::Army(XML_Helper* helper, enum ArmyContents contents)
  :Ownable((XML_Helper*) 0), d_pixmap(0), d_mask(0), d_name(""), 
    d_description(""), d_ship(false), d_gender(NONE), d_id(0), d_xp(0), 
    d_level(1), d_grouped(true), d_number_hashit(0), d_number_hasbeenhit(0), 
    d_defends_ruins(false), d_awardable(false), d_hero(false)
{
    d_max_hp = 2;
    d_visitedTemples.clear();
    // first, load the data that has to be loaded anyway
    helper->getData(d_strength, "strength");
    helper->getData(d_sight, "sight");
    helper->getData(d_max_moves, "max_moves");
    d_max_moves_multiplier = 1;
    d_max_moves_rest_bonus = 0;
    helper->getData(d_xp_value,"expvalue");
    
    // now if we are a prototype, we have to load different data than
    // otherwise
    if (contents == INSTANCE)
    {
        int ival = -1;
        //get the information which army we are
        helper->getData(d_type, "type");
        helper->getData(d_armyset, "armyset");

        copyVals(Armysetlist::getInstance()->getArmy(d_armyset,d_type));

        //adjust the values with the saved data
        helper->getData(d_id, "id");
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
    else if (contents == TYPE || contents == PRODUCTION_BASE)
    {
        helper->getData(d_name, "name");
        helper->getData(d_image, "image");
        helper->getData(d_description, "description");
        helper->getData(d_production, "production");
        helper->getData(d_production_cost, "production_cost");
        helper->getData(d_upkeep, "upkeep");
	std::string move_bonus_str;
        helper->getData(move_bonus_str, "move_bonus");
	d_move_bonus = moveFlagsFromString(move_bonus_str);
	std::string army_bonus_str;
        helper->getData(army_bonus_str, "army_bonus");
	d_army_bonus = bonusFlagsFromString(army_bonus_str);


	helper->getData(d_defends_ruins,"defends_ruins");
	helper->getData(d_awardable,"awardable");

	std::string gender_str;
        if (!helper->getData(gender_str, "gender"))
	  d_gender = NONE;
	else
	  d_gender = genderFromString(gender_str);

        d_hp = d_max_hp;
        d_moves = d_max_moves;
        d_battles_number = 0;

        for(int i = 0; i < 3; i++)
            d_medal_bonus[i] = false;
      helper->getData(d_hero,"hero");
    }
    if (contents == PRODUCTION_BASE)
      {
	helper->getData(d_type, "type");
	helper->getData(d_armyset, "armyset");
      }
}

Army::~Army()
{
    if (d_pixmap)
        SDL_FreeSurface(d_pixmap);
    if (d_mask)
        SDL_FreeSurface(d_mask);

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

void Army::setArmyset(Uint32 armyset, Uint32 type)
{
    d_armyset = armyset;
    d_type = type;
}

SDL_Surface* Army::getPixmap() const
{
    // if we have a pixmap (== prototype of an army) return it
    if (d_pixmap)
        return d_pixmap;
    
    //use the GraphicsCache to get a picture of the army's armyset_army
    return GraphicsCache::getInstance()->getArmyPic(d_armyset, d_type,
                                         d_owner, d_medal_bonus);
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


bool Army::save(XML_Helper* helper, enum ArmyContents contents) const
{
    bool retval = true;

    retval &= helper->openTag("army");
    retval &= saveData(helper, contents);
    retval &= helper->closeTag();

    return retval;
}

bool Army::saveData(XML_Helper* helper, enum ArmyContents contents) const
{
    bool retval = true;
    
    if (contents == TYPE || contents == PRODUCTION_BASE)
      {
	retval &= helper->saveData("name", d_name);
	retval &= helper->saveData("image", d_image);
	retval &= helper->saveData("description", d_description);
	retval &= helper->saveData("production", d_production);
	retval &= helper->saveData("production_cost", d_production_cost);
	retval &= helper->saveData("upkeep", d_upkeep);
	std::string gender_str = genderToString(Army::Gender(d_gender));
	retval &= helper->saveData("gender", gender_str);
	retval &= helper->saveData("awardable", d_awardable);
	retval &= helper->saveData("defends_ruins", d_defends_ruins);
	std::string move_bonus_str = moveFlagsToString(d_move_bonus);
	retval &= helper->saveData("move_bonus", move_bonus_str);
	std::string army_bonus_str = bonusFlagsToString(d_army_bonus);
	retval &= helper->saveData("army_bonus", army_bonus_str);
      }
    else if (contents == INSTANCE)
      {
	retval &= helper->saveData("id", d_id);
	retval &= helper->saveData("armyset", d_armyset);
	retval &= helper->saveData("type", d_type);
	retval &= helper->saveData("hp", d_hp);
	retval &= helper->saveData("ship", d_ship);
	retval &= helper->saveData("moves", d_moves);
	retval &= helper->saveData("xp", d_xp);
	retval &= helper->saveData("max_moves_multiplier", 
				   d_max_moves_multiplier);
	retval &= helper->saveData("level", d_level);
      }

    if (contents == PRODUCTION_BASE)
      {
	retval &= helper->saveData("type", d_type);
	retval &= helper->saveData("armyset", d_armyset);
      }

    retval &= helper->saveData("max_moves", d_max_moves);
    retval &= helper->saveData("hero", d_hero);
    retval &= helper->saveData("strength", d_strength);
    retval &= helper->saveData("sight", d_sight);
    retval &= helper->saveData("expvalue", getXpReward());

    if (contents == PRODUCTION_BASE || contents == TYPE)
      return retval;

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
    std::cerr << "name = " << d_name << std::endl;
    std::cerr << "pixmap = " << d_pixmap << std::endl;
    std::cerr << "mask = " << d_mask << std::endl;
    std::cerr << "max_hp = " << d_max_hp << std::endl;
    std::cerr << "xp_value = " << d_xp_value << std::endl;
    std::cerr << "strength = " << d_strength << std::endl;
    std::cerr << "max_moves = " << d_max_moves << std::endl;
    std::cerr << "max_moves_multiplier = " 
      << d_max_moves_multiplier << std::endl;
    std::cerr << "max_moves_rest_bonus = " 
      << d_max_moves_rest_bonus << std::endl;
    std::cerr << "upkeep = " << d_upkeep << std::endl;
    std::cerr << "defends_ruins = " << d_defends_ruins << std::endl;
    std::cerr << "awardable = " << d_awardable << std::endl;
    std::cerr << "production = " << d_production << std::endl;
    std::cerr << "production_cost = " << d_production_cost << std::endl;
    std::cerr << "move_bonus = " << d_move_bonus << std::endl;
    std::cerr << "ship = " << d_ship << std::endl;
    std::cerr << "army_bonus = " << d_army_bonus << std::endl;

    std::cerr << "type = "    << d_type    << std::endl;
    std::cerr << "hero = "    << d_hero    << std::endl;
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


void Army::copyVals(const Army* a)
{
    d_name = a->getName();
    d_description = a->getDescription();
    d_production = a->getProduction();
    d_production_cost = a->getProductionCost();
    d_upkeep = a->getUpkeep();
    d_move_bonus = a->getStat(MOVE_BONUS);
    d_army_bonus = a->getStat(ARMY_BONUS);
    d_ship = a->getStat(SHIP);
    d_gender = a->getGender();
    d_defends_ruins = a->getDefendsRuins();
    d_awardable = a->getAwardable();
    d_visitedTemples = a->d_visitedTemples;
    setOwner(a->getOwner());
    d_hero = a->d_hero;
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

void Army::setPixmap(SDL_Surface* pixmap)
{
  if (d_pixmap)
    SDL_FreeSurface(d_pixmap);
  d_pixmap = pixmap;
}
        
void Army::setMask(SDL_Surface* mask)
{
  if (d_mask)
    SDL_FreeSurface(d_mask);
  d_mask = mask;
}

std::string Army::getArmyBonusDescription() const
{
  Uint32 bonus = getStat(Army::ARMY_BONUS, false);
  Glib::ustring s = "";
  if (bonus & Army::ADD1STRINOPEN)
    s += String::ucompose(_("%1%2"), s == "" ? " " : "& ", 
			  _("+1 str in open"));
  if (bonus & Army::ADD2STRINOPEN)
    s += String::ucompose(_("%1%2"), s == "" ? " " : "& ", 
			  _("+2 str in open"));
  if (bonus & Army::ADD1STRINFOREST)
    s += String::ucompose(_("%1%2"), s == "" ? " " : "& ", 
			  _("+1 str in woods"));
  if (bonus & Army::ADD1STRINHILLS)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("+1 str in hills"));
  if (bonus & Army::ADD1STRINCITY)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("+1 str in city"));
  if (bonus & Army::ADD2STRINCITY)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("+2 str in city"));
  if (bonus & Army::ADD1STACKINHILLS)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("+1 stack in hills"));
  if (bonus & Army::SUBALLCITYBONUS)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("Cancel city bonus"));
  if (bonus & Army::SUB1ENEMYSTACK)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("-1 enemy stack"));
  if (bonus & Army::ADD1STACK)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", _("+1 stack"));
  if (bonus & Army::ADD2STACK)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", _("+2 stack"));
  if (bonus & Army::SUBALLNONHEROBONUS)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("cancel non-hero"));
  if (bonus & Army::SUBALLHEROBONUS)
    s += String::ucompose(_("%1%2"), s == "" ? " " : " & ", 
			  _("cancel hero"));
  return s;
}

bool Army::blessedAtTemple(Uint32 temple_id)
{
  unsigned int id = temple_id;
  if (find (d_visitedTemples.begin(), d_visitedTemples.end(), id) ==
      d_visitedTemples.end())
    return false;
      
  return true;
}

std::string Army::moveFlagsToString(const Uint32 bonus)
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
  return move_bonuses;
}

Uint32 Army::moveFlagsFromString(const std::string str)
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

std::string Army::genderToString(const Army::Gender gender)
{
  switch (gender)
    {
      case Army::NONE:
	return "Army::NONE";
	break;
      case Army::MALE:
	return "Army::MALE";
	break;
      case Army::FEMALE:
	return "Army::FEMALE";
	break;
    }
  return "Army::FEMALE";
}

Army::Gender Army::genderFromString(const std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Army::Gender(atoi(str.c_str()));
  if (str == "Army::MALE")
    return Army::MALE;
  else if (str == "Army::NONE")
    return Army::NONE;
  else if (str == "Army::FEMALE")
    return Army::FEMALE;
  return Army::FEMALE;
}

std::string Army::bonusFlagToString(const Army::Bonus bonus)
{
  switch (bonus)
    {
    case Army::ADD1STRINOPEN:
      return "Army::ADD1STRINOPEN";
    case Army::ADD2STRINOPEN:
      return "Army::ADD2STRINOPEN";
    case Army::ADD1STRINFOREST:
      return "Army::ADD1STRINFOREST";
    case Army::ADD1STRINHILLS:
      return "Army::ADD1STRINHILLS";
    case Army::ADD1STRINCITY:
      return "Army::ADD1STRINCITY";
    case Army::ADD2STRINCITY:
      return "Army::ADD2STRINCITY";
    case Army::ADD1STACKINHILLS:
      return "Army::ADD1STACKINHILLS";
    case Army::SUBALLCITYBONUS:
      return "Army::SUBALLCITYBONUS";
    case Army::SUB1ENEMYSTACK:
      return "Army::SUB1ENEMYSTACK";
    case Army::ADD1STACK:
      return "Army::ADD1STACK";
    case Army::ADD2STACK:
      return "Army::ADD2STACK";
    case Army::SUBALLNONHEROBONUS:
      return "Army::SUBALLNONHEROBONUS";
    case Army::SUBALLHEROBONUS:
      return "Army::SUBALLHEROBONUS";
    case Army::FORTIFY:
      return "Army::FORTIFY";
    }
  return "";
}

std::string Army::bonusFlagsToString(const Uint32 bonus)
{
  std::string bonuses;
  if (bonus & Army::ADD1STRINOPEN)
    bonuses += " " + bonusFlagToString(Army::ADD1STRINOPEN);
  if (bonus & Army::ADD2STRINOPEN)
    bonuses += " " + bonusFlagToString(Army::ADD2STRINOPEN);
  if (bonus & Army::ADD1STRINFOREST)
    bonuses += " " + bonusFlagToString(Army::ADD1STRINFOREST);
  if (bonus & Army::ADD1STRINHILLS)
    bonuses += " " + bonusFlagToString(Army::ADD1STRINHILLS);
  if (bonus & Army::ADD1STRINCITY)
    bonuses += " " + bonusFlagToString(Army::ADD1STRINCITY);
  if (bonus & Army::ADD2STRINCITY)
    bonuses += " " + bonusFlagToString(Army::ADD2STRINCITY);
  if (bonus & Army::ADD1STACKINHILLS)
    bonuses += " " + bonusFlagToString(Army::ADD1STACKINHILLS);
  if (bonus & Army::SUBALLCITYBONUS)
    bonuses += " " + bonusFlagToString(Army::SUBALLCITYBONUS);
  if (bonus & Army::SUB1ENEMYSTACK)
    bonuses += " " + bonusFlagToString(Army::SUB1ENEMYSTACK);
  if (bonus & Army::ADD1STACK)
    bonuses += " " + bonusFlagToString(Army::ADD1STACK);
  if (bonus & Army::ADD2STACK)
    bonuses += " " + bonusFlagToString(Army::ADD2STACK);
  if (bonus & Army::SUBALLNONHEROBONUS)
    bonuses += " " + bonusFlagToString(Army::SUBALLNONHEROBONUS);
  if (bonus & Army::SUBALLHEROBONUS)
    bonuses += " " + bonusFlagToString(Army::SUBALLHEROBONUS);
  if (bonus & Army::FORTIFY)
    bonuses += " " + bonusFlagToString(Army::FORTIFY);
  return bonuses;
}

Uint32 Army::bonusFlagsFromString(const std::string str)
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

Army::Bonus Army::bonusFlagFromString(const std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Army::Bonus(atoi(str.c_str()));
  if (str == "Army::ADD1STRINOPEN")
    return Army::ADD1STRINOPEN;
  else if (str == "Army::ADD2STRINOPEN")
    return Army::ADD2STRINOPEN;
  else if (str == "Army::ADD1STRINFOREST")
    return Army::ADD1STRINFOREST;
  else if (str == "Army::ADD1STRINHILLS")
    return Army::ADD1STRINHILLS;
  else if (str == "Army::ADD1STRINCITY")
    return Army::ADD1STRINCITY;
  else if (str == "Army::ADD2STRINCITY")
    return Army::ADD2STRINCITY;
  else if (str == "Army::ADD1STACKINHILLS")
    return Army::ADD1STACKINHILLS;
  else if (str == "Army::SUBALLCITYBONUS")
    return Army::SUBALLCITYBONUS;
  else if (str == "Army::ADD2GOLDPERCITY")
    return Army::SUB1ENEMYSTACK;
  else if (str == "Army::SUB1ENEMYSTACK")
    return Army::ADD1STACK;
  else if (str == "Army::ADD1STACK")
    return Army::ADD2STACK;
  else if (str == "Army::ADD2STACK")
    return Army::ADD2STACK;
  else if (str == "Army::SUBALLNONHEROBONUS")
    return Army::SUBALLNONHEROBONUS;
  else if (str == "Army::SUBALLHEROBONUS")
    return Army::SUBALLHEROBONUS;
  return Army::ADD1STRINOPEN;
}
	
void Army::syncNewId()
{
  fl_counter->syncToId(d_id);
}
