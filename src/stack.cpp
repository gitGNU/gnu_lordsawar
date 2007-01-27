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

#include "stack.h"
#include <sstream>
#include <assert.h>
#include "playerlist.h"
#include "path.h"
#include "armysetlist.h"
#include "counter.h"
#include "army.h"
#include "hero.h"
#include "GameMap.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Stack::Stack(Player* player, PG_Point pos)
    :d_player(player), d_defending(false), d_pos(pos), d_deleting(false)
{
    d_path = new Path();
    d_id = fl_counter->getNextId();
}

Stack::Stack(Stack& s)
    :d_id(s.d_id), d_player(s.d_player), d_defending(s.d_defending), d_pos(s.d_pos),
     d_deleting(false)
{
    clear();
    d_path = new Path();
    //deep copy the other stack's armies
    for (iterator sit = s.begin(); sit != s.end(); sit++)
    {
        Army* a;
        a = new Army((**sit));
        push_back(a);
    }
}

Stack::Stack(XML_Helper* helper)
    :d_deleting(false)
{
    int i;

    helper->getData(d_id, "id");
    helper->getData(i, "x");
    d_pos.x = i;
    helper->getData(i, "y");
    d_pos.y = i;
    helper->getData(d_defending, "defending");

    Uint32 ui;
    helper->getData(ui, "player");
    d_player = Playerlist::getInstance()->getPlayer(ui);


    helper->registerTag("path", SigC::slot((*this), &Stack::load));
    helper->registerTag("army", SigC::slot((*this), &Stack::load));
    helper->registerTag("hero", SigC::slot((*this), &Stack::load));
}

Stack::~Stack()
{
    d_deleting = true;
    sdying.emit(this);

    delete d_path;
    flClear();
}

void Stack::setPlayer(Player* p)
{
    // we need to change the armies' loyalties as well!!
    d_player = p;
    for (iterator it = begin(); it != end(); it++)
        (*it)->setPlayer(p);
}

bool Stack::moveOneStep()
{
    debug("move_one_step()");

    d_pos = **d_path->begin();

    //now remove first point of the path
    d_path->flErase(d_path->begin());

    return true;
}

// return the maximum moves of this stack by checking the moves of each army

Uint32 Stack::getGroupMoves()
{
    Uint32 maximum = 0;

    for (iterator it = begin(); it != end(); it++)
    {
        if (((*it)->getMoves() < maximum) || (!maximum))
        {
            maximum = (*it)->getMoves();
	    if (maximum == 0)
	         break;
        }
    }

    return maximum;
}

Uint32 Stack::getMinTilesAroundMoves(int x, int y)
{

  GameMap *tmp=GameMap::getInstance();
  Uint32 tmpmove=32767;
  int mapwidth=tmp->getWidth();
  int mapheight=tmp->getHeight();
  Path * tmppath =new Path();
  PG_Point* p = new PG_Point;

  tmppath->checkPath(this);

  if (x>0 && x<mapwidth && y>0 && y<mapheight)
  {
    debug("x>0<mapsize y>0<mapsize  TMPMOVE= " << tmpmove)

    p->x=x+1;
    p->y=y+1;
    if(tmppath->canMoveThere(this,p))
    {
        tmpmove=tmp->getTile(x+1,y+1)->getMoves();
        debug("Checked Position: " << x+1 << " " << y+1 )
    }
    p->x=x;
    p->y=y+1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x,y+1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x,y+1)->getMoves();
        debug("Checked Position: " << x << " " << y+1 )
    }
    p->x=x+1;
    p->y=y;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x+1,y)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x+1,y)->getMoves();
        debug("Checked Position: " << x+1 << " " << y )
    }
    p->x=x;
    p->y=y-1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x,y-1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x,y-1)->getMoves();
        debug("Checked Position: " << x << " " << y-1 )
    }
    p->x=x+1;
    p->y=y-1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x+1,y-1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x+1,y-1)->getMoves();
        debug("Checked Position: " << x+1 << " " << y-1 )
    }
    p->x=x-1;
    p->y=y;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x-1,y)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x-1,y)->getMoves();
        debug("Checked Position: " << x-1 << " " << y )
    }
    p->x=x-1;
    p->y=y-1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x-1,y-1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x-1,y-1)->getMoves();
        debug("Checked Position: " << x-1 << " " << y-1 )
    }
    p->x=x-1;
    p->y=y+1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x-1,y+1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x-1,y+1)->getMoves();
        debug("Checked Position: " << x-1 << " " << y+1 )
    }
    delete tmppath;
    delete p;
    if (tmpmove!=32767)
        return tmpmove;
    else
        return 0;
  }


  if (x==0 && y==0)
  {
    debug("x=0 y=0 TMPMOVE= " << tmpmove)

    p->x=x+1;
    p->y=y+1;
    if(tmppath->canMoveThere(this,p))
    {
        tmpmove=tmp->getTile(x+1,y+1)->getMoves();
        debug("Checked Position: " << x+1 << " " << y+1 )
    }
    p->x=x;
    p->y=y+1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x,y+1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x,y+1)->getMoves();
        debug("Checked Position: " << x << " " << y+1 )
    }
    p->x=x+1;
    p->y=y;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x+1,y)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x+1,y)->getMoves();
        debug("Checked Position: " << x+1 << " " << y )
    }
    delete tmppath;
    delete p;
    if (tmpmove!=32767)
        return tmpmove;
    else
        return 0;
  }


  if (x==mapwidth && y==mapheight)
  {
    debug("x=mapwidth y=mapheight  TMPMOVE= " << tmpmove)

    p->x=x-1;
    p->y=y-1;
    if(tmppath->canMoveThere(this,p))
    {
        tmpmove=tmp->getTile(x-1,y-1)->getMoves();
        debug("Checked Position: " << x-1 << " " << y-1 )
    }
    p->x=x;
    p->y=y-1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x,y-1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x,y-1)->getMoves();
        debug("Checked Position: " << x << " " << y-1 )
    }
    p->x=x-1;
    p->y=y;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x-1,y)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x-1,y)->getMoves();
        debug("Checked Position: " << x-1 << " " << y )
    }
    delete tmppath;
    delete p;
    if (tmpmove!=32767)
        return tmpmove;
    else
        return 0;
  }


  if (x==mapwidth && y<mapheight && y>0)
  {
    debug("x=mapwidth y>0<mapheight  TMPMOVE= " << tmpmove)

    p->x=x-1;
    p->y=y-1;
    if(tmppath->canMoveThere(this,p))
    {
        tmpmove=tmp->getTile(x-1,y-1)->getMoves();
        debug("Checked Position: " << x-1 << " " << y-1 )
    }
    p->x=x;
    p->y=y-1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x,y-1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x,y-1)->getMoves();
        debug("Checked Position: " << x << " " << y-1 )
    }
    p->x=x-1;
    p->y=y;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x-1,y)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x-1,y)->getMoves();
        debug("Checked Position: " << x-1 << " " << y )
    }
    p->x=x-1;
    p->y=y+1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x-1,y+1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x-1,y+1)->getMoves();
        debug("Checked Position: " << x-1 << " " << y+1 )
    }
    p->x=x;
    p->y=y+1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x,y+1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x,y+1)->getMoves();
        debug("Checked Position: " << x << " " << y+1 )
    }
    delete tmppath;
    delete p;
    if (tmpmove!=32767)
        return tmpmove;
    else
        return 0;
  }


  if (x<mapwidth && x>0 && y==mapheight)
  {
    debug("x>0<mapwidth y=mapheight  TMPMOVE= " << tmpmove)

    p->x=x-1;
    p->y=y-1;
    if(tmppath->canMoveThere(this,p))
    {
        tmpmove=tmp->getTile(x-1,y-1)->getMoves();
        debug("Checked Position: " << x-1 << " " << y-1 )
    }
    p->x=x;
    p->y=y-1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x,y-1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x,y-1)->getMoves();
        debug("Checked Position: " << x << " " << y-1 )
    }
    p->x=x-1;
    p->y=y;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x-1,y)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x-1,y)->getMoves();
        debug("Checked Position: " << x-1 << " " << y )
    }
    p->x=x+1;
    p->y=y-1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x+1,y-1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x+1,y-1)->getMoves();
        debug("Checked Position: " << x+1 << " " << y-1 )
    }

    p->x=x+1;
    p->y=y;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x+1,y)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x+1,y)->getMoves();
        debug("Checked Position: " << x+1 << " " << y )
    }
    delete tmppath;
    delete p;
    if (tmpmove!=32767)
        return tmpmove;
    else
        return 0;
  }


  if (x>0 && x<mapwidth && y==0)
  {
    debug("x>0<mapwidth y=0 TMPMOVE= " << tmpmove)

    p->x=x+1;
    p->y=y+1;
    if(tmppath->canMoveThere(this,p))
    {
        tmpmove=tmp->getTile(x+1,y+1)->getMoves();
        debug("Checked Position: " << x+1 << " " << y+1 )
    }
    p->x=x;
    p->y=y+1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x,y+1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x,y+1)->getMoves();
        debug("Checked Position: " << x << " " << y+1 )
    }
    p->x=x+1;
    p->y=y;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x+1,y)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x+1,y)->getMoves();
        debug("Checked Position: " << x+1 << " " << y )
    }
    p->x=x-1;
    p->y=y+1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x-1,y+1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x-1,y+1)->getMoves();
        debug("Checked Position: " << x-1 << " " << y+1 )
    }
    p->x=x-1;
    p->y=y;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x-1,y)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x-1,y)->getMoves();
        debug("Checked Position: " << x-1 << " " << y )
    }
    delete tmppath;
    delete p;
    if (tmpmove!=32767)
        return tmpmove;
    else
        return 0;
  }


  if (x==0 && y>0 && y<mapheight)
  {
    debug("x=0 y>0<mapheight  TMPMOVE= " << tmpmove)

    p->x=x+1;
    p->y=y+1;
    if(tmppath->canMoveThere(this,p))
    {
        tmpmove=tmp->getTile(x+1,y+1)->getMoves();
        debug("Checked Position: " << x+1 << " " << y+1 )
    }
    p->x=x;
    p->y=y+1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x,y+1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x,y+1)->getMoves();
        debug("Checked Position: " << x << " " << y+1 )
    }
    p->x=x+1;
    p->y=y;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x+1,y)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x+1,y)->getMoves();
        debug("Checked Position: " << x+1 << " " << y )
    }

    p->x=x;
    p->y=y-1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x,y-1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x,y-1)->getMoves();
        debug("Checked Position: " << x << " " << y-1 )
    }
    p->x=x+1;
    p->y=y-1;
    if(tmppath->canMoveThere(this,p) && tmp->getTile(x+1,y-1)->getMoves()<tmpmove)
    {
        tmpmove=tmp->getTile(x+1,y-1)->getMoves();
        debug("Checked Position: " << x+1 << " " << y-1 )
    }
    delete tmppath;
    delete p;
    if (tmpmove!=32767)
        return tmpmove;
    else
        return 0;
  }

  delete tmppath;
  delete p;
  return 0;
}

// decrement each armys moves by needed moves to travel

void Stack::decrementMoves(Uint32 moves)
{
    debug("decrement_moves()");

    for (iterator it = begin(); it != end(); it++)
    {
        (*it)->decrementMoves(moves);
    }
}

// Purpose: Return the strongest army of a group
// Note: If a hero is present return it. If there are two similar armies
// (two of the same strength, or two heroes) return the first in the sequence.

Army* Stack::getStrongestArmy() const
{
    Army *strongest = 0;
    Uint32 highest_strength = 0;

    for (const_iterator it = begin(); it != end(); ++it)
    {
        // if hero
        if ((*it)->isHero())
        {
            return *it;
        }
        else if ((*it)->getStat(Army::STRENGTH) > highest_strength)
        {
            highest_strength = (*it)->getStat(Army::STRENGTH);
            strongest = *it;
        }
    }
    return strongest;
}

void Stack::selectAll()
{
    for (const_iterator it = begin(); it != end(); ++it)
    {
	(*it)->setGrouped(true);
    }
    return;
}

bool Stack::hasHero() const
{
    for (const_iterator it = begin(); it != end(); it++)
        if ((*it)->isHero())
            return true;

    return false;
}

Army* Stack::getFirstHero() const
{
    for (const_iterator it = begin(); it != end(); it++)
        if ((*it)->isHero())
            return (*it);

    return 0;
}

void Stack::getHeroes(std::vector<Uint32>& dst) const
{
    debug("getHeroes - stack = " << this)
    for (const_iterator it = begin(); it != end(); ++it)
    {
        // if hero - add it to the vector
        debug("Army type: " << (*it)->getType())
        if ((*it)->isHero() && (*it)->getHP() > 0)
            dst.push_back((*it)->getId());
    }
}

void Stack::bless()
{
    for (iterator it = begin(); it != end(); it++)
    {
        (*it)->bless();
    }
}

bool Stack::enoughMoves() const
{
    PG_Point p = **(d_path->begin());
    Uint32 needed = 0;

    Maptile* tile = GameMap::getInstance()->getTile(p.x, p.y);

    // find out how many MP we need for travelling, first
    if (tile->getBuilding() == Maptile::CITY)
        needed = 1;
    else
        for (const_iterator it = begin(); it != end(); it++)
            if ((*it)->getStat(Army::MOVE_BONUS) & tile->getMaptileType())
                needed = 2;
    if (needed == 0)
        needed = tile->getMoves();
    
    // now check if all armies fulfill this requirement
    for (const_iterator it = begin(); it != end(); it++)
        if ((*it)->getMoves() < needed)
            return false;

    return true;
}

Army* Stack::getFirstUngroupedArmy() const
{
    for (const_iterator it = begin(); it != end(); it++)
    {
        if (!(*it)->isGrouped())
        {
            return *it;
        }
    }
    return 0;
}

bool Stack::isFriend(Player* player) const
{
    return d_player == player;
}

Uint32 Stack::getMaxSight() const
{
    Uint32 max = 0;
    for (const_iterator it = begin(); it != end(); it++)
        if ((*it)->getStat(Army::SIGHT) > max)
            max = (*it)->getStat(Army::SIGHT);

    return max;
}

void Stack::nextTurn()
{
    d_defending = false;
    for (iterator it = begin(); it != end(); it++)
    {
        (*it)->resetMoves();
        // TODO: should be moved in a more appropriate place => class Player
        if (d_player)
            d_player->withdrawGold((*it)->getUpkeep());
        (*it)->heal();
    }
}

bool Stack::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("stack");
    retval &= helper->saveData("id", d_id);
    if (d_player)
        retval &= helper->saveData("player", d_player->getId());
    else
        retval &= helper->saveData("player", 0);
    retval &= helper->saveData("x", d_pos.x);
    retval &= helper->saveData("y", d_pos.y);
    retval &= helper->saveData("defending", d_defending);

    //save path
    retval &= d_path->save(helper);

    //save armies
    for (const_iterator it = begin(); it != end(); it++)
        retval &= (*it)->save(helper);

    retval &= helper->closeTag();

    return retval;
}

bool Stack::load(std::string tag, XML_Helper* helper)
{
    if (tag == "path")
    {
        d_path = new Path(helper);

        return true;
    }

    if (tag == "army")
    {
        Army* a = new Army(helper);
        a->setPlayer(d_player);
        push_back(a);

        return true;
    }

    if (tag == "hero")
    {
        Hero* h = new Hero(helper);
        h->setPlayer(d_player);
        push_back(h);

        return true;
    }

    return false;
}

void Stack::flClear()
{
    for (iterator it = begin(); it != end(); it++)
        delete (*it);
    clear();
}

Stack::iterator Stack::flErase(Stack::iterator object)
{
    delete (*object);
    return erase(object);
}

Uint32 Stack::calculateMoveBonus(bool * has_ship ,bool * has_land)
{
    Uint32 d_bonus = !0;
    *has_ship= false;
    *has_land= false;

    //check if there are ships/non-water units in the stack and
    //calculate the move bonuses
    for (Stack::const_iterator it = this->begin(); it != this->end(); it++)
    {
        d_bonus &= (*it)->getStat(Army::MOVE_BONUS);

        if (((*it)->getStat(Army::ARMY_BONUS)) & Army::SHIP)
        {
            //ship in stack
            if (!(*has_ship)) debug("ship in stack")
            *has_ship = true;
            continue; 
        }

        if (!((*it)->getStat(Army::MOVE_BONUS) & Tile::WATER))
        {
            //unit which can't cross water
            if (!(*has_land)) debug("land unit in stack")
            *has_land = true;
            continue; 
        }
    }

    return d_bonus;
}

// End of file
