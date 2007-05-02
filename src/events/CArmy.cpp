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

#include "CArmy.h"
#include "../playerlist.h"
#include "../stacklist.h"
#include "../army.h"
#include "../stack.h"

CArmy::CArmy(Uint32 army)
    :Condition(ARMY), d_army(army)
{
}

CArmy::CArmy(XML_Helper* helper)
    :Condition(ARMY)
{
    helper->getData(d_army, "army");
}

CArmy::~CArmy()
{
}

bool CArmy::check()
{
    Stack* s = Playerlist::getActiveplayer()->getStacklist()->getActivestack();
    if (!s)
        return false;

    for (Stack::iterator it = s->begin(); it != s->end(); it++)
        if ((*it)->getId() == d_army)
            return true;

    return false;
}

bool CArmy::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("condition");
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("army", d_army);
    retval &= helper->closeTag();
    
    return retval;
}
