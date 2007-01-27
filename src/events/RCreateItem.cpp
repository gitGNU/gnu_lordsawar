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

#include "RCreateItem.h"
#include "../Itemlist.h"
#include "../GameMap.h"

RCreateItem::RCreateItem(Uint32 index, PG_Point pos)
    :Reaction(CREATEITEM), d_item(index), d_pos(pos)
{
}

RCreateItem::RCreateItem(XML_Helper* helper)
    :Reaction(helper)
{
    d_type = CREATEITEM;
    helper->getData(d_item, "item");
    
    int data;
    helper->getData(data, "x"); d_pos.x = data;
    helper->getData(data, "y"); d_pos.y = data;
}

RCreateItem::~RCreateItem()
{
}

bool RCreateItem::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag("reaction");
    retval &= helper->saveData("item", d_item);
    retval &= helper->saveData("x", d_pos.x);
    retval &= helper->saveData("y", d_pos.y);
    retval &= Reaction::save(helper);
    retval &= helper->closeTag();

    return retval;
}

bool RCreateItem::trigger() const
{
    if (!check())
        return false;

    //Get us an item, first
    Item* item = getItem();

    // append the item to the maptile at the correct position
    GameMap::getInstance()->getTile(d_pos.x, d_pos.y)->addItem(item);
    
    return true;
}

Item* RCreateItem::getItem() const
{
    // We copy an item from the itemlist; see there to find out how this works
    Item* retval = new Item(*(*Itemlist::getInstance())[d_item]);
    
    return retval;
}
