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

#include <list>
#include "player_preferences.h"
#include "armysetlist.h"
#include "defs.h"

using namespace std;

Player_preferences::Player_preferences(Type type, string name,
        PG_Widget* parent, PG_Rect rect)
    :PG_Widget(parent, rect)
{   
    d_cb_type = new PG_DropDown(this,PG_Rect(0, 0, 110, 25),1);
    d_cb_type->AddItem(_("None"));
    d_cb_type->AddItem(_("Human"));
    d_cb_type->AddItem(_("Easy"));
    d_cb_type->AddItem(_("Smart"));

    d_e_name = new PG_LineEdit(this, PG_Rect(120, 0, 100, 25));
    d_e_name->SetText(name.c_str());
    d_cb_armyset = new PG_DropDown(this, PG_Rect(230, 0, 160, 25),2);

    // fill the armyset combobox with names
    std::vector<unsigned int> sets = Armysetlist::getInstance()->getArmysets();
    for (unsigned int i = 0; i < sets.size(); i++)
        d_cb_armyset->AddItem(Armysetlist::getInstance()->getName(sets[i]).c_str());

    d_cb_armyset->SelectFirstItem();
    d_cb_armyset->SetEditable(false);

    if (type == HUMAN)
    {
        d_cb_type->SetText(_("Human"));
    }
    else if (type == HUMAN_OR_COMPUTER)
    {
        d_cb_type->SetText(_("Smart"));
    }
    else
    {
        d_cb_type->SetText(_("None"));
        d_e_name->Hide();
        d_cb_armyset->Hide();
    }

    d_cb_type->sigSelectItem.connect(slot(*this, &Player_preferences::b_typeChanged));
}

Player_preferences::~Player_preferences()
{
    delete d_cb_type;
    delete d_e_name;
    delete d_cb_armyset;
}

bool Player_preferences::b_typeChanged(PG_ListBoxBaseItem* cb)
{
    std::string text;
    text = std::string(d_cb_type->GetText());
    if(text == _("None"))
    {
        d_e_name->Hide();
        d_cb_armyset->Hide();
    }
    else
    {
        d_e_name->Show();
        d_cb_armyset->Show();
    }
    return true;
}

bool Player_preferences::isActive()
{
    return (std::string(d_cb_type->GetText()) != _("None"));
}

bool Player_preferences::isComputer()
{
    return (std::string(d_cb_type->GetText()) == _("Easy")) || (std::string(d_cb_type->GetText()) == _("Smart"));
}

bool Player_preferences::isEasy()
{
    return std::string(d_cb_type->GetText()) == _("Easy");
}

unsigned int Player_preferences::getArmyset()
{
    // we need to find the armyset's id from the name of the armyset
    std::string name(d_cb_armyset->GetText());
    const Armysetlist* al = Armysetlist::getInstance();

    std::vector<unsigned int> sets = al->getArmysets();
    for (unsigned int i = 0; i < sets.size(); i++)
        if (al->getName(sets[i]) == name)
        {
            return sets[i];
        }

    std::cerr <<_("User selected wrong armytype ????\n") <<std::flush;
    return 0;
}

// End of file
