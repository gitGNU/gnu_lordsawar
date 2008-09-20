// Copyright (C) 2000, 2001 Michael Bartl
// Copyright (C) 2001, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004 John Farrell
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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

#include <sigc++/functors/mem_fun.h>

#include "ruinlist.h"
#include "xmlhelper.h"
#include "playerlist.h"
#include "reward.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

std::string Ruinlist::d_tag = "ruinlist";

Ruinlist* Ruinlist::s_instance = 0;

Ruinlist* Ruinlist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Ruinlist();

    return s_instance;
}

Ruinlist* Ruinlist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Ruinlist(helper);
    return s_instance;
}

void Ruinlist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Ruinlist::Ruinlist()
{
}

Ruinlist::Ruinlist(XML_Helper* helper)
{
    helper->registerTag(Ruin::d_tag, sigc::mem_fun(this, &Ruinlist::load));
}

bool Ruinlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Ruinlist::d_tag);

    for (const_iterator it = begin(); it != end(); it++)
        retval &= (*it).save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Ruinlist::load(std::string tag, XML_Helper* helper)
{
    // Shouldn't happen, but one never knows...
    if (tag != Ruin::d_tag)
        return false;

    Ruin r(helper);
    push_front(r);

    //! since the ruin has only now been copied to its final state, we need
    //to register the callback for the occupants here.
    helper->registerTag(Stack::d_tag, sigc::mem_fun(*begin(), &Ruin::load));
    // same with rewards in ruins
    helper->registerTag(Reward::d_tag, sigc::mem_fun(*begin(), &Ruin::load));

    return true;
}

static bool isHiddenAndNotOwnedByActivePlayer(void *r)
{
  Ruin *ruin = ((Ruin *)r);
  if (ruin->isHidden() == true && 
      ruin->getOwner() != Playerlist::getInstance()->getActiveplayer())
    return true;
  return false;
}

static bool isFogged(void *r)
{
  return ((Ruin*)r)->isFogged();
}

static bool isSearched(void *r)
{
  return ((Ruin*)r)->isSearched();
}

Ruin* Ruinlist::getNearestUnsearchedRuin(const Vector<int>& pos)
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isHiddenAndNotOwnedByActivePlayer);
  filters.push_back(isSearched);
  return getNearestObject(pos, &filters);
}

Ruin* Ruinlist::getNearestRuin(const Vector<int>& pos)
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isHiddenAndNotOwnedByActivePlayer);
  return getNearestObject(pos, &filters);
}
Ruin* Ruinlist::getNearestRuin(const Vector<int>& pos, int dist)
{
  Ruin *r = getNearestRuin(pos);
  if (r->getPos().x <= pos.x + dist && r->getPos().x >= pos.x - dist &&
      r->getPos().y <= pos.y + dist && r->getPos().y >= pos.y - dist)
    return r;
  return NULL;
}

Ruin* Ruinlist::getNearestVisibleRuin(const Vector<int>& pos)
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isFogged);
  filters.push_back(isHiddenAndNotOwnedByActivePlayer);
  return getNearestObject(pos, &filters);
}
Ruin* Ruinlist::getNearestVisibleRuin(const Vector<int>& pos, int dist)
{
  Ruin *r = getNearestVisibleRuin(pos);
  if (r->getPos().x <= pos.x + dist && r->getPos().x >= pos.x - dist &&
      r->getPos().y <= pos.y + dist && r->getPos().y >= pos.y - dist)
    return r;
  return NULL;
}

void Ruinlist::changeOwnership(Player *old_owner, Player *new_owner)
{
  for (iterator it = begin(); it != end(); it++)
    if ((*it).getOwner() == old_owner)
      (*it).setOwner(new_owner);
}

