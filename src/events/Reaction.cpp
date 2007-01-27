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

#include <string>
#include "Reaction.h"
#include "RMessage.h"
#include "RAddGold.h"
#include "RAddUnit.h"
#include "RUpdate.h"
#include "RDelUnit.h"
#include "RCenter.h"
#include "RCenterObj.h"
#include "RCreateItem.h"
#include "RWinGame.h"
#include "RLoseGame.h"
#include "RKillPlayer.h"
#include "RRevivePlayer.h"
#include "RTransferCity.h"
#include "RRaiseEvent.h"
#include "RActEvent.h"

Reaction::Reaction(Type type)
    :d_type(type)
{
}

Reaction::Reaction(XML_Helper* helper)
{
    helper->registerTag("condition", SigC::slot(*this, &Reaction::loadCondition));
}

Reaction::~Reaction()
{
    for (Uint32 i = 0; i < d_conditions.size(); i++)
        delete d_conditions[i];
}

Reaction* Reaction::loadReaction(XML_Helper* helper)
{
    Uint32 type;
    helper->getData(type, "type");


    switch(type)
    {
        case MESSAGE:
            return new RMessage(helper);
        case ADDGOLD:
            return new RAddGold(helper);
        case ADDUNIT:
            return new RAddUnit(helper);
        case UPDATE:
            return new RUpdate(helper);
        case DELUNIT:
            return new RDelUnit(helper);
        case CENTER:
            return new RCenter(helper);
        case CENTEROBJ: 
            return new RCenterObj(helper);
        case CREATEITEM:
            return new RCreateItem(helper);
        case WINGAME:   
            return new RWinGame(helper);
        case LOSEGAME:
            return new RLoseGame(helper);
        case KILLPLAYER:
            return new RKillPlayer(helper);
        case REVIVEPLAYER:
            return new RRevivePlayer(helper);
        case TRANSFERCITY:
            return new RTransferCity(helper);
        case RAISEEVENT:
            return new RRaiseEvent(helper);
        case ACTEVENT:
            return new RActEvent(helper);
    }
    
    return 0;
}

bool Reaction::loadCondition(std::string tag, XML_Helper* helper)
{
    if (tag == "condition")
    {
        d_conditions.push_back(Condition::loadCondition(helper));
        return true;
    }
    return false;
}

bool Reaction::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->saveData("type", d_type);
    
    for (Uint32 i = 0; i < d_conditions.size();i++)
        retval &= d_conditions[i]->save(helper);

    return retval;
}

bool Reaction::addCondition(Condition* c, int index)
{
    if (index == -1)
    {
        d_conditions.push_back(c);
        return true;
    }

    if (index < 0 || index > static_cast<int>(d_conditions.size()))
        return false;

    std::vector<Condition*>::iterator it;
    for (it = d_conditions.begin(); index > 0; it++, index--);
    d_conditions.insert(it, c);

    return true;
}

bool Reaction::removeCondition(Uint32 index)
{
    std::vector<Condition*>::iterator it = d_conditions.begin();

    while (index > 0)
    {
        it++;
        if (it == d_conditions.end())
            return false;
        index--;
    }

    delete (*it);
    d_conditions.erase(it);
    return true;
}

Condition* Reaction::getCondition(Uint32 index)
{
    if (index >= d_conditions.size())
        return 0;

    return d_conditions[index];
}

bool Reaction::check() const
{
    for (Uint32 i = 0; i < d_conditions.size(); i++)
        if (!d_conditions[i]->check())
            return false;

    return true;
}
