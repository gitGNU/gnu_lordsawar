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

#include "stackinfo.h"
#include <algorithm>
#include "army.h"
#include "ArmyInfo.h"
#include "stacklist.h"
#include "playerlist.h"
#include "stack.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Stackinfo::Stackinfo(PG_Widget* parent, Rectangle rect)
    :PG_ThemeWidget(parent, rect)
{
}

Stackinfo::~Stackinfo()
{
    for (std::vector<ArmyInfo*>::iterator it = d_armyinfovector.begin();
         it != d_armyinfovector.end(); ++it)
    {
        delete (*it);
    }
}

void Stackinfo::readData()
{
    debug("readData()");

    Stack* stack = Playerlist::getActiveplayer()->getActivestack();

    // free ArmyInfo's memory
    for (std::vector<ArmyInfo*>::iterator it = d_armyinfovector.begin();
         it != d_armyinfovector.end(); ++it)
    {
        delete (*it);
    }
    d_armyinfovector.clear();

    if (!stack)
        return;
    
    int i = 0;
    int selected = 0;
    int last_selected_army = 0;
    for (Stack::iterator it = stack->begin(); it != stack->end(); ++it, ++i)
    {
        d_armyinfovector.push_back(new ArmyInfo(*it, true, this, Rectangle(i*60, 0, 58, 86)));
                if ((*it)->isGrouped())
                {
                   last_selected_army = i;
                   selected++;
                }
    }

        // if there is only 1 selected army in the stack don't allow deselecting it
        if (selected == 1) ArmyInfo::setPressable(d_armyinfovector[last_selected_army], false);
        else
    {
        for_each(d_armyinfovector.begin(), d_armyinfovector.end(), ArmyInfo::setPressable);
    }
}

// End of file
