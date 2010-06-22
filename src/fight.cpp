// Copyright (C) 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2004 Bryan Duff
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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

#include "fight.h"
#include <algorithm>
#include <assert.h>
#include <stdlib.h>     // for random numbers
#include <math.h>       // for has_hit()
#include "army.h"
#include "hero.h"
#include "stacklist.h"
#include "player.h"
#include "playerlist.h"
#include "Item.h"
#include "GameMap.h"
#include "citylist.h"
#include "city.h"
#include "stack.h"
#include "Backpack.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

// Helper class; the single units participating in the fight are saved with
// additional information. This should be a struct, but I don't know how to
// forward the declaration properly.
//! A particpant in a Fight.
class Fighter
{
    public:
        Fighter(Army* a, Vector<int> p);
        
        Army* army;
        Vector<int> pos;       // location on the map (needed to calculate boni)
        int terrain_strength;
	//! needed for sorting
        //bool operator() ( const Fighter* f1, const Fighter* f2 );

};

Fighter::Fighter(Army* a, Vector<int> p)
    :army(a), pos(p)
{
}


//take a list of stacks and create an ordered list of armies
void Fight::orderArmies(std::list<Stack*> stacks, std::vector<Army*> &armies)
{
  std::list<Stack*>::iterator it;
  if (stacks.empty())
    return;
  for (it = stacks.begin(); it != stacks.end(); it++)
    for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
      armies.push_back((*sit));

  //okay now sort the army list according to the player's fight order
  std::sort(armies.begin(), armies.end(), Stack::armyCompareFightOrder);

  return;
}

Fight::Fight(Stack* attacker, Stack* defender, FightType type)
    : d_turn(0), d_result(DRAW), d_type(type)
{

    debug("Fight between " <<attacker->getId() <<" and " <<defender->getId())
    
    // Duel case: two stacks fight each other; Nothing further to be done
    // Important: We always assume that the attacking/defending stacks are
    // the first in the list!!!
    d_attackers.push_back(attacker);
    d_defenders.push_back(defender);

    // What we do here: In the setup, we need to find out all armies that
    // participate in the fight.  If a city is being attacked then the
    // defender gets any other stacks in the cities.
    //

    City *city = GameMap::getCity(defender->getPos());
    //Vector<int> p = defender->getPos();

    if (city && city->isBurnt() == false)
      {
        printf("attacking city %s with attacker %d\n", city->getName().c_str(), attacker->getId());
        printf("city is owned by %s\n", city->getOwner()->getName().c_str());
	std::vector<Stack*> stacks = city->getDefenders();
	for (std::vector<Stack*>::iterator it = stacks.begin(); 
	     it != stacks.end(); it++)
	  {
            printf("city defender is %d\n", (*it)->getId());
	    Stack *s = *it;
	    if (s == d_defenders.front())
	      continue;
	    d_defenders.push_back(s);
	  }
      }
	
    std::list<Stack*>::iterator it;
    
    //setup fighters
    it = d_defenders.begin();
    std::vector<Army*> def;
    orderArmies (d_defenders, def);
    for (std::vector<Army*>::iterator ait = def.begin(); ait != def.end(); ait++)
      {
	Fighter* f = new Fighter((*ait), (*it)->getPos());
	d_def_close.push_back(f);
      }

    it = d_attackers.begin();
    std::vector<Army*> att;
    orderArmies (d_attackers, att);
    for (std::vector<Army*>::iterator ait = att.begin(); ait != att.end(); ait++)
      {
	Fighter* f = new Fighter((*ait), (*it)->getPos());
	d_att_close.push_back(f);
      }

  fillInInitialHPs();

  // Before the battle starts, calculate the bonuses
  // bonuses remain even if the unit providing a stackwide bonus dies


  calculateBonus();
}

Fight::Fight(std::list<Stack*> attackers, std::list<Stack*> defenders,
             std::list<FightItem> history)
{
  d_attackers = attackers;
  d_defenders = defenders;
  d_actions = history;

  fillInInitialHPs();
}

Fight::~Fight()
{
      
  d_attackers.clear();
  d_defenders.clear();

  // clear all fighter items in all lists
  while (!d_att_close.empty())
    {
      delete (*d_att_close.begin());
      d_att_close.erase(d_att_close.begin());
    }

  while (!d_def_close.empty())
    {
      delete (*d_def_close.begin());
      d_def_close.erase(d_def_close.begin());
    }
}

void Fight::battle(bool intense)
{
  d_intense_combat = intense;

  // first, fight until the fight is over
  for (d_turn = 0; doRound(); d_turn++);

  // Now we have to set the fight result.

  // First, look if the attacker died; the attacking stack is the first
  // one in the list
  bool survivor = false;
  Stack* s = d_attackers.front();
  for (Stack::const_iterator it = s->begin(); it != s->end(); it++)
    if ((*it)->getHP() > 0)
      {
	survivor = true;
	break;
      }

  if (!survivor)
      d_result = DEFENDER_WON;
  else
    {
      // Now look if the defender died; also the first in the list
      survivor = false;
      s = d_defenders.front();
      for (Stack::const_iterator it = s->begin(); it != s->end(); it++)
	if ((*it)->getHP() > 0)
	  {
	    survivor = true;
	    break;
	  }

      if (!survivor)
	d_result = ATTACKER_WON;
    }

  if (d_type == FOR_KICKS)
    {
      //fixme: this will heal armies who happen to have a single hitpoint left.
      std::list<Stack*>::iterator it;
      //heal the attackers and defenders to full hit points
      for (it = d_attackers.begin(); it != d_attackers.end(); it++)
	for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
	  (*sit)->heal((*sit)->getStat(Army::HP));

      for (it = d_defenders.begin(); it != d_defenders.end(); it++)
	for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
	  (*sit)->heal((*sit)->getStat(Army::HP));
    }
}

Army *findArmyById(const std::list<Stack *> &l, guint32 id)
{
  for (std::list<Stack *>::const_iterator i = l.begin(), end = l.end();
       i != end; ++i) {
    Army *a = (*i)->getArmyById(id);
    if (a)
      return a;
  }

  return 0;
}

Fight::Result Fight::battleFromHistory()
{
  for (std::list<FightItem>::iterator i = d_actions.begin(),
         end = d_actions.end(); i != end; ++i) {
    FightItem &f = *i;

    Army *a = findArmyById(d_attackers, f.id);
    if (!a)
      a = findArmyById(d_defenders, f.id);

    if (!a)
      {
	printf ("uh oh.  army id %d can't be found in the battle.\n", f.id);
	printf ("do we have an army id of that in the stacklist anywhere?\n");
      assert (false);
      }
    a->damage(f.damage);
  }
  //is there anybody alive in the attackers?
  for (std::list<Stack*>::iterator it = d_attackers.begin(); it != d_attackers.end(); it++)
    {
      for (Stack::iterator i = (*it)->begin(); i != (*it)->end(); i++)
	{
	  if ((*i)->getHP() > 0)
	    return Fight::ATTACKER_WON;
	}
    }

  return Fight::DEFENDER_WON;
}

bool Fight::doRound()
{
  if (MAX_ROUNDS && d_turn >= MAX_ROUNDS)
    return false;

  debug ("Fight round #" <<d_turn);

  //fight the first one in attackers with the first one in defenders
  std::list<Fighter*>::iterator ffit = d_att_close.begin();
  std::list<Fighter*>::iterator efit = d_def_close.begin();

  //have the attacker and defender try to hit each other
  fightArmies(*ffit, *efit);

  if (*efit && (*efit)->army->getHP() <= 0)
    remove((*efit));

  if (*ffit && (*ffit)->army->getHP() <= 0)
    remove((*ffit));

  if (d_def_close.empty() || d_att_close.empty())
    return false;

  return true;
}

void Fight::calculateBaseStrength(std::list<Fighter*> fighters)
{
  std::list<Fighter*>::iterator fit;
  for (fit = fighters.begin(); fit != fighters.end(); fit++)
    {
      if ((*fit)->army->getStat(Army::SHIP))
	(*fit)->terrain_strength = 4;
      else
	(*fit)->terrain_strength = (*fit)->army->getStat(Army::STRENGTH);
    }
}

void Fight::calculateTerrainModifiers(std::list<Fighter*> fighters)
{ 
  guint32 army_bonus;
  GameMap *gm = GameMap::getInstance();
  Maptile *mtile;
  std::list<Fighter*>::iterator fit;
  for (fit = d_att_close.begin(); fit != d_att_close.end(); fit++)
    {
      if ((*fit)->army->getStat(Army::SHIP))
	continue;

      mtile = gm->getTile((*fit)->pos);
      army_bonus = (*fit)->army->getStat(Army::ARMY_BONUS);

      if (army_bonus & Army::ADD1STRINOPEN && mtile->isOpenTerrain())
	(*fit)->terrain_strength += 1;

      if (army_bonus & Army::ADD1STRINFOREST && 
	  mtile->getType() == Tile::FOREST && !mtile->isCityTerrain())
	(*fit)->terrain_strength += 1;

      if (army_bonus & Army::ADD1STRINHILLS && mtile->isHillyTerrain())
	(*fit)->terrain_strength += 1;

      if (army_bonus & Army::ADD1STRINCITY && mtile->isCityTerrain())
	(*fit)->terrain_strength += 1;

      if (army_bonus & Army::ADD2STRINCITY && mtile->isCityTerrain())
	(*fit)->terrain_strength += 2;

      if (army_bonus & Army::ADD2STRINOPEN && mtile->isOpenTerrain())
	(*fit)->terrain_strength += 2;

      if ((*fit)->terrain_strength > 9) //terrain strength can't ever exceed 9
	(*fit)->terrain_strength = 9;

    }
}

void Fight::calculateModifiedStrengths (std::list<Fighter*>friendly, 
					std::list<Fighter*>enemy, 
					bool friendlyIsDefending,
					Hero *strongestHero)
{
  guint32 army_bonus;
  GameMap *gm = GameMap::getInstance();
  Maptile *mtile;
  std::list<Fighter*>::iterator fit;

  //find highest non-hero bonus
  guint32 highest_non_hero_bonus = 0;
  for (fit = friendly.begin(); fit != friendly.end(); fit++)
    {
      guint32 non_hero_bonus = 0;
      if ((*fit)->army->isHero())
	continue;
      mtile = gm->getTile((*fit)->pos);
      army_bonus = (*fit)->army->getStat(Army::ARMY_BONUS);

      if (army_bonus & Army::ADD1STACKINHILLS && mtile->isHillyTerrain())
	non_hero_bonus += 1;

      if (army_bonus & Army::ADD1STACK)
	non_hero_bonus += 1;

      if (army_bonus & Army::ADD2STACK)
	non_hero_bonus += 2;

      if (non_hero_bonus > highest_non_hero_bonus)
	highest_non_hero_bonus = non_hero_bonus;
    }

  // does the defender cancel our non hero bonus?
  for (fit = enemy.begin(); fit != enemy.end(); fit++)
    {
      army_bonus = (*fit)->army->getStat(Army::ARMY_BONUS);
      if (army_bonus & Army::SUBALLNONHEROBONUS)
	{
	  highest_non_hero_bonus = 0; //yes
	  break;
	}
    }

  //find hero bonus of strongest hero
  guint32 hero_bonus = 0;
  if (strongestHero)
    {
      // first get command items from ALL heroes in the stack
      for (fit = friendly.begin(); fit != friendly.end(); fit++)
	{
	  if ((*fit)->army->isHero())
	    {
	      Hero *h = dynamic_cast<Hero*>((*fit)->army);
	      hero_bonus = h->getBackpack()->countStackStrengthBonuses();
	    }
	}
    }

  //now add on the hero's natural command
  if (strongestHero)
    {
      hero_bonus += strongestHero->calculateNaturalCommand();
    }

  // does the defender cancel our hero bonus?
  for (fit = enemy.begin(); fit != enemy.end(); fit++)
    {
      army_bonus = (*fit)->army->getStat(Army::ARMY_BONUS);
      if (army_bonus & Army::SUBALLHEROBONUS)
	{
	  hero_bonus = 0; //yep
	  break;
	}
    }

  guint32 fortify_bonus = 0;
  for (fit = friendly.begin(); fit != friendly.end(); fit++)
    {
      army_bonus = (*fit)->army->getStat(Army::ARMY_BONUS);
      if (army_bonus & Army::FORTIFY)
	{
	  fortify_bonus = 1;
	  break;
	}
    }

  guint32 city_bonus = 0;
  if (friendlyIsDefending)
    {
      // calculate the city bonus
      fit = friendly.begin();
      mtile = gm->getTile((*fit)->pos);
      City *c = Citylist::getInstance()->getNearestCity((*fit)->pos);
      if (c && mtile->getBuilding() == Maptile::CITY)
	{
	  if (c->isBurnt()) 
	    city_bonus = 0;
	  else
	    city_bonus = c->getDefenseLevel() - 1;
	}
      else
	{
	  if (mtile->getBuilding() == Maptile::TEMPLE)
	    city_bonus = 2;
	  else if (mtile->getBuilding() == Maptile::RUIN)
	    city_bonus = 2;
	}

      // does the attacker cancel our city bonus?
      for (fit = enemy.begin(); fit != enemy.end(); fit++)
	{
	  army_bonus = (*fit)->army->getStat(Army::ARMY_BONUS);
	  if (army_bonus & Army::SUBALLCITYBONUS)
	    {
	      city_bonus = 0; //yep
	      break;
	    }
	}
    }

  guint32 total_bonus = highest_non_hero_bonus + hero_bonus + fortify_bonus + 
    city_bonus;

  if (total_bonus > 5) //total bonus can't exceed 5
    total_bonus = 5;

  //add it to the terrain strength of each unit
  for (fit = friendly.begin(); fit != friendly.end(); fit++)
    {
      (*fit)->terrain_strength += total_bonus;
    }
}

void Fight::calculateFinalStrengths (std::list<Fighter*> friendly, std::list<Fighter*> enemy)
{
  guint32 army_bonus;
  std::list<Fighter*>::iterator efit;
  std::list<Fighter*>::iterator ffit;
  for (efit = enemy.begin(); efit != enemy.end(); efit++)
    {
      if ((*efit)->army->getStat(Army::SHIP))
	continue;
      army_bonus = (*efit)->army->getStat(Army::ARMY_BONUS);
      if (army_bonus & Army::SUB1ENEMYSTACK)
	{
	  for (ffit = friendly.begin(); ffit != friendly.end(); ffit++)
	    {
	      (*ffit)->terrain_strength -= 1;
	      if ((*ffit)->terrain_strength <= 0)
		(*ffit)->terrain_strength = 1;
	    }
	  break;
	}
    }
}

void Fight::calculateBonus()
{
  // If there is a hero, add a +1 strength bonus
  std::list<Stack*>::const_iterator it;
  Stack::const_iterator sit;
  std::list<Fighter*>::iterator fit;

  // go get the base strengths of all attackers
  // this includes items with battle bonuses for the hero
  // naval units always have strength = 4
  calculateBaseStrength (d_att_close);
  calculateBaseStrength (d_def_close);

  // now determine the terrain strength by adding the terrain modifiers 
  // to the base strength
  // naval units always have a strength of 4
  calculateTerrainModifiers (d_att_close);
  calculateTerrainModifiers (d_def_close);

  //calculate hero, non-hero, city, and fortify bonuses
  it = d_attackers.begin();
  Army *a = (*it)->getStrongestHero();
  Hero *h = dynamic_cast<Hero*>(a);
  calculateModifiedStrengths (d_att_close, d_def_close, false, h);
  Hero *strongestHero = 0;
  guint32 highest_strength = 0;
  for (it = d_defenders.begin(); it != d_defenders.end(); it++)
    {
      a = (*it)->getStrongestHero();
      if (!a)
	continue;
      h = dynamic_cast<Hero*>(a);
      if (h->getStat(Army::STRENGTH) > highest_strength)
	{
	  highest_strength = h->getStat(Army::STRENGTH);
	  strongestHero = h;
	}
    }
  calculateModifiedStrengths (d_def_close, d_att_close, true, strongestHero);

  calculateFinalStrengths (d_att_close, d_def_close);
  calculateFinalStrengths (d_def_close, d_att_close);

}

void Fight::fightArmies(Fighter* attacker, Fighter* defender)
{
  static int misses_in_a_row;
  guint32 sides = 0;

  if (!attacker || !defender)
    return;

  Army *a = attacker->army;
  Army *d = defender->army;

  debug("Army " << a->getId() << " attacks " << d->getId())

    if (d_intense_combat == true)
      sides = 24;
    else
      sides = 20;

  // factor used for some calculation regarding gaining medals
  double xp_factor = a->getXpReward() / d->getXpReward();

  // the clash has to be documented for later use in the fight dialog

  // make a swing at the opponent
  // take one hit point off, per hit.

  FightItem item;
  item.turn = d_turn;
  int damage = 0;
  item.id = d->getId();

  while (damage == 0)
    {
      int attacker_roll = rand() % sides;
      int defender_roll = rand() % sides;

      if (attacker_roll <= attacker->terrain_strength &&
	  defender_roll > defender->terrain_strength)
	{
	  //hit defender
	  if (d_type == FOR_KEEPS)
	    {
	      a->setNumberHasHit(a->getNumberHasHit() + (1/xp_factor));
	      d->setNumberHasBeenHit(d->getNumberHasBeenHit() + (1/xp_factor));
	    }
	  d->damage(1);
	  damage = 1;
	  item.id = d->getId();
	  misses_in_a_row = 0;
	}
      else if (defender_roll <= defender->terrain_strength &&
	       attacker_roll > attacker->terrain_strength)
	{
	  //hit attacker
	  if (d_type == FOR_KEEPS)
	    {
	      d->setNumberHasHit(d->getNumberHasHit() + (1/xp_factor));
	      a->setNumberHasBeenHit(a->getNumberHasBeenHit() + (1/xp_factor));
	    }
	  a->damage(1);
	  damage = 1;
	  item.id = a->getId();
	  misses_in_a_row = 0;
	}
      else
	{
	  misses_in_a_row++;
	  if (misses_in_a_row >= 10000)
	    {
	      //defender automatically wins
	      //hit attacker for however much it takes
	      if (d_type == FOR_KEEPS)
		{
		  d->setNumberHasHit(d->getNumberHasHit() + (1/xp_factor));
		  a->setNumberHasBeenHit(a->getNumberHasBeenHit() + 
					 (1/xp_factor));
		}
	      item.id = a->getId();
	      damage = a->getHP();
	      a->damage (damage);
	      misses_in_a_row = 0;
	    }
	}
    }
  // continue documenting the engagement
  item.damage = damage;
  d_actions.push_back(item);

}

void Fight::remove(Fighter* f)
{
  std::list<Fighter*>::iterator it;

  // is the fighter in the attacker lists?
  for (it = d_att_close.begin(); it != d_att_close.end(); it++)
    if ((*it) == f)
      {
	d_att_close.erase(it);
	delete f;
	return;
      }

  // or in the defender lists?
  for (it = d_def_close.begin(); it != d_def_close.end(); it++)
    if ((*it) == f)
      {
	d_def_close.erase(it);
	delete f;
	return;
      }

  // if the fighter wa sin no list, we are rather careful and don't do anything
  debug("Fight: fighter without list!")
}
	
guint32 Fight::getModifiedStrengthBonus(Army *a)
{
  std::list<Fighter*>::iterator it;
  for (it = d_att_close.begin(); it != d_att_close.end(); it++)
    if ((*it)->army == a)
      return (*it)->terrain_strength;
  for (it = d_def_close.begin(); it != d_def_close.end(); it++)
    if ((*it)->army == a)
      return (*it)->terrain_strength;
  return 0;
}

void Fight::fillInInitialHPs()
{
  for (std::list<Stack *>::iterator i = d_attackers.begin();
       i != d_attackers.end(); ++i)
    for (Stack::iterator j = (*i)->begin(); j != (*i)->end(); ++j)
      initial_hps[(*j)->getId()] = (*j)->getHP();
  
  for (std::list<Stack *>::iterator i = d_defenders.begin();
       i != d_defenders.end(); ++i)
    for (Stack::iterator j = (*i)->begin(); j != (*i)->end(); ++j)
      initial_hps[(*j)->getId()] = (*j)->getHP();
}
	
LocationBox Fight::calculateFightBox(Fight &fight)
{
  Citylist *cl = Citylist::getInstance();
  Vector<int> dest = fight.getAttackers().front()->getPos();
  if (cl->getObjectAt(dest) == NULL)
    return LocationBox(dest);
  Player *p = fight.getAttackers().front()->getOwner();
  Stack *s = fight.getAttackers().front();
  std::list<Vector<int> > tracks = p->getStackTrack(s);
  if (tracks.size() >= 2)
    {
      std::list<Vector<int> >::iterator it = tracks.end();
      it--; it--;
      return LocationBox(*it, dest);
    }
  else
    {
      //this shouldn't be the case
      return LocationBox(s->getPos(), dest);
    }
}
