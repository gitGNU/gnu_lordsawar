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

#include "Quest.h"
#include "QuestsManager.h"
#include "hero.h"
#include "playerlist.h"
#include "stacklist.h"
using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)


Quest::Quest(QuestsManager& q_mgr, Uint32 hero, Type type)
    :d_q_mgr(q_mgr), d_hero(hero), d_type(type), d_pending(true)
{
}

Quest::Quest(QuestsManager& q_mgr, XML_Helper* helper)
    :d_q_mgr(q_mgr), d_pending(true)
{
    helper->getData(d_type, "type");
    helper->getData(d_hero, "hero");
}

Hero* Quest::getHeroById(Uint32 hero, Stack** stack)
{
    Playerlist* pl = Playerlist::getInstance();
	Playerlist::const_iterator pit;
    for (pit = pl->begin(); pit != pl->end(); pit++)
    {
        Stacklist* sl = (*pit)->getStacklist();
        for (Stacklist::const_iterator it = sl->begin(); it != sl->end(); it++)
        {
            for (Stack::iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
            {
                if ( ((*sit)->isHero()) && ((*sit)->getId() == hero) )
                {
                    if (stack)
                        *stack = (*it);
                    // isHero is TRUE, so dynamic_cast should succeed
                    return dynamic_cast<Hero*>(*sit);
                }
            }
        }
    }
    return NULL;
}

bool Quest::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("hero", d_hero);

    return retval;
}

bool Quest::isActive()
{
    if (!d_pending)
        return false;

    Hero* h = getHero();
    if (!h || (h->getHP() <= 0))
    {
        deactivate();
        return false;
    }

    return true;
}
