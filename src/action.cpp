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

#include <stdlib.h>
#include <sstream>

#include "action.h"
#include "stack.h"
#include "army.h"
#include "QKillHero.h"
#include "QEnemyArmies.h"
#include "QRuinSearch.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<< x << endl<<flush;}
//#define debug(x)

Action::Action(Type type)
    :d_type(type)
{
}

Action::~Action()
{
}

Action* Action::handle_load(XML_Helper* helper)
{
    Uint32 t;
    helper->getData(t, "type");

    switch (t)
    {
        case STACK_MOVE:
            return (new Action_Move(helper));
        case STACK_SPLIT:
            return (new Action_Split(helper));
        case STACK_FIGHT:
            return (new Action_Fight(helper));
        case STACK_JOIN:
            return(new Action_Join(helper));
        case RUIN_SEARCH:
            return (new Action_Ruin(helper));
        case TEMPLE_SEARCH:
            return (new Action_Temple(helper));
        case CITY_OCCUPY:
            return (new Action_Occupy(helper));
        case CITY_PILLAGE:
            return (new Action_Pillage(helper));
        case CITY_SACK:
            return (new Action_Sack(helper));
        case CITY_RAZE:
            return (new Action_Raze(helper));
        case CITY_UPGRADE:
            return (new Action_Upgrade(helper));
        case CITY_BUY:
            return (new Action_Buy(helper));
        case CITY_PROD:
            return (new Action_Production(helper));
        case REWARD: 
            return (new Action_Reward(helper));
        case QUEST:
            return (new Action_Quest(helper));
        case HERO_EQUIP:
            return (new Action_Equip(helper));
        case UNIT_ADVANCE:
            return (new Action_Level(helper));
    }

    return 0;
}

Action* Action::copy(const Action* a)
{
    switch(a->getType())
    {
        case STACK_MOVE:
            return (new Action_Move(*dynamic_cast<const Action_Move*>(a)));
        case STACK_SPLIT:
            return (new Action_Split(*dynamic_cast<const Action_Split*>(a)));
        case STACK_FIGHT:
            return (new Action_Fight(*dynamic_cast<const Action_Fight*>(a)));
        case STACK_JOIN:
            return(new Action_Join(*dynamic_cast<const Action_Join*>(a)));
        case RUIN_SEARCH:
            return (new Action_Ruin(*dynamic_cast<const Action_Ruin*>(a)));
        case TEMPLE_SEARCH:
            return (new Action_Temple(*dynamic_cast<const Action_Temple*>(a)));
        case CITY_OCCUPY:
            return (new Action_Occupy(*dynamic_cast<const Action_Occupy*>(a)));
        case CITY_PILLAGE:
            return (new Action_Pillage(*dynamic_cast<const Action_Pillage*>(a)));
        case CITY_SACK:
            return (new Action_Sack(*dynamic_cast<const Action_Sack*>(a)));
        case CITY_RAZE:
            return (new Action_Raze(*dynamic_cast<const Action_Raze*>(a)));
        case CITY_UPGRADE:
            return (new Action_Upgrade(*dynamic_cast<const Action_Upgrade*>(a)));
        case CITY_BUY:
            return (new Action_Buy(*dynamic_cast<const Action_Buy*>(a)));
        case CITY_PROD:
            return (new Action_Production(*dynamic_cast<const Action_Production*>(a)));
        case REWARD: 
            return (new Action_Reward(*dynamic_cast<const Action_Reward*>(a)));
        case QUEST:
            return (new Action_Quest(*dynamic_cast<const Action_Quest*>(a)));
        case HERO_EQUIP:
            return (new Action_Equip(*dynamic_cast<const Action_Equip*>(a)));
        case UNIT_ADVANCE:
            return (new Action_Level(*dynamic_cast<const Action_Level*>(a)));
    }

    return 0;
}

//-----------------------------------------------------------------------------
//Action_Move_Step

Action_Move::Action_Move()
    :Action(Action::STACK_MOVE), d_stack(0)
{
    d_dest.x = d_dest.y = 0;
}

Action_Move::Action_Move(XML_Helper* helper)
    :Action(Action::STACK_MOVE)
{
    helper->getData(d_stack, "stack");

    int i;
    helper->getData(i, "x");
    d_dest.x = i;
    helper->getData(i, "y");
    d_dest.y = i;
}

Action_Move::~Action_Move()
{
}

std::string Action_Move::dump() const
{
    std::stringstream s;

    s <<"Stack " <<d_stack <<" moved to (";
    s <<d_dest.x <<"," <<d_dest.y <<")\n";
    
    return s.str();
}

bool Action_Move::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("stack", d_stack);
    retval &= helper->saveData("x", d_dest.x);
    retval &= helper->saveData("y", d_dest.y);
    retval &= helper->closeTag();

    return retval;
}

bool Action_Move::fillData(Stack* s, PG_Point dest)
{
    d_stack = s->getId();
    d_dest = dest;
    return true;
}

//-----------------------------------------------------------------------------
//Action_Split

Action_Split::Action_Split()
    :Action(Action::STACK_SPLIT), d_orig(0), d_added(0)
{
    for (int i = 0; i < 8; i++)
        d_armies_moved[i] = 0;
}

Action_Split::Action_Split(XML_Helper* helper)
    :Action(Action::STACK_SPLIT)
{
    helper->getData(d_orig, "orig_stack");
    helper->getData(d_added, "new_stack");

    std::string s;
    std::istringstream si;

    helper->getData(s, "moved");
    si.str(s);
    for (int i = 0; i < 8; i++)
        si >>d_armies_moved[i];
}

Action_Split::~Action_Split()
{
}

std::string Action_Split::dump() const
{
    std::stringstream s;

    s <<"Stack " <<d_orig<<" splitted with new stack ";
    s <<d_added<<".\n";

    s <<"moved armies: ";
    for (int i = 0; i < 8; i++)
        s <<d_armies_moved[i] <<" ";
    s <<"\n";

    return s.str();
}

bool Action_Split::save(XML_Helper* helper) const
{
    bool retval = true;
    std::stringstream s;

    for (int i = 0; i < 7; i++)
        s <<d_armies_moved[i] <<" ";
    s <<d_armies_moved[7];

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", Action::STACK_SPLIT);
    retval &= helper->saveData("orig_stack", d_orig);
    retval &= helper->saveData("new_stack", d_added);
    retval &= helper->saveData("moved", s.str());
    retval &= helper->closeTag();

    return retval;
}

bool Action_Split::fillData(Stack* orig, Stack* added)
{
    if ((orig->size() > 8) ||(added->size() > 8)
        || (orig->empty()) || (added->empty()))
    {
        std::cerr <<"Action_Split::fillData(): wrong stack size\n";
        return false;
    }
    
    debug("Action_Split::fillData()")

    d_orig = orig->getId();
    d_added = added->getId();

    Stack::iterator it = added->begin();
    for (int i = 0; it != added->end(); it++, i++)
        d_armies_moved[i] = (*it)->getId();
    
    return true;
}


//-----------------------------------------------------------------------------
//Action_Fight

Action_Fight::Action_Fight()
    :Action(Action::STACK_FIGHT)
{
}

Action_Fight::Action_Fight(XML_Helper* helper)
    :Action(Action::STACK_FIGHT)
{
    std::string s;
    std::istringstream si;
    Uint32 ui;

    helper->registerTag("item", SigC::slot(*this, &Action_Fight::loadItem));

    // get attacking and defending stacks
    helper->getData(s, "attackers");
    si.str(s);
    while (si >> ui)
        d_attackers.push_back(ui);

    helper->getData(s, "defenders");
    si.str(s);
    while (si >> ui)
        d_defenders.push_back(ui);
}

Action_Fight::~Action_Fight()
{
}

std::string Action_Fight::dump() const
{
    std::stringstream s;
    std::list<Uint32>::const_iterator uit;

    s << "Battle fought.\n Attacking stacks: ";
    for (uit = d_attackers.begin(); uit != d_attackers.end(); uit++)
        s << (*uit) <<" ";

    s << "\nDefending stacks: ";
    for (uit = d_defenders.begin(); uit != d_defenders.end(); uit++)
        s << (*uit) <<" ";

    return s.str();
}

bool Action_Fight::save(XML_Helper* helper) const
{
    std::stringstream si;
    std::list<Uint32>::const_iterator uit;
    bool retval = true;
    
    retval &= helper->openTag("action");
    retval &= helper->saveData("type", Action::STACK_FIGHT);

    // save the stack's ids
    for (uit = d_attackers.begin(); uit != d_attackers.end(); uit++)
        si << (*uit) << " ";
    retval &= helper->saveData("attackers", si.str());

    for (uit = d_defenders.begin(), si.str(""); uit != d_defenders.end(); uit++)
        si << (*uit) << " ";
    retval &= helper->saveData("defenders", si.str());

    // save what happened
    for (std::list<FightItem>::const_iterator fit = d_history.begin(); 
            fit != d_history.end(); fit++)
    {
        retval &= helper->openTag("item");
        retval &= helper->saveData("turn", (*fit).turn);
        retval &= helper->saveData("id", (*fit).id);
        retval &= helper->saveData("damage", (*fit).damage);
        retval &= helper->closeTag();
    }

    retval &= helper->closeTag();

    return retval;
}

bool Action_Fight::fillData(const Fight* f)
{
    std::list<Stack*> list = f->getAttackers();
    std::list<Stack*>::const_iterator it;

    for (it = list.begin(); it != list.end(); it++)
        d_attackers.push_back((*it)->getId());
        
    list = f->getDefenders();

    for (it = list.begin(); it != list.end(); it++)
        d_attackers.push_back((*it)->getId());
    
    d_history   = f->getCourseOfEvents();

    return true;
}

bool Action_Fight::loadItem(std::string tag, XML_Helper* helper)
{
    FightItem item;
    
    helper->getData(item.turn, "turn");
    helper->getData(item.id, "id");
    helper->getData(item.damage, "damage");

    return true;
}

//-----------------------------------------------------------------------------
//Action_Join

Action_Join::Action_Join()
    :Action(Action::STACK_JOIN), d_orig_id(0), d_joining_id(0)
{
}

Action_Join::Action_Join(XML_Helper* helper)
    :Action(Action::STACK_JOIN)
{
    helper->getData(d_orig_id, "receiver");
    helper->getData(d_joining_id, "joining");
}

Action_Join::~Action_Join()
{
}

std::string Action_Join::dump() const
{
    std::stringstream s;

    s <<"Stack " <<d_joining_id <<" joined stack " <<d_orig_id <<"\n";

    return s.str();
}

bool Action_Join::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("receiver", d_orig_id);
    retval &= helper->saveData("joining", d_joining_id);
    retval &= helper->closeTag();

    return retval;
}

bool Action_Join::fillData(Stack* orig, Stack* joining)
{
    if ((orig->empty()) || (joining->empty())
        || (orig->size() + joining->size() > 8))
    {
        std::cerr <<"Action_Join::fillData(): wrong stack size\n";
        return false;
    }
    
    debug("Action_Join::fillData")
    
    d_orig_id = orig->getId();
    d_joining_id = joining->getId();
    return true;
}

//-----------------------------------------------------------------------------
//Action_Ruin

Action_Ruin::Action_Ruin()
    :Action(Action::RUIN_SEARCH), d_ruin(0), d_stack(0), d_searched(false)
{
}

Action_Ruin::Action_Ruin(XML_Helper* helper)
    :Action(Action::RUIN_SEARCH)
{
    helper->getData(d_ruin, "ruin");
    helper->getData(d_stack, "seeker");
    helper->getData(d_searched, "searched");
}

Action_Ruin::~Action_Ruin()
{
}

std::string Action_Ruin::dump() const
{
    std::stringstream s;

    s <<"Ruin " <<d_ruin <<" searched by stack " <<d_stack <<".\n";
    s <<"Ruin has" <<(d_searched? " ":" not ") <<"been searched.\n";

    return s.str();
}

bool Action_Ruin::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", Action::RUIN_SEARCH);
    retval &= helper->saveData("ruin", d_ruin);
    retval &= helper->saveData("seeker", d_stack);
    retval &= helper->saveData("searched", d_searched);
    retval &= helper->closeTag();

    return retval;
}

bool Action_Ruin::fillData(Ruin* r, Stack* explorers)
{
    d_ruin = r->getId();
    
    if (explorers)
        d_stack = explorers->getId();

    return true;
}

//-----------------------------------------------------------------------------
//Action_Temple

Action_Temple::Action_Temple()
    :Action(Action::TEMPLE_SEARCH), d_temple(0), d_stack(0)
{
}

Action_Temple::Action_Temple(XML_Helper* helper)
    :Action(Action::TEMPLE_SEARCH)
{
    helper->getData(d_temple, "temple");
    helper->getData(d_stack, "stack");
}

Action_Temple::~Action_Temple()
{
}

std::string Action_Temple::dump() const
{
    std::stringstream s;

    s <<"Stack " <<d_stack <<"visited temple " <<d_temple <<".\n";

    return s.str();
}

bool Action_Temple::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", Action::TEMPLE_SEARCH);
    retval &= helper->saveData("temple", d_temple);
    retval &= helper->saveData("stack", d_stack);
    retval &= helper->closeTag();

    return retval;
}

bool Action_Temple::fillData(Temple* t, Stack* s)
{
    d_temple = t->getId();
    d_stack = s->getId();

    return true;
}


//-----------------------------------------------------------------------------
//Action_Occupy

Action_Occupy::Action_Occupy()
    :Action(Action::CITY_OCCUPY), d_city(0)
{
}

Action_Occupy::Action_Occupy(XML_Helper* helper)
    :Action(Action::CITY_OCCUPY)
{
    helper->getData(d_city, "city");
}

Action_Occupy::~Action_Occupy()
{
}

std::string Action_Occupy::dump() const
{
    std::stringstream s;

    s <<"City " <<d_city <<" occupied\n";

    return s.str();
}

bool Action_Occupy::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", Action::CITY_OCCUPY);
    retval &= helper->saveData("city", d_city);
    retval &= helper->closeTag();

    return retval;
}

bool Action_Occupy::fillData(City* c)
{
    d_city = c->getId();
    return true;
}

//-----------------------------------------------------------------------------
//Action_Pillage

Action_Pillage::Action_Pillage()
    :Action(Action::CITY_PILLAGE), d_city(0)
{
}

Action_Pillage::Action_Pillage(XML_Helper* helper)
    :Action(Action::CITY_PILLAGE)
{
    helper->getData(d_city, "city");
}

Action_Pillage::~Action_Pillage()
{
}

std::string Action_Pillage::dump() const
{
    std::stringstream s;
    s <<"city " <<d_city <<"pillaged.\n";
    return s.str();
}

bool Action_Pillage::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", Action::CITY_PILLAGE);
    retval &= helper->saveData("city", d_city);
    retval &= helper->closeTag();

    return retval;
}

bool Action_Pillage::fillData(City* c)
{
    d_city = c->getId();
    return true;
}

//-----------------------------------------------------------------------------
//Action_Sack

Action_Sack::Action_Sack()
    :Action(Action::CITY_SACK), d_city(0)
{
}

Action_Sack::Action_Sack(XML_Helper* helper)
    :Action(Action::CITY_SACK)
{
    helper->getData(d_city, "city");
}

Action_Sack::~Action_Sack()
{
}

std::string Action_Sack::dump() const
{
    std::stringstream s;
    s <<"city " <<d_city <<"sacked.\n";
    return s.str();
}

bool Action_Sack::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", Action::CITY_SACK);
    retval &= helper->saveData("city", d_city);
    retval &= helper->closeTag();

    return retval;
}

bool Action_Sack::fillData(City* c)
{
    d_city = c->getId();
    return true;
}

//-----------------------------------------------------------------------------
//Action_Raze

Action_Raze::Action_Raze()
    :Action(Action::CITY_RAZE), d_city(0)
{
}

Action_Raze::Action_Raze(XML_Helper* helper)
    :Action(Action::CITY_RAZE)
{
    helper->getData(d_city, "city");
}

Action_Raze::~Action_Raze()
{
}

std::string Action_Raze::dump() const
{
    std::stringstream s;

    s <<"City " <<d_city <<" razed.\n";

    return s.str();
}

bool Action_Raze::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", Action::CITY_RAZE);
    retval &= helper->saveData("city", d_city);
    retval &= helper->closeTag();

    return retval;
}

bool Action_Raze::fillData(City* c)
{
    d_city = c->getId();
    return true;
}

//-----------------------------------------------------------------------------
//Action_Upgrade

Action_Upgrade::Action_Upgrade()
    :Action(Action::CITY_UPGRADE), d_city(0)
{
}

Action_Upgrade::Action_Upgrade(XML_Helper* helper)
    :Action(Action::CITY_UPGRADE)
{
    helper->getData(d_city, "city");
}

Action_Upgrade::~Action_Upgrade()
{
}

std::string Action_Upgrade::dump() const
{
    std::stringstream s;

    s <<"Defense of city " <<d_city <<" upgraded.\n";

    return s.str();
}

bool Action_Upgrade::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", Action::CITY_UPGRADE);
    retval &= helper->saveData("city", d_city);
    retval &= helper->closeTag();

    return retval;
}

bool Action_Upgrade::fillData(City* c)
{
    d_city = c->getId();
    return true;
}

//-----------------------------------------------------------------------------
//Action_Buy

Action_Buy::Action_Buy()
    :Action(Action::CITY_BUY), d_city(0), d_slot(-1), d_prod(-1),
     d_advanced(false)
{
}

Action_Buy::Action_Buy(XML_Helper* helper)
    :Action(Action::CITY_BUY)
{
    helper->getData(d_city, "city");
    helper->getData(d_slot, "slot");
    helper->getData(d_prod, "production");
    helper->getData(d_advanced, "advanced");
}

Action_Buy::~Action_Buy()
{
}

std::string Action_Buy::dump() const
{
    std::stringstream s;

    s <<"Production " <<d_prod <<"bought in city " <<d_city;
    s <<"slot: " <<d_slot <<", at " <<(d_advanced?"advanced":"basic");
    s <<"level.\n";

    return s.str();
}

bool Action_Buy::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", Action::CITY_BUY);
    retval &= helper->saveData("city", d_city);
    retval &= helper->saveData("slot", d_slot);
    retval &= helper->saveData("production", d_prod);
    retval &= helper->saveData("advanced", d_advanced);
    retval &= helper->closeTag();

    return retval;
}

bool Action_Buy::fillData(City* c, int slot, int prod, bool advanced)
{
    d_city = c->getId();
    d_slot = slot;
    d_prod = prod;
    d_advanced = advanced;

    return true;
}

//-----------------------------------------------------------------------------
//Action_Change_Production

Action_Production::Action_Production()
    :Action(Action::CITY_PROD), d_city(0), d_prod(0), d_advanced(false)
{
}

Action_Production::Action_Production(XML_Helper* helper)
    :Action(Action::CITY_PROD)
{
    helper->getData(d_city, "city");
    helper->getData(d_prod, "production");
    helper->getData(d_advanced, "advanced");
}

Action_Production::~Action_Production()
{
}

std::string Action_Production::dump() const
{
    std::stringstream s;

    s <<"Production in city " <<d_city <<" changed to " <<d_prod;
    s <<(d_advanced?" (advanced)":" (basic)") <<".\n";

    return s.str();
}

bool Action_Production::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", Action::CITY_PROD);
    retval &= helper->saveData("city", d_city);
    retval &= helper->saveData("production", d_prod);
    retval &= helper->saveData("advanced", d_advanced);
    retval &= helper->closeTag();

    return retval;
}

bool Action_Production::fillData(City* c, int prod, bool advanced)
{
    d_city = c->getId();
    d_prod = prod;
    d_advanced = advanced;

    return true;
}

//-----------------------------------------------------------------------------
//Action_Reward

Action_Reward::Action_Reward()
    :Action(Action::REWARD), d_gold(0)
{
}

Action_Reward::Action_Reward(XML_Helper* helper)
    :Action(Action::REWARD)
{
    helper->getData(d_gold, "gold");
}

Action_Reward::~Action_Reward()
{
}

std::string Action_Reward::dump() const
{
    std::stringstream s;

    s <<"Got a reward of " <<d_gold <<"gold.\n";
    
    return s.str();
}

bool Action_Reward::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", Action::REWARD);
    retval &= helper->saveData("gold", d_gold);
    retval &= helper->closeTag();

    return retval;
}

bool Action_Reward::fillData(int gold)
{
    d_gold = gold;

    return true;
}

//-----------------------------------------------------------------------------
// Action_Quest

Action_Quest::Action_Quest()
    :Action(Action::QUEST), d_hero(0), d_data(0)
{
}

Action_Quest::Action_Quest(XML_Helper* helper)
    :Action(Action::QUEST)
{
    
    helper->getData(d_hero, "hero");
    helper->getData(d_questtype, "quest");
    helper->getData(d_data, "data");
}

Action_Quest::~Action_Quest()
{
}

std::string Action_Quest::dump() const
{
    std::stringstream ss;

    ss <<"Hero " <<d_hero <<"has got quest of type " <<d_questtype;
    ss <<" with data " <<d_data <<" to fulfill\n";
    
    return ss.str();
}

bool Action_Quest::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("hero", d_hero);
    retval &= helper->saveData("quest", d_questtype);
    retval &= helper->saveData("data", d_data);
    retval &= helper->closeTag();
    
    return retval;
}

bool Action_Quest::fillData(Quest* q)
{
    d_hero = q->getHeroId();
    d_questtype = q->getType();
    
    // fill the data depending on the quest's type
    switch (d_questtype)
    {
        case Quest::RUINSEARCH:
            d_data = dynamic_cast<QuestRuinSearch*>(q)->getRuinId();
            break;
        case Quest::KILLHERO:
            d_data = dynamic_cast<QuestKillHero*>(q)->getVictim();
            break;
        case Quest::KILLARMIES:
            d_data = dynamic_cast<QuestEnemyArmies*>(q)->getArmiesToKill();
    }

    return true;
}

//-----------------------------------------------------------------------------
// Action_Equip

Action_Equip::Action_Equip()
    :Action(Action::HERO_EQUIP), d_hero(0), d_item(0)
{
}

Action_Equip::Action_Equip(XML_Helper* helper)
    :Action(Action::HERO_EQUIP)
{
    helper->getData(d_hero, "hero");
    helper->getData(d_item, "item");
    helper->getData(d_slot, "dest");
    helper->getData(d_index, "index");
}

Action_Equip::~Action_Equip()
{
}

std::string Action_Equip::dump() const
{
    std::stringstream ss;

    ss <<"Hero " <<d_hero <<" moved item " <<d_item <<"to slot " <<d_slot;
    ss <<", index " <<d_index <<std::endl;
    
    return ss.str();
}

bool Action_Equip::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("hero", d_hero);
    retval &= helper->saveData("item", d_item);
    retval &= helper->saveData("dest", d_slot);
    retval &= helper->saveData("index", d_index);
    retval &= helper->closeTag();
    
    return retval;
}

bool Action_Equip::fillData(Uint32 hero, Uint32 item, Action_Equip::Slot slot, int index)
{
    d_hero = hero;
    d_item = item;
    d_slot = slot;
    d_index = index;
    
    return true;
}

//-----------------------------------------------------------------------------
// Action_Level

Action_Level::Action_Level()
    :Action(Action::UNIT_ADVANCE), d_army(0)
{
}

Action_Level::Action_Level(XML_Helper* helper)
    :Action(Action::UNIT_ADVANCE)
{
    helper->getData(d_army, "army");
    helper->getData(d_stat, "stat");
}

Action_Level::~Action_Level()
{
}

std::string Action_Level::dump() const
{
    std::stringstream ss;

    ss <<"Unit " <<d_army <<" advanced level and increased stat " <<d_stat;
    ss <<std::endl;
    
    return ss.str();
}

bool Action_Level::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("action");
    retval &= helper->saveData("army", d_army);
    retval &= helper->saveData("stat", d_stat);
    retval &= helper->closeTag();
    
    return retval;
}

bool Action_Level::fillData(Uint32 unit, Army::Stat raised)
{
    d_army = unit;
    d_stat = raised;

    return true;
}

