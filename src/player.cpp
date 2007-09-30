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
#include "GameMap.h"
#include "armysetlist.h"
#include "real_player.h"
#include "ai_dummy.h"
#include "ai_fast.h"
#include "ai_smart.h"
#include "counter.h"
#include "army.h"
#include "action.h"
#include "history.h"
#include "AI_Analysis.h"
#include "AI_Allocation.h"
#include "FogMap.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
#define debug(x)

SDL_Color Player::get_color_for_no(int player_no)
{
    SDL_Color color;
    color.r = color.b = color.g = color.unused = 0;
    switch (player_no % 8)
    {
    case 0: color.r = 252; color.b = 252; color.g = 252; break;
    case 1: color.r = 80; color.b = 28; color.g = 172; break;
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

Player::Player(string name, Uint32 armyset, SDL_Color color, Type type,
	       int player_no)
    :d_color(color), d_name(name), d_armyset(armyset), d_gold(1000),
    d_dead(false), d_immortal(false), d_type(type)
{
    if (player_no != -1)
	d_id = player_no;
    else
	d_id = fl_counter->getNextId();
    d_stacklist = new Stacklist();
    debug("type of " << d_name << " is " << type)
        
    d_fogmap = new FogMap();

    //initial fight order is the order in which the armies appear
    //in the default.xml file.
    Uint32 size = Armysetlist::getInstance()->getSize(d_armyset);
    for (unsigned int i = 0; i < size; i++)
    {
      d_fight_order.push_back(i);
    }
    memset(d_triumph, 0, sizeof(d_triumph));
}

Player::Player(const Player& player)
    :d_color(player.d_color), d_name(player.d_name), d_armyset(player.d_armyset),
    d_gold(player.d_gold), d_dead(player.d_dead), d_immortal(player.d_immortal),
    d_type(player.d_type), d_id(player.d_id), d_fogmap(player.d_fogmap),
    d_fight_order(player.d_fight_order)
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
    d_fogmap = new FogMap();
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

    string s;
    helper->getData(s, "color");
    
    int i;
    istringstream scolor(s);
    scolor >> i; d_color.r = i;
    scolor >> i; d_color.g = i;
    scolor >> i; d_color.b = i;

    helper->getData(d_armyset, "armyset");

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

    for (std::list<Action*>::iterator it = d_actions.begin(); it != d_actions.end(); it++)
        delete (*it);
    d_fight_order.clear();
    for (std::list<History*>::iterator it = d_history.begin(); 
         it != d_history.end(); it++)
        delete (*it);
}

Player* Player::create(std::string name, Uint32 armyset, SDL_Color color, Type type)
{
    switch(type)
    {
        case HUMAN:
            return new RealPlayer(name, armyset, color);
        case AI_FAST:
            return new AI_Fast(name, armyset, color);
        case AI_DUMMY:
            return new AI_Dummy(name, armyset, color);
        case AI_SMART:
            return new AI_Smart(name, armyset, color);
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
    }

    return 0;
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
        if ((*it).getPlayer() == this)
            (*it).setPlayer(Playerlist::getInstance()->getNeutral());
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

    std::stringstream fight_order;
    for (std::list<Uint32>::const_iterator it = d_fight_order.begin();
         it != d_fight_order.end(); it++)
      {
        fight_order << (*it) << " ";
      }
    retval &= helper->saveData("fight_order", fight_order.str());

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
            return (new RealPlayer(helper));
        case AI_FAST:
            return (new AI_Fast(helper));
        case AI_SMART:
            return (new AI_Smart(helper));
        case AI_DUMMY:
            return (new AI_Dummy(helper));
    }

    return 0;
}

bool Player::load(string tag, XML_Helper* helper)
{
    if (tag == "action")
    {
        Action* action;
        action = Action::handle_load(helper);
        d_actions.push_back(action);
    }

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

// End of file
