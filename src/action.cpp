// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2007, 2008, 2010, 2011 Ben Asselstine
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

#include <stdlib.h>
#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "action.h"
#include "stack.h"
#include "army.h"
#include "city.h"
#include "signpost.h"
#include "ruin.h"
#include "temple.h"
#include "Quest.h"
#include "QKillHero.h"
#include "QEnemyArmies.h"
#include "QEnemyArmytype.h"
#include "QCitySack.h"
#include "QCityRaze.h"
#include "QCityOccupy.h"
#include "QPillageGold.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "player.h"
#include "armyprodbase.h"
#include "heroproto.h"
#include "Item.h"
#include "stacklist.h" //remove me

std::string Action::d_tag = "action";
using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<< x << endl<<flush;}
//#define debug(x)

Action::Action(Type type)
    :d_type(type)
{
}

Action::Action(const Action &action)
:d_type(action.d_type)
{

}
Action::Action(XML_Helper *helper)
{
  std::string type_str;
  helper->getData(type_str, "type");
  d_type = actionTypeFromString(type_str);
}

Action::~Action()
{
}

bool Action::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Action::d_tag);
    retval &= saveContents(helper);
    retval &= helper->closeTag();

    return retval;
}

bool Action::saveContents(XML_Helper* helper) const
{
    bool retval = true;

    std::string type_str = actionTypeToString(Action::Type(d_type));
    retval &= helper->saveData("type", type_str);
    retval &= doSave(helper);

    return retval;
}

Action* Action::handle_load(XML_Helper* helper)
{
  std::string type_str;

  helper->getData(type_str, "type");
  Action::Type t = actionTypeFromString(type_str);

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
      case STACK_DISBAND:
          return (new Action_Disband(helper));
      case MODIFY_SIGNPOST:
          return (new Action_ModifySignpost(helper));
      case CITY_RENAME:
          return (new Action_RenameCity(helper));
      case CITY_VECTOR:
          return (new Action_Vector(helper));
      case FIGHT_ORDER:
          return (new Action_FightOrder(helper));
      case RESIGN:
          return (new Action_Resign(helper));
      case ITEM_PLANT:
          return (new Action_Plant(helper));
      case PRODUCE_UNIT:
          return (new Action_Produce(helper));
      case PRODUCE_VECTORED_UNIT:
          return (new Action_ProduceVectored(helper));
      case DIPLOMATIC_STATE:
          return (new Action_DiplomacyState(helper));
      case DIPLOMATIC_PROPOSAL:
          return (new Action_DiplomacyProposal(helper));
      case DIPLOMATIC_SCORE:
          return (new Action_DiplomacyScore(helper));
      case END_TURN:
          return (new Action_EndTurn(helper));
      case CITY_CONQUER:
          return (new Action_ConquerCity(helper));
      case RECRUIT_HERO:
          return (new Action_RecruitHero(helper));
      case PLAYER_RENAME:
          return (new Action_RenamePlayer(helper));
      case CITY_DESTITUTE:
          return (new Action_CityTooPoorToProduce(helper));
      case INIT_TURN:
          return (new Action_InitTurn(helper));
      case CITY_LOOT:
          return (new Action_Loot(helper));
      case USE_ITEM:
          return (new Action_UseItem(helper));
      case STACK_ORDER:
          return (new Action_ReorderArmies(helper));
      case STACKS_RESET:
          return (new Action_ResetStacks(helper));
      case RUINS_RESET:
          return (new Action_ResetRuins(helper));
      case COLLECT_TAXES_AND_PAY_UPKEEP:
          return (new Action_CollectTaxesAndPayUpkeep(helper));
      case KILL_PLAYER:
          return (new Action_Kill(helper));
      case STACK_DEFEND:
          return (new Action_DefendStack(helper));
      case STACK_UNDEFEND:
          return (new Action_UndefendStack(helper));
      case STACK_PARK:
          return (new Action_ParkStack(helper));
      case STACK_UNPARK:
          return (new Action_UnparkStack(helper));
      case STACK_SELECT:
          return (new Action_SelectStack(helper));
      case STACK_DESELECT:
          return (new Action_DeselectStack(helper));
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
        case STACK_DISBAND:
            return (new Action_Disband(*dynamic_cast<const Action_Disband*>(a)));
        case MODIFY_SIGNPOST:
            return (new Action_ModifySignpost(*dynamic_cast<const Action_ModifySignpost*>(a)));
        case CITY_RENAME:
            return (new Action_RenameCity(*dynamic_cast<const Action_RenameCity*>(a)));
        case CITY_VECTOR:
            return (new Action_Vector(*dynamic_cast<const Action_Vector*>(a)));
        case FIGHT_ORDER:
            return (new Action_FightOrder(*dynamic_cast<const Action_FightOrder*>(a)));
        case RESIGN:
            return (new Action_Resign(*dynamic_cast<const Action_Resign*>(a)));
        case ITEM_PLANT:
            return (new Action_Plant(*dynamic_cast<const Action_Plant*>(a)));
        case PRODUCE_UNIT:
            return (new Action_Produce(*dynamic_cast<const Action_Produce*>(a)));
        case PRODUCE_VECTORED_UNIT:
            return 
              (new Action_ProduceVectored
                (*dynamic_cast<const Action_ProduceVectored*>(a)));
        case DIPLOMATIC_STATE:
            return 
              (new Action_DiplomacyState
                (*dynamic_cast<const Action_DiplomacyState*>(a)));
        case DIPLOMATIC_PROPOSAL:
            return 
              (new Action_DiplomacyProposal
                (*dynamic_cast<const Action_DiplomacyProposal*>(a)));
        case DIPLOMATIC_SCORE:
            return 
              (new Action_DiplomacyScore
                (*dynamic_cast<const Action_DiplomacyScore*>(a)));
        case END_TURN:
            return 
              (new Action_EndTurn
                (*dynamic_cast<const Action_EndTurn*>(a)));
        case CITY_CONQUER:
            return 
              (new Action_ConquerCity
                (*dynamic_cast<const Action_ConquerCity*>(a)));
        case RECRUIT_HERO:
            return 
              (new Action_RecruitHero
                (*dynamic_cast<const Action_RecruitHero*>(a)));
        case PLAYER_RENAME:
            return 
              (new Action_RenamePlayer
                (*dynamic_cast<const Action_RenamePlayer*>(a)));
        case CITY_DESTITUTE:
            return 
              (new Action_CityTooPoorToProduce
                (*dynamic_cast<const Action_CityTooPoorToProduce*>(a)));
        case INIT_TURN:
            return 
              (new Action_InitTurn
                (*dynamic_cast<const Action_InitTurn*>(a)));
        case CITY_LOOT:
            return 
              (new Action_Loot
                (*dynamic_cast<const Action_Loot*>(a)));
        case USE_ITEM:
            return 
              (new Action_UseItem
                (*dynamic_cast<const Action_UseItem*>(a)));
        case STACK_ORDER:
            return 
              (new Action_ReorderArmies
                (*dynamic_cast<const Action_ReorderArmies*>(a)));
        case STACKS_RESET:
            return 
              (new Action_ResetStacks
                (*dynamic_cast<const Action_ResetStacks*>(a)));
        case RUINS_RESET:
            return 
              (new Action_ResetRuins
                (*dynamic_cast<const Action_ResetRuins*>(a)));
        case COLLECT_TAXES_AND_PAY_UPKEEP:
            return 
              (new Action_CollectTaxesAndPayUpkeep
                (*dynamic_cast<const Action_CollectTaxesAndPayUpkeep*>(a)));
        case KILL_PLAYER:
            return (new Action_Kill (*dynamic_cast<const Action_Kill*>(a)));
        case STACK_DEFEND:
            return (new Action_DefendStack
                    (*dynamic_cast<const Action_DefendStack*>(a)));
        case STACK_UNDEFEND:
            return (new Action_UndefendStack
                    (*dynamic_cast<const Action_UndefendStack*>(a)));
        case STACK_PARK:
            return (new Action_ParkStack
                    (*dynamic_cast<const Action_ParkStack*>(a)));
        case STACK_UNPARK:
            return (new Action_UnparkStack
                    (*dynamic_cast<const Action_UnparkStack*>(a)));
        case STACK_SELECT:
            return (new Action_SelectStack
                    (*dynamic_cast<const Action_SelectStack*>(a)));
        case STACK_DESELECT:
            return (new Action_DeselectStack
                    (*dynamic_cast<const Action_DeselectStack*>(a)));
    }

    return 0;
}

//-----------------------------------------------------------------------------
//Action_Move_Step

Action_Move::Action_Move()
    :Action(Action::STACK_MOVE), d_stack(0)
{
    d_dest.x = d_dest.y = 0;
    d_delta.x = d_delta.y = 0;
}

Action_Move::Action_Move (const Action_Move &action)
:Action(action), d_stack(action.d_stack), d_dest(action.d_dest), 
    d_delta(action.d_delta)
{
}

Action_Move::Action_Move(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_stack, "stack");

    int i;
    helper->getData(i, "x");
    d_dest.x = i;
    helper->getData(i, "y");
    d_dest.y = i;
    helper->getData(i, "delta_x");
    d_delta.x = i;
    helper->getData(i, "delta_y");
    d_delta.y = i;
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

bool Action_Move::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("stack", d_stack);
    retval &= helper->saveData("x", d_dest.x);
    retval &= helper->saveData("y", d_dest.y);
    retval &= helper->saveData("delta_x", d_delta.x);
    retval &= helper->saveData("delta_y", d_delta.y);

    return retval;
}

bool Action_Move::fillData(Stack* s, Vector<int> dest)
{
    d_stack = s->getId();
    d_dest = dest;
    d_delta = dest - s->getPos();
    return true;
}

//-----------------------------------------------------------------------------
//Action_Split

Action_Split::Action_Split()
    :Action(Action::STACK_SPLIT), d_orig(0), d_added(0)
{
    for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
        d_armies_moved[i] = 0;
}

Action_Split::Action_Split(const Action_Split &action)
: Action(action), d_orig(action.d_orig), 
    d_added(action.d_added)
{
  for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
    d_armies_moved[i] = action.d_armies_moved[i];
}

Action_Split::Action_Split(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_orig, "orig_stack");
    helper->getData(d_added, "new_stack");

    std::string s;
    std::istringstream si;

    helper->getData(s, "moved");
    si.str(s);
    for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
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
    for (unsigned int i = 0; i < MAX_STACK_SIZE; i++)
        s <<d_armies_moved[i] <<" ";
    s <<"\n";

    return s.str();
}

bool Action_Split::doSave(XML_Helper* helper) const
{
    bool retval = true;
    std::stringstream s;

    for (unsigned int i = 0; i < MAX_STACK_SIZE - 1; i++)
        s <<d_armies_moved[i] <<" ";
    s <<d_armies_moved[MAX_STACK_SIZE - 1];

    retval &= helper->saveData("orig_stack", d_orig);
    retval &= helper->saveData("new_stack", d_added);
    retval &= helper->saveData("moved", s.str());

    return retval;
}

bool Action_Split::fillData(Stack* orig, Stack* added)
{
  if (orig->validate() == false || added->validate() == false)
    {
        std::cerr <<"Action_Split::fillData(): stacks don't validate\n";
	std::cerr <<"Action_Split:: orig has " << orig->size() << 
	  " and added has " <<added->size();
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

Action_Fight::Action_Fight(const Action_Fight &action)
: Action(action), d_history(action.d_history), d_attackers(action.d_attackers), 
    d_defenders(action.d_defenders)
{
}

Action_Fight::Action_Fight(XML_Helper* helper)
    :Action(helper)
{
    std::string s;
    std::istringstream si;
    int ival = -1;

    helper->registerTag(Item::d_tag, sigc::mem_fun(this, &Action_Fight::loadItem));

    // get attacking and defending stacks
    helper->getData(s, "attackers");
    si.str(s);
    while (si.eof() == false)
      {
        ival = -1;
        si >> ival;
        if (ival != -1)
          d_attackers.push_back((guint32)ival);
      }
    si.clear();

    helper->getData(s, "defenders");
    si.str(s);
    while (si.eof() == false)
      {
        ival = -1;
        si >> ival;
        if (ival != -1)
          d_defenders.push_back((guint32) ival);
      }
}

Action_Fight::~Action_Fight()
{
}

std::string Action_Fight::dump() const
{
    std::stringstream s;
    std::list<guint32>::const_iterator uit;

    s << "Battle fought.\n Attacking stacks: ";
    for (uit = d_attackers.begin(); uit != d_attackers.end(); uit++)
        s << (*uit) <<" ";

    s << "\n Defending stacks: ";
    for (uit = d_defenders.begin(); uit != d_defenders.end(); uit++)
        s << (*uit) <<" ";

    return s.str();
}

bool Action_Fight::doSave(XML_Helper* helper) const
{
    std::stringstream si;
    std::list<guint32>::const_iterator uit;
    bool retval = true;
    

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
        retval &= helper->openTag(Item::d_tag);
        retval &= helper->saveData("turn", (*fit).turn);
        retval &= helper->saveData("id", (*fit).id);
        retval &= helper->saveData("damage", (*fit).damage);
        retval &= helper->closeTag();
    }

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
        d_defenders.push_back((*it)->getId());
    
    d_history = f->getCourseOfEvents();

    return true;
}

bool Action_Fight::stack_ids_to_stacks(std::list<guint32> stack_ids, std::list<Stack*> &stacks, guint32 &stack_id) const
{
  for (std::list<guint32>::iterator i = stack_ids.begin(); i != stack_ids.end(); i++)
    {
      bool found = false;
      for (Playerlist::iterator j = Playerlist::getInstance()->begin(), jend = Playerlist::getInstance()->end();
           j != jend; ++j) 
        {
          Stack *s = (*j)->getStacklist()->getStackById(*i);
          if (s)
            {
              found = true;
              stacks.push_back(s);
              break;
            }
        }
      if (found == false)
        {
          stack_id = *i;
          return false;
        }
    }
  return true;
}

bool Action_Fight::is_army_id_in_stacks(guint32 id, std::list<guint32> stack_ids) const
{
  std::list<Stack*> stacks;
  guint32 stack_id = 0;
  bool success = stack_ids_to_stacks(stack_ids, stacks, stack_id);
  if (!success)
    return false;
  bool found = false;
  for (std::list<Stack*>::iterator i = stacks.begin(); i != stacks.end(); i++)
    {
      if ((*i)->getArmyById(id))
        {
          found = true;
          break;
        }
    }
  if (found)
    return true;
  else
    return false;
}

bool Action_Fight::loadItem(std::string tag, XML_Helper* helper)
{
    FightItem item;
    
    helper->getData(item.turn, "turn");
    helper->getData(item.id, "id");
    helper->getData(item.damage, "damage");

    d_history.push_back(item);

    return true;
}

//-----------------------------------------------------------------------------
//Action_Join

Action_Join::Action_Join()
    :Action(Action::STACK_JOIN), d_orig_id(0), d_joining_id(0)
{
}

Action_Join::Action_Join(const Action_Join &action)
: Action(action), d_orig_id(action.d_orig_id), d_joining_id(action.d_joining_id)
{
}

Action_Join::Action_Join(XML_Helper* helper)
    :Action(helper)
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

bool Action_Join::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("receiver", d_orig_id);
    retval &= helper->saveData("joining", d_joining_id);

    return retval;
}

bool Action_Join::fillData(Stack* orig, Stack* joining)
{
    if ((orig->empty()) || (joining->empty())
        || (orig->size() + joining->size() > MAX_STACK_SIZE))
    {
        std::cerr <<"Action_Join::fillData(): wrong stack size\n";
	std::cerr <<"Action_Join:: orig has " << orig->size() << 
	  " and joining has " <<joining->size() <<"\n";
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

Action_Ruin::Action_Ruin(const Action_Ruin&action)
: Action(action), d_ruin(action.d_ruin), d_stack(action.d_stack),
    d_searched(action.d_searched)
{
}

Action_Ruin::Action_Ruin(XML_Helper* helper)
    :Action(helper)
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

    s <<"Ruin " <<d_ruin <<" searched by stack " <<d_stack <<".  ";
    s <<"Ruin has" <<(d_searched? " ":" not ") <<"been searched.\n";

    return s.str();
}

bool Action_Ruin::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("ruin", d_ruin);
    retval &= helper->saveData("seeker", d_stack);
    retval &= helper->saveData("searched", d_searched);

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

Action_Temple::Action_Temple(const Action_Temple &action)
: Action(action), d_temple(action.d_temple), d_stack(action.d_stack)
{
}

Action_Temple::Action_Temple(XML_Helper* helper)
    :Action(helper)
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

    s <<"Stack " <<d_stack <<" visited temple " <<d_temple <<".\n";

    return s.str();
}

bool Action_Temple::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("temple", d_temple);
    retval &= helper->saveData("stack", d_stack);

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

Action_Occupy::Action_Occupy(const Action_Occupy &action)
: Action(action), d_city(action.d_city)
{
}

Action_Occupy::Action_Occupy(XML_Helper* helper)
    :Action(helper)
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

bool Action_Occupy::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);

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

Action_Pillage::Action_Pillage(const Action_Pillage &action)
: Action(action), d_city(action.d_city)
{
}

Action_Pillage::Action_Pillage(XML_Helper* helper)
    :Action(helper)
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

bool Action_Pillage::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);

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

Action_Sack::Action_Sack(const Action_Sack &action)
: Action(action), d_city(action.d_city)
{
}

Action_Sack::Action_Sack(XML_Helper* helper)
    :Action(helper)
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

bool Action_Sack::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);

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

Action_Raze::Action_Raze(const Action_Raze &action)
: Action(action), d_city(action.d_city)
{
}

Action_Raze::Action_Raze(XML_Helper* helper)
    :Action(helper)
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

bool Action_Raze::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);

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

Action_Upgrade::Action_Upgrade(const Action_Upgrade &action)
: Action(action), d_city(action.d_city)
{
}

Action_Upgrade::Action_Upgrade(XML_Helper* helper)
    :Action(helper)
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

bool Action_Upgrade::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);

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
    :Action(Action::CITY_BUY), d_city(0), d_slot(-1), d_prod(-1)
{
}

Action_Buy::Action_Buy(const Action_Buy &action)
: Action(action), d_city(action.d_city), d_slot(action.d_slot), 
    d_prod(action.d_prod)
{
}

Action_Buy::Action_Buy(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_city, "city");
    helper->getData(d_slot, "slot");
    helper->getData(d_prod, "production");
}

Action_Buy::~Action_Buy()
{
}

std::string Action_Buy::dump() const
{
    std::stringstream s;

    s <<"Production " <<d_prod <<" bought in city " <<d_city;
    s <<" slot: " <<d_slot << "\n";

    return s.str();
}

bool Action_Buy::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);
    retval &= helper->saveData("slot", d_slot);
    retval &= helper->saveData("production", d_prod);

    return retval;
}

bool Action_Buy::fillData(City* c, int slot, const ArmyProto *prod)
{
    d_city = c->getId();
    d_slot = slot;
    d_prod = prod->getTypeId();

    return true;
}

//-----------------------------------------------------------------------------
//Action_Change_Production

Action_Production::Action_Production()
    :Action(Action::CITY_PROD), d_city(0), d_prod(0)
{
}

Action_Production::Action_Production (const Action_Production &action)
: Action(action), d_city(action.d_city), d_prod(action.d_prod)
{
}

Action_Production::Action_Production(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_city, "city");
    helper->getData(d_prod, "production");
}

Action_Production::~Action_Production()
{
}

std::string Action_Production::dump() const
{
    std::stringstream s;

    s <<"Production in city " <<d_city <<" changed to " <<d_prod;
    s <<".\n";

    return s.str();
}

bool Action_Production::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);
    retval &= helper->saveData("production", d_prod);

    return retval;
}

bool Action_Production::fillData(City* c, int slot)
{
    d_city = c->getId();
    d_prod = slot;

    return true;
}

//-----------------------------------------------------------------------------
//Action_Reward

Action_Reward::Action_Reward()
    :Action(Action::REWARD)
{
  d_reward = NULL;
}

bool Action_Reward::load(std::string tag, XML_Helper *helper)
{
    if (tag == Reward::d_tag)
      {
	guint32 t;
	std::string type_str;
	helper->getData(type_str, "type");
	t = Reward::rewardTypeFromString(type_str);
	switch (t)
	  {
	  case  Reward::GOLD:
	    d_reward = new Reward_Gold(helper); break;
	  case  Reward::ALLIES:
	    d_reward = new Reward_Allies(helper); break;
	  case Reward::ITEM:
	    d_reward = new Reward_Item(helper); break;
	  case Reward::RUIN:
	    d_reward = new Reward_Ruin(helper); break;
	  case Reward::MAP:
	    d_reward = new Reward_Map(helper); break;
	  }
	return true;
      }
    return false;
}

Action_Reward::Action_Reward (const Action_Reward &action)
: Action(action), d_stack(action.d_stack)
{
  const Reward *reward = action.d_reward;
  if (reward)
    d_reward = Reward::copy(reward);
  else
    d_reward = NULL;
}

Action_Reward::Action_Reward(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack, "stack");
  d_reward = NULL;
  helper->registerTag(Reward::d_tag, sigc::mem_fun(this, &Action_Reward::load));
}

Action_Reward::~Action_Reward()
{
}

bool Action_Reward::fillData(Stack *s, Reward* r)
{
  d_stack = s->getId();
  d_reward = r;
  return true;
}

std::string Action_Reward::dump() const
{
  std::stringstream s;

  if (d_reward)
    s <<"Got a reward of " <<d_reward->getType() <<"\n";

  return s.str();
}

bool Action_Reward::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("stack", d_stack);
  if (d_reward->getType() == Reward::GOLD)
    retval &= dynamic_cast<Reward_Gold*>(d_reward)->save(helper);
  else if (d_reward->getType() == Reward::ALLIES)
    retval &= dynamic_cast<Reward_Allies*>(d_reward)->save(helper);
  else if (d_reward->getType() == Reward::ITEM)
    retval &= dynamic_cast<Reward_Item*>(d_reward)->save(helper);
  else if (d_reward->getType() == Reward::RUIN)
    retval &= dynamic_cast<Reward_Ruin*>(d_reward)->save(helper);
  else if (d_reward->getType() == Reward::MAP)
    retval &= dynamic_cast<Reward_Map*>(d_reward)->save(helper);

  return retval;
}

//-----------------------------------------------------------------------------
// Action_Quest

Action_Quest::Action_Quest()
:Action(Action::QUEST), d_hero(0), d_data(0), d_victim_player(0)
{
}

Action_Quest::Action_Quest (const Action_Quest &action)
: Action(action), d_hero(action.d_hero), d_questtype(action.d_questtype),
    d_data(action.d_data), d_victim_player(action.d_victim_player)
{
}

Action_Quest::Action_Quest(XML_Helper* helper)
:Action(helper)
{

  helper->getData(d_hero, "hero");
  std::string s;
  helper->getData(s, "quest");
  d_questtype = Quest::questTypeFromString(s);
  helper->getData(d_data, "data");
  helper->getData(d_victim_player, "victim_player");
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

bool Action_Quest::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);
  std::string s = Quest::questTypeToString(Quest::Type(d_questtype));
  retval &= helper->saveData("quest", s);
  retval &= helper->saveData("data", d_data);
  retval &= helper->saveData("victim_player", d_victim_player);

  return retval;
}

bool Action_Quest::fillData(Quest* q)
{
  d_hero = q->getHeroId();
  d_questtype = q->getType();

  // fill the data depending on the quest's type
  switch (d_questtype)
    {
    case Quest::KILLHERO:
      d_data = dynamic_cast<QuestKillHero*>(q)->getVictim();
      break;
    case Quest::KILLARMIES:
      d_data = dynamic_cast<QuestEnemyArmies*>(q)->getArmiesToKill();
      d_victim_player = dynamic_cast<QuestEnemyArmies*>(q)->getVictimPlayerId();
      break;
    case Quest::CITYSACK:
      d_data = dynamic_cast<QuestCitySack*>(q)->getCityId();
      break;
    case Quest::CITYRAZE:
      d_data = dynamic_cast<QuestCityRaze*>(q)->getCityId();
      break;
    case Quest::CITYOCCUPY:
      d_data = dynamic_cast<QuestCityOccupy*>(q)->getCityId();
      break;
    case Quest::KILLARMYTYPE:
      d_data = dynamic_cast<QuestEnemyArmytype*>(q)->getArmytypeToKill();
      break;
    case Quest::PILLAGEGOLD:
      d_data = dynamic_cast<QuestPillageGold*>(q)->getGoldToPillage();
      break;
    }

  return true;
}

//-----------------------------------------------------------------------------
// Action_Equip

Action_Equip::Action_Equip()
:Action(Action::HERO_EQUIP), d_hero(0), d_item(0), d_pos(Vector<int>(-1,-1))
{
}

Action_Equip::Action_Equip (const Action_Equip &action)
: Action(action), d_hero(action.d_hero), d_item(action.d_item),
    d_slot(action.d_slot), d_pos(action.d_pos)
{
}

Action_Equip::Action_Equip(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_hero, "hero");
  helper->getData(d_item, "item");
  helper->getData(d_slot, "dest");
  int i;
  helper->getData(i, "x");
  d_pos.x = i;
  helper->getData(i, "y");
  d_pos.y = i;
}

Action_Equip::~Action_Equip()
{
}

std::string Action_Equip::dump() const
{
  std::stringstream ss;

  ss <<"Hero " <<d_hero <<" moved item " <<d_item <<" to slot " <<d_slot;
  ss <<" at tile " << d_pos.x << "," << d_pos.y;
  ss <<std::endl;

  return ss.str();
}

bool Action_Equip::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);
  retval &= helper->saveData("item", d_item);
  retval &= helper->saveData("dest", d_slot);
  int i = d_pos.x;
  retval &= helper->saveData("x", i);
  i = d_pos.y;
  retval &= helper->saveData("y", i);

  return retval;
}

bool Action_Equip::fillData(Hero *hero, Item *item, Action_Equip::Slot slot,
			    Vector<int> pos)
{
  d_hero = hero->getId();
  d_item = item->getId();
  d_slot = slot;
  d_pos = pos;

  return true;
}

//-----------------------------------------------------------------------------
// Action_Level

Action_Level::Action_Level()
:Action(Action::UNIT_ADVANCE), d_army(0)
{
}

Action_Level::Action_Level (const Action_Level &action)
: Action(action), d_army(action.d_army), d_stat(action.d_stat)
{
}

Action_Level::Action_Level(XML_Helper* helper)
:Action(helper)
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

bool Action_Level::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("army", d_army);
  retval &= helper->saveData("stat", d_stat);

  return retval;
}

bool Action_Level::fillData(Army *unit, Army::Stat raised)
{
  d_army = unit->getId();
  d_stat = raised;

  return true;
}

//-----------------------------------------------------------------------------
//Action_Disband

Action_Disband::Action_Disband()
:Action(Action::STACK_DISBAND), d_stack(0)
{
}

Action_Disband::Action_Disband(const Action_Disband &action)
: Action(action), d_stack(action.d_stack)
{
}

Action_Disband::Action_Disband(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack, "stack");
}

Action_Disband::~Action_Disband()
{
}

std::string Action_Disband::dump() const
{
  std::stringstream s;

  s <<"Stack " <<d_stack <<" disbanded\n";

  return s.str();
}

bool Action_Disband::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("stack", d_stack);

  return retval;
}

bool Action_Disband::fillData(Stack* s)
{
  d_stack = s->getId();
  return true;
}

//-----------------------------------------------------------------------------
//Action_ModifySignpost

Action_ModifySignpost::Action_ModifySignpost()
:Action(Action::MODIFY_SIGNPOST), d_signpost(0), d_message("")
{
}

Action_ModifySignpost::Action_ModifySignpost(const Action_ModifySignpost &action)
: Action(action), d_signpost(action.d_signpost), d_message(action.d_message)
{
}

Action_ModifySignpost::Action_ModifySignpost(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_signpost, "signpost");
  helper->getData(d_message, "message");
}

Action_ModifySignpost::~Action_ModifySignpost()
{
}

std::string Action_ModifySignpost::dump() const
{
  std::stringstream s;

  s <<"Signpost " <<d_signpost <<" modified to read" << d_message <<".\n";

  return s.str();
}

bool Action_ModifySignpost::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("signpost", d_signpost);
  retval &= helper->saveData("message", d_message);

  return retval;
}

bool Action_ModifySignpost::fillData(Signpost * s, std::string message)
{
  d_signpost = s->getId();
  d_message = message; 
  return true;
}

//-----------------------------------------------------------------------------
//Action_RenameCity

Action_RenameCity::Action_RenameCity()
:Action(Action::CITY_RENAME), d_city(0), d_name("")
{
}

Action_RenameCity::Action_RenameCity(const Action_RenameCity &action)
: Action(action), d_city(action.d_city), d_name(action.d_name)
{
}

Action_RenameCity::Action_RenameCity(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_city, "city");
  helper->getData(d_name, "name");
}

Action_RenameCity::~Action_RenameCity()
{
}

std::string Action_RenameCity::dump() const
{
  std::stringstream s;

  s <<"City " <<d_city <<" renamed to " << d_name<<".\n";

  return s.str();
}

bool Action_RenameCity::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("city", d_city);
  retval &= helper->saveData("name", d_name);

  return retval;
}

bool Action_RenameCity::fillData(City* c, std::string name)
{
  d_city = c->getId();
  d_name = name; 
  return true;
}

//-----------------------------------------------------------------------------
//Action_Vector

Action_Vector::Action_Vector()
:Action(Action::CITY_VECTOR)
{
}

Action_Vector::Action_Vector(const Action_Vector &action)
: Action(action), d_city(action.d_city), d_dest(action.d_dest)
{
}

Action_Vector::Action_Vector(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_city, "city");
  int i;
  helper->getData(i, "x");
  d_dest.x = i;
  helper->getData(i, "y");
  d_dest.y = i;
}

Action_Vector::~Action_Vector()
{
}

std::string Action_Vector::dump() const
{
  std::stringstream s;

  s <<"Vectoring new units from city " <<d_city <<" to ";
  s <<d_dest.x <<"," <<d_dest.y <<")\n";

  return s.str();
}

bool Action_Vector::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("city", d_city);
  retval &= helper->saveData("x", d_dest.x);
  retval &= helper->saveData("y", d_dest.y);

  return retval;
}

bool Action_Vector::fillData(City* src, Vector <int> dest)
{
  d_city = src->getId();
  d_dest = dest;
  return true;
}

//-----------------------------------------------------------------------------
//Action_FightOrder

Action_FightOrder::Action_FightOrder()
:Action(Action::FIGHT_ORDER)
{
}

Action_FightOrder::Action_FightOrder(const Action_FightOrder &action)
: Action(action), d_order(action.d_order)
{
}

Action_FightOrder::Action_FightOrder(XML_Helper* helper)
:Action(helper)
{
  std::string fight_order;
  std::stringstream sfight_order;
  guint32 val;
  helper->getData(fight_order, "order");
  sfight_order.str(fight_order);
  guint32 size = Armysetlist::getInstance()->getSize(Playerlist::getInstance()->getFirstLiving()->getArmyset());
  for (unsigned int i = 0; i < size; i++)
    {
      sfight_order >> val;
      d_order.push_back(val);
    }
}

Action_FightOrder::~Action_FightOrder()
{
}

std::string Action_FightOrder::dump() const
{
  std::stringstream s;

  s <<"changed fight order to:" ;
  std::list<guint32>::const_iterator it = d_order.begin();
  for ( ;it != d_order.end(); it++)
    {
      s <<" " << (*it);
    }
  s << "\n";

  return s.str();
}

bool Action_FightOrder::doSave(XML_Helper* helper) const
{
  bool retval = true;

  std::stringstream fight_order;
  for (std::list<guint32>::const_iterator it = d_order.begin();
       it != d_order.end(); it++)
    {
      fight_order << (*it) << " ";
    }
  retval &= helper->saveData("order", fight_order.str());

  return retval;
}

bool Action_FightOrder::fillData(std::list<guint32> order)
{
  d_order = order;
  return true;
}

//-----------------------------------------------------------------------------
//Action_Resign

Action_Resign::Action_Resign()
:Action(Action::RESIGN)
{
}

Action_Resign::Action_Resign(const Action_Resign &action)
: Action(action)
{
}

Action_Resign::Action_Resign(XML_Helper* helper)
:Action(helper)
{
}

Action_Resign::~Action_Resign()
{
}

std::string Action_Resign::dump() const
{
  std::stringstream s;
  s << "this player resigns\n";

  return s.str();
}

bool Action_Resign::doSave(XML_Helper* helper) const
{
  bool retval = true;

  return retval;
}

bool Action_Resign::fillData()
{
  return true;
}

//-----------------------------------------------------------------------------
//Action_Plant

Action_Plant::Action_Plant()
:Action(Action::ITEM_PLANT)
{
}

Action_Plant::Action_Plant(const Action_Plant &action)
: Action(action), d_hero(action.d_hero), d_item(action.d_item)
{
}

Action_Plant::Action_Plant(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_hero, "hero");
  helper->getData(d_item, "item");
}

Action_Plant::~Action_Plant()
{
}

std::string Action_Plant::dump() const
{
  std::stringstream s;
  s << "hero " << d_hero << " plants item " << d_item;

  return s.str();
}

bool Action_Plant::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);
  retval &= helper->saveData("item", d_item);

  return retval;
}

bool Action_Plant::fillData(Hero *hero, Item *item)
{
  d_hero = hero->getId();
  d_item = item->getId();
  return true;
}

//-----------------------------------------------------------------------------
//Action_Produce

Action_Produce::Action_Produce()
:Action(Action::PRODUCE_UNIT)
{
  d_army = NULL;
}

Action_Produce::Action_Produce(const Action_Produce &action)
: Action(action), d_city(action.d_city), d_vectored(action.d_vectored),
    d_dest(action.d_dest), d_army_id(action.d_army_id), 
    d_stack_id(action.d_stack_id)
{
  if (action.d_army)
    d_army = new ArmyProdBase (*action.d_army);
  else
    d_army = NULL;
}

Action_Produce::Action_Produce(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_city, "city");
  helper->getData(d_vectored, "vectored");
  helper->getData(d_dest.x, "dest_x");
  helper->getData(d_dest.y, "dest_y");
  helper->getData(d_army_id, "army_id");
  helper->getData(d_stack_id, "stack_id");
  d_army = NULL;
  helper->registerTag(ArmyProdBase::d_tag, sigc::mem_fun(this, &Action_Produce::load));
}

bool Action_Produce::load(std::string tag, XML_Helper *helper)
{
    if (tag == ArmyProdBase::d_tag)
      {
	d_army = new ArmyProdBase(helper);

	return true;
      }
    return false;
}

Action_Produce::~Action_Produce()
{
  if (d_army)
    delete d_army;
}

std::string Action_Produce::dump() const
{
  std::stringstream s;
  s << "army id " << d_army_id << " of type " << d_army->getTypeId() << " shows up at city " << d_city;
  if (d_vectored)
    s <<" but it is vectored to another city at " << d_dest.x << "," << d_dest.y;
  else
    s <<" at position " << d_dest.x << "," << d_dest.y << " in stack " << d_stack_id ;
  s <<"\n";

  return s.str();
}

bool Action_Produce::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("city", d_city);
  retval &= helper->saveData("vectored", d_vectored);
  retval &= helper->saveData("dest_x", d_dest.x);
  retval &= helper->saveData("dest_y", d_dest.y);
  retval &= helper->saveData("army_id", d_army_id);
  retval &= helper->saveData("stack_id", d_stack_id);
  retval &= d_army->save(helper);

  return retval;
}

bool Action_Produce::fillData(const ArmyProdBase *army, City *city, bool vectored, Vector<int> pos, guint32 army_id, guint32 stack_id)
{
  d_army = new ArmyProdBase(*army);
  d_city = city->getId();
  d_vectored = vectored;
  d_dest = pos;
  d_army_id = army_id;
  d_stack_id = stack_id;
  return true;
}

//-----------------------------------------------------------------------------
//Action_ProduceVectored

Action_ProduceVectored::Action_ProduceVectored()
:Action(Action::PRODUCE_VECTORED_UNIT)
{
  d_army = NULL;
  d_dest = Vector<int>(-1,-1);
  d_src = Vector<int>(-1,-1);
}

Action_ProduceVectored::Action_ProduceVectored(const Action_ProduceVectored &action)
: Action(action), d_dest(action.d_dest), d_src(action.d_src)
{
  if (action.d_army)
    d_army = new ArmyProdBase(*action.d_army);
  else
    d_army = NULL;
}

Action_ProduceVectored::Action_ProduceVectored(XML_Helper* helper)
:Action(helper), d_army(NULL)
{
  int i;
  helper->getData(i, "dest_x");
  d_dest.x = i;
  helper->getData(i, "dest_y");
  d_dest.y = i;
  helper->getData(i, "src_x");
  d_src.x = i;
  helper->getData(i, "src_y");
  d_src.y = i;
  d_army = NULL;
  helper->registerTag(ArmyProdBase::d_tag, sigc::mem_fun(this, &Action_ProduceVectored::load));
}

bool Action_ProduceVectored::load(std::string tag, XML_Helper *helper)
{
    if (tag == ArmyProdBase::d_tag)
      {
	d_army = new ArmyProdBase(helper);

	return true;
      }
    return false;
}

Action_ProduceVectored::~Action_ProduceVectored()
{
  if (d_army)
    delete d_army;
}

std::string Action_ProduceVectored::dump() const
{
  std::stringstream s;
  s << "vectored army of type " << d_army->getTypeId() << " shows up at ";
  s <<d_dest.x <<"," <<d_dest.y << " from " << d_src.x << "," << d_src.y;
  s <<"\n";

  return s.str();
}

bool Action_ProduceVectored::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("dest_x", d_dest.x);
  retval &= helper->saveData("dest_y", d_dest.y);
  retval &= helper->saveData("src_x", d_src.x);
  retval &= helper->saveData("src_y", d_src.y);
  retval &= d_army->save(helper);

  return retval;
}

bool Action_ProduceVectored::fillData(ArmyProdBase *army, Vector<int> dest,
				      Vector<int> src)
{
  if (army)
    d_army = new ArmyProdBase (*army);
  d_dest = dest;
  d_src = src;
  return true;
}

//-----------------------------------------------------------------------------
//Action_DiplomacyState

Action_DiplomacyState::Action_DiplomacyState()
:Action(Action::DIPLOMATIC_STATE)
{
}

Action_DiplomacyState::Action_DiplomacyState(const Action_DiplomacyState &action)
: Action(action), d_opponent_id(action.d_opponent_id), 
    d_diplomatic_state(action.d_diplomatic_state)
{
}

Action_DiplomacyState::Action_DiplomacyState(XML_Helper* helper)
:Action(helper)
{
  guint32 diplomatic_state;
  helper->getData(d_opponent_id, "opponent_id");
  helper->getData(diplomatic_state, "state");
  d_diplomatic_state = Player::DiplomaticState(diplomatic_state);
}

Action_DiplomacyState::~Action_DiplomacyState()
{
}

std::string Action_DiplomacyState::dump() const
{
  std::stringstream s;
  s << "declaring ";
  switch (d_diplomatic_state)
    {
    case Player::AT_WAR: s <<"war"; break;
    case Player::AT_WAR_IN_FIELD: s <<"war in the field"; break;
    case Player::AT_PEACE: s <<"peace"; break;
    }
  s << " with player " << d_opponent_id <<".\n";

  return s.str();
}

bool Action_DiplomacyState::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("opponent_id", d_opponent_id);
  retval &= helper->saveData("state", (guint32)d_diplomatic_state);

  return retval;
}

bool Action_DiplomacyState::fillData(Player *opponent, 
				      Player::DiplomaticState state)
{
  d_opponent_id = opponent->getId();
  d_diplomatic_state = state;
  return true;
}

//-----------------------------------------------------------------------------
//Action_DiplomacyProposal

Action_DiplomacyProposal::Action_DiplomacyProposal()
:Action(Action::DIPLOMATIC_PROPOSAL)
{
}

Action_DiplomacyProposal::Action_DiplomacyProposal(const Action_DiplomacyProposal &action)
: Action(action), d_opponent_id(action.d_opponent_id), 
	d_diplomatic_proposal(action.d_diplomatic_proposal)
{
}

Action_DiplomacyProposal::Action_DiplomacyProposal(XML_Helper* helper)
:Action(helper)
{
  guint32 diplomatic_proposal;
  helper->getData(d_opponent_id, "opponent_id");
  helper->getData(diplomatic_proposal, "proposal");
  d_diplomatic_proposal = Player::DiplomaticProposal(diplomatic_proposal);
}

Action_DiplomacyProposal::~Action_DiplomacyProposal()
{
}

std::string Action_DiplomacyProposal::dump() const
{
  std::stringstream s;
  s << "proposing ";
  switch (d_diplomatic_proposal)
    {
    case Player::NO_PROPOSAL: s <<"nothing"; break;
    case Player::PROPOSE_WAR: s <<"war"; break;
    case Player::PROPOSE_WAR_IN_FIELD: s <<"war in the field"; break;
    case Player::PROPOSE_PEACE: s <<"peace"; break;
    }
  s << " with player " << d_opponent_id << ".\n";

  return s.str();
}

bool Action_DiplomacyProposal::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("opponent_id", d_opponent_id);
  retval &= helper->saveData("proposal", (guint32)d_diplomatic_proposal);

  return retval;
}

bool Action_DiplomacyProposal::fillData(Player *opponent, 
					 Player::DiplomaticProposal proposal)
{
  d_opponent_id = opponent->getId();
  d_diplomatic_proposal = proposal;
  return true;
}

//-----------------------------------------------------------------------------
//Action_DiplomacyScore

Action_DiplomacyScore::Action_DiplomacyScore()
:Action(Action::DIPLOMATIC_SCORE)
{
}

Action_DiplomacyScore::Action_DiplomacyScore(const Action_DiplomacyScore &action)
: Action(action), d_opponent_id(action.d_opponent_id), d_amount(action.d_amount)
{
}

Action_DiplomacyScore::Action_DiplomacyScore(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_opponent_id, "opponent_id");
  helper->getData(d_amount, "amount");
}

Action_DiplomacyScore::~Action_DiplomacyScore()
{
}

std::string Action_DiplomacyScore::dump() const
{
  std::stringstream s;
  if(d_amount > 0)
    s << "adding " << d_amount << " to " ;
  else
    s << "subtracting " << d_amount << " from "; 
  s << "player " << d_opponent_id << ".\n";

  return s.str();
}

bool Action_DiplomacyScore::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("opponent_id", d_opponent_id);
  retval &= helper->saveData("amount", d_amount);

  return retval;
}

bool Action_DiplomacyScore::fillData(Player *opponent, int amount)
{
  d_opponent_id = opponent->getId();
  d_amount = amount;
  return true;
}

//-----------------------------------------------------------------------------
//Action_EndTurn

Action_EndTurn::Action_EndTurn()
:Action(Action::END_TURN)
{
}

Action_EndTurn::Action_EndTurn(const Action_EndTurn &action)
: Action(action)
{
}

Action_EndTurn::Action_EndTurn(XML_Helper* helper)
:Action(helper)
{
}

Action_EndTurn::~Action_EndTurn()
{
}

std::string Action_EndTurn::dump() const
{
  return "ending turn\n";
}

bool Action_EndTurn::doSave(XML_Helper* helper) const
{
  bool retval = true;

  return retval;
}

//-----------------------------------------------------------------------------
//Action_ConquerCity

Action_ConquerCity::Action_ConquerCity()
  :Action(Action::CITY_CONQUER), d_city(0)
{
}

Action_ConquerCity::Action_ConquerCity(const Action_ConquerCity &action)
: Action(action), d_city(action.d_city)
{
}

Action_ConquerCity::Action_ConquerCity(XML_Helper* helper)
  :Action(helper)
{
    helper->getData(d_city, "city");
}

Action_ConquerCity::~Action_ConquerCity()
{
}

std::string Action_ConquerCity::dump() const
{
    std::stringstream s;

    s <<"City " <<d_city <<" occupied." << std::endl;

    return s.str();
}

bool Action_ConquerCity::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);

    return retval;
}

bool Action_ConquerCity::fillData(City* c)
{
    d_city = c->getId();
    return true;
}

//-----------------------------------------------------------------------------
//Action_RecruitHero

Action_RecruitHero::Action_RecruitHero()
  :Action(Action::RECRUIT_HERO), d_hero(0)
{
}

Action_RecruitHero::Action_RecruitHero(XML_Helper* helper)
  :Action(helper)
{
    helper->getData(d_city, "city");
    helper->getData(d_cost, "cost");
    helper->getData(d_allies, "allies");
    helper->getData(d_ally_army_type, "ally_army_type");
    helper->registerTag(HeroProto::d_tag, sigc::mem_fun(this, &Action_RecruitHero::load));
}

Action_RecruitHero::Action_RecruitHero(const Action_RecruitHero &action)
: Action(action), d_city(action.d_city), d_cost(action.d_cost), 
    d_allies(action.d_allies), d_ally_army_type(action.d_ally_army_type)
{
  d_hero = new HeroProto(*action.d_hero);
}

bool Action_RecruitHero::load(std::string tag, XML_Helper *helper)
{
    if (tag == HeroProto::d_tag)
      {
	d_hero = new HeroProto(helper);

	return true;
      }
    return false;
}

Action_RecruitHero::~Action_RecruitHero()
{
}

std::string Action_RecruitHero::dump() const
{
    std::stringstream s;

    s << "Hero " << d_hero->getName() << " recruited with " << d_allies << " allies\n";

    return s.str();
}

bool Action_RecruitHero::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);
    retval &= helper->saveData("cost", d_cost);
    retval &= helper->saveData("allies", d_allies);
    retval &= helper->saveData("ally_army_type", d_ally_army_type);
    retval &= d_hero->save(helper);

    return retval;
}

bool Action_RecruitHero::fillData(HeroProto* hero, City *city, int cost, int alliesCount, const ArmyProto *ally)
{
    d_hero = hero;
    d_city = city->getId();
    d_cost = cost;
    d_allies = alliesCount;
    if (alliesCount > 0)
      d_ally_army_type = ally->getTypeId();
    else
      d_ally_army_type = 0;
    return true;
}

//-----------------------------------------------------------------------------
//Action_RenamePlayer

Action_RenamePlayer::Action_RenamePlayer()
  :Action(Action::PLAYER_RENAME), d_name("")
{
}

Action_RenamePlayer::Action_RenamePlayer(const Action_RenamePlayer &action)
:Action(action), d_name(action.d_name)
{
}

Action_RenamePlayer::Action_RenamePlayer(XML_Helper* helper)
  :Action(helper)
{
    helper->getData(d_name, "name");
}

Action_RenamePlayer::~Action_RenamePlayer()
{
}

std::string Action_RenamePlayer::dump() const
{
    std::stringstream s;

    s << "Player changes name to " << d_name <<".\n";

    return s.str();
}

bool Action_RenamePlayer::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("name", d_name);

    return retval;
}

bool Action_RenamePlayer::fillData(std::string name)
{
  d_name = name;
  return true;
}

//-----------------------------------------------------------------------------
//Action_CityTooPoorToProduce

Action_CityTooPoorToProduce::Action_CityTooPoorToProduce()
    :Action(Action::CITY_DESTITUTE), d_city(0), d_army_type(0)
{
}

Action_CityTooPoorToProduce::Action_CityTooPoorToProduce(const Action_CityTooPoorToProduce &action)
: Action(action), d_city(action.d_city), d_army_type(action.d_army_type)
{
}

Action_CityTooPoorToProduce::Action_CityTooPoorToProduce(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_city, "city");
    helper->getData(d_army_type, "army_type");
}

Action_CityTooPoorToProduce::~Action_CityTooPoorToProduce()
{
}

std::string Action_CityTooPoorToProduce::dump() const
{
    std::stringstream s;

    s << "City " << d_city << " is too poor to produce army type " 
      << d_army_type << "\n";

    return s.str();
}

bool Action_CityTooPoorToProduce::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("city", d_city);
    retval &= helper->saveData("army_type", d_army_type);

    return retval;
}

bool Action_CityTooPoorToProduce::fillData(City* c, const ArmyProdBase *army)
{
    d_city = c->getId();
    d_army_type = army->getTypeId();

    return true;
}

//-----------------------------------------------------------------------------
//Action_InitTurn

Action_InitTurn::Action_InitTurn()
:Action(Action::INIT_TURN)
{
}

Action_InitTurn::Action_InitTurn(const Action_InitTurn &action)
: Action(action)
{
}

Action_InitTurn::Action_InitTurn(XML_Helper* helper)
:Action(helper)
{
}

Action_InitTurn::~Action_InitTurn()
{
}

std::string Action_InitTurn::dump() const
{
  return "initializing turn\n";
}

bool Action_InitTurn::doSave(XML_Helper* helper) const
{
  bool retval = true;

  return retval;
}

//-----------------------------------------------------------------------------
//Action_Loot

Action_Loot::Action_Loot()
    :Action(Action::CITY_LOOT), d_looting_player_id(0), d_looted_player_id(0),
    d_gold_added(0), d_gold_removed(0)
{
}

Action_Loot::Action_Loot(const Action_Loot &action)
: Action(action), d_looting_player_id(action.d_looting_player_id),
    d_looted_player_id(action.d_looted_player_id),
    d_gold_added(action.d_gold_added), d_gold_removed(action.d_gold_removed)
{
}

Action_Loot::Action_Loot(XML_Helper* helper)
    :Action(helper)
{
    helper->getData(d_looting_player_id, "looting_player_id");
    helper->getData(d_looted_player_id, "looted_player_id");
    helper->getData(d_gold_added, "gold_added");
    helper->getData(d_gold_removed, "gold_removed");
}

Action_Loot::~Action_Loot()
{
}

std::string Action_Loot::dump() const
{
    std::stringstream s;
    s <<"player " <<d_looting_player_id << " took " << d_gold_added << 
      " gold pieces from player " << d_looted_player_id << 
      " who lost a total of " << d_gold_removed << "gold pieces.\n";
    return s.str();
}

bool Action_Loot::doSave(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("looting_player_id", d_looting_player_id);
    retval &= helper->saveData("looted_player_id", d_looted_player_id);
    retval &= helper->saveData("gold_added", d_gold_added);
    retval &= helper->saveData("gold_removed", d_gold_removed);

    return retval;
}

        
bool Action_Loot::fillData(Player *looter, Player *looted, guint32 amount_to_add,
			   guint32 amount_to_subtract)
{
    d_looting_player_id = looter->getId();
    d_looted_player_id = looted->getId();
    d_gold_added = amount_to_add;
    d_gold_removed = amount_to_subtract;
    return true;
}

//-----------------------------------------------------------------------------
// Action_UseItem

Action_UseItem::Action_UseItem()
:Action(Action::USE_ITEM), d_hero(0), d_item(0), d_victim_player(0),
    d_friendly_city(0), d_enemy_city(0), d_neutral_city(0), d_city(0)
{
}

Action_UseItem::Action_UseItem(const Action_UseItem &action)
: Action(action), d_hero(action.d_hero), d_item(action.d_item),
    d_victim_player(action.d_victim_player),
    d_friendly_city(action.d_friendly_city), d_enemy_city(action.d_enemy_city),
    d_neutral_city(action.d_neutral_city), d_city(action.d_city)
{
}

Action_UseItem::Action_UseItem(XML_Helper* helper)
:Action(helper)
{

  helper->getData(d_hero, "hero");
  helper->getData(d_item, "item");
  helper->getData(d_victim_player, "victim_player");
  helper->getData(d_friendly_city, "friendly_city");
  helper->getData(d_enemy_city, "enemy_city");
  helper->getData(d_neutral_city, "neutral_city");
  helper->getData(d_city, "city");
}

Action_UseItem::~Action_UseItem()
{
}

std::string Action_UseItem::dump() const
{
  std::stringstream ss;

  ss <<"Hero " <<d_hero <<" uses item " <<d_item << " and targets player << " << d_victim_player << " friendly city " << d_friendly_city << ", enemy city "
    << d_enemy_city << ", neutral city " << d_neutral_city << ", city " <<
    d_city << "\n";

  return ss.str();
}

bool Action_UseItem::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("hero", d_hero);
  retval &= helper->saveData("item", d_item);
  retval &= helper->saveData("victim_player", d_victim_player);
  retval &= helper->saveData("friendly_city", d_friendly_city);
  retval &= helper->saveData("enemy_city", d_enemy_city);
  retval &= helper->saveData("neutral_city", d_neutral_city);
  retval &= helper->saveData("city", d_city);

  return retval;
}

bool Action_UseItem::fillData(Hero *hero, Item *item, Player *victim,
                              City *friendly_city, City *enemy_city,
                              City *neutral_city, City *city)
{
  d_hero = hero->getId();
  d_item = item->getId();
  if (victim)
    d_victim_player = victim->getId();
  if (friendly_city)
    d_friendly_city = friendly_city->getId();
  if (enemy_city)
    d_enemy_city = enemy_city->getId();
  if (neutral_city)
    d_neutral_city = neutral_city->getId();
  if (city)
    d_city = city->getId();

  return true;
}

//-----------------------------------------------------------------------------
// Action_ReorderArmies

Action_ReorderArmies::Action_ReorderArmies()
:Action(Action::STACK_ORDER), d_stack_id(0), d_player_id(MAX_PLAYERS + 1)
{
}

Action_ReorderArmies::Action_ReorderArmies(const Action_ReorderArmies &action)
: Action(action), d_stack_id(action.d_stack_id), 
    d_player_id(action.d_player_id), d_army_ids(action.d_army_ids)
{
}

Action_ReorderArmies::Action_ReorderArmies(XML_Helper* helper)
:Action(helper)
{

  helper->getData(d_stack_id, "stack_id");
  helper->getData(d_player_id, "player_id");
  std::string armies;
  helper->getData(armies, "armies");
  std::stringstream sarmies;
  sarmies.str(armies);
  int ival = -1;
  while (sarmies.eof() == false)
    {
      ival = -1;
      sarmies >> ival;
      if (ival != -1)
        d_army_ids.push_back((guint32)ival);
    }
}

Action_ReorderArmies::~Action_ReorderArmies()
{
}

std::string Action_ReorderArmies::dump() const
{
  std::stringstream ss;

  ss <<"Stack " <<d_stack_id <<" belonging to " << d_player_id << " has new order: ";
 
  for (std::list<guint32>::const_iterator i = d_army_ids.begin(); 
       i != d_army_ids.end(); i++)
    ss << (*i) << " ";
  ss << "\n";

  return ss.str();
}

bool Action_ReorderArmies::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("stack_id", d_stack_id);
  retval &= helper->saveData("player_id", d_player_id);

  std::stringstream armies;
  std::list<guint32>::const_iterator tit = d_army_ids.begin();
  std::list<guint32>::const_iterator tend = d_army_ids.end();
  for(;tit != tend;++tit)
    armies << (*tit) << " ";
  retval &= helper->saveData("armies", armies.str());
  return retval;
}

bool Action_ReorderArmies::fillData(Stack *s)
{
  d_stack_id = s->getId();
  d_player_id = s->getOwner()->getId();
  for (Stack::iterator i = s->begin(); i != s->end(); i++)
    d_army_ids.push_back((*i)->getId());

  return true;
}

//-----------------------------------------------------------------------------
// Action_ResetStacks

Action_ResetStacks::Action_ResetStacks()
:Action(Action::STACKS_RESET), d_player_id(0)
{
}

Action_ResetStacks::Action_ResetStacks(const Action_ResetStacks &action)
: Action(action), d_player_id(action.d_player_id)
{
}

Action_ResetStacks::Action_ResetStacks(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_player_id, "player_id");
}

Action_ResetStacks::~Action_ResetStacks()
{
}

std::string Action_ResetStacks::dump() const
{
  std::stringstream ss;

  ss <<"Stacks for " <<d_player_id <<" are being recharged.";
  ss << "\n";

  return ss.str();
}

bool Action_ResetStacks::doSave(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->saveData("player_id", d_player_id);
  return retval;
}

bool Action_ResetStacks::fillData(Player *p)
{
  d_player_id = p->getId();

  return true;
}

//-----------------------------------------------------------------------------
// Action_ResetStack

Action_ResetRuins::Action_ResetRuins()
:Action(Action::RUINS_RESET)
{
}

Action_ResetRuins::Action_ResetRuins(const Action_ResetRuins &action)
: Action(action)
{
}

Action_ResetRuins::Action_ResetRuins(XML_Helper* helper)
:Action(helper)
{
}

Action_ResetRuins::~Action_ResetRuins()
{
}

std::string Action_ResetRuins::dump() const
{
  std::stringstream ss;
  ss <<"Ruins are being recharged.";
  ss << "\n";

  return ss.str();
}

bool Action_ResetRuins::doSave(XML_Helper* helper) const
{
  bool retval = true;
  return retval;
}

bool Action_ResetRuins::fillData()
{
  return true;
}

//-----------------------------------------------------------------------------
// Action_CollectTaxesAndPayUpkeep

Action_CollectTaxesAndPayUpkeep::Action_CollectTaxesAndPayUpkeep()
:Action(Action::COLLECT_TAXES_AND_PAY_UPKEEP)
{
}

Action_CollectTaxesAndPayUpkeep::Action_CollectTaxesAndPayUpkeep(const Action_CollectTaxesAndPayUpkeep &action)
: Action(action)
{
}

Action_CollectTaxesAndPayUpkeep::Action_CollectTaxesAndPayUpkeep(XML_Helper* helper)
:Action(helper)
{
}

Action_CollectTaxesAndPayUpkeep::~Action_CollectTaxesAndPayUpkeep()
{
}

std::string Action_CollectTaxesAndPayUpkeep::dump() const
{
  std::stringstream ss;
  ss <<"Collecting taxes from cities and paying the troops.";
  ss << "\n";

  return ss.str();
}

bool Action_CollectTaxesAndPayUpkeep::doSave(XML_Helper* helper) const
{
  bool retval = true;
  return retval;
}

bool Action_CollectTaxesAndPayUpkeep::fillData()
{
  return true;
}

//-----------------------------------------------------------------------------
// Action_Kill

Action_Kill::Action_Kill()
:Action(Action::KILL_PLAYER)
{
}

Action_Kill::Action_Kill(const Action_Kill &action)
: Action(action)
{
}

Action_Kill::Action_Kill(XML_Helper* helper)
:Action(helper)
{
}

Action_Kill::~Action_Kill()
{
}

std::string Action_Kill::dump() const
{
  std::stringstream ss;
  ss <<"player is vanquished.";
  ss << "\n";

  return ss.str();
}

bool Action_Kill::doSave(XML_Helper* helper) const
{
  bool retval = true;
  return retval;
}

bool Action_Kill::fillData()
{
  return true;
}

//-----------------------------------------------------------------------------
// Action_DefendStack

Action_DefendStack::Action_DefendStack()
:Action(Action::STACK_DEFEND), d_stack_id(0)
{
}

Action_DefendStack::Action_DefendStack(const Action_DefendStack &action)
: Action(action), d_stack_id(action.d_stack_id)
{
}

Action_DefendStack::Action_DefendStack(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack_id, "stack_id");
}

Action_DefendStack::~Action_DefendStack()
{
}

std::string Action_DefendStack::dump() const
{
  std::stringstream ss;

  ss <<"Stack " <<d_stack_id <<" is going into defend mode. " << std::endl;

  return ss.str();
}

bool Action_DefendStack::doSave(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->saveData("stack_id", d_stack_id);
  return retval;
}

bool Action_DefendStack::fillData(Stack *s)
{
  d_stack_id = s->getId();
  return true;
}

//-----------------------------------------------------------------------------
// Action_UndefendStack

Action_UndefendStack::Action_UndefendStack()
:Action(Action::STACK_UNDEFEND), d_stack_id(0)
{
}

Action_UndefendStack::Action_UndefendStack(const Action_UndefendStack &action)
: Action(action), d_stack_id(action.d_stack_id)
{
}

Action_UndefendStack::Action_UndefendStack(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack_id, "stack_id");
}

Action_UndefendStack::~Action_UndefendStack()
{
}

std::string Action_UndefendStack::dump() const
{
  std::stringstream ss;

  ss <<"Stack " <<d_stack_id <<" is going out of defend mode. " << std::endl;

  return ss.str();
}

bool Action_UndefendStack::doSave(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->saveData("stack_id", d_stack_id);
  return retval;
}

bool Action_UndefendStack::fillData(Stack *s)
{
  d_stack_id = s->getId();
  return true;
}

//-----------------------------------------------------------------------------
// Action_ParkStack

Action_ParkStack::Action_ParkStack()
:Action(Action::STACK_PARK), d_stack_id(0)
{
}

Action_ParkStack::Action_ParkStack(const Action_ParkStack &action)
: Action(action), d_stack_id(action.d_stack_id)
{
}

Action_ParkStack::Action_ParkStack(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack_id, "stack_id");
}

Action_ParkStack::~Action_ParkStack()
{
}

std::string Action_ParkStack::dump() const
{
  std::stringstream ss;

  ss <<"Stack " <<d_stack_id <<" is going into parked mode. " << std::endl;

  return ss.str();
}

bool Action_ParkStack::doSave(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->saveData("stack_id", d_stack_id);
  return retval;
}

bool Action_ParkStack::fillData(Stack *s)
{
  d_stack_id = s->getId();
  return true;
}

//-----------------------------------------------------------------------------
// Action_UnparkStack

Action_UnparkStack::Action_UnparkStack()
:Action(Action::STACK_UNPARK), d_stack_id(0)
{
}

Action_UnparkStack::Action_UnparkStack(const Action_UnparkStack &action)
: Action(action), d_stack_id(action.d_stack_id)
{
}

Action_UnparkStack::Action_UnparkStack(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack_id, "stack_id");
}

Action_UnparkStack::~Action_UnparkStack()
{
}

std::string Action_UnparkStack::dump() const
{
  std::stringstream ss;

  ss <<"Stack " <<d_stack_id <<" is going out of parked mode. " << std::endl;

  return ss.str();
}

bool Action_UnparkStack::doSave(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->saveData("stack_id", d_stack_id);
  return retval;
}

bool Action_UnparkStack::fillData(Stack *s)
{
  d_stack_id = s->getId();
  return true;
}

//-----------------------------------------------------------------------------
// Action_SelectStack

Action_SelectStack::Action_SelectStack()
:Action(Action::STACK_SELECT), d_stack_id(0)
{
}

Action_SelectStack::Action_SelectStack(const Action_SelectStack &action)
: Action(action), d_stack_id(action.d_stack_id)
{
}

Action_SelectStack::Action_SelectStack(XML_Helper* helper)
:Action(helper)
{
  helper->getData(d_stack_id, "stack_id");
}

Action_SelectStack::~Action_SelectStack()
{
}

std::string Action_SelectStack::dump() const
{
  std::stringstream ss;

  ss <<"Stack " <<d_stack_id <<" is selected. " << std::endl;

  return ss.str();
}

bool Action_SelectStack::doSave(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->saveData("stack_id", d_stack_id);
  return retval;
}

bool Action_SelectStack::fillData(Stack *s)
{
  d_stack_id = s->getId();
  return true;
}

//-----------------------------------------------------------------------------
// Action_DeselectStack

Action_DeselectStack::Action_DeselectStack()
:Action(Action::STACK_DESELECT)
{
}

Action_DeselectStack::Action_DeselectStack(const Action_DeselectStack &action)
: Action(action)
{
}

Action_DeselectStack::Action_DeselectStack(XML_Helper* helper)
:Action(helper)
{
}

Action_DeselectStack::~Action_DeselectStack()
{
}

std::string Action_DeselectStack::dump() const
{
  std::stringstream ss;

  ss <<"deselecting stack. " << std::endl;

  return ss.str();
}

bool Action_DeselectStack::doSave(XML_Helper* helper) const
{
  return true;
}

bool Action_DeselectStack::fillData()
{
  return true;
}

std::string Action::actionTypeToString(Action::Type type)
{
  switch (type)
    {
    case Action::STACK_MOVE:
      return "Action::STACK_MOVE";
    case Action::STACK_SPLIT:
      return "Action::STACK_SPLIT";
    case Action::STACK_FIGHT:
      return "Action::STACK_FIGHT";
    case Action::STACK_JOIN:
      return "Action::STACK_JOIN";
    case Action::RUIN_SEARCH:
      return "Action::RUIN_SEARCH";
    case Action::TEMPLE_SEARCH:
      return "Action::TEMPLE_SEARCH";
    case Action::CITY_OCCUPY:
      return "Action::CITY_OCCUPY";
    case Action::CITY_PILLAGE:
      return "Action::CITY_PILLAGE";
    case Action::CITY_SACK:
      return "Action::CITY_SACK";
    case Action::CITY_RAZE:
      return "Action::CITY_RAZE";
    case Action::CITY_UPGRADE:
      return "Action::CITY_UPGRADE";
    case Action::CITY_BUY:
      return "Action::CITY_BUY";
    case Action::CITY_PROD:
      return "Action::CITY_PROD";
    case Action::REWARD: 
      return "Action::REWARD" ;
    case Action::QUEST:
      return "Action::QUEST";
    case Action::HERO_EQUIP:
      return "Action::HERO_EQUIP";
    case Action::UNIT_ADVANCE:
      return "Action::UNIT_ADVANCE";
    case Action::STACK_DISBAND:
      return "Action::STACK_DISBAND";
    case Action::MODIFY_SIGNPOST:
      return "Action::MODIFY_SIGNPOST";
    case Action::CITY_RENAME:
      return "Action::CITY_RENAME";
    case Action::CITY_VECTOR:
      return "Action::CITY_VECTOR";
    case Action::FIGHT_ORDER:
      return "Action::FIGHT_ORDER";
    case Action::RESIGN:
      return "Action::RESIGN";
    case Action::ITEM_PLANT:
      return "Action::ITEM_PLANT";
    case Action::PRODUCE_UNIT:
      return "Action::PRODUCE_UNIT";
    case Action::PRODUCE_VECTORED_UNIT:
      return "Action::PRODUCE_VECTORED_UNIT";
    case Action::DIPLOMATIC_STATE:
      return "Action::DIPLOMATIC_STATE";
    case Action::DIPLOMATIC_PROPOSAL:
      return "Action::DIPLOMATIC_PROPOSAL";
    case Action::DIPLOMATIC_SCORE:
      return "Action::DIPLOMATIC_SCORE";
    case Action::END_TURN:
      return "Action::END_TURN";
    case Action::CITY_CONQUER:
      return "Action::CITY_CONQUER";
    case Action::RECRUIT_HERO:
      return "Action::RECRUIT_HERO";
    case Action::PLAYER_RENAME:
      return "Action::PLAYER_RENAME";
    case Action::CITY_DESTITUTE:
      return "Action::CITY_DESTITUTE";
    case Action::INIT_TURN:
      return "Action::INIT_TURN";
    case Action::CITY_LOOT:
      return "Action::CITY_LOOT";
    case Action::USE_ITEM:
      return "Action::USE_ITEM";
    case Action::STACK_ORDER:
      return "Action::STACK_ORDER";
    case Action::STACKS_RESET:
      return "Action::STACKS_RESET";
    case Action::RUINS_RESET:
      return "Action::RUINS_RESET";
    case Action::COLLECT_TAXES_AND_PAY_UPKEEP:
      return "Action::COLLECT_TAXES_AND_PAY_UPKEEP";
    case Action::KILL_PLAYER:
      return "Action::KILL_PLAYER";
    case Action::STACK_DEFEND:
      return "Action::STACK_DEFEND";
    case Action::STACK_UNDEFEND:
      return "Action::STACK_UNDEFEND";
    case Action::STACK_PARK:
      return "Action::STACK_PARK";
    case Action::STACK_UNPARK:
      return "Action::STACK_UNPARK";
    case Action::STACK_SELECT:
      return "Action::STACK_SELECT";
    case Action::STACK_DESELECT:
      return "Action::STACK_DESELECT";
    }
      
  return "Action::MOVE";
}

Action::Type Action::actionTypeFromString(std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Action::Type(atoi(str.c_str()));
  if (str == "Action::STACK_MOVE")
    return Action::STACK_MOVE;
  else if (str == "Action::STACK_SPLIT")
    return Action::STACK_SPLIT;
  else if (str == "Action::STACK_FIGHT")
    return Action::STACK_FIGHT;
  else if (str == "Action::STACK_JOIN")
    return Action::STACK_JOIN;
  else if (str == "Action::RUIN_SEARCH")
    return Action::RUIN_SEARCH;
  else if (str == "Action::TEMPLE_SEARCH")
    return Action::TEMPLE_SEARCH;
  else if (str == "Action::CITY_OCCUPY")
    return Action::CITY_OCCUPY;
  else if (str == "Action::CITY_PILLAGE")
    return Action::CITY_PILLAGE;
  else if (str == "Action::CITY_SACK")
    return Action::CITY_SACK;
  else if (str == "Action::CITY_RAZE")
    return Action::CITY_RAZE;
  else if (str == "Action::CITY_UPGRADE")
    return Action::CITY_UPGRADE;
  else if (str == "Action::CITY_BUY")
    return Action::CITY_BUY;
  else if (str == "Action::CITY_PROD")
    return Action::CITY_PROD;
  else if (str == "Action::REWARD" )
    return Action::REWARD; 
  else if (str == "Action::QUEST")
    return Action::QUEST;
  else if (str == "Action::HERO_EQUIP")
    return Action::HERO_EQUIP;
  else if (str == "Action::UNIT_ADVANCE")
    return Action::UNIT_ADVANCE;
  else if (str == "Action::STACK_DISBAND")
    return Action::STACK_DISBAND;
  else if (str == "Action::MODIFY_SIGNPOST")
    return Action::MODIFY_SIGNPOST;
  else if (str == "Action::CITY_RENAME")
    return Action::CITY_RENAME;
  else if (str == "Action::CITY_VECTOR")
    return Action::CITY_VECTOR;
  else if (str == "Action::FIGHT_ORDER")
    return Action::FIGHT_ORDER;
  else if (str == "Action::RESIGN")
    return Action::RESIGN;
  else if (str == "Action::ITEM_PLANT")
    return Action::ITEM_PLANT;
  else if (str == "Action::PRODUCE_UNIT")
    return Action::PRODUCE_UNIT;
  else if (str == "Action::PRODUCE_VECTORED_UNIT")
    return Action::PRODUCE_VECTORED_UNIT;
  else if (str == "Action::DIPLOMATIC_STATE")
    return Action::DIPLOMATIC_STATE;
  else if (str == "Action::DIPLOMATIC_PROPOSAL")
    return Action::DIPLOMATIC_PROPOSAL;
  else if (str == "Action::DIPLOMATIC_SCORE")
    return Action::DIPLOMATIC_SCORE;
  else if (str == "Action::END_TURN")
    return Action::END_TURN;
  else if (str == "Action::CITY_CONQUER")
    return Action::CITY_CONQUER;
  else if (str == "Action::RECRUIT_HERO")
    return Action::RECRUIT_HERO;
  else if (str == "Action::PLAYER_RENAME")
    return Action::PLAYER_RENAME;
  else if (str == "Action::CITY_DESTITUTE")
    return Action::CITY_DESTITUTE;
  else if (str == "Action::INIT_TURN")
    return Action::INIT_TURN;
  else if (str == "Action::CITY_LOOT")
    return Action::CITY_LOOT;
  else if (str == "Action::USE_ITEM")
    return Action::USE_ITEM;
  else if (str == "Action::STACK_ORDER")
    return Action::STACK_ORDER;
  else if (str == "Action::STACKS_RESET")
    return Action::STACKS_RESET;
  else if (str == "Action::RUINS_RESET")
    return Action::RUINS_RESET;
  else if (str == "Action::COLLECT_TAXES_AND_PAY_UPKEEP")
    return Action::COLLECT_TAXES_AND_PAY_UPKEEP;
  else if (str == "Action::KILL_PLAYER")
    return Action::KILL_PLAYER;
  else if (str == "Action::STACK_DEFEND")
    return Action::STACK_DEFEND;
  else if (str == "Action::STACK_UNDEFEND")
    return Action::STACK_UNDEFEND;
  else if (str == "Action::STACK_PARK")
    return Action::STACK_PARK;
  else if (str == "Action::STACK_UNPARK")
    return Action::STACK_UNPARK;
  else if (str == "Action::STACK_SELECT")
    return Action::STACK_SELECT;
  else if (str == "Action::STACK_DESELECT")
    return Action::STACK_DESELECT;
  return Action::STACK_MOVE;
}

