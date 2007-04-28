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
#include "stacklist.h"
#include "player.h"
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
        Fighter(Army* a, PG_Point p);
        
        Army* army;
        PG_Point pos;       // location on the map (needed to calculate boni)
        int shots;          // number of shots left
        int att_bonus;
        int def_bonus;
};

Fighter::Fighter(Army* a, PG_Point p)
    :army(a), pos(p), shots(a->getStat(Army::SHOTS)), att_bonus(0),
    def_bonus(0)
{
}



Fight::Fight(Stack* attacker, Stack* defender, bool duel)
    :d_duel(duel), d_turn(0), d_result(DRAW)
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
        d_att_ranged.push_back(f);
    }

    for (Stack::iterator it = defender->begin(); it != defender->end(); it++)
    {
        Fighter* f = new Fighter((*it), defender->getPos());
        d_def_ranged.push_back(f);
    }
    
    if (d_duel)
        return;
    
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
    PG_Point p = defender->getPos();

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

                    if ((land && (*sit)->getStat(Army::ARMY_BONUS) & Army::SHIP)
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
                        Fighter* f = new Fighter((*sit), PG_Point(x,y));
                        d_def_ranged.push_back(f);
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
    
    while (!d_att_ranged.empty())
    {
        delete (*d_att_ranged.begin());
        d_att_ranged.erase(d_att_ranged.begin());
    }
    
    while (!d_def_ranged.empty())
    {
        delete (*d_def_ranged.begin());
        d_def_ranged.erase(d_def_ranged.begin());
    }
}

void Fight::battle()
{
    // first, fight until the fight is over
    for (d_turn = 0; doRound(); d_turn++);

    // Now we have to set the fight result.
    
    // First, look if the attacker died; the attacking stack is the first
    // one in the list
    bool survivor = false;
    Stack* s = (*d_attackers.begin());
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
    s = (*d_defenders.begin());
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
    if (d_turn >= MAX_ROUNDS)
        return false;
    
    debug ("Fight round #" <<d_turn)
    
    // at the beginning of the round, set the defense boni and separate fighters
    // into close and ranged combat units.
    calculateBonus();
    shuffleLines();

    // Now, to give the attacker a bonus, his units attack first.
    std::list<Fighter*>::iterator it;
    Fighter* f;
    
    for (it = d_att_close.begin(); it != d_att_close.end(); it++)
    {
        f = findVictim(true, false);
        fightArmies((*it), f, true);
        if (f && f->army->getHP() <= 0)
            remove(f);
    }
            

    for (it = d_att_ranged.begin(); it != d_att_ranged.end(); it++)
    {
        f = findVictim(true, true);
        fightArmies((*it), f, true);
        if (f && f->army->getHP() <= 0)
            remove(f);
    }

    // noone left for fighting
    if (d_def_close.empty() && d_def_ranged.empty())
        return false;

    // now the remaining defenders strike back
    for (it = d_def_close.begin(); it != d_def_close.end(); it++)
    {
        f = findVictim(false, false);
        fightArmies((*it), f, true);
        if (f && f->army->getHP() <= 0)
            remove(f);
    }

    for (it = d_def_ranged.begin(); it != d_def_ranged.end(); it++)
    {
        f = findVictim(false, true);
        fightArmies((*it), f, true);
        if (f && f->army->getHP() <= 0)
            remove(f);
    }

    // if attackers were defeated, signal to stop the battle
    if (d_att_close.empty() && d_att_ranged.empty())
        return false;

    
    // last job: loop through all lists and look if there are any regenerating
    // units. If so, heal them by 1 HP.
    healArmies(d_att_ranged);
    healArmies(d_def_ranged);
    healArmies(d_att_close);
    healArmies(d_def_close);

    
    return true;
}

void Fight::calculateBonus()
{
    // we currently know of two different boni: 
    // 1. If there is a hero, add a +1 strength bonus
    // 2. If we don't have a duel situation, the defender gets a bonus
    //    depending on the terrain
    std::list<Stack*>::const_iterator it;
    Stack::const_iterator sit;
    std::list<Fighter*>::iterator fit;

    // first, check for attacker heroes
    bool hasHero = false;
    for (it = d_attackers.begin(); it != d_attackers.end(); it++)
        for (sit = (*it)->begin(); sit != (*it)->end(); sit++)
            if (((*sit)->getStat(Army::ARMY_BONUS) & Army::LEADER)
                && ((*sit)->getHP() > 0))
            {
                hasHero = true;
                break;
            }

    if (hasHero)
    {
        // add a +1 strength bonus to all units; heroes don't get this bonus :)
        for (fit = d_att_close.begin(); fit != d_att_close.end(); fit++)
            if (!((*fit)->army->getStat(Army::ARMY_BONUS) & Army::LEADER))
                (*fit)->att_bonus = 1;

        for (fit = d_att_ranged.begin(); fit != d_att_ranged.end(); fit++)
            if (!((*fit)->army->getStat(Army::ARMY_BONUS) & Army::LEADER))
                (*fit)->att_bonus = 1;
    }

    // now check for defender heroes
    bool hasLeader = false;
    for (it = d_defenders.begin(); it != d_defenders.end(); it++)
        for (sit = (*it)->begin(); sit != (*it)->end(); sit++)
            if (((*sit)->getStat(Army::ARMY_BONUS) & Army::LEADER)
                && ((*sit)->getHP() > 0))
            {
                hasLeader = true;
                break;
            }

    if (hasLeader)
    {
        // add a +1 strength bonus to all units; heroes don't get this bonus :)
        for (fit = d_def_close.begin(); fit != d_def_close.end(); fit++)
            if (!((*fit)->army->getStat(Army::ARMY_BONUS) & Army::LEADER))
                (*fit)->att_bonus = 1;

        for (fit = d_def_ranged.begin(); fit != d_def_ranged.end(); fit++)
            if (!((*fit)->army->getStat(Army::ARMY_BONUS) & Army::LEADER))
                (*fit)->att_bonus = 1;
    }


    // Second step: defender units get a defense bonus
    if (d_duel)
        return;

    for (fit = d_def_close.begin(); fit != d_def_close.end(); fit++)
    {
        // the defense bonus is given in 10% steps
        int bonus = 10 * GameMap::getInstance()->getTile((*fit)->pos)->getDefense();

        (*fit)->def_bonus = (bonus * (int)(*fit)->army->getStat(Army::DEFENSE))/100;
    }

    for (fit = d_def_ranged.begin(); fit != d_def_ranged.end(); fit++)
    {
        // the defense bonus is given in 10% steps
        int bonus = 10 * GameMap::getInstance()->getTile((*fit)->pos)->getDefense();

        (*fit)->def_bonus = (bonus * (int)(*fit)->army->getStat(Army::DEFENSE))/100;
    }
}

void Fight::shuffleLines()
{
    debug("Fight::shuffleLines")
        
    // the basic problem is simple: Take all ranged units that have no shots
    // left and add them to the list of close combat units.

    // first the attackers
    std::list<Fighter*>::iterator it;
    for (it = d_att_ranged.begin(); it != d_att_ranged.end();)
    {
        if ((*it)->shots == 0)
        {
            Fighter* f = *it;
            it = d_att_ranged.erase(it);
            d_att_close.push_back(f);
            debug("Attacker "<<f->army->getId()<<" has no shots, moving to front")
            continue;
        }

        debug("Attacker " <<(*it)->army->getId() <<" has "
                <<(*it)->shots <<" shots left")
        it++;
    }

    // then exactly the same for the defenders
    for (it = d_def_ranged.begin(); it != d_def_ranged.end();)
    {
        if ((*it)->shots == 0)
        {
            Fighter* f = *it;
            it = d_def_ranged.erase(it);
            d_def_close.push_back(f);
            debug("Defender "<<f->army->getId()<<" has no shots, moving to front")
            continue;
        }

        debug("Defender " <<(*it)->army->getId() <<" has "
                <<(*it)->shots <<" shots left")
        it++;
    }
}

Fighter* Fight::findVictim(bool attacker, bool ranged) const
{
    const std::list<Fighter*>* lst=0;

    // first, find the list to take the victim from, depending on the parameters

    if (attacker && !ranged)
    {
        // take close combat defender list; if it is empty, attack ranged units
        lst = &d_def_close;
        if (d_def_close.empty())
            lst = &d_def_ranged;
    }

    if (!attacker && !ranged)
    {
        // the same, but the other way round
        lst = &d_att_close;
        if (d_att_close.empty())
            lst = &d_att_ranged;
    }

    if (attacker && ranged)
    {
        // with a chance of 1/3, take enemy ranged units, else attack the
        // enemy's close combat units, but only if there are any
        lst = &d_def_close;
        if (d_def_close.empty() || (rand() % 3) == 0)
            lst = &d_def_ranged;
        if (d_def_ranged.empty())
            lst = &d_def_close;
    }
    
    if (!attacker && ranged)
    {
        // the same for defenders
        lst = &d_att_close;
        if (d_att_close.empty() || (rand() % 3) == 0)
            lst = &d_att_ranged;
        if (d_att_ranged.empty())
            lst = &d_att_close;
    }

    // if the list is empty, there are no enemy units; return 0 and hope that
    // this problem is caught somewhere else. ;)
    if (lst->empty())
        return 0;

    // take a random unit from the list we have
    int no = rand() % lst->size();
    std::list<Fighter*>::const_iterator it;
    for (it = lst->begin(); no > 0; no--, it++);
    
    return *it;
}

void Fight::fightArmies(Fighter* culprit, Fighter* victim, bool attack)
{
    if (!victim || !culprit)
        return;

    debug("Army " <<culprit->army->getId() <<" attacks " <<victim->army->getId())

    // I implicitely assume here that armies with ammunition left attack from
    // the distance (should be safe).
    int strength;
    int defense = victim->army->getStat(Army::DEFENSE) + victim->def_bonus;
    bool melee = (culprit->shots == 0);

    // factor used for some calculation regarding gaining medals
    double xp_factor = culprit->army->getXpReward() / victim->army->getXpReward();
    
    if (melee)
        strength = culprit->army->getStat(Army::STRENGTH) + culprit->att_bonus;
    else
    {
        strength = culprit->army->getStat(Army::RANGED) + culprit->att_bonus;
        culprit->shots--;
    }
    
    // cavalry gets a bonus on open terrain if it charges
    if (attack && melee && (culprit->army->getStat(Army::ARMY_BONUS) & Army::CAVALRY)
            && (GameMap::getInstance()->getTile(victim->pos)->getMaptileType() == Tile::GRASS))
        strength++;

    // anticavalry units have their strength doubled against mounted troops
    if (melee && (culprit->army->getStat(Army::ARMY_BONUS) & Army::ANTICAVALRY)
            && (victim->army->getStat(Army::ARMY_BONUS) & Army::CAVALRY))
        strength *= 2;

    // ships in cities have their strength halved
    if ((culprit->army->getStat(Army::ARMY_BONUS) & Army::SHIP)
            && (GameMap::getInstance()->getTile(culprit->pos)->getBuilding() == Maptile::CITY))
        strength /= 2;
    
    debug("strength: " <<strength <<" defense: "<<defense)

    // the clash has to be documented for later use in the fight dialog
    FightItem item;
    item.turn = d_turn;
    item.id = victim->army->getId();
    int hp = victim->army->getHP();
    
    // The fighting algorithm is as follows:
    // For each point of strength, the attacking unit rolls once to damage the
    // defender. The propability to damage the defender in one roll is
    // 1/(defense). Additionally, units which can do critical hits have a chance
    // to instantly kill the other unit with each successful hit. Otherwise, the
    // defender looses one hitpoint per successful roll.
    for (int i = 0; i < strength; i++)
    {
        if (rand() % defense > 0)
        {
            // miss
            victim->army->setNumberHasBeenHit(victim->army->getNumberHasBeenHit()
                                              - xp_factor);
            continue;
        }

        debug("Hit!")

        // modify hit stats of attacker/defender
        culprit->army->setNumberHasHit(culprit->army->getNumberHasHit()
                                        + (1/xp_factor));
        victim->army->setNumberHasBeenHit(victim->army->getNumberHasBeenHit()
                                        + (1/xp_factor));

        // hit, check for instant death
        if ((culprit->army->getStat(Army::ARMY_BONUS) & Army::CRITICAL)
            && (rand() % 100 == 0))
        {
            debug ("Instant kill!")
            
            victim->army->damage(victim->army->getHP());
            break;
        }

        // else the defender looses just one hitpoint
        victim->army->damage(1);
        if (victim->army->getHP() <= 0)
            break;
    }


    // continue documenting the engagement
    item.damage = hp - victim->army->getHP();
    d_actions.push_back(item);
}

void Fight::healArmies(std::list<Fighter*>& list)
{
    std::list<Fighter*>::iterator it;
    for (it = list.begin(); it != list.end(); it++)
        if ((*it)->army->getStat(Army::ARMY_BONUS) & Army::REGENERATE)
        {
            debug ("Healing army" <<(*it)->army->getId())
            (*it)->army->heal(1);
            FightItem item;
            item.turn = d_turn;
            item.id = (*it)->army->getId();
            item.damage = -1;
            d_actions.push_back(item);
        }
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
    
    for (it = d_att_ranged.begin(); it != d_att_ranged.end(); it++)
        if ((*it) == f)
        {
            d_att_ranged.erase(it);
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
    
    for (it = d_def_ranged.begin(); it != d_def_ranged.end(); it++)
        if ((*it) == f)
        {
            d_def_ranged.erase(it);
            delete f;
            return;
        }

    // if the fighter wa sin no list, we are rather careful and don't do anything
    debug("Fight: fighter without list!")
}
