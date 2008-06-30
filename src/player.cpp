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
#include "Configuration.h"
#include "GameScenarioOptions.h"
#include "action.h"
#include "history.h"
#include "AI_Analysis.h"
#include "AI_Allocation.h"
#include "FogMap.h"
#include "QuestsManager.h"
#include "GameMap.h"
#include "signpost.h"
#include "ucompose.hpp"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
#define debug(x)

SDL_Color Player::get_color_for_no(int player_no)
{
    SDL_Color color;
    color.r = color.b = color.g = color.unused = 0;
    switch (player_no % MAX_PLAYERS)
    {
    case 0: color.r = 252; color.b = 252; color.g = 252; break;
    //case 1: color.r = 80; color.b = 28; color.g = 172; break;
    case 1: color.r = 80; color.b = 28; color.g = 193; break;
    case 2: color.r = 252; color.b = 32; color.g = 236; break;
    case 3: color.r = 92; color.b = 208; color.g = 92; break;
    case 4: color.r = 252; color.b = 0; color.g = 160;break;
    case 5: color.r = 44; color.b = 252; color.g = 184; break;
    case 6: color.r = 196; color.b = 0; color.g = 28; break;
    case 7: color.r = color.g = color.b = 0; break;
    }
    
    return color;
}

SDL_Color Player::get_color_for_neutral()
{
    SDL_Color color;
    color.r = color.g = color.b = 204; color.unused = 0;
    return color;
}


// signal
sigc::signal<void, Player::Type> sendingTurn;

Player::Player(string name, Uint32 armyset, SDL_Color color, int width,
	       int height, Type type, int player_no)
    :d_color(color), d_name(name), d_armyset(armyset), d_gold(1000),
    d_dead(false), d_immortal(false), d_type(type), d_upkeep(0), d_income(0)
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

    memset(d_triumph, 0, sizeof(d_triumph));

    for (unsigned int i = 0 ; i < MAX_PLAYERS; i++)
    {
      d_diplomatic_state[i] = AT_PEACE;
      d_diplomatic_proposal[i] = NO_PROPOSAL;
      d_diplomatic_score[i] = DIPLOMACY_STARTING_SCORE;
    }
    d_diplomatic_rank = 0;
    d_diplomatic_title = std::string("");
}

Player::Player(const Player& player)
    :d_color(player.d_color), d_name(player.d_name), d_armyset(player.d_armyset),
    d_gold(player.d_gold), d_dead(player.d_dead), d_immortal(player.d_immortal),
    d_type(player.d_type), d_id(player.d_id), 
    d_fight_order(player.d_fight_order), d_upkeep(player.d_upkeep), 
    d_income(player.d_income)
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
}

Player::Player(XML_Helper* helper)
    :d_stacklist(0), d_fogmap(0)
{
    helper->getData(d_id, "id");
    helper->getData(d_name, "name");
    helper->getData(d_gold, "gold");
    helper->getData(d_dead, "dead");
    helper->getData(d_immortal, "immortal");
    helper->getData(d_type, "type");
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

    //last but not least, register the load function for actionlist
    helper->registerTag("action", sigc::mem_fun(this, &Player::load));
    helper->registerTag("history", sigc::mem_fun(this, &Player::load));
    helper->registerTag("stacklist", sigc::mem_fun(this, &Player::load));
    helper->registerTag("fogmap", sigc::mem_fun(this, &Player::load));
    helper->registerTag("triumphs", sigc::mem_fun(this, &Player::load));

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
  d_history.push_back(item);
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
    schangingStatus.emit();
}

void Player::withdrawGold(int gold)
{
    d_gold -= gold;
    if (d_gold < 0)
      d_gold = 0; /* bankrupt.  should we start turning off city production? */
    schangingStatus.emit();
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
#if 0  // FIXME: temp. leak to avoid double-delete
      delete (*it);
#endif
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
    if (d_immortal)
        // ignore it
        return;
    
                        
    History_PlayerVanquished* item;
    item = new History_PlayerVanquished();
    d_history.push_back(item);

    d_dead = true;
    d_stacklist->flClear();

    // Since in some cases the player can be killed rather innocently
    // (using reactions), we also need to clear the player's traces in the
    // single cities
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
        if ((*it).getOwner() == this)
            (*it).setOwner(Playerlist::getInstance()->getNeutral());

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
    retval &= helper->saveData("type", d_type);
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

#if 0
    //save the actionlist
    for (list<Action*>::const_iterator it = d_actions.begin();
            it != d_actions.end(); it++)
        retval &= (*it)->save(helper);
#endif
    
    //save the pasteventlist
    for (list<History*>::const_iterator it = d_history.begin();
            it != d_history.end(); it++)
        retval &= (*it)->save(helper);

    retval &= d_stacklist->save(helper);
    retval &= d_fogmap->save(helper);

    //save the triumphs
	    
    helper->openTag("triumphs");
    for (unsigned int i = 0; i < 5; i++)
      {
	std::stringstream tally;
	for (unsigned int j = 0; j < MAX_PLAYERS; j++)
	    tally << d_triumph[j][i] << " ";
	switch (TriumphType(i))
	  {
	  case TALLY_HERO:
	    retval &= helper->saveData("hero", tally.str());
	    break;
	  case TALLY_NORMAL:
	    retval &= helper->saveData("normal", tally.str());
	    break;
	  case TALLY_SPECIAL:
	    retval &= helper->saveData("special", tally.str());
	    break;
	  case TALLY_SHIP:
	    retval &= helper->saveData("ship", tally.str());
	    break;
	  case TALLY_FLAG:
	    retval &= helper->saveData("flag", tally.str());
	    break;
	  }
      }
    helper->closeTag();

    return retval;
}

Player* Player::loadPlayer(XML_Helper* helper)
{
    Type type;
    int t;
    
    helper->getData(t, "type");
    type = static_cast<Type>(t);

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
#if 0
    if (tag == "action")
    {
        Action* action;
        action = Action::handle_load(helper);
        d_actions.push_back(action);
    }
#endif
    if (tag == "history")
    {
        History* history;
        history = History::handle_load(helper);
        d_history.push_back(history);
    }

    if (tag == "stacklist")
        d_stacklist = new Stacklist(helper);

    if (tag == "fogmap")
        d_fogmap = new FogMap(helper);

    if (tag == "triumphs")
      {
	for (unsigned int i = 0; i < 5; i++)
	  {
	    std::string tally;
	    std::stringstream stally;
	    Uint32 val;
	    switch (TriumphType(i))
	      {
	      case TALLY_HERO:
		helper->getData(tally, "hero");
		break;
	      case TALLY_NORMAL:
		helper->getData(tally, "normal");
		break;
	      case TALLY_SPECIAL:
		helper->getData(tally, "special");
		break;
	      case TALLY_SHIP:
		helper->getData(tally, "ship");
		break;
	      case TALLY_FLAG:
		helper->getData(tally, "flag");
		break;
	      }
	    stally.str(tally);
	    for (unsigned int j = 0; j < MAX_PLAYERS; j++)
	      {
		stally >> val;
		d_triumph[j][i] = val;
	      }
	  }
      }

    return true;
}

void Player::addAction(Action *action)
{
  action->setPlayer(getId());
  
  // FIXME
  if (!action_done.empty())
    action_done.emit(action);
#if 0
  else
    delete action;
#endif
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
	if ((*i).getOwner() == this)
	  d_income += (*i).getGold();
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
    {
        return 0;
    }

    bool all_ungrouped = true;    //the whole stack would be split
    for (Stack::iterator it = s->begin(); it != s->end(); it++)
    {
        if ((*it)->isGrouped())
        {
            all_ungrouped = false;
        }
    }
    if (all_ungrouped)
    {
        return 0;
    }

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
    // (by default, only the joining stacks armies will continue to move). Note
    // that the computer player silently ignores the grouped value and always
    // moves all armies in a stack.
    for (Stack::iterator it = receiver->begin(); it != receiver->end(); it++)
        (*it)->setGrouped(grouped);

    for (Stack::iterator it = joining->begin(); it != joining->end(); it++)
    {
        receiver->push_back(*it);
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
	Stack *already_there = Stacklist::getAmbiguity(joining);
	stackJoin(joining, already_there,  false);
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
    while (s->getPath()->size() > 1 && stackMoveOneStep(s))
    {
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
	    SDL_Delay(Configuration::s_displaySpeedDelay);
    
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
        SDL_Delay(Configuration::s_displaySpeedDelay);
    
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

    return new MoveResult(true);
}

bool Player::stackMoveOneStep(Stack* s)
{
  if (!s)
    return false;

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
  float base_factor = 0.33;
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
    cerr <<_("Error: searching stack and ruin to be searched not on same position\n");
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
    d_history.push_back(history);
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
	cerr <<_("Stack tried to visit temple at wrong location\n");
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
  d_history.push_back(history);
  return q;
}

float Player::stackFightAdvise(Stack* s, Vector<int> tile, 
                               bool intense_combat)
{
  float percent = 0.0;
        
  City* city = Citylist::getInstance()->getObjectAt(tile);
  Stack* target = Stacklist::getObjectAt(tile);
                
  if (!target && city)
    target = new Stack(city->getOwner(), tile);

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

int Player::lootCity(City *city)
{
  Player *defender = city->getOwner();
  int gold = 0;

  // if the attacked city isn't neutral, loot some gold
  if (defender != Playerlist::getInstance()->getNeutral())
  {
    Citylist *clist = Citylist::getInstance();
    int amt = (defender->getGold() / (2 * clist->countCities (defender)) * 2);
    // give (Enemy-Gold/(2Enemy-Cities)) to the attacker 
    // and then take away twice that from the defender.
    // the idea here is that some money is taken in the invasion
    // and other monies are lost forever
    defender->withdrawGold (amt);
    amt /= 2;
    addGold (amt);
    gold = amt;
  }

  return gold;
}

void Player::doConquerCity(City *city, Stack *stack)
{
  int gold = lootCity(city);
  sinvadingCity.emit(city, gold);
  
  History_CityWon *item = new History_CityWon();
  item->fillData(city);
  d_history.push_back(item);
  if (stack->hasHero())
  {
    History_HeroCityWon *another = new History_HeroCityWon();
    Hero *hero = dynamic_cast<Hero *>(stack->getFirstHero());
    another->fillData(hero, city);
    d_history.push_back(another);
  }
}

void Player::conquerCity(City *city, Stack *stack)
{
  
  Action_ConquerCity *action = new Action_ConquerCity();
  action->fillData(city, stack);
  addAction(action);
                
  doConquerCity(city, stack);
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
  takeCityInPossession(c);
  
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

void Player::doCityPillage(City *c, int& gold, int& pillaged_army_type)
{
  gold = 0;
  pillaged_army_type = -1;
  
  // get rid of the most expensive army type and trade it in for 
  // half it's cost
  // it is presumed that the last army type is the most expensive

  if (c->getNoOfProductionBases() > 0)
    {
      int i;
      unsigned int max_cost = 0;
      int slot = -1;
      for (i = 0; i < c->getNoOfProductionBases(); i++)
	{
	  const Army *a = c->getProductionBase(i);
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
	  const Army *a = c->getProductionBase(slot);
	  pillaged_army_type = a->getType();
	  if (a->getProductionCost() == 0)
	    gold += 1500;
	  else
	    gold += a->getProductionCost() / 2;
	  c->removeProductionBase(slot);
	}
    }

  addGold(gold);
  std::list<Uint32> sacked_types;
  sacked_types.push_back (pillaged_army_type);
  Stack *s = getActivestack();
  spillagingCity.emit(c, s, gold, sacked_types);
  QuestsManager::getInstance()->cityPillaged(c, s, gold);
  takeCityInPossession(c);
}

void Player::cityPillage(City* c, int& gold, int& pillaged_army_type)
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
      const Army *a;
      int i, max = 0;
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
	      sacked_types->push_back(a->getType());
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
  takeCityInPossession(c);
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
  d_history.push_back(history);

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
  c->addProductionBase(slot, new Army(*al->getArmy(as, type)));

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
  if (slot >= c->getMaxNoOfProductionBases())
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
          const Army *a = dynamic_cast<Reward_Allies*>(reward)->getArmy();

          Reward_Allies::addAllies(s->getOwner(), s->getPos(), a,
      			     dynamic_cast<Reward_Allies*>(reward)->getNoOfAllies());
  
        }
      break;
    case Reward::ITEM:
      static_cast<Hero*>(s->getFirstHero())->addToBackpack(
      						     dynamic_cast<Reward_Item*>(reward)->getItem());
      break;
    case Reward::RUIN:
        {
          //assign the hidden ruin to this player
          Ruin *r = dynamic_cast<Reward_Ruin*>(reward)->getRuin();
          r->setHidden(true);
          r->setOwner(this);
        }
      break;
    case Reward::MAP:
        {
          Reward_Map *map = dynamic_cast<Reward_Map*>(reward);
          d_fogmap->alterFogRectangle(map->getPos(), 
      				map->getHeight(), map->getWidth(), 
      				FogMap::OPEN);
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

  return true;
}

void Player::doStackDisband(Stack* s)
{
    getStacklist()->setActivestack(0);
    getStacklist()->deleteStack(s);
    supdatingStack.emit(0);
}

bool Player::stackDisband(Stack* s)
{
    debug("Player::stackDisband(Stack*)")
    if (!s)
      s = getActivestack();
    
    Action_Disband* item = new Action_Disband();
    item->fillData(s);
    addAction(item);

    doStackDisband(s);
    
    return true;
}

void Player::doHeroDropItem(Hero *h, Item *i, Vector<int> pos)
{
  GameMap::getInstance()->getTile(pos)->addItem(i);
  h->removeFromBackpack(i);
}

bool Player::heroDropItem(Hero *h, Item *i, Vector<int> pos)
{
  doHeroDropItem(h, i, pos);
  
  Action_Equip* item = new Action_Equip();
  item->fillData(h, i, Action_Equip::GROUND);
  addAction(item);
  
  return true;
}

bool Player::heroDropAllItems(Hero *h, Vector<int> pos)
{
  std::list<Item*> backpack = h->getBackpack();
  for (std::list<Item*>::iterator i = backpack.begin(), end = backpack.end();
       i != end; ++i)
    heroDropItem(h, *i, pos);
  return true;
}

void Player::doHeroPickupItem(Hero *h, Item *i, Vector<int> pos)
{
  GameMap::getInstance()->getTile(pos)->removeItem(i);
  h->addToBackpack(i, 0);
}

bool Player::heroPickupItem(Hero *h, Item *i, Vector<int> pos)
{
  doHeroPickupItem(h, i, pos);
  
  Action_Equip* item = new Action_Equip();
  item->fillData(h, i, Action_Equip::BACKPACK);
  addAction(item);
  
  return true;
}

bool Player::heroPickupAllItems(Hero *h, Vector<int> pos)
{
  std::list<Item*> bag = GameMap::getInstance()->getTile(pos)->getItems();
  for (std::list<Item*>::iterator i = bag.begin(), end = bag.end();
       i != end; ++i)
    heroPickupItem(h, *i, pos);
  return true;
}

bool Player::heroCompletesQuest(Hero *h)
{
  // record it for posterity
  History_HeroQuestCompleted* item = new History_HeroQuestCompleted();
  item->fillData(h);
  d_history.push_back(item);
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
      if ((*it).getOwner() == this)
	{
	  (*it).setBurnt(true);
	  History_CityRazed* history = new History_CityRazed();
	  history->fillData(&(*it));
	  d_history.push_back(history);
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

void Player::doVectorFromCity(City * c, Vector<int> dest)
{
  c->setVectoring(dest);
}

bool Player::vectorFromCity(City * c, Vector<int> dest)
{
  doVectorFromCity(c, dest);
  
  Action_Vector* item = new Action_Vector();
  item->fillData(c, dest);
  addAction(item);
  return true;
}

bool Player::changeVectorDestination(City *c, Vector<int> dest)
{
  bool retval = true;
  Citylist *cl = Citylist::getInstance();
  std::list<City*> sources = cl->getCitiesVectoringTo(c);
  std::list<City*>::iterator it = sources.begin();
  for (; it != sources.end(); it++)
    retval &= (*it)->changeVectorDestination(dest);
  return retval;
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
      std::list<Item*> backpack = hero->getBackpack();
      for (std::list<Item*>::iterator i = backpack.begin(), 
             end = backpack.end(); i != end; ++i)
      {
        if ((*i)->isPlantable() && (*i)->getPlantableOwner() == this)
        {
          //drop the item, and plant it
          doHeroPlantStandard(hero, *i, s->getPos());
                  
          Action_Plant * item = new Action_Plant();
          item->fillData(hero, *i);
          addAction(item);
          return true;
        }
      }
    }
  }
  return true;
}

void Player::doHeroPlantStandard(Hero *hero, Item *item, Vector<int> pos)
{
  item->setPlanted(true);
  GameMap *gm = GameMap::getInstance();
  gm->getTile(pos)->addItem(item);
  hero->removeFromBackpack(item);
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
		  tallyTriumph((*sit)->getOwner(), TALLY_SPECIAL);
		else if ((*sit)->isHero() == false)
		  tallyTriumph((*sit)->getOwner(), TALLY_NORMAL);
		if ((*sit)->getStat(Army::SHIP, false)) //hey it was on a boat
		  tallyTriumph((*sit)->getOwner(), TALLY_SHIP);
                debug("Army: " << (*sit)->getName())
                debug("Army: " << (*sit)->getXpReward())
                if ((*sit)->isHero())
                {
		  tallyTriumph((*sit)->getOwner(), TALLY_HERO);
		  Hero *hero = dynamic_cast<Hero*>((*sit));
		  std::list<Item*> backpack = hero->getBackpack();
		  for (std::list<Item*>::iterator i = backpack.begin(), 
		       end = backpack.end(); i != end; ++i)
		    {
		      if ((*i)->isPlantable())
			tallyTriumph((*sit)->getOwner(), TALLY_FLAG);
		    }
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
                        h->getOwner()->getHistorylist()->push_back(item);
                        heroDropAllItems (h, (*it)->getPos());
                    }
                    else if (tile->getBuilding() == Maptile::CITY)
                    {
		        Citylist *clist = Citylist::getInstance();
                        City* c = clist->getObjectAt((*it)->getPos());
                        History_HeroKilledInCity* item;
                        item = new History_HeroKilledInCity();
                        item->fillData(h, c);
                        h->getOwner()->getHistorylist()->push_back(item);
                        heroDropAllItems (h, (*it)->getPos());
                    }
                    else //somewhere else
                    {
                        History_HeroKilledInBattle* item;
                        item = new History_HeroKilledInBattle();
                        item->fillData(h);
                        h->getOwner()->getHistorylist()->push_back(item);
                        heroDropAllItems (h, (*it)->getPos());
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
                owner->deleteStack(*it);
            }
            else // there is no owner - like for the ruin's occupants
                debug("No owner for this stack - do stacklist too");

            debug("Removing from the vector too (the vetor had "
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

            if (army->isHero())
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

void Player::tallyTriumph(Player *p, TriumphType type)
{
  //ignore monsters in a ruin who aren't owned by a player
  if (!p) 
    return;
  Uint32 id = p->getId();
  //let's not tally fratricide
  if (p == this) 
    return;
  //let's not tally neutrals
  if (p == Playerlist::getInstance()->getNeutral()) 
    return;
  //we (this player) have killed P's army. it was of type TYPE.
  d_triumph[id][type]++;
}

void Player::doRecruitHero(Hero* herotemplate, City *city, int cost, int alliesCount, const Army *ally)
{
  History_HeroEmerges *item = new History_HeroEmerges();
  item->fillData(herotemplate, city);
  d_history.push_back(item);

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
    newhero->addToBackpack(battle_standard, 0);
  }
  withdrawGold(cost);
  supdatingStack.emit(0);
}

void Player::recruitHero(Hero* hero, City *city, int cost, int alliesCount, const Army *ally)
{
  Action_RecruitHero *action = new Action_RecruitHero();
  action->fillData(hero, city, cost, alliesCount, ally);
  addAction(action);
  doRecruitHero(hero, city, cost, alliesCount, ally);
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
  Playerlist *pl = Playerlist::getInstance();
  if (pl->getNeutral() == player)
    return;
  if (player == this)
    return;
  if (proposal == d_diplomatic_proposal[player->getId()])
    return;
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

// End of file
