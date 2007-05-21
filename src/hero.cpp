//  This program is free software; you can redistribute it and/or modify
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


#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <sigc++/functors/mem_fun.h>

#include "hero.h"
#include "stacklist.h"
#include "templelist.h"

using namespace std;

Hero::Hero(const Army& a, std::string name, Player *owner)
 : Army (a, owner)
{
    d_name = name;
}

Hero::Hero(Hero& h)
    :Army(h, h.d_player)
{
    std::list<Item*>::iterator it;

    // copy the backpack of the other hero
    for (it = h.d_backpack.begin(); it != h.d_backpack.end(); it++)
    {
        Item* item = new Item(**it);
        d_backpack.push_back(item);
    }

    // copy the equipment of the other hero
    for (it = h.d_equipment.begin(); it != h.d_equipment.end(); it++)
    {
        Item* item = new Item(**it);
        d_equipment.push_back(item);
    }
}

Hero::Hero(XML_Helper* helper)
    :Army(helper)
{
    int i;
    helper->getData(d_name, "name");
    helper->getData(i, "gender");
    d_gender = static_cast<Army::Gender>(i);

    helper->registerTag("backpack", sigc::mem_fun(*this, &Hero::loadItems));
    helper->registerTag("equipment", sigc::mem_fun(*this, &Hero::loadItems));
    helper->registerTag("item", sigc::mem_fun(*this, &Hero::loadItems));

}


Hero::~Hero()
{
    //clear the backpack and the equipment
    while (!d_backpack.empty())
    {
        delete (*d_backpack.begin());
        d_backpack.erase(d_backpack.begin());
    }

    while (!d_equipment.empty())
    {
        delete (*d_equipment.begin());
        d_equipment.erase(d_equipment.begin());
    }
}

bool Hero::save(XML_Helper* helper) const
{
    bool retval = true;
    std::list<Item*>::const_iterator it;

    retval &= helper->openTag("hero");
    retval &= helper->saveData("name", d_name);
    // Save the sex as well, whatever this is good for...
    retval &= helper->saveData("gender", d_gender);

    retval &= saveData(helper);

    // Now save the equipment and the backpack
    retval &= helper->openTag("backpack");
    for (it = d_backpack.begin(); it != d_backpack.end(); it++)
        retval &= (*it)->save(helper);
    retval &= helper->closeTag();

    retval &= helper->openTag("equipment");
    for (it = d_equipment.begin(); it != d_equipment.end(); it++)
        retval &= (*it)->save(helper);
    retval &= helper->closeTag();
    

    retval &= helper->closeTag();

    return retval;
}

bool Hero::loadItems(std::string tag, XML_Helper* helper)
{
    // determines whether an item is put in the backpack or to the equipment
    static bool equipment = false;

    if (tag == "backpack")
    {
        equipment = false;
        return true;
    }

    if (tag == "equipment")
    {
        equipment = true;
        return true;
    }

    if (tag == "item")
    {
        Item* item = new Item(helper);
        if (equipment)
            d_equipment.push_back(item);
        else
            d_backpack.push_back(item);
    }
    
    return true;
}

Uint32 Hero::getStat(Stat stat, bool modified) const
{
    Uint32 value = Army::getStat(stat, modified);

    if (!modified)
        return value;

    // Add item boni
    // since (move|army) boni are unsigned integers to be or'ed, deal with
    // them separately
    if (stat == Army::MOVE_BONUS || stat == Army::ARMY_BONUS)
    {
        Uint32 bonus = 0;
        
        std::list<Item*>::const_iterator it;
        for (it = d_equipment.begin(); it != d_equipment.end(); it++)
            if ((*it)->getBonus(stat))
                bonus |= (*it)->getValue(Army::STRENGTH);

        return (value | bonus);
    }

    int bonus = 0;
    std::list<Item*>::const_iterator it;
    for (it = d_equipment.begin(); it != d_equipment.end(); it++)
        if ((*it)->getBonus(stat))
            bonus += (*it)->getValue(stat);

    return (value+bonus);
}

bool Hero::addToBackpack(Item* item, int position)
{
    std::list<Item*>::iterator it = d_backpack.begin();
    for (; position > 0; position--, it++);
    
    d_backpack.insert(it, item);
    return true;
}

bool Hero::addToEquipment(Item* item)
{
    //First, check if it is allowed to take the item at all
    Uint32 newtype = item->getType();

    std::list<Item*>::iterator it;
    for (it = d_equipment.begin(); it != d_equipment.end(); it++)
        if ((*it)->getType() == newtype)
            return false;
    
    //! OK, we don't have such an item already, so equip it
    d_equipment.push_back(item);
    return true;
}

bool Hero::removeFromBackpack(Item* item)
{
    std::list<Item*>::iterator it;
    for (it = d_backpack.begin(); it != d_backpack.end(); it++)
        if ((*it) == item)
        {
            d_backpack.erase(it);
            return true;
        }

    return false;
}

bool Hero::removeFromEquipment(Item* item)
{
    std::list<Item*>::iterator it;
    for (it = d_equipment.begin(); it != d_equipment.end(); it++)
        if ((*it) == item)
        {
            d_equipment.erase(it);
            return true;
        }

    return false;
}

