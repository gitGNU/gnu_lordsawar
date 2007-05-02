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

#include <sigc++/functors/mem_fun.h>

#include "RAddUnit.h"
#include "../playerlist.h"
#include "../defs.h"
#include "../stacklist.h"
#include "../path.h"
#include "../citylist.h"
#include "../GameMap.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)


RAddUnit::RAddUnit(Stack* s, Uint32 player, Vector<int> pos)
    :Reaction(ADDUNIT), d_stack(s), d_player(player), d_pos(pos)
{
}

RAddUnit::RAddUnit(XML_Helper* helper)
    :Reaction(helper), d_stack(0)
{
    d_type = ADDUNIT;
    
    int i;
    
    helper->getData(d_player, "player");
    helper->getData(i, "x");    d_pos.x = i;
    helper->getData(i, "y");    d_pos.y = i;

    //we need to load the stack separately
    helper->registerTag("stack", sigc::mem_fun(*this, &RAddUnit::loadStack));
}

RAddUnit::~RAddUnit()
{
    if (d_stack != 0)
        delete d_stack;
}

bool RAddUnit::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("reaction");
    retval &= helper->saveData("player", d_player);
    retval &= helper->saveData("x", d_pos.x);
    retval &= helper->saveData("y", d_pos.y);

    retval &= Reaction::save(helper);

    if (d_stack)
        retval &= d_stack->save(helper);

    retval &= helper->closeTag();
    
    return retval;
}

bool RAddUnit::trigger() const
{
    debug("trigger")
        
    if (!check() || !d_stack)
        return false;
        
    // The procedure is as follows:
    // First, we look if the position where we are to place the stack is free.
    // If this is not so, we try to place the stack at one of the surrounding
    // tiles. If this doesn't work either, we ...ehm...well... ignore this
    // event. (This is why a scenario designer has to be careful with the
    // placing of events).
    
    // First, create a temporary new stack. We don't want to mess around with our
    // original stack, so we do the whole stuff with a testing one
    Stack* s = new Stack(*d_stack);
    Player* p = Playerlist::getInstance()->getPlayer(d_player);
    GameMap* map = GameMap::getInstance();
    
    
    // 1. Try to place the stack at the position
    if (!Stacklist::getObjectAt(d_pos))
    {
        // We assume that the creator has made a sane decision and just place
        // the stack there.
        debug("position free")
        s->setPlayer(p);
        s->setPosition(d_pos);
        p->getStacklist()->push_back(s);
        return true;
    }
    
    // 2. Try to place the stack somehwere around the original position.
    for (int dx=-1; dx <=1; dx++)
        for (int dy = -1; dy <= 1; dy++)
        {
            if (dx == 0 && dy == 0)
                continue;
           
            Vector<int> newpos;
            newpos.x = d_pos.x+dx;
            newpos.y = d_pos.y+dy;
            debug("checking position"<<newpos.x<<","<<newpos.y)

            // Now check if the tile is sane at all
            if ((newpos.x >= map->getWidth()) || (newpos.y >= map->getHeight())
                || (newpos.x < 0) || (newpos.y < 0))
                continue;

            // Check for existing enemy cities
            City* c = Citylist::getInstance()->getObjectAt(newpos);
            if (c && (c->getPlayer() != p))
                continue;

            // Check for existing stacks
            if (Stacklist::getObjectAt(newpos))
                continue;

            // At this point we know that this tile is sane and not blocked. Now
            // we need to know whether the tile is suitable for this stack at
            // all. We can use the stack's path to determine this.
            if (!s->getPath()->calculate(s, newpos))
                continue;

            // All tests have been passed => place the stack here
            s->getPath()->flClear();
            s->setPlayer(p);
            s->setPosition(newpos);
            p->getStacklist()->push_back(s);
            return true;
        }
    
    // Stack placement didn't succeed
    debug("placement failed")
    return false;
}

Stack* RAddUnit::setStack(Stack* s)
{
    Stack* temp = d_stack;
    d_stack = s;
    return temp;
}

bool RAddUnit::loadStack(std::string tag, XML_Helper* helper)
{
    if (tag == "stack")
        d_stack = new Stack(helper);
    else
    {
        std::cerr <<_("RAddUnit: got strange tag: ") <<tag <<std::endl;
        return false;
    }

    return true;
}
