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

#include "CLiving.h"
#include "../playerlist.h"

CLiving::CLiving(Uint32 player)
    :Condition(LIVING), d_player(player)
{
}

CLiving::CLiving(XML_Helper* helper)
    :Condition(LIVING)
{
    helper->getData(d_player, "player");
}

CLiving::~CLiving()
{
}

bool CLiving::check()
{
    Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
        if ((*it)->getId() == d_player)
        {
            if (!(*it)->isDead())
                return true;
            break;
        }
    
    return false;
}

bool CLiving::save(XML_Helper* helper) const
{
    bool retval = true;
    
    retval &= helper->openTag("condition");
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("player", d_player);
    retval &= helper->closeTag();
    
    return retval;
}
