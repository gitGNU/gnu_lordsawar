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

#include "Condition.h"
#include "CPlayer.h"
#include "CCounter.h"
#include "CLiving.h"
#include "CDead.h"
#include "CArmy.h"

Condition::Condition(Type type)
    :d_type(type)
{
}

Condition::~Condition()
{
}

Condition* Condition::loadCondition(XML_Helper* helper)
{
    Uint32 type;
    helper->getData(type, "type");

    switch(type)
    {
        case PLAYER:
            return new CPlayer(helper);
        case COUNTER:
            return new CCounter(helper);
        case LIVING:
            return new CLiving(helper);
        case DEAD:
            return new CDead(helper);
        case ARMY:
            return new CArmy(helper);
    }
        
    return 0;
}
