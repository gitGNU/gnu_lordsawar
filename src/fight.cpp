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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include "fight.h"
#include <algorithm>
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
#include "defs.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

// Helper class; the single units participating in the fight are saved with
// additional information. This should be a struct, but I don't know how to
// forward the declaration properly.
class Fighter
{
    public:
        Fighter(Army* a, Vector<int> p);
        
        Army* army;
        Vector<int> pos;       // location on the map (needed to calculate boni)
        int terrain_strength;
};

Fighter::Fighter(Army* a, Vector<int> p)
    :army(a), pos(p)
{
}



Fight::Fight(Stack* attacker, Stack* defender)
    : d_turn(0), d_result(DRAW)
{
    debug("Fight between " <<attacker->getId() <<" and " <<defender->getId())
    
    // Duel case: two stacks fight each other; Nothing further to be done
    // Important: We always assume that the attacking/defending stacks are
    // the first in the list!!!
    d_attackers.push_back(attacker);
    d_defenders.push_back(defender);

    for (Stack::iterator it = attacker->begin(); it != attacker->end(); it++)
    {
        Fighter* f = new Fighter((*it), attacker->getPos());
        d_att_close.push_back(f);
    }

    for (Stack::iterator it = defender->begin(); it != defender->end(); it++)
    {
        Fighter* f = new Fighter((*it), defender->getPos());
        d_def_close.push_back(f);
    }
    
    // What we do here: In the setup, we need to find out all armies that
    // participate in the fight. For this we separate three cases:
    //
    // 1. Land unit is attacked
    // 2. Sea unit is attacked
    // 3. City is attacked
    //
    // In the first and second case, we take all stacks around the 
    // attacked unit that can move on (water/non-water), i.e.
    // exclude ships from land-fights and land-units from sea battles.
    // In the third case, Attacker and defenders throw in everything they have

    Maptile* tile = GameMap::getInstance()->getTile(defender->getPos());
    Vector<int> p = defender->getPos();

    bool land = false;
    bool sea = false;
    
    if (tile->getBuilding() == Maptile::CITY || tile->getMaptileType() == Tile::WATER)
        sea = true;
    if (tile->getBuilding() == Maptile::CITY || tile->getMaptileType() != Tile::WATER)
        land = true;

    for (int x = p.x - 1; x <= p.x + 1; x++)
        for (int y = p.y - 1; y <= p.y + 1; y++)
        {
            if (x < 0 || x >= GameMap::getInstance()->getWidth()
                || y < 0 || y >= GameMap::getInstance()->getHeight())
                continue;

            tile = GameMap::getInstance()->getTile(x,y);
            
            // look for attackers
            Stack* s = Stacklist::getObjectAt(x,y);
            Stack::const_iterator sit;

            if (!s)
                continue;
            
            if (s->getPlayer() == defender->getPlayer()
                && s != (*d_defenders.begin()))
            {
                // check if stack may participate
                bool valid = true;
                for (sit = s->begin(); sit != s->end(); sit++)
                {
                    if (land && sea)
                        break;

                    if ((land && (*sit)->getStat(Army::SHIP))
                        || (sea && !((*sit)->getStat(Army::MOVE_BONUS) & Tile::WATER)))
                    {
                        valid = false;
                        break;
                    }
                }

                if (valid)
                {
                    debug("Adding stack " <<s->getId() <<" to defenders")
                        
                    // add units to list of fighters
                    d_defenders.push_back(s);
                    for (sit = s->begin(); sit != s->end(); sit++)
                    {
                        Fighter* f = new Fighter((*sit), Vector<int>(x,y));
                        d_def_close.push_back(f);
                    }
                }
            }
        }

    // Now some last initializing. Each unit has its battle number increased.
    std::list<Stack*>::iterator it;
    
    for (it = d_attackers.begin(); it != d_attackers.end(); it++)
        for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
            (*sit)->setBattlesNumber((*sit)->getBattlesNumber() + 1);
    
    for (it = d_defenders.begin(); it != d_defenders.end(); it++)
        for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
            (*sit)->setBattlesNumber((*sit)->getBattlesNumber() + 1);
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

void Fight::battle()
{
    // at the beginning of the battle, calculate the bonuses
    // bonuses remain even if the unit providing a stackwide bonus dies
    calculateBonus();

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
    {
        d_result = DEFENDER_WON;
        return;
    }

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

bool Fight::doRound()
{
    if (MAX_ROUNDS && d_turn >= MAX_ROUNDS)
        return false;
    
    debug ("Fight round #" <<d_turn)
    
    // Now, to give the attacker a bonus, his units attack first.
    std::list<Fighter*>::iterator it;
    
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
  Uint32 army_bonus;
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
  Uint32 army_bonus;
  GameMap *gm = GameMap::getInstance();
  Maptile *mtile;
  std::list<Fighter*>::iterator fit;

  //find highest non-hero bonus
  Uint32 highest_non_hero_bonus = 0;
  for (fit = friendly.begin(); fit != friendly.end(); fit++)
    {
      Uint32 non_hero_bonus = 0;
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
  Uint32 hero_bonus = 0;
  if (strongestHero)
    {
      std::list<Item*> backpack = strongestHero->getBackpack();
      std::list<Item*>::const_iterator item;
      // count up the bonuses from command items
      for (item = backpack.begin(); item != backpack.end(); item++)
        {
          if ((*item)->getBonus(Item::ADD1STACK))
            hero_bonus += 1;
          if ((*item)->getBonus(Item::ADD2STACK))
            hero_bonus += 2;
          if ((*item)->getBonus(Item::ADD3STACK))
            hero_bonus += 3;
        }
    }

   //FIXME: confirm that we only add the strongest hero's command items.
   //(and not all items from every hero in the stack)
   //yes, we need to go grab ALL command items

  //now add on the hero's natural command
  if (strongestHero)
    {
      Uint32 hero_strength = strongestHero->getStat(Army::STRENGTH, true);
      if (hero_strength == 9)
        hero_bonus += 3;
      else  if (hero_strength > 6)
        hero_bonus += 2;
      else  if (hero_strength > 3)
        hero_bonus += 1;
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

  Uint32 fortify_bonus = 0;
  for (fit = friendly.begin(); fit != friendly.end(); fit++)
    {
      army_bonus = (*fit)->army->getStat(Army::ARMY_BONUS);
      if (army_bonus & Army::FORTIFY)
        {
          fortify_bonus = 1;
          break;
        }
    }

  Uint32 city_bonus = 0;
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
          else if (c->getNoOfBasicProd() <= 2 && c->getPlayer() ==
                   Playerlist::getInstance()->getNeutral())
            city_bonus = 0;
          else if (c->getNoOfBasicProd() <= 2 && c->getPlayer() ==
                               Playerlist::getInstance()->getActiveplayer())
            city_bonus = 1;
          else if (c->getNoOfBasicProd() > 2 && c->getPlayer() ==
                   Playerlist::getInstance()->getNeutral())
            city_bonus = 1;
          else if (c->getNoOfBasicProd() > 2 && c->getPlayer() ==
                   Playerlist::getInstance()->getActiveplayer())
            city_bonus = 2;
        }
      else
        {
          if (mtile->getBuilding() == Maptile::TEMPLE)
            city_bonus = 2;
          else if (mtile->getBuilding() == Maptile::RUIN)
            city_bonus = 2;
        }

      // FIXME: implement towers.  they get a city bonus

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

  Uint32 total_bonus = highest_non_hero_bonus + hero_bonus + fortify_bonus + 
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
  Uint32 army_bonus;
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
    Uint32 highest_strength = 0;
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
  bool intense_combat = false;
  Uint32 sides = 0;

  if (!attacker || !defender)
    return;

  Army *a = attacker->army;
  Army *d = defender->army;

  debug("Army " << a->getId() << " attacks " << d->getId())

  if (intense_combat)
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
          a->setNumberHasHit(a->getNumberHasHit() + (1/xp_factor));
          d->setNumberHasBeenHit(d->getNumberHasBeenHit() + (1/xp_factor));
          d->damage(1);
          damage = 1;
          item.id = d->getId();
          misses_in_a_row = 0;
        }
      else if (defender_roll <= defender->terrain_strength &&
               attacker_roll > attacker->terrain_strength)
        {
          //hit attacker
          d->setNumberHasHit(d->getNumberHasHit() + (1/xp_factor));
          a->setNumberHasBeenHit(a->getNumberHasBeenHit() + (1/xp_factor));
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
              d->setNumberHasHit(d->getNumberHasHit() + (1/xp_factor));
              a->setNumberHasBeenHit(a->getNumberHasBeenHit() + (1/xp_factor));
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
