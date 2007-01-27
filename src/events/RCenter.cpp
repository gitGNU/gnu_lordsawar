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

#include "RCenter.h"

SigC::Signal1<void, PG_Point> RCenter::scentering;


RCenter::RCenter(PG_Point pos)
    :Reaction(CENTER), d_pos(pos)
{
}

RCenter::RCenter(XML_Helper* helper)
    :Reaction(helper)
{
    d_type = CENTER;
    
    int data;
    helper->getData(data, "x"); d_pos.x = data;
    helper->getData(data, "y"); d_pos.y = data;
}

RCenter::~RCenter()
{
}

bool RCenter::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("reaction");
    retval &= helper->saveData("x", d_pos.x);
    retval &= helper->saveData("y", d_pos.y);
    retval &= Reaction::save(helper);
    retval &= helper->closeTag();

    return retval;
}

bool RCenter::trigger() const
{
    if (!check())
        return false;

    scentering.emit(d_pos);

    return true;
}
