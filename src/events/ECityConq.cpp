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

#include "ECityConq.h"
#include "../citylist.h"
#include "../playerlist.h"


ECityConq::ECityConq(Uint32 city)
    :Event(CITYCONQUERED), d_city(city)
{
}

ECityConq::ECityConq(XML_Helper* helper)
    :Event(helper)
{
    helper->getData(d_city, "city");
    d_type = CITYCONQUERED;
}

ECityConq::~ECityConq()
{
}

bool ECityConq::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("event");
    retval &= helper->saveData("city", d_city);
    retval &= Event::save(helper);
    retval &= helper->closeTag();
    
    return retval;
}

void ECityConq::init()
{
    Playerlist* plist = Playerlist::getInstance();
    for (Playerlist::iterator it = plist->begin(); it != plist->end(); it++)
    {
        if ((*it) == plist->getNeutral())
            continue;
        (*it)->soccupyingCity.connect(SigC::slot((*this), &ECityConq::trigger));
    }
}

void ECityConq::trigger(City* city)
{
    if (city->getId() != d_city)
        return;

    raise();
}
