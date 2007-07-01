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

#include <sstream>
#include <map>
#include "Item.h"
#include "Itemlist.h"
#include "File.h"
#include "counter.h"

using namespace std;

Item::Item(XML_Helper* helper)
{
    int i;
    
    // Loading of items is a bit complicated, so i'd better loose some words.
    // In general, items can be loaded from the items description file or
    // from a savegame. They both differ a bit, more on that when we encounter
    // such a situation. First, let us deal with the common things.

    helper->getData(d_type, "type");
    
    // Loading the bonus and the values is a bit tricky
    int num = 0;
    Army::Stat bonus;
    std::string sbonus, svalue;
    std::stringstream isbonus, isvalue;
    
    helper->getData(num, "num_bonus");
    helper->getData(sbonus, "bonus");
    helper->getData(svalue, "value");
    
    isbonus.str(sbonus);
    isvalue.str(svalue);

    for (int pos = 0; pos < num; pos++)
    {
        isbonus >> i;
        bonus = static_cast<Army::Stat>(i);
        isvalue >> i;
        d_bonus[bonus] = i;
    }
        
    
    helper->getData(d_name, "name");
    helper->getData(d_plantable, "plantable");

    // Now come the differences. Items in the game have an id, other items
    // just the dummy id "0"
    helper->getData(d_id, "id");

}

Item::Item(Uint32 type, std::string name, bool plantable)
{
  d_type = type;
  d_name = name;
  d_plantable = plantable;
  d_id = fl_counter->getNextId();
}

Item::Item(const Item& orig)
    :d_type(orig.d_type), d_bonus(orig.d_bonus),
    d_name(orig.d_name), d_plantable(orig.d_plantable)
{
    // Some things we don't copy from the template; we rather get an own ID
    d_id = fl_counter->getNextId();
}

Item::~Item()
{
}

bool Item::save(XML_Helper* helper) const
{
    bool retval = true;
    
    // A template is never saved, so we assume this class is a real-life item
    retval &= helper->openTag("item");
    retval &= helper->saveData("type", d_type);
    retval &= helper->saveData("name", d_name);
    retval &= helper->saveData("plantable", d_plantable);
    retval &= helper->saveData("id", d_id);
    retval &= helper->saveData("image", std::string(""));

    // Last, save the boni of the item. We put all of them in a string separated
    // by spaces
    std::stringstream sbonus, svalue;
    for (map<Army::Stat,int>::const_iterator it = d_bonus.begin(); it != d_bonus.end(); it++)
    {
        sbonus << (*it).first << " ";
        svalue << (*it).second << " ";
    }
    retval &= helper->saveData("num_bonus", d_bonus.size());
    retval &= helper->saveData("bonus", sbonus.str());
    retval &= helper->saveData("value", svalue.str());
    
    retval &= helper->closeTag();
    
    return retval;
}

bool Item::getBonus(Army::Stat bonus) const
{
    map<Army::Stat,int>::const_iterator it;
    for (it = d_bonus.begin(); it != d_bonus.end(); it++)
        if ((*it).first == bonus)
            return true;

    return false;
}

int Item::getValue(Army::Stat bonus)
{
    if (getBonus(bonus))
        return d_bonus[bonus];

    return 0;
}

void Item::setBonus(Army::Stat bonus, int amount)
{
  d_bonus[bonus] = amount;
}
