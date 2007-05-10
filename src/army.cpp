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

#include <iostream>
#include <sstream>
#include "army.h"
#include "armysetlist.h"
#include "counter.h"
#include "GraphicsCache.h"
#include "xmlhelper.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

sigc::signal<void, Army*> Army::sdying;

Army::Army(const Army& a, Player* p)
    :d_type(a.d_type), d_armyset(a.d_armyset), d_pixmap(0), d_portrait(0),
     d_mask(0), d_name(a.d_name), d_description(a.d_description), 
     d_production(a.d_production),
     d_production_cost(a.d_production_cost), d_upkeep(a.d_upkeep),
     d_strength(a.d_strength), d_ranged(a.d_ranged), d_shots(a.d_shots),
     d_defense(a.d_defense), d_max_hp(a.d_max_hp),
     d_max_moves(a.d_max_moves), d_vitality(a.d_vitality), d_sight(a.d_sight),
     d_xp_value(a.d_xp_value), d_move_bonus(a.d_move_bonus),
     d_army_bonus(a.d_army_bonus), d_gender(a.d_gender), d_player(p),
     d_id(a.d_id), d_hp(a.d_hp), d_moves(a.d_moves), d_xp(a.d_xp),
     d_level(a.d_level), d_blessed(a.d_blessed), d_grouped(a.d_grouped),
     d_battles_number(a.d_battles_number), d_number_hashit(a.d_number_hashit),
     d_number_hasbeenhit(a.d_number_hasbeenhit), 
     d_defends_ruins(a.d_defends_ruins)
{
    // if we have been copied from an army prototype, initialise several values
    if (d_id == 0)
    {
        d_id = fl_counter->getNextId();
        d_hp = d_max_hp;
        d_moves = d_max_moves;
    }
        
    for(int i=0;i<3;i++)
    {
        d_medal_bonus[i] = a.d_medal_bonus[i];
    }  
}

Army::Army(XML_Helper* helper, bool prototype)
  :d_pixmap(0), d_portrait(0), d_mask(0), d_name(""), d_description(""),
   d_gender(NONE), d_player(0),
   d_id(0), d_xp(0), d_level(1), d_blessed(false), d_grouped(true),
   d_number_hashit(0), d_number_hasbeenhit(0), d_defends_ruins(false)
{
    // first, load the data that has to be loaded anyway
    helper->getData(d_strength, "strength");
    helper->getData(d_ranged, "ranged");
    helper->getData(d_shots, "shots");
    helper->getData(d_defense, "defense");
    helper->getData(d_vitality, "vitality");
    helper->getData(d_sight, "sight");
    helper->getData(d_max_hp, "maxhp");
    helper->getData(d_max_moves, "max_moves");
    helper->getData(d_xp_value,"expvalue");
    
    // now if we are a prototype, we have to load different data than
    // otherwise
    if (!prototype)
    {
        //get the information which army we are
        helper->getData(d_type, "type");
        helper->getData(d_armyset, "armyset");

        copyVals(Armysetlist::getInstance()->getArmy(d_armyset,d_type));

        //adjust the values with the saved data
        helper->getData(d_id, "id");
        helper->getData(d_hp, "hp");
        helper->getData(d_moves, "moves");
        helper->getData(d_xp, "xp");
        helper->getData(d_level, "level");
        helper->getData(d_blessed, "blessed");

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
    }
    else
    {
        helper->getData(d_name, "name");
        helper->getData(d_description, "description");
        helper->getData(d_production, "production");
        helper->getData(d_production_cost, "production_cost");
        helper->getData(d_upkeep, "upkeep");
        helper->getData(d_move_bonus, "move_bonus");
        helper->getData(d_army_bonus, "army_bonus");

	helper->getData(d_defends_ruins,"defends_ruins");
	fprintf (stderr, "d_defends_ruins is %d\n", d_defends_ruins);

        if (!helper->getData(d_gender, "gender"))
            d_gender = NONE;


        d_hp = d_max_hp;
        d_moves = d_max_moves;
        d_battles_number = 0;

        for(int i = 0; i < 3; i++)
            d_medal_bonus[i] = false;
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
        case STRENGTH:  d_strength = value;
                        break;
        case DEFENSE:   d_defense = value;
                        break;
        case VITALITY:  d_vitality = value;
                        break;
        case HP:        d_max_hp = value;
                        if (d_hp > d_max_hp)
                            d_hp = value;
                        break;
        case MOVES:     d_max_moves = value;
                        if (d_moves > d_max_moves)
                            d_moves = value;
                        break;
        case MOVE_BONUS:    d_move_bonus = value;
                            break;
        case ARMY_BONUS:    d_army_bonus = value;
                            break;
        case SIGHT:         d_sight = value;
                            break;
        case RANGED:    d_ranged = value;
                        break;
        case SHOTS:     d_shots = value;
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
                                         d_player, d_level, d_medal_bonus);
}

SDL_Surface* Army::getPortrait() const
{
    // only the prototypes, which also keep the images, have the portrait
    if (!d_pixmap)
        return Armysetlist::getInstance()->getArmy(d_armyset, d_type)->getPortrait();

    return d_portrait;
}


Uint32 Army::getStat(Stat stat, bool modified) const
{
    switch (stat)
    {
        case STRENGTH:
            return d_strength;
        case DEFENSE:
            return d_defense;
        case VITALITY:
            return d_vitality;
        case HP:
            return d_max_hp;
        case MOVES:
            return d_max_moves;
        case MOVE_BONUS:
            return d_move_bonus;
        case ARMY_BONUS:
            return d_army_bonus;
        case SIGHT:
            return d_sight;
        case RANGED:
            return d_ranged;
        case SHOTS:
            return d_shots;
    }

    // should never come to this
    return 0;
}

void Army::resetMoves()
{
    d_moves = getStat(MOVES);
}

void Army::bless()
{
    if (!d_blessed)
    {
        d_blessed = true;
        d_strength++;
    }
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
        if (hp + getStat(VITALITY) <= 5)
            hp = 1;
        else
            hp += getStat(VITALITY) - 5;
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
    int tmp=(int) getXP()/getLevel();

    if (((int) tmp/10)>=1)
        return true;
    else
        return false;
}

int Army::gainLevel(Stat stat, bool dummy)
{
    if (!dummy && !canGainLevel())
        return -1;

    if (stat == MOVE_BONUS || stat == ARMY_BONUS || stat == SHOTS)
        return -1;
    
    if (!dummy)
    {
        d_level++;
        d_xp_value *= 1.2;

        switch (stat)
        {
            case STRENGTH:
                d_strength++;
                break;
            case DEFENSE:
                d_defense++;
                break;
            case VITALITY:
                d_vitality++;
                break;
            case HP:
                d_max_hp += 4;
                break;
            case MOVES:
                d_max_moves +=4;
                break;
            case SIGHT:
                d_sight++;
                break;
            case RANGED:
                d_ranged++;
                break;
            default:
                break;
        }
    }

    switch (stat)
    {
        case STRENGTH:
        case DEFENSE:
        case VITALITY:
        case SIGHT:
        case RANGED:
            return 1;
        case HP:
        case MOVES:
            return 4;
        default:
            return -1;
    }
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
    
    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("armyset", d_armyset);
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("hp", d_hp);
    retval &= helper->saveData("strength", d_strength);
    retval &= helper->saveData("ranged", d_ranged);
    retval &= helper->saveData("shots", d_shots);
    retval &= helper->saveData("defense", d_defense);
    retval &= helper->saveData("vitality", d_vitality);
    retval &= helper->saveData("sight", d_sight);
    retval &= helper->saveData("maxhp", d_max_hp);
    retval &= helper->saveData("moves", d_moves);
    retval &= helper->saveData("max_moves", d_max_moves);
    retval &= helper->saveData("xp", d_xp);
    retval &= helper->saveData("expvalue", getXpReward());
    retval &= helper->saveData("level", d_level);
    retval &= helper->saveData("blessed", d_blessed);

    std::stringstream medals;
    for (int i=0;i<3;i++)
    {
        medals << d_medal_bonus[i] << " ";
    }
    retval &= helper->saveData("medals", medals.str());
    retval &= helper->saveData("battlesnumber",d_battles_number);    

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
    std::cerr << "ranged = " << d_ranged << std::endl;
    std::cerr << "shots = " << d_shots << std::endl;
    std::cerr << "defense = " << d_defense << std::endl;
    std::cerr << "vitality = " << d_vitality << std::endl;
    std::cerr << "max_moves = " << d_max_moves << std::endl;
    std::cerr << "upkeep = " << d_upkeep << std::endl;
    std::cerr << "defends_ruins = " << d_defends_ruins << std::endl;
    std::cerr << "production = " << d_production << std::endl;
    std::cerr << "production_cost = " << d_production_cost << std::endl;
    std::cerr << "move_bonus = " << d_move_bonus << std::endl;
    std::cerr << "army_bonus = " << d_army_bonus << std::endl;

    std::cerr << "type = "    << d_type    << std::endl;
    std::cerr << "level = "   << d_level   << std::endl;
    std::cerr << "xp = "      << d_xp      << std::endl;
    std::cerr << "blessed = " << d_blessed << std::endl;
    std::cerr << "grouped = " << d_grouped << std::endl;

    std::cerr << "medal[0] = " << d_medal_bonus[0] << std::endl;
    std::cerr << "medal[1] = " << d_medal_bonus[1] << std::endl;
    std::cerr << "medal[2] = " << d_medal_bonus[2] << std::endl;

    std::cerr << "battle number = "     << d_battles_number    << std::endl;
    std::cerr << "has hit = "           << d_number_hashit     << std::endl;
    std::cerr << "has been hit = "      << d_number_hasbeenhit << std::endl;
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
    d_gender = a->getGender();
    d_defends_ruins = a->getDefendsRuins();
}
