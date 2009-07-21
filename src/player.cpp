// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2004 John Farrell
// Copyright (C) 2005 Bryan Duff
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

#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "player.h"
#include "playerlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "templelist.h"
#include "city.h"
#include "path.h"
#include "armysetlist.h"
#include "real_player.h"
#include "ai_dummy.h"
#include "ai_fast.h"
#include "ai_smart.h"
#include "network_player.h"
#include "counter.h"
#include "army.h"
#include "hero.h"
#include "heroproto.h"
#include "Configuration.h"
#include "GameScenarioOptions.h"
#include "action.h"
#include "network-action.h"
#include "history.h"
#include "network-history.h"
#include "AI_Analysis.h"
#include "AI_Allocation.h"
#include "FogMap.h"
#include "QuestsManager.h"
#include "GameMap.h"
#include "signpost.h"
#include "vectoredunit.h"
#include "ucompose.hpp"
#include "armyprodbase.h"
#include "Triumphs.h"
#include "Backpack.h"
#include "MapBackpack.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
#define debug(x)

std::string Player::d_tag = "player";

// signal
sigc::signal<void, Player::Type> sendingTurn;

Player::Player(string name, Uint32 armyset, SDL_Color color, int width,
	       int height, Type type, int player_no)
    :d_color(color), d_name(name), d_armyset(armyset), d_gold(1000),
    d_dead(false), d_immortal(false), d_type(type), d_upkeep(0), d_income(0),
    d_observable(true), surrendered(false)
{
    if (player_no != -1)
	d_id = player_no;
    else
	d_id = fl_counter->getNextId();
    d_stacklist = new Stacklist();
    debug("type of " << d_name << " is " << type)
        
    d_fogmap = new FogMap(width, height);

    //initial fight order is the order in which the armies appear
    //in the default.xml file.
    Uint32 size = Armysetlist::getInstance()->getSize(d_armyset);
    for (unsigned int i = 0; i < size; i++)
    {
      d_fight_order.push_back(i);
    }

    for (unsigned int i = 0 ; i < MAX_PLAYERS; i++)
    {
      d_diplomatic_state[i] = AT_PEACE;
      d_diplomatic_proposal[i] = NO_PROPOSAL;
      d_diplomatic_score[i] = DIPLOMACY_STARTING_SCORE;
    }
    d_diplomatic_rank = 0;
    d_diplomatic_title = std::string("");

    d_triumphs = new Triumphs();
}

Player::Player(const Player& player)
    :d_color(player.d_color), d_name(player.d_name), d_armyset(player.d_armyset),
    d_gold(player.d_gold), d_dead(player.d_dead), d_immortal(player.d_immortal),
    d_type(player.d_type), d_id(player.d_id), 
    d_fight_order(player.d_fight_order), d_upkeep(player.d_upkeep), 
    d_income(player.d_income), d_observable(player.d_observable),
    surrendered(player.surrendered)
{
    // as the other player is propably dumped somehow, we need to deep copy
    // everything. This costs a lot, but the only useful situation for this
    // I can think of is a change of the player type, as occurs in the editor.
    d_stacklist = new Stacklist();
    for (Stacklist::iterator it = player.d_stacklist->begin(); 
	 it != player.d_stacklist->end(); it++)
    {
        Stack* mine = new Stack(**it);
        // change the stack's loyalty
        mine->setPlayer(this);
        d_stacklist->push_back(mine);
    }

    // copy actions
    std::list<Action*>::const_iterator ait;
    for (ait = player.d_actions.begin(); ait != player.d_actions.end(); ait++)
        d_actions.push_back(Action::copy(*ait));

    // copy events
    std::list<History*>::const_iterator pit;
    for (pit = player.d_history.begin(); pit != player.d_history.end(); pit++)
        d_history.push_back(History::copy(*pit));

    // copy fogmap; TBD
    d_fogmap = new FogMap(*player.getFogMap());

    // copy diplomatic states
    for (unsigned int i = 0 ; i < MAX_PLAYERS; i++)
      {
	d_diplomatic_state[i] = player.d_diplomatic_state[i];
	d_diplomatic_proposal[i] = player.d_diplomatic_proposal[i];
	d_diplomatic_score[i] = player.d_diplomatic_score[i];
      }
    d_diplomatic_rank = player.d_diplomatic_rank;
    d_diplomatic_title = player.d_diplomatic_title;

    d_triumphs = new Triumphs(*player.getTriumphs());
}

Player::Player(XML_Helper* helper)
    :d_stacklist(0), d_fogmap(0), surrendered(false)
{
    helper->getData(d_id, "id");
    helper->getData(d_name, "name");
    helper->getData(d_gold, "gold");
    helper->getData(d_dead, "dead");
    helper->getData(d_immortal, "immortal");
    std::string type_str;
    helper->getData(type_str, "type");
    d_type = playerTypeFromString(type_str);
    helper->getData(d_upkeep, "upkeep");
    helper->getData(d_income, "income");

    string s;
    helper->getData(s, "color");
    
    int i;
    istringstream scolor(s);
    scolor >> i; d_color.r = i;
    scolor >> i; d_color.g = i;
    scolor >> i; d_color.b = i;

    helper->getData(d_armyset, "armyset");

    // Read in Fight Order.  One ranking per army type.
    std::string fight_order;
    std::stringstream sfight_order;
    Uint32 val;
    helper->getData(fight_order, "fight_order");
    sfight_order.str(fight_order);
    Uint32 size = Armysetlist::getInstance()->getSize(d_armyset);
    for (unsigned int i = 0; i < size; i++)
    {
            sfight_order >> val;
            d_fight_order.push_back(val);
    }

    // Read in Diplomatic States.  One state per player.
    std::string diplomatic_states;
    std::stringstream sdiplomatic_states;
    helper->getData(diplomatic_states, "diplomatic_states");
    sdiplomatic_states.str(diplomatic_states);
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
            sdiplomatic_states >> val;
	    d_diplomatic_state[i] = DiplomaticState(val);
    }

    helper->getData(d_diplomatic_rank, "diplomatic_rank");
    helper->getData(d_diplomatic_title, "diplomatic_title");

    // Read in Diplomatic Proposals.  One proposal per player.
    std::string diplomatic_proposals;
    std::stringstream sdiplomatic_proposals;
    helper->getData(diplomatic_proposals, "diplomatic_proposals");
    sdiplomatic_proposals.str(diplomatic_proposals);
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
            sdiplomatic_proposals>> val;
	    d_diplomatic_proposal[i] = DiplomaticProposal(val);
    }

    // Read in Diplomatic Scores.  One score per player.
    std::string diplomatic_scores;
    std::stringstream sdiplomatic_scores;
    helper->getData(diplomatic_scores, "diplomatic_scores");
    sdiplomatic_scores.str(diplomatic_scores);
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
            sdiplomatic_scores >> val;
	    d_diplomatic_score[i] = val;
    }
    helper->getData(d_observable, "observable");

    helper->registerTag(Action::d_tag, sigc::mem_fun(this, &Player::load));
    helper->registerTag(History::d_tag, sigc::mem_fun(this, &Player::load));
    helper->registerTag(Stacklist::d_tag, sigc::mem_fun(this, &Player::load));
    helper->registerTag(FogMap::d_tag, sigc::mem_fun(this, &Player::load));
    helper->registerTag(Triumphs::d_tag, sigc::mem_fun(this, &Player::load));

}

Player::~Player()
{
    if (d_stacklist)
    {
        d_stacklist->flClear();
        delete d_stacklist;
    }
    if (d_fogmap)
        delete d_fogmap;

    clearActionlist();
    clearHistorylist();
    d_fight_order.clear();
}

Player* Player::create(std::string name, Uint32 armyset, SDL_Color color, int width, int height, Type type)
{
  switch(type)
  {
  case HUMAN:
    return (new RealPlayer(name, armyset, color, width, height));
  case AI_FAST:
    return (new AI_Fast(name, armyset, color, width, height));
  case AI_DUMMY:
    return (new AI_Dummy(name, armyset, color, width, height));
  case AI_SMART:
    return (new AI_Smart(name, armyset, color, width, height));
  case NETWORKED:
    return (new NetworkPlayer(name, armyset, color, width, height));
  }

  return 0;
}

Player* Player::create(Player* orig, Type type)
{
    switch(type)
    {
        case HUMAN:
            return new RealPlayer(*orig);
        case AI_FAST:
            return new AI_Fast(*orig);
        case AI_DUMMY:
            return new AI_Dummy(*orig);
        case AI_SMART:
            return new AI_Smart(*orig);
        case NETWORKED:
            return new NetworkPlayer(*orig);
    }

    return 0;
}

void Player::initTurn()
{
  clearActionlist();
  History_StartTurn* item = new History_StartTurn();
  addHistory(item);
  Action_InitTurn* action = new Action_InitTurn();
  addAction(action);
}

void Player::setColor(SDL_Color c)
{
    d_color = c;
}

SDL_Color Player::getMaskColor() const
{
    // This is a bit tricky. The color values we return here encode additional
    // shifts that are performed when getting the color. I.e. a color value for
    // red of 8 means that the red color is completely ignored.

    // For each color component, find the n where 2^n best describes the color.
    // The mask value then is (8-n).
    SDL_Color c;
    c.r = c.g = c.b = 0;

    for (int i = 8, diff = 257; i > 0; i--)
    {
        int color = 1<<i;
        int tmp_diff = abs(d_color.r - color);
        c.r = 8 - (i+1);

        if (diff < tmp_diff)
            break;
        else
            diff = tmp_diff;
    }
        
    for (int i = 8, diff = 257; i > 0; i--)
    {
        int color = 1<<i;
        int tmp_diff = abs(d_color.g - color);
        c.g = 8 - (i+1);

        if (diff < tmp_diff)
            break;
        else
            diff = tmp_diff;
    }
        
    for (int i = 8, diff = 257; i > 0; i--)
    {
        int color = 1<<i;
        int tmp_diff = abs(d_color.b - color);
        c.b = 8 - (i+1);

        if (diff < tmp_diff)
            break;
        else
            diff = tmp_diff;
    }
        
    return c;
}

void Player::addGold(int gold)
{
    d_gold += gold;
    schangingStats.emit();
}

void Player::withdrawGold(int gold)
{
    d_gold -= gold;
    if (d_gold < 0)
      d_gold = 0; /* bankrupt.  should we start turning off city production? */
    schangingStats.emit();
}

std::string Player::getName(bool translate) const
{
    if (translate)
        return __(d_name);

    return d_name;
}

Stack* Player::getActivestack()
{
    return d_stacklist->getActivestack();
}

void Player::dumpActionlist() const
{
    for (list<Action*>::const_iterator it = d_actions.begin();
        it != d_actions.end(); it++)
    {
        cerr <<(*it)->dump() << endl;
    }    
}

void Player::dumpHistorylist() const
{
    for (list<History*>::const_iterator it = d_history.begin();
        it != d_history.end(); it++)
    {
        cerr <<(*it)->dump() << endl;
    }    
}

void Player::clearActionlist()
{
    for (list<Action*>::iterator it = d_actions.begin();
        it != d_actions.end(); it++)
    {
      delete (*it);
    }
    d_actions.clear();
}

void Player::clearHistorylist()
{
    for (list<History*>::iterator it = d_history.begin();
        it != d_history.end(); it++)
    {
        delete (*it);
    }
    d_history.clear();
}

void Player::addStack(Stack* stack)
{
    stack->setPlayer(this);
    d_stacklist->push_back(stack);
}

bool Player::deleteStack(Stack* stack)
{
    AI_Analysis::deleteStack(stack);
    AI_Allocation::deleteStack(stack);
    return d_stacklist->flRemove(stack);
}

void Player::kill()
{
  doKill();
}

void Player::doKill()
{
    if (d_immortal)
        // ignore it
        return;

    d_observable = false;
    History_PlayerVanquished* item;
    item = new History_PlayerVanquished();
    addHistory(item);

    d_dead = true;
    d_stacklist->flClear();

    // Since in some cases the player can be killed rather innocently
    // (using reactions), we also need to clear the player's traces in the
    // single cities
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
        if ((*it)->getOwner() == this && (*it)->isBurnt() == false)
            Playerlist::getInstance()->getNeutral()->takeCityInPossession(*it);

    d_diplomatic_rank = 0;
    d_diplomatic_title = std::string("");
}

bool Player::save(XML_Helper* helper) const
{
    bool retval = true;

    // we do not want to have the character of the colors written out
    // (savefile is unicode encoded => may create problems)
    std::stringstream s;
    s << static_cast<Uint32>(d_color.r) <<" ";
    s << static_cast<Uint32>(d_color.g) <<" ";
    s << static_cast<Uint32>(d_color.b);

    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("color", s.str());
    retval &= helper->saveData("armyset", d_armyset);
    retval &= helper->saveData("gold", d_gold);
    retval &= helper->saveData("dead", d_dead);
    retval &= helper->saveData("immortal", d_immortal);
    std::string type_str = playerTypeToString(Player::Type(d_type));
    retval &= helper->saveData("type", type_str);
    debug("type of " << d_name << " is " << d_type)
    retval &= helper->saveData("upkeep", d_upkeep);
    retval &= helper->saveData("income", d_income);

    // save the fight order, one ranking per army type
    std::stringstream fight_order;
    for (std::list<Uint32>::const_iterator it = d_fight_order.begin();
         it != d_fight_order.end(); it++)
      {
        fight_order << (*it) << " ";
      }
    retval &= helper->saveData("fight_order", fight_order.str());

    // save the diplomatic states, one state per player
    std::stringstream diplomatic_states;
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
      {
	diplomatic_states << d_diplomatic_state[i] << " ";
      }
    retval &= helper->saveData("diplomatic_states", diplomatic_states.str());

    retval &= helper->saveData("diplomatic_rank", d_diplomatic_rank);
    retval &= helper->saveData("diplomatic_title", d_diplomatic_title);

    // save the diplomatic proposals, one proposal per player
    std::stringstream diplomatic_proposals;
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
      {
	diplomatic_proposals << d_diplomatic_proposal[i] << " ";
      }
    retval &= helper->saveData("diplomatic_proposals", 
			       diplomatic_proposals.str());

    // save the diplomatic scores, one score per player
    std::stringstream diplomatic_scores;
    for (unsigned int i = 0; i < MAX_PLAYERS; i++)
      {
	diplomatic_scores << d_diplomatic_score[i] << " ";
      }
    retval &= helper->saveData("diplomatic_scores", diplomatic_scores.str());

    retval &= helper->saveData("observable", d_observable);

    //save the actionlist
    for (list<Action*>::const_iterator it = d_actions.begin();
            it != d_actions.end(); it++)
        retval &= (*it)->save(helper);
    
    //save the pasteventlist
    for (list<History*>::const_iterator it = d_history.begin();
            it != d_history.end(); it++)
        retval &= (*it)->save(helper);

    retval &= d_stacklist->save(helper);
    retval &= d_fogmap->save(helper);
    retval &= d_triumphs->save(helper);

    return retval;
}

Player* Player::loadPlayer(XML_Helper* helper)
{
    Type type;
    std::string type_str;
    helper->getData(type_str, "type");
    type = playerTypeFromString(type_str);

    switch (type)
    {
        case HUMAN:
            return new RealPlayer(helper);
        case AI_FAST:
            return new AI_Fast(helper);
        case AI_SMART:
            return new AI_Smart(helper);
        case AI_DUMMY:
            return new AI_Dummy(helper);
        case NETWORKED:
            return new NetworkPlayer(helper);
    }

    return 0;
}

bool Player::load(string tag, XML_Helper* helper)
{
    if (tag == Action::d_tag)
    {
        Action* action;
        action = Action::handle_load(helper);
        d_actions.push_back(action);
    }
    if (tag == History::d_tag)
    {
        History* history;
        history = History::handle_load(helper);
        d_history.push_back(history);
    }

    if (tag == Stacklist::d_tag)
        d_stacklist = new Stacklist(helper);

    if (tag == FogMap::d_tag)
        d_fogmap = new FogMap(helper);

    if (tag == Triumphs::d_tag)
	d_triumphs = new Triumphs(helper);

    return true;
}

void Player::addAction(Action *action)
{
  d_actions.push_back(action);
  NetworkAction *copy = new NetworkAction(action, this);
  acting.emit(copy);
  //free'd in game-server
}

void Player::addHistory(History *history)
{
  d_history.push_back(history);
  NetworkHistory *copy = new NetworkHistory(history, this);
  history_written.emit(copy);
  //free'd in game-server
}


Uint32 Player::getScore()
{
  //go get our last published score in the history
  Uint32 score = 0;
  std::list<History*>::iterator it = d_history.begin();
  for (; it != d_history.end(); it++)
    {
      if ((*it)->getType() == History::SCORE)
	score = static_cast<History_Score*>(*it)->getScore();
    }
  return score;
}

void Player::calculateUpkeep()
{
    d_upkeep = 0;
    Stacklist *sl = getStacklist();
    for (Stacklist::iterator i = sl->begin(), iend = sl->end(); i != iend; ++i)
      d_upkeep += (*i)->getUpkeep();
}

void Player::calculateIncome()
{
    d_income = 0;
    Citylist *cl = Citylist::getInstance();
    for (Citylist::iterator i = cl->begin(), iend = cl->end(); i != iend; ++i)
      {
	if ((*i)->getOwner() == this)
	  d_income += (*i)->getGold();
      }
}

void Player::doSetFightOrder(std::list<Uint32> order)
{
  d_fight_order = order;
}

void Player::setFightOrder(std::list<Uint32> order) 
{
  doSetFightOrder(order);
  
  Action_FightOrder * item = new Action_FightOrder();
  item->fillData(order);
  addAction(item);
}

Stack *Player::doStackSplit(Stack* s)
{
    debug("Player::doStackSplit("<<s->getId()<<")")

    Army* ungrouped = s->getFirstUngroupedArmy();
    if (!ungrouped)        //no armies to split
        return 0;

    bool all_ungrouped = true;    //the whole stack would be split
    for (Stack::iterator it = s->begin(); it != s->end(); it++)
    {
        if ((*it)->isGrouped())
        {
            all_ungrouped = false;
        }
    }
    if (all_ungrouped)
        return 0;

    Stack* new_stack = new Stack(this, s->getPos());

    while (ungrouped)
    {
        new_stack->push_back(ungrouped);
        s->erase(find(s->begin(), s->end(), ungrouped));

        ungrouped->setGrouped(true);
        ungrouped = s->getFirstUngroupedArmy();
    }

    d_stacklist->push_back(new_stack);
    
    return new_stack;
}

bool Player::stackSplit(Stack* s)
{
  Stack *new_stack = doStackSplit(s);

  if (new_stack)
  {
    Action_Split* item = new Action_Split();
    item->fillData(s, new_stack);
    addAction(item);
  }
  
  return new_stack;
}

void Player::doStackJoin(Stack* receiver, Stack* joining, bool grouped)
{
    // Now if grouped is set to false, ungroup all the receiving stack's armies
    // (by default, only the joining stacks armies will continue to move). 
    for (Stack::iterator it = receiver->begin(); it != receiver->end(); it++)
        (*it)->setGrouped(grouped);

    for (Stack::iterator it = joining->begin(); it != joining->end(); it++)
    {
        receiver->push_front(*it);
        (*it)->setGrouped(true);
    }

    joining->clear();    //clear only erases the pointers not the armies
    d_stacklist->flRemove(joining);
    
    d_stacklist->setActivestack(receiver);
}

bool Player::stackJoin(Stack* receiver, Stack* joining, bool grouped)
{
    debug("Player::stackJoin("<<receiver->getId()<<","<<joining->getId()<<")")

    if ((receiver == 0) || (joining == 0))
        return false;

    if (joining->canJoin(receiver) == false)
      {
	//fixme: this is a bad idea.  it seems recursively bad.
	//Stack *already_there = Stacklist::getAmbiguity(joining);
	//doStackJoin(joining, already_there,  false);
        return false;
      }
    
    Action_Join* item = new Action_Join();
    item->fillData(receiver, joining);
    addAction(item);

    doStackJoin(receiver, joining, grouped);
    
    return true;
}

bool Player::stackSplitAndMove(Stack* s)
{
  if (s->getPath()->size() == 0)
    return false;
  Stack *join = getStacklist()->getObjectAt(*s->getPath()->back());
  if (join)
    return stackSplitAndMoveToJoin(s, join);
  else
    return stackSplitAndMoveToAttack(s);
}

bool Player::stackSplitAndMoveToJoin(Stack* s, Stack *join)
{
  //the stack can't get there, but maybe part of the stack can.
  if (s->getPath()->empty())
    return false;

  if (s->canJoin(join) == false)
    return false;

  Path::iterator it = s->getPath()->end();
  it--;
  std::vector<Uint32> ids;
  ids = s->determineReachableArmies(**(it));
  if (ids.size() == 0)
    return false;
  //if they're all reachable and we can join, just move them
  if (ids.size() == s->size())
    return stackMove(s);

  for (Stack::iterator it = s->begin(); it != s->end(); it++)
    {
      (*it)->setGrouped(false);
      if (find(ids.begin(), ids.end(), (*it)->getId()) != ids.end())
	{
	  if (s->countGroupedArmies() + join->size() < MAX_STACK_SIZE)
	    (*it)->setGrouped(true);
	}
    }
  //this splits the ungrouped armies into their own stack
  stackSplit(s);
  return stackMove(s);
}

bool Player::stackSplitAndMoveToAttack(Stack* s)
{
  //the stack can't get there, but maybe part of the stack can.
  if (s->getPath()->empty())
    return false;

  Path::iterator it = s->getPath()->end();
  it--;
  std::vector<Uint32> ids;
  ids = s->determineReachableArmies(**(it));
  if (ids.size() == 0)
    return false;
  if (ids.size() == s->size())
    return stackMove(s);

  for (Stack::iterator it = s->begin(); it != s->end(); it++)
    {
      (*it)->setGrouped(false);
      if (find(ids.begin(), ids.end(), (*it)->getId()) != ids.end())
	(*it)->setGrouped(true);
    }
  //this splits the ungrouped armies into their own stack
  stackSplit(s);
  return stackMove(s);
}

bool Player::stackMove(Stack* s)
{
    debug("Player::stackMove(Stack*)")

    if (s->getPath()->empty())
    {
        return false;
    }

    Path::iterator it = s->getPath()->end();
    it--;
    MoveResult *result = stackMove(s, **(it), true);
    bool ret = result->didSomething();//result->moveSucceeded();
    delete result;
    result = 0;
    return ret;
}

MoveResult *Player::stackMove(Stack* s, Vector<int> dest, bool follow)
{
    debug("Player::stack_move()");
    //if follow is set to true, follow an already calculated way, else
    //calculate it here
    if (!follow)
    {
        s->getPath()->calculate(s, dest);
    }
    else
    {
        s->getPath()->checkPath(s);
    }

    if (s->getPath()->empty())
    {
        return new MoveResult(false);        //way to destination blocked
    }

    int stepCount = 0;
    while ((s->getPath()->size() > 1 && stackMoveOneStep(s)) ||
	   stackMoveOneStepOverTooLargeFriendlyStacks(s))
      {
	stepCount++;
        supdatingStack.emit(0);
      }

    if (s->getPath()->size() == 1 && s->enoughMoves())
    //now look for fight targets, joins etc.
    {
        Vector<int> pos = **(s->getPath()->begin());
        City* city = Citylist::getInstance()->getObjectAt(pos);
        Stack* target = Stacklist::getObjectAt(pos);

        //first fight_city to avoid ambiguity with fight_army
        if (city && (city->getOwner() != this) && (!city->isBurnt()))
        {
	    if (this->getDiplomaticState (city->getOwner()) != AT_WAR)
	      {
		if (streacheryStack.emit (s, city->getOwner(), 
					  city->getPos()) == false)
		  {
		    s->getPath()->flClear();
		    MoveResult *moveResult = new MoveResult(false);
		    return moveResult;
		  }
	      }
            Fight::Result result;
            MoveResult *moveResult = new MoveResult(true);
	    if (stackMoveOneStep(s))
	      {
		stepCount++;
		moveResult = new MoveResult(true);
	      }
	    else
	      {
		moveResult = new MoveResult(false);
		moveResult->setStepCount(stepCount);
		return moveResult;
	      }

            vector<Stack*> def_in_city = Stacklist::defendersInCity(city);
            if (!def_in_city.empty())
            {
                // This is a hack to circumvent the limitations of stackFight.
                if (!target)
                  target = def_in_city[0];
 
                result = stackFight(&s, &target);
            }
            else
                result = Fight::ATTACKER_WON;

            moveResult->setFightResult(result);

            // We may only take the city if we have defeated all defenders
            if (result == Fight::ATTACKER_WON)
            {
                adjustDiplomacyFromConqueringCity(city);
                conquerCity(city, s);
                invadeCity(city); //let AIs determine what to do with city
            }
            
	    cityfight_finished(city, result);
            moveResult->setStepCount(stepCount);
            supdatingStack.emit(0);
            
            return moveResult;
        }
        
        //another friendly stack => join it
        else if (target && target->getOwner() == this)
          {
	    bool moved = false;
            if (stackMoveOneStep(s))
	      {
		moved = true;
                stepCount++;
	      }
	    Stack *other_stack = Stacklist::getAmbiguity(s);
            stackJoin(other_stack, s, false);

	    if (other_stack)
	      d_stacklist->getActivestack()->sortForViewing(false);
	      
	    supdatingStack.emit(0);
    
            MoveResult *moveResult = new MoveResult(moved);
            moveResult->setStepCount(stepCount);
            moveResult->setJoin(moved);
	    //if (moved == false)
	      //d_stacklist->getActivestack()->getPath()->flClear();
            return moveResult;
         }
        
        //enemy stack => fight
        else if (target)
        {
	  if (this->getDiplomaticState (target->getOwner()) == AT_PEACE)
	    {
	      if (streacheryStack.emit (s, target->getOwner(), 
					target->getPos()) == false)
		{
		  s->getPath()->flClear();
		  MoveResult *moveResult = new MoveResult(false);
		  moveResult->setStepCount(stepCount);
		  return moveResult;
		}
	    }
            MoveResult *moveResult = new MoveResult(true);
        
            Fight::Result result = stackFight(&s, &target);
            moveResult->setFightResult(result);
            if (!target)
            {
                if (stackMoveOneStep(s))
                    stepCount++;
            }
            else if (s)
                s->decrementMoves(2);
            
            supdatingStack.emit(0);
            moveResult->setStepCount(stepCount);
            return moveResult;
        }
        
        //else
        if (stackMoveOneStep(s))
            stepCount++;

        supdatingStack.emit(0);
    
        MoveResult *moveResult = new MoveResult(true);
        moveResult->setStepCount(stepCount);
        return moveResult;
    }
    else if (s->getPath()->size() == 1 && s->enoughMoves() == false)
    {
      /* if we can't attack a city, don't remember it in the stack's path. */
        Vector<int> pos = **(s->getPath()->begin());
        City* city = Citylist::getInstance()->getObjectAt(pos);
	if (city && city->getOwner() != this)
	  s->getPath()->flClear();
    }

    //If there is another stack where we landed, join it. We can't have two
    //stacks share the same maptile
    if (Stacklist::getAmbiguity(s))
    {
        stackJoin(Stacklist::getAmbiguity(s), s, false);
        supdatingStack.emit(0);
        MoveResult *moveResult = new MoveResult(true);
        moveResult->setStepCount(stepCount);
        moveResult->setJoin(true);
        return moveResult;
    }

    MoveResult *moveResult = new MoveResult(true);
    moveResult->setStepCount(stepCount);
    return moveResult;
}

	   
bool Player::stackMoveOneStepOverTooLargeFriendlyStacks(Stack *s)
{
  if (!s)
    return false;

  if (!s->enoughMoves())
    return false;

  if (s->getPath()->size() <= 1)
    return false;

  Vector<int> dest = *s->getPath()->front();
  Stack *another_stack = d_stacklist->getObjectAt(dest);
  if (!another_stack)
    return false;

  if (another_stack->getOwner() != s->getOwner())
    return false;

  if (d_stacklist->canJumpOverTooLargeStack(s) == false)
    return false;

  Action_Move* item = new Action_Move();
  item->fillData(s, dest);
  addAction(item);

  s->moveOneStep(true);
  return true;
}

bool Player::stackMoveOneStep(Stack* s)
{
  if (!s)
    return false;

  sbusy.emit();

  if (!s->enoughMoves())
    return false;

  Vector<int> dest = *s->getPath()->front();
  
  Stack *another_stack = d_stacklist->getObjectAt(dest);
  if (another_stack)
    {
      if (another_stack->getOwner() == s->getOwner())
	{
	  if (s->canJoin(another_stack) == false)
	    return false;
	}
      else
	{
	  //if we're attacking, then jump onto the square with the enemy.
	  if (s->getPath()->size() != 1)
	    return false;
	}

    }
  Action_Move* item = new Action_Move();
  item->fillData(s, dest);
  addAction(item);

  s->moveOneStep();

  return true;
}

void Player::cleanupAfterFight(std::list<Stack*> &attackers,
                               std::list<Stack*> &defenders)
{
  // get attacker and defender heroes and more...
  std::vector<Uint32> attackerHeroes, defenderHeroes;
    
  getHeroes(attackers, attackerHeroes);
  getHeroes(defenders, defenderHeroes);

  // here we calculate also the total XP to add when a player have a battle
  // clear dead defenders
  debug("clean dead defenders");
  double defender_xp = removeDeadArmies(defenders, attackerHeroes);

  // and dead attackers
  debug("clean dead attackers");
  double attacker_xp = removeDeadArmies(attackers, defenderHeroes);

  debug("after fight: attackers empty? " << attackers.empty()
        << "(" << attackers.size() << ")");

  if (!attackers.empty() && defender_xp != 0)
    updateArmyValues(attackers, defender_xp);
    
  if (attacker_xp != 0)
    updateArmyValues(defenders, attacker_xp);

  supdatingStack.emit(0);
}

Fight::Result Player::stackFight(Stack** attacker, Stack** defender) 
{
    debug("stackFight: player = " << getName()<<" at position "
          <<(*defender)->getPos().x<<","<<(*defender)->getPos().y);

    // save the defender's player for future use
    Player* pd = (*defender)->getOwner();

    // I suppose, this should be always true, but one can never be sure
    bool attacker_active = *attacker == d_stacklist->getActivestack();

    Fight fight(*attacker, *defender);
    fight.battle(GameScenarioOptions::s_intense_combat);
    
    fight_started.emit(fight);
    // cleanup
    
    // add a fight item about the combat
    Action_Fight* item = new Action_Fight();
    item->fillData(&fight);
    addAction(item);

    std::list<Stack *> attackers = fight.getAttackers(),
      defenders = fight.getDefenders();
    
    cleanupAfterFight(attackers, defenders);
    
    // Set the attacker and defender stack to 0 if neccessary. This is a great
    // help for the functions calling stackFight (e.g. if a stack attacks
    // another stack and destroys it without winning the battle, it may take the
    // position of this stack)

    // First, the attacker...
    bool exists =
	std::find(d_stacklist->begin(), d_stacklist->end(), *attacker)
	!= d_stacklist->end();
    
    if (!exists)
    {
        (*attacker) = 0;
        if (attacker_active)
            d_stacklist->setActivestack(0);
    }

    // ...then the defender.
    exists = false;
    if (pd)
      exists = 
	std::find(pd->getStacklist()->begin(), pd->getStacklist()->end(), 
		  *defender) != pd->getStacklist()->end();
    else
        exists = true;
    if (!exists)
        (*defender) = 0;

    return fight.getResult();
}

/*
 *
 * To help factor in the advantage of hero experience/strength and 
 * ruin-monster strength as well as the stack strength, I think you'll 
 * find it'll be easier to calculate in terms of the odds of failure [than
 * the odds of success].  A new hero (minimum strength) with nothing in 
 * the stack to help him might have 10-20% odds of failure at a wimpy ruin.
 * The same novice hero facing a dragon in the ruin might have 50% odds of 
 * failure.  So a rule of thumb would be to start with a 25% chance of
 * failure.  The odds would be doubled by the worst monster and halved by 
 * the easiest.  I agree that a strength-9 hero with 8 in the stack should i
 * definitely be at 99%.  A reasonable formula might be:
 *
 * OddsOfFailure = BaseOdds * MonsterFactor * StackFactor * HeroFactor,
 *
 * with
 *        BaseOdds = 0.10
 * and
 *        MonsterFactor = 2, 1 or 0.5 depending on hard vs. easy
 * and
 *        StackFactor = (9 - SizeOfStack)/8,
 * and
 *        HeroFactor = (10-StrengthOfHero)/5.
 */
Fight::Result ruinfight (Stack **attacker, Stack **defender)
{
  Stack *loser;
  Fight::Result result;
  Uint32 hero_strength, monster_strength;
  hero_strength = (*attacker)->getFirstHero()->getStat(Army::STRENGTH, true);
  monster_strength = (*defender)->getStrongestArmy()->getStat(Army::STRENGTH, true);
  float base_factor = 0.28;
  float stack_factor = ((float)(MAX_STACK_SIZE + 1) - (*attacker)->size()) / (float)MAX_STACK_SIZE;
  float hero_factor = (10.0 - hero_strength) / 5.0;
  float monster_factor;
  if (monster_strength >= 8)
    monster_factor = 2.0;
  else if (monster_strength >= 6)
    monster_factor = 1.0;
  else
    monster_factor = 0.5;
  float fail = base_factor * monster_factor * stack_factor * hero_factor;

  if (rand() % 100 > (int)(fail * 100.0))
    {
      result = Fight::ATTACKER_WON;
      loser = *defender;
      for (Stack::iterator sit = loser->begin(); sit != loser->end();)
        {
          (*sit)->setHP (0);
          sit++;
        }
    }
  else
    {
      result = Fight::DEFENDER_WON;
      loser = *attacker;
      loser->getFirstHero()->setHP(0); /* only the hero dies */
    }
        
  return result;
}

Fight::Result Player::stackRuinFight (Stack **attacker, Stack **defender)
{
    Fight::Result result = Fight::DRAW;
    if (*defender == NULL)
      return Fight::ATTACKER_WON;
    debug("stackRuinFight: player = " << getName()<<" at position "
          <<(*defender)->getPos().x<<","<<(*defender)->getPos().y);

    ruinfight_started.emit(*attacker, *defender);
    result = ruinfight (attacker, defender);
    ruinfight_finished.emit(result);

    // cleanup
    
    // add a ruin fight item about the combat
    //Action_RuinFight* item = new Action_RuinFight();
    //item->fillData(*attacker, *defender, result);
    //addAction(item);
    /* FIXME: do we need an Action_RuinFight? */

    // get attacker and defender heroes and more...
    std::list<Stack*> attackers;
    attackers.push_back(*attacker);
    std::list<Stack*> defenders;
    defenders.push_back(*defender);

    cleanupAfterFight(attackers, defenders);

    return result;
}

bool Player::treachery (Stack *stack, Player *player, Vector <int> pos)
{
  return streachery.emit(stack, player, pos);
}

Reward* Player::stackSearchRuin(Stack* s, Ruin* r)
{
  Reward *retReward = NULL;
  debug("Player::stack_search_ruin");

  //throw out impossible actions
  if ((s->getPos().x != r->getPos().x) ||
      (s->getPos().y != r->getPos().y))
  {
    cerr <<  "Error: searching stack and ruin to be searched not on same position\n" ;
    exit(-1);
  }

  if (r->isSearched())
    return NULL;

  // start the action item
  Action_Ruin* item = new Action_Ruin();
  item->fillData(r, s);

  Stack* keeper = r->getOccupant();

  if (keeper)
  {
    stackRuinFight(&s, &keeper);

    // did the explorer not win?
    if (keeper && !keeper->empty())
    {
      item->setSearched(false);
      addAction(item);

      return NULL;
    }

    r->setOccupant(0);
    if (keeper)
      delete keeper;
  }

  if (r->hasSage())
  {
    History_FoundSage* history = new History_FoundSage();
    history->fillData(dynamic_cast<Hero *>(s->getFirstHero()));
    addHistory(history);
  }
  else
  {
    if (r->getReward() == NULL)
      r->populateWithRandomReward();
  }

  retReward = r->getReward();
  ssearchingRuin.emit(r, s, retReward);

  r->setSearched(true);

  // actualize the actionlist
  item->setSearched(true);
  addAction(item);

  supdatingStack.emit(0);
  return retReward;
}

int Player::doStackVisitTemple(Stack *s, Temple *t)
{
  // you have your stack blessed (+1 strength)
  int count = s->bless();

  svisitingTemple.emit(t, s);
  supdatingStack.emit(0);
  
  return count;
}

int Player::stackVisitTemple(Stack* s, Temple* t)
{
  debug("Player::stackVisitTemple");

  assert(s && t->getPos().x == s->getPos().x && t->getPos().y == s->getPos().y);

  Action_Temple* item = new Action_Temple();
  item->fillData(t, s);
  addAction(item);
  
  return doStackVisitTemple(s, t);
}

Quest* Player::stackGetQuest(Stack* s, Temple* t, bool except_raze)
{
  QuestsManager *qm = QuestsManager::getInstance();
  debug("Player::stackGetQuest")

    // bail out in case of senseless data
    if (!s || !t || (s->getPos().x != t->getPos().x) 
	|| (s->getPos().y != t->getPos().y))
      {
	cerr << "Stack tried to visit temple at wrong location\n";
	exit(-1);
      }

  std::vector<Quest*> quests = qm->getPlayerQuests(Playerlist::getActiveplayer());
  if (quests.size() > 0)
    return NULL;

  Quest* q=0;
  if (s->getFirstHero())
    {
      q = qm->createNewQuest
	(s->getFirstHero()->getId(), except_raze);
    }

  // couldn't assign a quest for various reasons
  if (!q)
    return 0;

  // Now fill the action item
  Action_Quest* action = new Action_Quest();
  action->fillData(q);
  addAction(action);

  // and record it for posterity
  History_HeroQuestStarted * history = new History_HeroQuestStarted();
  history->fillData(dynamic_cast<Hero *>(s->getFirstHero()));
  addHistory(history);
  return q;
}

float Player::stackFightAdvise(Stack* s, Vector<int> tile, 
                               bool intense_combat)
{
  float percent = 0.0;
        
  City* city = Citylist::getInstance()->getObjectAt(tile);
  Stack* target = Stacklist::getObjectAt(tile);
                
  if (!target && city)
    {
      vector<Stack*> def_in_city = Stacklist::defendersInCity(city);
      if (def_in_city.empty())
	return 100.0;
      target = def_in_city[0];
    }

  //what chance is there that stack will defeat defenders?
    
  for (unsigned int i = 0; i < 100; i++)
    {
      Fight fight(s, target, Fight::FOR_KICKS);
      fight.battle(intense_combat);
      if (fight.getResult() == Fight::ATTACKER_WON)
	percent += 1.0;
    }

  advice_asked.emit(percent);
  return percent;
}

void Player::adjustDiplomacyFromConqueringCity(City *city)
{
  Player *defender = city->getOwner();
  
  // See if this is the last city for that player, and alter the 
  // diplomatic scores.
  if (Citylist::getInstance()->countCities(defender) == 1)
  {
    if (defender->getDiplomaticRank() < getDiplomaticRank())
      deteriorateDiplomaticRelationship (2);
    else if (defender->getDiplomaticRank() > getDiplomaticRank())
      improveDiplomaticRelationship (2, defender);
  }
}

void Player::calculateLoot(Player *looted, Uint32 &added, Uint32 &subtracted)
{
  Player *defender = looted;
  int gold = 0;

  // if the attacked city isn't neutral, loot some gold
  if (defender != Playerlist::getInstance()->getNeutral())
  {
    Citylist *clist = Citylist::getInstance();
    int amt = (defender->getGold() / (2 * (clist->countCities (defender)+1)) * 2);
    // give (Enemy-Gold/(2Enemy-Cities)) to the attacker 
    // and then take away twice that from the defender.
    // the idea here is that some money is taken in the invasion
    // and other monies are lost forever
    // NOTE: +1 because the looted player just lost a city
    subtracted = amt;
    amt /= 2;
    added = amt;
    gold = amt;
  }

  return;
}

void Player::doConquerCity(City *city, Stack *stack)
{
  takeCityInPossession(city);
  
  History_CityWon *item = new History_CityWon();
  item->fillData(city);
  addHistory(item);
  if (stack->hasHero())
  {
    History_HeroCityWon *another = new History_HeroCityWon();
    Hero *hero = dynamic_cast<Hero *>(stack->getFirstHero());
    another->fillData(hero, city);
    addHistory(another);
  }
}

void Player::conquerCity(City *city, Stack *stack)
{
  
  Action_ConquerCity *action = new Action_ConquerCity();
  action->fillData(city, stack);
  addAction(action);

  Player *looted = city->getOwner();
  doConquerCity(city, stack);
  if (getType() != Player::NETWORKED)
    lootCity(city, looted);
}

void Player::lootCity(City *city, Player *looted)
{
  Uint32 added = 0;
  Uint32 subtracted = 0;
  calculateLoot(looted, added, subtracted);
  sinvadingCity.emit(city, added);
  doLootCity(looted, added, subtracted);
  Action_Loot *item = new Action_Loot();
  item->fillData(this, looted, added, subtracted);
  addAction(item);
  return;
}

void Player::doLootCity(Player *looted, Uint32 added, Uint32 subtracted)
{
  addGold(added);
  looted->withdrawGold(subtracted);
  return;
}

void Player::takeCityInPossession(City* c)
{
  c->conquer(this);

  //set the production to the cheapest armytype
  c->setActiveProductionSlot(-1);
  if (c->getArmytype(0) != -1)
    c->setActiveProductionSlot(0);

  supdatingCity.emit(c);
}

void Player::doCityOccupy(City *c)
{
  assert (c->getOwner() == this);
  
  soccupyingCity.emit(c, getActivestack());
  QuestsManager::getInstance()->cityOccupied(c, getActivestack());
}

void Player::cityOccupy(City* c)
{
  debug("cityOccupy");
  doCityOccupy(c);

  Action_Occupy* item = new Action_Occupy();
  item->fillData(c);
  addAction(item);
}

void Player::doCityPillage(City *c, int& gold, int* pillaged_army_type)
{
  gold = 0;
  if (pillaged_army_type)
    *pillaged_army_type = -1;
  
  // get rid of the most expensive army type and trade it in for 
  // half it's cost
  // it is presumed that the last army type is the most expensive

  if (c->getNoOfProductionBases() > 0)
    {
      unsigned int i;
      unsigned int max_cost = 0;
      int slot = -1;
      for (i = 0; i < c->getNoOfProductionBases(); i++)
	{
	  const ArmyProdBase *a = c->getProductionBase(i);
	  if (a != NULL)
	    {
	      if (a->getProductionCost() == 0)
		{
		  slot = i;
		  break;
		}
	      if (a->getProductionCost() > max_cost)
		{
		  max_cost = a->getProductionCost();
		  slot = i;
		}
	    }
	}
      if (slot > -1)
	{
	  const ArmyProdBase *a = c->getProductionBase(slot);
	  if (pillaged_army_type)
	    *pillaged_army_type = a->getTypeId();
	  if (a->getProductionCost() == 0)
	    gold += 1500;
	  else
	    gold += a->getProductionCost() / 2;
	  c->removeProductionBase(slot);
	}
  //*pillaged_army_type = 10;
  //gold = 300;
      addGold(gold);
      Stack *s = getActivestack();
      //printf ("%s emitting %p, %p, %d, %d\n", getName().c_str(), c, s, gold, *pillaged_army_type);
      spillagingCity.emit(c, s, gold, *pillaged_army_type);
      QuestsManager::getInstance()->cityPillaged(c, s, gold);
    }

  //takeCityInPossession(c);
}

void Player::cityPillage(City* c, int& gold, int* pillaged_army_type)
{
  debug("Player::cityPillage");
  
  Action_Pillage* item = new Action_Pillage();
  item->fillData(c);
  addAction(item);

  doCityPillage(c, gold, pillaged_army_type);
}

void Player::doCitySack(City* c, int& gold, std::list<Uint32> *sacked_types)
{
  gold = 0;
  //trade in all of the army types except for one
  //presumes that the army types are listed in order of expensiveness

  if (c->getNoOfProductionBases() > 1)
    {
      const ArmyProdBase *a;
      unsigned int i, max = 0;
      for (i = 0; i < c->getNoOfProductionBases(); i++)
	{
	  a = c->getProductionBase(i);
	  if (a)
	    max++;
	}

      i = c->getNoOfProductionBases() - 1;
      while (max > 1)
	{
	  a = c->getProductionBase(i);
	  if (a != NULL)
	    {
	      sacked_types->push_back(a->getTypeId());
	      if (a->getProductionCost() == 0)
		gold += 1500;
	      else
		gold += a->getProductionCost() / 2;
	      c->removeProductionBase(i);
	      max--;
	    }
	  i--;
	}
    }

  addGold(gold);
  Stack *s = getActivestack();
  ssackingCity.emit(c, s, gold, *sacked_types);
  QuestsManager::getInstance()->citySacked(c, s, gold);
  //takeCityInPossession(c);
}

void Player::citySack(City* c, int& gold, std::list<Uint32> *sacked_types)
{
  debug("Player::citySack");

  Action_Sack* item = new Action_Sack();
  item->fillData(c);
  addAction(item);

  doCitySack(c, gold, sacked_types);
}

void Player::doCityRaze(City *c)
{
  History_CityRazed* history = new History_CityRazed();
  history->fillData(c);
  addHistory(history);

  c->conquer(this);
  c->setBurnt(true);

  supdatingCity.emit(c);

  srazingCity.emit(c, getActivestack());
  QuestsManager::getInstance()->cityRazed(c, getActivestack());
}

void Player::cityRaze(City* c)
{
  debug("Player::cityRaze");

  Action_Raze* action = new Action_Raze();
  action->fillData(c);
  addAction(action);

  doCityRaze(c);
}

void Player::doCityBuyProduction(City* c, int slot, int type)
{
  const Armysetlist* al = Armysetlist::getInstance();
  Uint32 as = c->getOwner()->getArmyset();

  c->removeProductionBase(slot);
  c->addProductionBase(slot, new ArmyProdBase(*al->getArmy(as, type)));

  // and do the rest of the neccessary actions
  withdrawGold(al->getArmy(as, type)->getProductionCost());
}

bool Player::cityBuyProduction(City* c, int slot, int type)
{
  const Armysetlist* al = Armysetlist::getInstance();
  Uint32 as = c->getOwner()->getArmyset();

  // sort out unusual values (-1 is allowed and means "scrap production")
  if ((type <= -1) || (type >= (int)al->getSize(as)))
    return false;

  // return if we don't have enough money
  if ((type != -1) && ((int)al->getArmy(as, type)->getProductionCost() > d_gold))
    return false;

  // return if the city already has the production
  if (c->hasProductionBase(type, as))
    return false;

  // can't put it in that slot
  if (slot >= (int)c->getMaxNoOfProductionBases())
    return false;
  
  Action_Buy* item = new Action_Buy();
  item->fillData(c, slot, al->getArmy(as, type));
  addAction(item);

  doCityBuyProduction(c, slot, type);

  return true;
}

void Player::doCityChangeProduction(City* c, int slot)
{
  c->setActiveProductionSlot(slot);
}

bool Player::cityChangeProduction(City* c, int slot)
{
  doCityChangeProduction(c, slot);
  
  Action_Production* item = new Action_Production();
  item->fillData(c, slot);
  addAction(item);

  return true;
}

void Player::doGiveReward(Stack *s, Reward *reward)
{
  switch (reward->getType())
    {
    case Reward::GOLD:
      addGold(dynamic_cast<Reward_Gold*>(reward)->getGold());
      break;
    case Reward::ALLIES:
        {
          const ArmyProto *a = dynamic_cast<Reward_Allies*>(reward)->getArmy();

          Reward_Allies::addAllies(s->getOwner(), s->getPos(), a,
      			     dynamic_cast<Reward_Allies*>(reward)->getNoOfAllies());
  
        }
      break;
    case Reward::ITEM:
      static_cast<Hero*>(s->getFirstHero())->getBackpack()->addToBackpack
	(dynamic_cast<Reward_Item*>(reward)->getItem());
      break;
    case Reward::RUIN:
        {
          //assign the hidden ruin to this player
          Ruin *r = dynamic_cast<Reward_Ruin*>(reward)->getRuin();
          r->setHidden(true);
          r->setOwner(this);
	  r->deFog(this);
        }
      break;
    case Reward::MAP:
        {
          Reward_Map *map = dynamic_cast<Reward_Map*>(reward);
          d_fogmap->alterFog(map->getSightMap());
        }
      break;
    }
}

bool Player::giveReward(Stack *s, Reward *reward)
{
  debug("Player::give_reward");

  doGiveReward(s, reward);
  
  Action_Reward* item = new Action_Reward();
  item->fillData(s, reward);
  addAction(item);
  //FIXME: get rid of this reward now that we're done with it
  //but we need to show it still... (in the case of quest completions)

  return true;
}

bool Player::doStackDisband(Stack* s)
{
    getStacklist()->setActivestack(0);
    bool found = getStacklist()->deleteStack(s);
    supdatingStack.emit(0);
    return found;
}

bool Player::stackDisband(Stack* s)
{
    debug("Player::stackDisband(Stack*)")
    if (!s)
      s = getActivestack();
    
    Action_Disband* item = new Action_Disband();
    item->fillData(s);
    addAction(item);

    return doStackDisband(s);
}

void Player::doHeroDropItem(Hero *h, Item *i, Vector<int> pos)
{
  GameMap::getInstance()->getTile(pos)->getBackpack()->addToBackpack(i);
  h->getBackpack()->removeFromBackpack(i);
}

bool Player::heroDropItem(Hero *h, Item *i, Vector<int> pos)
{
  doHeroDropItem(h, i, pos);
  
  Action_Equip* item = new Action_Equip();
  item->fillData(h, i, Action_Equip::GROUND, pos);
  addAction(item);
  
  return true;
}

bool Player::heroDropAllItems(Hero *h, Vector<int> pos)
{
  while (h->getBackpack()->empty() == false)
    heroDropItem(h, h->getBackpack()->front(), pos);
  return true;
}

bool Player::doHeroDropAllItems(Hero *h, Vector<int> pos)
{
  while (h->getBackpack()->empty() == false)
    doHeroDropItem(h, h->getBackpack()->front(), pos);
  return true;
}

void Player::doHeroPickupItem(Hero *h, Item *i, Vector<int> pos)
{
  bool found = GameMap::getInstance()->getTile(pos)->getBackpack()->removeFromBackpack(i);
  if (found)
    h->getBackpack()->addToBackpack(i);
}

bool Player::heroPickupItem(Hero *h, Item *i, Vector<int> pos)
{
  doHeroPickupItem(h, i, pos);
  
  Action_Equip* item = new Action_Equip();
  item->fillData(h, i, Action_Equip::BACKPACK, pos);
  addAction(item);
  
  return true;
}

bool Player::heroPickupAllItems(Hero *h, Vector<int> pos)
{
  MapBackpack *backpack = GameMap::getInstance()->getTile(pos)->getBackpack();
  while (backpack->empty() == false)
    heroPickupItem(h, backpack->front(), pos);
  return true;
}

bool Player::heroCompletesQuest(Hero *h)
{
  // record it for posterity
  History_HeroQuestCompleted* item = new History_HeroQuestCompleted();
  item->fillData(h);
  addHistory(item);
  return true;
}

void Player::doResign()
{
  //disband all stacks
  getStacklist()->flClear();

  //raze all cities
  Citylist *cl = Citylist::getInstance();
  for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    {
      if ((*it)->getOwner() == this)
	{
	  (*it)->setBurnt(true);
	  History_CityRazed* history = new History_CityRazed();
	  history->fillData((*it));
	  addHistory(history);
	}
    }
  withdrawGold(getGold()); //empty the coffers!

  getStacklist()->setActivestack(0);
  supdatingStack.emit(0);
}

void Player::resign() 
{
  doResign();
  
  Action_Resign* item = new Action_Resign();
  item->fillData();
  addAction(item);
}

void Player::doSignpostChange(Signpost *s, std::string message)
{
  s->setName(message);
}

bool Player::signpostChange(Signpost *s, std::string message)
{
  if (!s)
    return false;
  
  doSignpostChange(s, message);
  
  Action_ModifySignpost* item = new Action_ModifySignpost();
  item->fillData(s, message);
  addAction(item);
  return true;
}

void Player::doCityRename(City *c, std::string name)
{
  c->setName(name);
}

bool Player::cityRename(City *c, std::string name)
{
  if (!c)
    return false;

  doCityRename(c, name);
  
  Action_RenameCity* item = new Action_RenameCity();
  item->fillData(c, name);
  addAction(item);
  return true;
}

void Player::doRename(std::string name)
{
  setName(name);
}

void Player::rename(std::string name)
{
  doRename(name);
  Action_RenamePlayer * item = new Action_RenamePlayer();
  item->fillData(name);
  addAction(item);
  return;
}

void Player::doVectorFromCity(City * c, Vector<int> dest)
{
  c->setVectoring(dest);
}

bool Player::vectorFromCity(City * c, Vector<int> dest)
{
  if (dest != Vector<int>(-1,-1))
    {
      std::list<City*> cities;
      cities = Citylist::getInstance()->getCitiesVectoringTo(dest);
      if (cities.size() >= MAX_CITIES_VECTORED_TO_ONE_CITY)
	return false;
    }
  doVectorFromCity(c, dest);
  
  Action_Vector* item = new Action_Vector();
  item->fillData(c, dest);
  addAction(item);
  return true;
}

bool Player::doChangeVectorDestination(Vector<int> src, Vector<int> dest,
				       std::list<City*> &vectored)
{
  //DEST can be a flag.
  //SRC can be a flag too.
  //Note: we don't actually have a way in the gui to change the vectoring 
  //from the planted standard (flag).
  bool retval = true;
  //sanity checks:
  //disallow changing vectoring from or to a city that isn't ours
  //disallow vectoring to something that isn't our city or our planted 
  //standard.
  Citylist *cl = Citylist::getInstance();
  City *src_city = cl->getObjectAt(src);
  if (src_city == NULL)
    {
      //maybe it's a flag we're changing the vector destination from.
      if (GameMap::getInstance()->findPlantedStandard(this) != src)
	return false;
    }
  else
    {
      if (src_city->getOwner() != this)
	return false;
    }
  City *dest_city = cl->getObjectAt(dest);
  if (dest_city == NULL)
    {
      if (GameMap::getInstance()->findPlantedStandard(this) != dest)
	return false;
    }
  else
    {
      if (dest_city->getOwner() != this)
	return false;
    }

  //check to see if the destination has enough room to accept all of the
  //cities we want to send to it.
  std::list<City*> sources = cl->getCitiesVectoringTo(src);
  std::list<City*> alreadyvectored = cl->getCitiesVectoringTo(dest);

  if (alreadyvectored.size() + sources.size() > MAX_CITIES_VECTORED_TO_ONE_CITY)
    return false;

  //okay, do the vectoring changes.
  std::list<City*>::iterator it = sources.begin();
  for (; it != sources.end(); it++)
    retval &= (*it)->changeVectorDestination(dest);
  vectored = sources;
  return retval;
}

bool Player::changeVectorDestination(Vector<int> src, Vector<int> dest)
{
  std::list<City*> vectored;
  bool retval = doChangeVectorDestination(src, dest, vectored);
  if (retval == false)
    return retval;

  std::list<City*>::iterator it = vectored.begin();
  for (; it != vectored.end(); it++)
    {
      Action_Vector* item = new Action_Vector();
      item->fillData((*it), dest);
      addAction(item);
    }
  return true;
}

bool Player::heroPlantStandard(Stack* s)
{
  debug("Player::heroPlantStandard(Stack*)");
  if (!s)
    s = getActivestack();
  
  for (Stack::iterator it = s->begin(); it != s->end(); it++)
  {
    if ((*it)->isHero())
    {
      Hero *hero = dynamic_cast<Hero*>((*it));
      Item *item = hero->getBackpack()->getPlantableItem(this);
      if (item)
        {
          //drop the item, and plant it
          doHeroPlantStandard(hero, item, s->getPos());
                  
          Action_Plant * i = new Action_Plant();
          i->fillData(hero, item);
          addAction(i);
          return true;
        }
    }
  }
  return true;
}

void Player::doHeroPlantStandard(Hero *hero, Item *item, Vector<int> pos)
{
  item->setPlanted(true);
  GameMap *gm = GameMap::getInstance();
  gm->getTile(pos)->getBackpack()->addToBackpack(item);
  hero->getBackpack()->removeFromBackpack(item);
}

void Player::getHeroes(const std::list<Stack*> stacks, std::vector<Uint32>& dst)
{
    std::list<Stack*>::const_iterator it;
    for (it = stacks.begin(); it != stacks.end(); it++)
        (*it)->getHeroes(dst);
}

double Player::removeDeadArmies(std::list<Stack*>& stacks,
                                std::vector<Uint32>& culprits)
{
    double total=0;
    Player *owner = NULL;
    if (stacks.empty() == 0)
    {
        owner = (*stacks.begin())->getOwner();
        debug("Owner = " << owner);
        if (owner)
            debug("Owner of the stacks: " << owner->getName()
                  << ", his stacklist = " << owner->getStacklist());
    }
    for (unsigned int i = 0; i < culprits.size(); i++)
        debug("Culprit: " << culprits[i]);

    std::list<Stack*>::iterator it;
    for (it = stacks.begin(); it != stacks.end(); )
    {
        debug("Stack: " << (*it))
        for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end();)
        {
            debug("Army: " << (*sit))
            if ((*sit)->getHP() <= 0)
            {
		//Tally up the triumphs
		if ((*sit)->getAwardable()) //hey a special died
		  d_triumphs->tallyTriumph((*sit)->getOwner(), 
					   Triumphs::TALLY_SPECIAL);
		else if ((*sit)->isHero() == false)
		  d_triumphs->tallyTriumph((*sit)->getOwner(), 
					   Triumphs::TALLY_NORMAL);
		if ((*sit)->getStat(Army::SHIP, false)) //hey it was on a boat
		  d_triumphs->tallyTriumph((*sit)->getOwner(), 
					   Triumphs::TALLY_SHIP);
                debug("Army: " << (*sit)->getName())
                debug("Army: " << (*sit)->getXpReward())
                if ((*sit)->isHero())
                {
		  d_triumphs->tallyTriumph((*sit)->getOwner(), 
					   Triumphs::TALLY_HERO);
		  Hero *hero = dynamic_cast<Hero*>((*sit));
		  Uint32 count = hero->getBackpack()->countPlantableItems();
		  for (Uint32 i = 0; i < count; i++)
		    d_triumphs->tallyTriumph((*sit)->getOwner(), 
					     Triumphs::TALLY_FLAG);

		  //one of our heroes died
		  //drop hero's stuff
		  Hero *h = static_cast<Hero *>(*sit);
		  //now record the details of the death
		  GameMap *gm = GameMap::getInstance();
		  Maptile *tile = gm->getTile((*it)->getPos());
		  if (tile->getBuilding() == Maptile::RUIN)
		    {
		      History_HeroKilledSearching* item;
		      item = new History_HeroKilledSearching();
		      item->fillData(h);
		      h->getOwner()->addHistory(item);
		      doHeroDropAllItems (h, (*it)->getPos());
		    }
		  else if (tile->getBuilding() == Maptile::CITY)
		    {
		      Citylist *clist = Citylist::getInstance();
		      City* c = clist->getObjectAt((*it)->getPos());
		      History_HeroKilledInCity* item;
		      item = new History_HeroKilledInCity();
		      item->fillData(h, c);
		      h->getOwner()->addHistory(item);
		      doHeroDropAllItems (h, (*it)->getPos());
		    }
		  else //somewhere else
		    {
		      History_HeroKilledInBattle* item;
		      item = new History_HeroKilledInBattle();
		      item->fillData(h);
		      h->getOwner()->addHistory(item);
		      doHeroDropAllItems (h, (*it)->getPos());
		    }
		}
		//Add the XP bonus to the total of the battle;
		total+=(*sit)->getXpReward();
		//tell the quest manager that someone died
		//(maybe it was a hero, or a target that's an army)
		QuestsManager::getInstance()->armyDied(*sit, culprits);
		// here we destroy the army, so we send
		// the signal containing the fight data
		debug("sending sdyingArmy!")
		  sdyingArmy.emit(*sit, culprits);
		sit = (*it)->flErase(sit);
		continue;
	    }

	    // heal this army to full hitpoints
	    (*sit)->heal((*sit)->getStat(Army::HP));

	    sit++;
	}

	debug("Is stack empty?")

	  if ((*it)->empty())
	    {
	      if (owner)
		{
		  debug("Removing this stack from the owner's stacklist");
		  bool found = owner->deleteStack(*it);
		  assert (found == true);
		}
	      else // there is no owner - like for the ruin's occupants
		debug("No owner for this stack - do stacklist too");

	      debug("Removing from the vector too (the vector had "
		    << stacks.size() << " elt)");
	      it = stacks.erase(it);
	    }
	  else
	    it++;
    }
    debug("after removeDead: size = " << stacks.size());
    return total;
}

void Player::doLevelArmy(Army *army, Army::Stat stat)
{
  army->gainLevel(stat);
}


void Player::updateArmyValues(std::list<Stack*>& stacks, double xp_sum)
{
  std::list<Stack*>::iterator it;
  double numberarmy = 0;

  for (it = stacks.begin(); it != stacks.end(); it++)
    numberarmy += (*it)->size();

  for (it = stacks.begin(); it != stacks.end(); )
    {
      debug("Stack: " << (*it))

	for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end();)
	  {
	    Army *army = *sit;
	    debug("Army: " << army)

	      // here we adds XP
	      army->gainXp((double)((xp_sum)/numberarmy));
	    debug("Army gets " << (double)((xp_sum)/numberarmy) << " XP")

	      // here we adds 1 to number of battles
	      army->setBattlesNumber(army->getBattlesNumber()+1);
	    debug("Army battles " <<  army->getBattlesNumber())

	      // medals only go to non-ally armies.
	      if ((*it)->hasHero() && army->isHero() == false && 
		  army->getAwardable() == false)
		{
		  if((army->getBattlesNumber())>10 && 
		     !(army->getMedalBonus(2)))
		    {
		      army->setMedalBonus(2,true);
		      // We must recalculate the XPValue of this unit since it 
		      // got a medal
		      army->setXpReward(army->getXpReward()+1);
		      // We get the medal bonus here
		      army->setStat(Army::STRENGTH, army->getStat(Army::STRENGTH, false)+1);
		      // Emit signal
		      snewMedalArmy.emit(army);
		    }

		  debug("Army hits " <<  army->getNumberHasHit())

		    // Only give medals if the unit has attacked often enough, else
		    // medals lose the flair of something special; a value of n 
		    // means roughly to hit an equally strong unit around n 
		    // times. (note: one hit! An attack can consist of up to 
		    // strength hits)
		    if((army->getNumberHasHit()>50) && !army->getMedalBonus(0))
		      {
			army->setMedalBonus(0,true);
			// We must recalculate the XPValue of this unit since it
			// got a medal
			army->setXpReward(army->getXpReward()+1);
			// We get the medal bonus here
			army->setStat(Army::STRENGTH, army->getStat(Army::STRENGTH, false)+1);
			// Emit signal
			snewMedalArmy.emit(army);
		      }

		  debug("army being hit " <<  army->getNumberHasBeenHit())

		    // Gives the medal for good defense. The more negative the 
		    // number the more blows the unit evaded. n means roughly 
		    // avoid n hits from an equally strong unit. Since we want 
		    // to punish the case of the unit hiding among many others, 
		    // we set this value quite high.
		    if((army->getNumberHasBeenHit() < -100) && !army->getMedalBonus(1))
		      {
			army->setMedalBonus(1,true);
			// We must recalculate the XPValue of this unit since it 
			// got a medal
			army->setXpReward(army->getXpReward()+1);
			// We get the medal bonus here
			army->setStat(Army::STRENGTH, army->getStat(Army::STRENGTH, false)+1);
			// Emit signal
			snewMedalArmy.emit(army);
		      }
		  debug("Army hits " <<  army->getNumberHasHit())

		    for(int i=0;i<3;i++)
		      {
			debug("MEDAL[" << i << "]==" << army->getMedalBonus(i))
		      }
		}

	    // We reset the hit values after the battle
	    army->setNumberHasHit(0);
	    army->setNumberHasBeenHit(0);

	    if (army->isHero() && getType() != Player::NETWORKED)
	      {
		while(army->canGainLevel())
		  {
		    // Units not associated to a player never raise levels.
		    if (army->getOwner() == 
			Playerlist::getInstance()->getNeutral())
		      break;

		    //Here this for is to check if army must raise 2 or more 
		    //levels per time depending on the XP and level itself

		    debug("ADVANCING LEVEL "<< "CANGAINLEVEL== " << army->canGainLevel());
		    army->getOwner()->levelArmy(army);
		  }
		debug("Army new XP=" << army->getXP())
	      }
	    sit++;
	  }
      it++;
    }
}

Hero* Player::doRecruitHero(HeroProto* herotemplate, City *city, int cost, int alliesCount, const ArmyProto *ally)
{
  Hero *newhero = new Hero(*herotemplate);
  newhero->setOwner(this);
  GameMap::getInstance()->addArmy(city, newhero);

  if (alliesCount > 0)
    {
      Reward_Allies::addAllies(this, city->getPos(), ally, alliesCount);
      hero_arrives_with_allies.emit(alliesCount);
    }

  if (cost == 0)
    {
      // Initially give the first hero the player's standard.
      std::string name = String::ucompose(_("%1 Standard"), getName());
      Item *battle_standard = new Item (name, true, this);
      battle_standard->addBonus(Item::ADD1STACK);
      newhero->getBackpack()->addToBackpack(battle_standard, 0);
    }
  withdrawGold(cost);
  supdatingStack.emit(0);
  return newhero;
}

void Player::recruitHero(HeroProto* heroproto, City *city, int cost, int alliesCount, const ArmyProto *ally)
{
  Action_RecruitHero *action = new Action_RecruitHero();
  action->fillData(heroproto, city, cost, alliesCount, ally);
  addAction(action);

  Hero *hero = doRecruitHero(heroproto, city, cost, alliesCount, ally);
  if (hero)
    {
      History_HeroEmerges *item = new History_HeroEmerges();
      item->fillData(hero, city);
      addHistory(item);
    }
}

void Player::doDeclareDiplomacy (DiplomaticState state, Player *player)
{
  Playerlist *pl = Playerlist::getInstance();
  if (pl->getNeutral() == player)
    return;
  if (player == this)
    return;
  if (state == d_diplomatic_state[player->getId()])
    return;
  d_diplomatic_state[player->getId()] = state;
}

void Player::declareDiplomacy (DiplomaticState state, Player *player)
{
  doDeclareDiplomacy(state, player);

  Action_DiplomacyState * item = new Action_DiplomacyState();
  item->fillData(player, state);
  addAction(item);

  // FIXME: update diplomatic scores? 
}

void Player::doProposeDiplomacy (DiplomaticProposal proposal, Player *player)
{
  if (GameScenarioOptions::s_diplomacy == false)
    return;
  Playerlist *pl = Playerlist::getInstance();
  if (pl->getNeutral() == player)
    return;
  if (player == this)
    return;
  if (proposal == d_diplomatic_proposal[player->getId()])
    return;
  if (proposal == PROPOSE_PEACE)
    {
      std::string s = _("Peace negotiated with ") + player->getName();
      if (getDiplomaticState(player) == AT_PEACE ||
	  getDiplomaticProposal(player) == PROPOSE_PEACE)
	schangingStatus.emit(s);
    }
  else if (proposal == PROPOSE_WAR)
    {
      std::string s = _("War declared with ") + player->getName();
      if (getDiplomaticState(player) == AT_WAR ||
	  getDiplomaticProposal(player) == PROPOSE_WAR)
      schangingStatus.emit(s);
    }
  d_diplomatic_proposal[player->getId()] = proposal;
}

void Player::proposeDiplomacy (DiplomaticProposal proposal, Player *player)
{
  doProposeDiplomacy(proposal, player);

  Action_DiplomacyProposal * item = new Action_DiplomacyProposal();
  item->fillData(player, proposal);
  addAction(item);

  // FIXME: update diplomatic scores? 
}

Player::DiplomaticState Player::negotiateDiplomacy (Player *player)
{
  DiplomaticState state = getDiplomaticState(player);
  DiplomaticProposal them = player->getDiplomaticProposal(this);
  DiplomaticProposal me = getDiplomaticProposal(player);
  DiplomaticProposal winning_proposal;

  /* Check if we both want the status quo. */
  if (me == NO_PROPOSAL && them == NO_PROPOSAL)
    return state;

  /* Okay, we both want a change from the status quo. */

  /* In the absense of a new proposal, the status quo is the proposal. */
  if (me == NO_PROPOSAL)
    {
      switch (state)
	{
	case AT_PEACE: me = PROPOSE_PEACE; break;
	case AT_WAR_IN_FIELD: me = PROPOSE_WAR_IN_FIELD; break;
	case AT_WAR: me = PROPOSE_WAR; break;
	}
    }
  if (them == NO_PROPOSAL)
    {
      switch (state)
	{
	case AT_PEACE: them = PROPOSE_PEACE; break;
	case AT_WAR_IN_FIELD: them = PROPOSE_WAR_IN_FIELD; break;
	case AT_WAR: them = PROPOSE_WAR; break;
	}
    }

  /* Check if we have agreement. */
  if (me == PROPOSE_PEACE && them == PROPOSE_PEACE)
    return AT_PEACE;
  else if (me == PROPOSE_WAR_IN_FIELD && them == PROPOSE_WAR_IN_FIELD)
    return AT_WAR_IN_FIELD;
  else if (me == PROPOSE_WAR && them == PROPOSE_WAR)
    return AT_WAR;

  /* Still we don't have an agreement.  
     Unfortunately the greater violence is the new diplomatic state. 
     Because there are two different proposals and the proposal with
     greater violence will be the new status quo, there can't 
     possibly be peace at this juncture.  */

  winning_proposal = me;
  if (them > me)
    winning_proposal = them;

  switch (winning_proposal)
    {
    case PROPOSE_WAR_IN_FIELD: return AT_WAR_IN_FIELD; break;
    case PROPOSE_WAR: return AT_WAR; break;
    default: return AT_PEACE; break; //impossible
    }

}

Player::DiplomaticState Player::getDiplomaticState (Player *player)
{
  if (player == Playerlist::getInstance()->getNeutral())
    return AT_WAR;
  if (player == this)
    return AT_PEACE;
  return d_diplomatic_state[player->getId()];
}

Player::DiplomaticProposal Player::getDiplomaticProposal (Player *player)
{
  if (player == Playerlist::getInstance()->getNeutral())
    return PROPOSE_WAR;
  if (player == this)
    return NO_PROPOSAL;
  return d_diplomatic_proposal[player->getId()];
}

Uint32 Player::getDiplomaticScore (Player *player)
{
  Playerlist *pl = Playerlist::getInstance();
  if (pl->getNeutral() == player)
    return 8;
  return d_diplomatic_score[player->getId()];
}

void Player::alterDiplomaticRelationshipScore (Player *player, int amount)
{
  if (amount > 0)
    {
      if (d_diplomatic_score[player->getId()] + amount > DIPLOMACY_MAX_SCORE)
	d_diplomatic_score[player->getId()] = DIPLOMACY_MAX_SCORE;
      else
	d_diplomatic_score[player->getId()] += amount;
    }
  else if (amount < 0)
    {
      if ((Uint32) (amount * -1) > d_diplomatic_score[player->getId()])
	d_diplomatic_score[player->getId()] = DIPLOMACY_MIN_SCORE;
      else
	d_diplomatic_score[player->getId()] += amount;
    }
}

void Player::improveDiplomaticRelationship (Player *player, Uint32 amount)
{
  Playerlist *pl = Playerlist::getInstance();
  if (pl->getNeutral() == player || player == this)
    return;

  alterDiplomaticRelationshipScore (player, amount);

  Action_DiplomacyScore* item = new Action_DiplomacyScore();
  item->fillData(player, amount);
  addAction(item);
}

void Player::deteriorateDiplomaticRelationship (Player *player, Uint32 amount)
{
  Playerlist *pl = Playerlist::getInstance();
  if (pl->getNeutral() == player || player == this)
    return;

  alterDiplomaticRelationshipScore (player, -amount);

  Action_DiplomacyScore* item = new Action_DiplomacyScore();
  item->fillData(player, -amount);
  addAction(item);
}

void Player::deteriorateDiplomaticRelationship (Uint32 amount)
{
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator it = pl->begin(); it != pl->end(); ++it)
    {
      if ((*it)->isDead())
	continue;
      if (pl->getNeutral() == (*it))
	continue;
      if (*it == this)
	continue;
      (*it)->deteriorateDiplomaticRelationship (this, amount);
    }
}

void Player::improveDiplomaticRelationship (Uint32 amount, Player *except)
{
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator it = pl->begin(); it != pl->end(); ++it)
    {
      if ((*it)->isDead())
	continue;
      if (pl->getNeutral() == (*it))
	continue;
      if (*it == this)
	continue;
      if (except && *it == except)
	continue;
      (*it)->improveDiplomaticRelationship (this, amount);
    }
}

void Player::deteriorateAlliesRelationship(Player *player, Uint32 amount,
					   Player::DiplomaticState state)
{
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator it = pl->begin(); it != pl->end(); ++it)
    {
      if ((*it)->isDead())
	continue;
      if (pl->getNeutral() == (*it))
	continue;
      if (*it == this)
	continue;
      if (getDiplomaticState(*it) == state)
	(*it)->deteriorateDiplomaticRelationship (player, amount);
    }
}

void Player::improveAlliesRelationship(Player *player, Uint32 amount,
				       Player::DiplomaticState state)
{
  Playerlist *pl = Playerlist::getInstance();
  for (Playerlist::iterator it = pl->begin(); it != pl->end(); ++it)
    {
      if ((*it)->isDead())
	continue;
      if (pl->getNeutral() == (*it))
	continue;
      if (*it == this)
	continue;
      if (player->getDiplomaticState(*it) == state)
	(*it)->improveDiplomaticRelationship (this, amount);
    }
}

void Player::AI_maybeBuyScout()
{
  bool hero_exists = false;
  for (Stacklist::iterator it = d_stacklist->begin(); 
       it != d_stacklist->end(); it++)

    if ((*it)->hasHero())
      hero_exists = true; 

  if (Citylist::getInstance()->countCities(this) == 1 && 
      hero_exists == false)
    {
      bool one_turn_army_exists = false;
      City *c = Citylist::getInstance()->getFirstCity(this);
      //do we already have something that can be produced in one turn?
      for (unsigned int i = 0; i < c->getMaxNoOfProductionBases(); i++)
	{
	  if (c->getArmytype(i) == -1)    // no production in this slot
	    continue;

	  const ArmyProdBase *proto = c->getProductionBase(i);
	  if (proto->getProduction() == 1)
	    {
	      one_turn_army_exists = true;
	      break;
	    }
	}
      if (one_turn_army_exists == false)
	{
	  const Armysetlist* al = Armysetlist::getInstance();
	  int free_slot = c->getFreeBasicSlot();
	  if (free_slot == -1)
	    free_slot = 0;
	  ArmyProto *scout = al->getScout(getArmyset());
	  cityBuyProduction(c, free_slot, scout->getTypeId());
	}
    }
}
bool Player::AI_maybePickUpItems(Stack *s, int max_dist, int max_mp, 
				 bool &picked_up, bool &stack_died)
{
  int min_dist = -1;
  bool stack_moved = false;
  Vector<int> item_tile(-1, -1);

  // do we not have a hero?
  if (s->hasHero() == false)
    return false;

  //ok, which bag of stuff is closest?
  std::vector<Vector<int> > tiles = GameMap::getInstance()->getItems();
  std::vector<Vector<int> >::iterator it = tiles.begin();
  for(; it != tiles.end(); it++)
    {
      Vector<int> tile = *it;
      //don't consider bags of stuff that are inside enemy cities
      City *c = Citylist::getInstance()->getObjectAt(tile);
      if (c)
	{
	  if (c->getOwner() != s->getOwner())
	    continue;
	}

      int distance = dist (tile, s->getPos());
      if (distance < min_dist || min_dist == -1)
	{
	  min_dist = distance;
	  item_tile = tile;
	}
    }

  //if no bags of stuff, or the bag is too far away
  if (min_dist == -1 || min_dist > max_dist)
    return false;

  //are we not standing on it?
  if (s->getPos() != item_tile)
    {
      //can we really reach it?
      Vector<int> old_dest(-1,-1);
      if (s->getPath()->size())
	old_dest = *s->getPath()->back();
      Uint32 mp = s->getPath()->calculate(s, item_tile);
      if ((int)mp > max_mp)
	{
	  //nope.  unreachable.  set in our old path.
	  if (old_dest != Vector<int>(-1,-1))
	    s->getPath()->calculate(s, old_dest);
	  return false;
	}
      stack_moved = stackMove(s);
      //maybe we died -- an enemy stack was guarding the bag.
      if (!d_stacklist->getActivestack())
	{
	  stack_died = true;
	  return true;
	}
      s = d_stacklist->getActivestack();
    }

  //are we standing on it now?
  if (s->getPos() == item_tile)
    {
      Hero *hero = static_cast<Hero*>(s->getFirstHero());
      if (hero)
	picked_up = heroPickupAllItems(hero, s->getPos());
    }

  return stack_moved;
}

bool Player::AI_maybeVisitTempleForBlessing(Stack *s, int dist, int max_mp, 
					    double percent_can_be_blessed, 
					    bool &blessed, bool &stack_died)
{
  bool stack_moved = false;
  Templelist *tl = Templelist::getInstance();

  Temple *temple = tl->getNearestVisibleAndUsefulTemple(s, percent_can_be_blessed, dist);
  if (!temple)
    return false;

  //if we're not there yet
  if (s->getPos() != temple->getPos())
    {
      //can we really reach it?
      Vector<int> old_dest(-1,-1);
      if (s->getPath()->size())
	old_dest = *s->getPath()->back();
      Uint32 mp = s->getPath()->calculate(s, temple->getPos());
      if ((int)mp > max_mp)
	{
	  //nope.  unreachable.  set in our old path.
	  if (old_dest != Vector<int>(-1,-1))
	    s->getPath()->calculate(s, old_dest);
	  return false;
	}
      stack_moved = stackMove(s);

      //maybe we died -- an enemy stack was guarding the temple
      if (!d_stacklist->getActivestack())
	{
	  stack_died = true;
	  return true;
	}
      s = d_stacklist->getActivestack();
    }

  int num_blessed = 0;
  //are we there yet?
  if (s->getPos() == temple->getPos())
    {
      num_blessed = stackVisitTemple(s, temple);
    }

  blessed = num_blessed > 0;
  return stack_moved;
}

bool Player::safeFromAttack(City *c, Uint32 safe_mp, Uint32 min_defenders)
{
  //if there isn't an enemy city nearby to the source
  // calculate mp to nearest enemy city
  //   needs to be less than 18 mp with a scout
  //does the source city contain at least 3 defenders?

  City *enemy_city = Citylist::getInstance()->getNearestEnemyCity(c->getPos());
  if (enemy_city)
    {
      Uint32 mp = Stack::scout (c->getOwner(), c->getPos(), 
				enemy_city->getPos());
      if ((int)mp <= 0 || mp >= safe_mp)
	{
	  if (c->countDefenders() >= min_defenders)
	    return true;
	}
    }

  return false;
}

bool Player::AI_maybeDisband(Stack *s, City *city, Uint32 min_defenders, 
			     int safe_mp, bool &stack_killed)
{
  //to prevent armies from piling up in far away places, 
  //we disband some periodically.
  if (s->size() != MAX_STACK_SIZE)
    return false;

  //is the city in danger from a city?
  if (safeFromAttack(city, safe_mp, 0) == false)
    return false;

  if (city->countDefenders() - s->size() >= min_defenders)
    {
      return stackDisband(s);
    }

  //okay, we need to disband part of our stack
  //find a square to travel to
  std::list<Vector<int> > diffs;
  diffs.push_back(Vector<int>(0, 1));
  diffs.push_back(Vector<int>(0, -1));
  diffs.push_back(Vector<int>(-1, -1));
  diffs.push_back(Vector<int>(-1, 1));
  diffs.push_back(Vector<int>(1, -1));
  diffs.push_back(Vector<int>(1, 1));
  diffs.push_back(Vector<int>(1, 0));
  diffs.push_back(Vector<int>(-1, 0));

  Vector<int> found = Vector<int>(-1, -1);
  for (std::list<Vector<int> >::iterator it = diffs.begin();
       it != diffs.end(); it++)
    {
      Vector<int> dest = s->getPos() + (*it);
      if (d_stacklist->getObjectAt(dest) == NULL)
	{
	  Uint32 mp = s->getPath()->calculate(s, dest);
	  if ((int)mp <= 0)
	    continue;
	  found = dest;
	  break;
	}
    }

  //no place to move to (strange)
  if (found == Vector<int>(-1, -1))
    return false;

  //before we move, ungroup the lucky ones not being disbanded
  unsigned int count = 0;
  s->group();
  for (Stack::reverse_iterator i = s->rbegin(); i != s->rend(); i++)
    {
      if (count == min_defenders)
	break;
      if ((*i)->isHero() == false)
	{
	  count++;
	  (*i)->setGrouped(false);
	}
    }

  stackSplit(s);
  stackMove(s);
  s = d_stacklist->getActivestack();

  if (d_stacklist->getActivestack() == 0) 
    {
      //maybe we got lucky and inadvertently attacked an enemy stack and lost.
      stack_killed = true;
      return false;
    }

  return stackDisband(s);
}

bool Player::AI_maybeVector(City *c, Uint32 safe_mp, Uint32 min_defenders,
			    City *target, City **vector_city)
{
  assert (c->getOwner() == this);
  if (vector_city)
    *vector_city = NULL;
  Citylist *cl = Citylist::getInstance();

  //is this city producing anything that we can vector?
  if (c->getActiveProductionSlot() == -1)
    return false;

  //is it safe to vector from this city?
  bool safe = safeFromAttack(c, 18, 3);

  if (!safe)
    return false;

  //get the nearest city to the enemy city that can accept vectored units
  City *near_city = cl->getNearestFriendlyVectorableCity(target->getPos());
  if (!near_city)
    return false;
  assert (near_city->getOwner() == this);

  //if it's us then it's easier to just walk.
  if (near_city == c)
    return false;

  //is that city already vectoring?
  if (near_city->getVectoring() != Vector<int>(-1, -1))
    return false;

  //can i just walk there faster?

  //find mp from source to target city
  const ArmyProdBase *proto = c->getActiveProductionBase();
  Uint32 mp_from_source_city = Stack::scout(c->getOwner(), c->getPos(),
					    target->getPos(), proto);

  //find mp from nearer vectorable city to target city
  Uint32 mp_from_near_city = Stack::scout(c->getOwner(), near_city->getPos(),
					  target->getPos(), proto);

  Uint32 max_moves_per_turn = proto->getMaxMoves();

  double turns_to_move_from_source_city = 
    (double)mp_from_source_city / (double)max_moves_per_turn;
  double turns_to_move_from_near_city = 
    (double)mp_from_near_city / (double)max_moves_per_turn;
  turns_to_move_from_near_city += 1.0; //add extra turn to vector

  //yes i can walk there faster, so don't vector
  if (turns_to_move_from_source_city <= turns_to_move_from_near_city)
    return false;

  //great.  now do the vectoring.
  c->changeVectorDestination(near_city->getPos());

  if (vector_city)
    *vector_city = near_city;
  return true;
}

void Player::AI_setupVectoring(Uint32 safe_mp, Uint32 min_defenders,
			       Uint32 mp_to_front)
{
  Citylist *cl = Citylist::getInstance();
  //turn off vectoring where it isn't safe anymore
  //turn off vectoring for destinations that are far away from the
  //nearest enemy city


  debug("setting up vectoring\n");
  for (Citylist::iterator cit = cl->begin(); cit != cl->end(); ++cit)
    {
      City *c = *cit;
      if (c->getOwner() != this || c->isBurnt())
	continue;
      Vector<int> dest = c->getVectoring();
      if (dest == Vector<int>(-1, -1))
	continue;
      if (safeFromAttack(c, safe_mp, min_defenders) == false)
	{
	  //City *target_city = Citylist::getInstance()->getObjectAt(dest);
	  //debug("stopping vectoring from " << c->getName() <<" to " << target_city->getName() << " because it's not safe to anymore!\n")
	  c->setVectoring(Vector<int>(-1,-1));
	  continue;
	}

      City *enemy_city = cl->getNearestEnemyCity(dest);
      if (!enemy_city)
	{
	  //City *target_city = Citylist::getInstance()->getObjectAt(dest);
	  //debug("stopping vectoring from " << c->getName() <<" to " << target_city->getName() << " because there aren't any more enemy cities!\n")
	  c->setVectoring(Vector<int>(-1,-1));
	  continue;
	}

      Uint32 mp = Stack::scout(this, dest, enemy_city->getPos(), NULL);
      if ((int)mp <= 0 || mp > mp_to_front)
	{

	  //City *target_city = Citylist::getInstance()->getObjectAt(dest);
	  //debug("stopping vectoring from " << c->getName() <<" to " << target_city->getName() << " because it's too far away from an enemy city!\n")
	  c->setVectoring(Vector<int>(-1,-1));
	  continue;
	}
    }

  for (Citylist::iterator cit = cl->begin(); cit != cl->end(); ++cit)
    {
      City *c = *cit;
      if (c->getOwner() != this || c->isBurnt())
	continue;
      City *enemy_city = cl->getNearestEnemyCity(c->getPos());
      if (!enemy_city)
	continue;
      City *vector_city = NULL;
      //if the city isn't already vectoring
      if (c->getVectoring() == Vector<int>(-1,-1))
	{
	  bool vectored = AI_maybeVector(c, safe_mp, min_defenders, enemy_city, 
					 &vector_city);
	  if (vectored)
	    debug("begin vectoring from " << c->getName() <<" to " << vector_city->getName() << "!\n");
	}
    }
}

const Army * Player::doCityProducesArmy(City *city)
{
  return city->armyArrives();
}

bool Player::cityProducesArmy(City *city)
{
  Action_Produce *item = new Action_Produce();
  const Army *army = doCityProducesArmy(city);
  if (army)
    {
      const ArmyProdBase *source_army;
      source_army = city->getProductionBaseBelongingTo(army);
      if (city->getVectoring() == Vector<int>(-1, -1))
	item->fillData(source_army, city, false);
      else
	item->fillData(source_army, city, true);
      addAction(item);
    }
  return true;
}

Army* Player::doVectoredUnitArrives(VectoredUnit *unit)
{
  Army *army = unit->armyArrives();
  return army;
}

bool Player::vectoredUnitArrives(VectoredUnit *unit)
{
  Action_ProduceVectored *item = new Action_ProduceVectored();
  item->fillData(unit->getArmy(), unit->getDestination(), unit->getPos());
  addAction(item);
  Army *army = doVectoredUnitArrives(unit);
  if (!army)
    {
      printf("whooops... this vectored unit failed to show up.\n");
      exit (1);
    }

  return true;
}

std::list<Action_Produce *> Player::getUnitsProducedThisTurn()
{
  std::list<Action_Produce *> actions;
  std::list<Action *>::reverse_iterator it = d_actions.rbegin();
  for (; it != d_actions.rend(); it++)
    {
      if ((*it)->getType() == Action::PRODUCE_UNIT)
	actions.push_back(dynamic_cast<Action_Produce*>(*it));
      else if ((*it)->getType() == Action::INIT_TURN)
	break;
    }
  return actions;
}
std::list<Action *> Player::getReportableActions()
{
  std::list<Action *> actions;
  std::list<Action *>::iterator it = d_actions.begin();
  for (; it != d_actions.end(); it++)
    {
      if ((*it)->getType() == Action::PRODUCE_UNIT ||
	  (*it)->getType() == Action::PRODUCE_VECTORED_UNIT ||
	  (*it)->getType() == Action::CITY_DESTITUTE)
	actions.push_back(*it);
    }
  return actions;
}

void Player::cityTooPoorToProduce(City *city, int slot)
{
  cityChangeProduction(city, -1);
  const ArmyProdBase *a = city->getProductionBase(slot);
  Action_CityTooPoorToProduce *action = new Action_CityTooPoorToProduce();
  action->fillData(city, a);
  addAction(action);
}

void Player::pruneActionlist()
{
  pruneActionlist(d_actions);
}

void Player::pruneCityProductions(std::list<Action*> actions)
{
  //remove duplicate city production actions

  //enumerate the ones we want
  std::list<Action_Production*> keepers;
  std::list<Action*>::reverse_iterator ait;
  for (ait = actions.rbegin(); ait != actions.rend(); ait++)
    {
      if ((*ait)->getType() != Action::CITY_PROD)
	continue;
      //if this city isn't already in the keepers list, then add it.

      Action_Production *action = static_cast<Action_Production*>(*ait);
      bool found = false;
      std::list<Action_Production*>::const_iterator it;
      for (it = keepers.begin(); it != keepers.end(); it++)
	{
	  if (action->getCityId() == (*it)->getCityId())
	    {
	      found = true;
	      break;
	    }
	}
      if (found == false)
	keepers.push_back(action);

    }

  //now delete all city production events that aren't in keepers
  int total = 0;
  std::list<Action*>::iterator bit;
  for (bit = actions.begin(); bit != actions.end(); bit++)
    {
      if ((*bit)->getType() != Action::CITY_PROD)
	continue;
      if (find (keepers.begin(), keepers.end(), (*bit)) == keepers.end())
	{
	  total++;
	  actions.erase (bit);
	  bit = actions.begin();
	  continue;
	}
    }
  //if (total)
  //printf ("pruned %d city production actions.\n", total);
}

void Player::pruneCityVectorings(std::list<Action*> actions)
{
  //remove duplicate city vectoring actions

  //enumerate the ones we want
  std::list<Action_Vector*> keepers;
  std::list<Action*>::reverse_iterator ait;
  for (ait = actions.rbegin(); ait != actions.rend(); ait++)
    {
      if ((*ait)->getType() != Action::CITY_VECTOR)
	continue;
      //if this city isn't already in the keepers list, then add it.

      Action_Vector *action = static_cast<Action_Vector *>(*ait);
      bool found = false;
      std::list<Action_Vector*>::const_iterator it;
      for (it = keepers.begin(); it != keepers.end(); it++)
	{
	  if (action->getCityId() == (*it)->getCityId())
	    {
	      found = true;
	      break;
	    }
	}
      if (found == false)
	keepers.push_back(action);

    }

  //now delete all city vector events that aren't in keepers
  int total = 0;
  std::list<Action*>::iterator bit;
  for (bit = actions.begin(); bit != actions.end(); bit++)
    {
      if ((*bit)->getType() != Action::CITY_VECTOR)
	continue;
      if (find (keepers.begin(), keepers.end(), (*bit)) == keepers.end())
	{
	  total++;
	  actions.erase (bit);
	  bit = actions.begin();
	  continue;
	}
    }
  //if (total)
  //printf ("pruned %d city vector actions.\n", total);
}

void Player::pruneActionlist(std::list<Action*> actions)
{
  pruneCityProductions(actions);
  pruneCityVectorings(actions);

}

std::string Player::playerTypeToString(const Player::Type type)
{
  switch (type)
    {
    case Player::HUMAN:
      return "Player::HUMAN";
    case Player::AI_FAST:
      return "Player::AI_FAST";
    case Player::AI_DUMMY:
      return "Player::AI_DUMMY";
    case Player::AI_SMART:
      return "Player::AI_SMART";
    case Player::NETWORKED:
      return "Player::NETWORKED";
    }
  return "Player::HUMAN";
}

Player::Type Player::playerTypeFromString(const std::string str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Player::Type(atoi(str.c_str()));
  if (str == "Player::HUMAN")
    return Player::HUMAN;
  else if (str == "Player::AI_FAST")
    return Player::AI_FAST;
  else if (str == "Player::AI_DUMMY")
    return Player::AI_DUMMY;
  else if (str == "Player::AI_SMART")
    return Player::AI_SMART;
  else if (str == "Player::NETWORKED")
    return Player::NETWORKED;
  return Player::HUMAN;
}

bool Player::hasAlreadyInitializedTurn() const
{
  for (list<Action*>::const_iterator it = d_actions.begin();
       it != d_actions.end(); it++)
    if ((*it)->getType() == Action::INIT_TURN)
      return true;
  return false;
}

bool Player::hasAlreadyEndedTurn() const
{
  for (list<Action*>::const_iterator it = d_actions.begin();
       it != d_actions.end(); it++)
    if ((*it)->getType() == Action::END_TURN)
      return true;
  return false;
}

std::list<History*> Player::getHistoryForThisTurn() const
{
  std::list<History*> history;
  for (list<History*>::const_reverse_iterator it = d_history.rbegin();
       it != d_history.rend(); it++)
    {
      history.push_front(*it);
      if ((*it)->getType() == History::START_TURN)
	break;
    }
  return history;
}

Uint32 Player::countEndTurnHistoryEntries() const
{
  Uint32 count = 0;
  for (list<History*>::const_iterator it = d_history.begin();
       it != d_history.end(); it++)
    {
      if ((*it)->getType() == History::END_TURN)
	count++;
    }
  return count;
}

void Player::loadPbmGame() 
{
  for (list<Action*>::const_iterator it = d_actions.begin();
       it != d_actions.end(); it++)
    {
      NetworkAction *copy = new NetworkAction(*it, this);
      acting.emit(copy);
    }
  std::list<History*> history = getHistoryForThisTurn();
  for (list<History*>::const_iterator it = history.begin();
       it != history.end(); it++)
    {
      NetworkHistory *copy = new NetworkHistory(*it, this);
      history_written.emit(copy);
    }
}

void Player::saveNetworkActions(XML_Helper *helper)
{
  for (list<Action*>::const_iterator it = d_actions.begin();
       it != d_actions.end(); it++)
    {
      NetworkAction *copy = new NetworkAction(*it, this);
      copy->save(helper);
    }
}
	
bool Player::conqueredCity(City *c)
{
  if (!c)
    return false;
  for (list<History*>::const_iterator it = d_history.begin();
       it != d_history.end(); it++)
    {
      if ((*it)->getType() == History::CITY_WON)
	{
	  History_CityWon *event = dynamic_cast<History_CityWon*>(*it);
	  if (event->getCityId() == c->getId())
	    return true;
	}
    }    
  return false;
}

std::list<Vector<int> > Player::getStackTrack(Stack *s)
{
  std::list<Vector<int> > points;
  Vector<int> delta = Vector<int>(0,0);
  for (list<Action*>::const_iterator it = d_actions.begin();
       it != d_actions.end(); it++)
    {
      if ((*it)->getType() == Action::STACK_MOVE)
	{
	  Action_Move *action = dynamic_cast<Action_Move*>(*it);
	  if (action->getStackId() == s->getId())
	    {
	      if (points.size() == 0)
		delta = action->getPositionDelta();
	      points.push_back(action->getEndingPosition());
	    }
	}
    }
  if (points.size() >= 1)
    {
      Vector<int> pos = points.front() + delta;
      if (pos != points.front())
	points.push_front(pos);
    }
  return points;
}
	
std::list<History *>Player::getHistoryForHeroId(Uint32 id)
{
  std::string hero_name = "";
  std::list<History*> events;
  std::list<History*>::const_iterator pit;
  for (pit = d_history.begin(); pit != d_history.end(); pit++)
    {
      switch ((*pit)->getType())
	{
	case History::HERO_EMERGES:
	    {
	      History_HeroEmerges *event;
	      event = dynamic_cast<History_HeroEmerges *>(*pit);
	      if (event->getHeroId() == id)
		{
		  hero_name = event->getHeroName();
		  events.push_back(*pit);
		}
	      break;
	    }
	case History::FOUND_SAGE:
	    {
	      History_FoundSage *event;
	      event = dynamic_cast<History_FoundSage*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_QUEST_STARTED:
	    {
	      History_HeroQuestStarted *event;
	      event = dynamic_cast<History_HeroQuestStarted*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_QUEST_COMPLETED:
	    {
	      History_HeroQuestCompleted *event;
	      event = dynamic_cast<History_HeroQuestCompleted*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_KILLED_IN_CITY:
	    {
	      History_HeroKilledInCity *event;
	      event = dynamic_cast<History_HeroKilledInCity*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_KILLED_IN_BATTLE:
	    {
	      History_HeroKilledInBattle *event;
	      event = dynamic_cast<History_HeroKilledInBattle*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_KILLED_SEARCHING:
	    {
	      History_HeroKilledSearching*event;
	      event = dynamic_cast<History_HeroKilledSearching*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_CITY_WON:
	    {
	      History_HeroCityWon *event;
	      event = dynamic_cast<History_HeroCityWon*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	case History::HERO_FINDS_ALLIES:
	    {
	      History_HeroFindsAllies *event;
	      event = dynamic_cast<History_HeroFindsAllies*>(*pit);
	      if (event->getHeroName() == hero_name)
		events.push_back(*pit);
	      break;
	    }
	default:
	  break;
	}
    }
  return events;
}

void Player::setSurrendered(bool surr)
{
  surrendered = surr;
}
// End of file
