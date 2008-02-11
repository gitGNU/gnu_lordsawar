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

#include <algorithm>
#include <sigc++/functors/mem_fun.h>

#include "rewardlist.h"
#include "reward.h"
#include "xmlhelper.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Rewardlist* Rewardlist::s_instance = 0;

Rewardlist* Rewardlist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Rewardlist();

    return s_instance;
}

Rewardlist* Rewardlist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Rewardlist(helper);
    return s_instance;
}

void Rewardlist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

void Rewardlist::deleteReward(Reward* s)
{
  for (const_iterator it = this->begin(); it != this->end(); it++)
    if ((*it) == s)
      {
        this->flRemove(s);
        return;
      }
}

Rewardlist::Rewardlist()
{
}

Rewardlist::Rewardlist(Rewardlist *rewardlist)
{
  for (iterator it = rewardlist->begin(); it != rewardlist->end(); it++)
    {
      push_back(*it);
    }
}

Rewardlist::Rewardlist(XML_Helper* helper)
{
  helper->registerTag("reward", sigc::mem_fun((*this), &Rewardlist::load));
  load("rewardlist", helper);
}

Rewardlist::~Rewardlist()
{
}

void Rewardlist::flClear()
{
  for (iterator it = begin(); it != end(); it++)
    {
      delete (*it);
    }

  clear();
}

Rewardlist::iterator Rewardlist::flErase(iterator object)
{
  delete (*object);
  return erase(object);
}

bool Rewardlist::flRemove(Reward* object)
{
  debug("removing reward with id " << object->getId() << endl);
  iterator rewardit = find(begin(), end(), object);
  if (rewardit != end())
    {
      delete object;
      erase(rewardit);
      return true;
    }
  return false;
}

bool Rewardlist::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag("rewardlist");

  //save rewards
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getType() == Reward::GOLD)
        static_cast<Reward_Gold*>(*it)->save(helper);
      else if ((*it)->getType() == Reward::ALLIES)
        static_cast<Reward_Allies*>(*it)->save(helper);
      else if ((*it)->getType() == Reward::ITEM)
        static_cast<Reward_Item*>(*it)->save(helper);
      else if ((*it)->getType() == Reward::RUIN)
        static_cast<Reward_Ruin*>(*it)->save(helper);
      else if ((*it)->getType() == Reward::MAP)
        static_cast<Reward_Map*>(*it)->save(helper);
    }

  retval &= helper->closeTag();

  return retval;
}

bool Rewardlist::load(string tag, XML_Helper* helper)
{
  if (tag == "reward")
    {
      Reward *s = Reward::handle_load(helper);
      push_back(s);
      return true;
    }

    return false;
}

Reward *Rewardlist::popRandomReward(Reward::Type type)
{
  Rewardlist::iterator iter;
  std::vector<Reward*> rewards;
  for (iter = begin(); iter != end(); iter++)
    {
      if ((*iter)->getType() == type)
        rewards.push_back(*iter);
    }
  if (rewards.size())
    {
      Reward *newReward = rewards[rand() % rewards.size()];
      remove(newReward);
      return newReward;
    }
  else
    return NULL;
}
Reward *Rewardlist::popRandomItemReward()
{
  return popRandomReward(Reward::ITEM);
}

Reward *Rewardlist::popRandomRuinReward()
{
  return popRandomReward(Reward::RUIN);
}
// End of file
