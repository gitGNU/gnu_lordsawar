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

#include "stacklist.h"
#include "stack.h"
#include "path.h"
#include "playerlist.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

//the static functions first
Stack* Stacklist::getObjectAt(int x, int y)
{
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        for (const_iterator it = mylist->begin(); it !=mylist->end(); it++)
            if (((*it)->getPos().x == x) && ((*it)->getPos().y == y))
                return *it;
    }

    return 0;

}

Stack* Stacklist::getObjectAt(PG_Point point)
{
    return getObjectAt(point.x, point.y);
}

PG_Point Stacklist::getPosition(Uint32 id)
{
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        for (const_iterator it = mylist->begin(); it !=mylist->end(); it++)
            for (Stack::const_iterator sit = (*it)->begin(); sit != (*it)->end(); sit++)
                if ((*sit)->getId() == id)
                    return (*it)->getPos();
    }

    return PG_Point(-1,-1);
}

//We only expect one ambiguity at a time with stacks of the same player. This
//never happens except when a stack comes to halt on another stack during
//long movements
Stack* Stacklist::getAmbiguity(Stack* s)
{
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        for (const_iterator it = mylist->begin(); it != mylist->end();it++)
            if ((s->getPos().x == (*it)->getPos().x)
                && (s->getPos().y == (*it)->getPos().y) && (s != *it))
                return (*it);
    }

    return 0;
}

//search all player's stacklists to find this stack
void Stacklist::deleteStack(Stack* s)
{
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        for (const_iterator it = mylist->begin(); it != mylist->end(); it++)
            if ((*it) == s)
            {
                mylist->flRemove(s);
                return;
            }
    }
}

void Stacklist::nextTurn()
{
    debug("nextTurn()");
    for (iterator it = begin(); it != end(); it++)
        (*it)->nextTurn();
}

vector<Stack*> Stacklist::defendersInCity(City *city)
{
    debug("defendersInCity()");

    vector<Stack*> stackvector;
    PG_Point pos = city->getPos();

    for (int i = pos.x; i < pos.x + 2; i++)
    {
        for (int j = pos.y; j < pos.y + 2; j++)
        {
            Stack* stack = getObjectAt(i, j);
            if (stack)
            {
                stackvector.push_back(stack);
            }
        }
    }

    return stackvector;
}

unsigned int Stacklist::getNoOfStacks()
{
    unsigned int mysize = 0;

    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); pit++)
    {
        mysize += (*pit)->getStacklist()->size();
    }

    return mysize;
}

Stacklist::Stacklist()
    :d_activestack(0)
{
}

Stacklist::Stacklist(Stacklist *stacklist)
    :d_activestack(0)
{
    for (iterator it = stacklist->begin(); it != stacklist->end(); it++)
    {
        push_back(*it);
    }
}

Stacklist::Stacklist(XML_Helper* helper)
    :d_activestack(0)
{
    helper->registerTag("stack", SigC::slot((*this), &Stacklist::load));

    load("stacklist", helper);
}

Stacklist::~Stacklist()
{
}

Stack* Stacklist::setPrev()
{
    debug("setPrev()");

    reverse_iterator it = rbegin();

    //first, if we already have an active stack, loop through until we meet it
    if (d_activestack)
    {
        for (; (*it) != d_activestack; it++);
        it++;   //we want to start with the previous stack :)
    }

    //continue looping until we meet the previous not defending stack of this player
    for (; it != rend(); it++)
        if (((*it)->getPlayer() == Playerlist::getInstance()->getActiveplayer())
            && (!(*it)->getDefending()))
        {
            d_activestack = (*it);
            return d_activestack;
        }
    //still not found a stack? Then start looping from the beginning until we
    //meet the activestack again. If there is no activestack, we have already
    //looped through the whole list, so stop here
    if (!d_activestack)
        return 0;

    for (it = rbegin(); (*it) != d_activestack; it++)
    {
        if (((*it)->getPlayer() == Playerlist::getInstance()->getActiveplayer())
            && (!(*it)->getDefending()))
        {
            d_activestack = (*it);
            return d_activestack;
        }
    }

    //still there? well, then we have only one non-defending stack left.
    return d_activestack;
}

Stack* Stacklist::setNext()
{
    debug("setNext()");

    iterator it = begin();

    //first, if we already have an active stack, loop through until we meet it
    if (d_activestack)
    {
        for (; (*it) != d_activestack; it++);
        it++;   //we want to start with the next stack :)
    }

    //continue looping until we meet the next not defending stack of this player
    for (; it != end(); it++)
        if (((*it)->getPlayer() == Playerlist::getInstance()->getActiveplayer())
            && (!(*it)->getDefending()))
        {
            d_activestack = (*it);
            return d_activestack;
        }

    //still not found a stack? Then start looping from the beginning until we
    //meet the activestack again. If there is no activestack, we have already
    //looped through the whole list, so stop here
    if (!d_activestack)
        return 0;

    for (it = begin(); (*it) != d_activestack; it++)
    {
        if (((*it)->getPlayer() == Playerlist::getInstance()->getActiveplayer())
            && (!(*it)->getDefending()))
        {
            d_activestack = (*it);
            return d_activestack;
        }
    }

    //still there? well, then we have only one non-defending stack left.
    return d_activestack;
}

Stack* Stacklist::setNextWithMove(bool select)
{

    // the select flag is used to avoid the set of the active stack (used in w_edit to check if
    // there are units tha can move even if the active stack is not selected)

    debug("setNextWithMove()");
    
    iterator it = begin();

    //first, if we already have an active stack, loop through until we meet it
    if (d_activestack)
    {
        for (; (*it) != d_activestack; it++);
        it++;   //we want to start with the next stack :)
    }

    //continue looping until we meet the next not defending stack of this player
    for (; it != end(); it++)
    {
        if (((*it)->getPlayer() == Playerlist::getInstance()->getActiveplayer())
            && (!(*it)->getDefending()))
        {
            Uint32 mintilemove= ((*it)->getMinTilesAroundMoves((*it)->getPos().x,(*it)->getPos().y));

            if(((*it)->getGroupMoves() > 0) && (mintilemove >= 0)
                && ((*it)->getGroupMoves() >= mintilemove))
            {
                if (select) 
		{
                     d_activestack = (*it);
                     return d_activestack;
		}
		else return (*it);
            }
        }
    }
    //still not found a stack? Then start looping from the beginning until we
    //meet the activestack again. If there is no activestack, we have already
    //looped through the whole list, so stop here
    if (!d_activestack)
        return 0;

    for (it = begin(); (*it) != d_activestack; it++)
    {
        if (((*it)->getPlayer() == Playerlist::getInstance()->getActiveplayer())
            && (!(*it)->getDefending()))
        {
            Uint32 mintilemove= ((*it)->getMinTilesAroundMoves((*it)->getPos().x,(*it)->getPos().y));

            if(((*it)->getGroupMoves() > 0) && (mintilemove >= 0)
                && ((*it)->getGroupMoves()>=mintilemove))
            {
                if (select) 
                {
                   d_activestack = (*it);
                   return d_activestack;
                }
		else return (*it);
            }
        }
    }

    //still there? well, then we have only one non-defending stack left.
    return d_activestack;
}

void Stacklist::flClear()
{
    d_activestack = 0;

    for (iterator it = begin(); it != end(); it++)
    {
        delete (*it);
    }

    clear();
}

Stacklist::iterator Stacklist::flErase(iterator object)
{
    if (d_activestack == (*object))
        d_activestack = 0;
    delete (*object);
    return erase(object);
}

bool Stacklist::flRemove(Stack* object)
{
    debug("removing stack with id " << object->getId() << endl);
    iterator stackit = find(begin(), end(), object);
    if (stackit != end())
    {
        if (d_activestack == object)
            d_activestack = 0;
        delete object;
        erase(stackit);
        return true;
    }
    return false;
}

bool Stacklist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("stacklist");
    if (d_activestack)
    {
        retval &= helper->saveData("active", d_activestack->getId());
    }
    else
    {
        retval &= helper->saveData("active", 0);
    }

    //save stacks
    for (const_iterator it = begin(); it != end(); it++)
        retval &= (*it)->save(helper);

    retval &= helper->closeTag();

    return retval;
}

bool Stacklist::enoughMoves() const 
{
    for (const_iterator it = begin(); it != end(); it++)
    {
        Stack* s = *it;
        if (!s->getPath()->empty() && s->enoughMoves())
            return true;
    }

    return false;
}

bool Stacklist::load(string tag, XML_Helper* helper)
{
    static Uint32 active = 0;
    
    if (tag == "stacklist")
    {
        helper->getData(active, "active");
        return true;
    }

    if (tag == "stack")
    {
        Stack* s = new Stack(helper);
        if ((active != 0) && (s->getId() == active))
        {
            d_activestack = s;
        }

        push_back(s);
        return true;
    }

    return false;
}

// End of file
