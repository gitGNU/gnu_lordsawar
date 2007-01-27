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

#include "CCounter.h"

CCounter::CCounter(Uint32 number)
    :Condition(COUNTER), d_counter(number)
{
}

CCounter::CCounter(XML_Helper* helper)
    :Condition(COUNTER)
{
    helper->getData(d_counter, "counter");
}

CCounter::~CCounter()
{
}

bool CCounter::check()
{
    d_counter--;
    if (d_counter == 0)
        return true;

    return false;
}

bool CCounter::save(XML_Helper* helper) const
{
    bool retval = true;
    
    retval &= helper->openTag("condition");
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("counter", d_counter);
    retval &= helper->closeTag();
    
    return retval;
}
