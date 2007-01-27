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

#include <pgdropdown.h>
#include <pglistboxbaseitem.h>
#include <pgapplication.h>

#include "ArmyDialog.h"
#include "../File.h"
#include "../defs.h"
#include "../armysetlist.h"
#include "../GraphicsCache.h"
#include "../playerlist.h"


E_ArmyDialog::E_ArmyDialog(PG_Widget* parent, PG_Rect rect, unsigned int set)
    :PG_Window(parent, rect, _("ArmySelection"), PG_Window::MODAL), d_success(true)
{
    // only if no armyset is specified show the dropdown for the selection
    if (set == 0)
    {
        PG_DropDown* drop = new PG_DropDown(this, PG_Rect(my_width/2-80, 40, 160, 20));
        drop->sigSelectItem.connect(slot(*this, &E_ArmyDialog::armysetSelected));
        
        const Armysetlist* al = Armysetlist::getInstance();
        std::vector<Uint32> armysets = al->getArmysets(true);
        for (Uint32 i = 0; i < armysets.size(); i++)
            drop->AddItem(al->getName(armysets[i]).c_str());
        
        drop->SetText(al->getName(armysets[0]).c_str());
        set = armysets[0];
    }

    // set up all the labels
    PG_Rect r(40, my_height - 180, 80, 20);
    new PG_Label(this, r, _("Name:"));
    r.y += 20;
    new PG_Label(this, r, _("Duration:"));
    r.y += 20;
    new PG_Label(this, r, _("Cost:"));
    r.y += 20;
    new PG_Label(this, r, _("Upkeep:"));
    r.y += 20;
    new PG_Label(this, r, _("Strength:"));
    r.y += 20;
    new PG_Label(this, r, _("Ranged:"));

    r.SetRect(240, my_height - 180, 90, 20);
    new PG_Label(this, r, _("Shots:"));
    r.y += 20;
    new PG_Label(this, r, _("Defense:"));
    r.y += 20;
    new PG_Label(this, r, _("HP:"));
    r.y += 20;
    new PG_Label(this, r, _("Moves:"));
    r.y += 20;
    new PG_Label(this, r, _("Vitality:"));
    r.y += 20;
    new PG_Label(this, r, _("Sight:"));
    
    r.SetRect(120, my_height - 180, 110, 20);
    d_l_name = new PG_Label(this, r);
    r.y += 20;
    d_l_production = new PG_Label(this, r);
    r.y += 20;
    d_l_cost = new PG_Label(this, r);
    r.y += 20;
    d_l_upkeep = new PG_Label(this, r);
    r.y += 20;
    d_l_strength = new PG_Label(this, r);
    r.y += 20;
    d_l_ranged = new PG_Label(this, r);

    r.SetRect(340, my_height - 180, 50, 20);
    d_l_shots = new PG_Label(this, r);
    r.y += 20;
    d_l_defense = new PG_Label(this, r);
    r.y += 20;
    d_l_hp = new PG_Label(this, r);
    r.y += 20;
    d_l_moves = new PG_Label(this, r);
    r.y += 20;
    d_l_vitality = new PG_Label(this, r);
    r.y += 20;
    d_l_sight = new PG_Label(this, r);

    // load the default (whatever that means) set
    fillData(set);

    // place the OK button
    PG_Button* btn = new PG_Button(this, PG_Rect(my_width/2-100, my_height-40, 90, 30), _("OK"));
    btn->sigClick.connect(slot(*this, &E_ArmyDialog::okClicked));

    // place the cancel button
    btn = new PG_Button(this, PG_Rect(my_width/2 + 40, my_height-40, 90, 30), _("Cancel"));
    btn->sigClick.connect(slot(*this, &E_ArmyDialog::cancelClicked));
}

E_ArmyDialog::~E_ArmyDialog()
{
}

Army* E_ArmyDialog::getSelection()
{
    return new Army(*(Armysetlist::getInstance()->getArmy(d_set, d_index)), 0);
}

bool E_ArmyDialog::armysetSelected(PG_ListBoxBaseItem* item)
{
    // find the id of the selected armyset and fill the data
    std::string id(item->GetText());
    const Armysetlist* al = Armysetlist::getInstance();
    std::vector<Uint32> sets = al->getArmysets(true);

    for (Uint32 i = 0; i < sets.size(); i++)
        if (al->getName(sets[i]) == id)
            fillData(sets[i]);

    return true;
}

bool E_ArmyDialog::armySelected(PG_Button* btn)
{
    // disable automatic updating a bit (speed)
    PG_Application::SetBulkMode(true);
    
    for (unsigned int i = 0; i < d_armies.size(); i++)
        d_armies[i]->SetPressed(false);

    btn->SetPressed(true);

    // graphical stuff finished, so enable updating again
    PG_Application::SetBulkMode(false);
    
    // remember: the id of the button equals the id of the army in the set
    d_index = btn->GetID();

    updateStats(d_index);
    Update();

    return true;
}

bool E_ArmyDialog::okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool E_ArmyDialog::cancelClicked(PG_Button* btn)
{
    d_success = false;
    QuitModal();
    return true;
}

void E_ArmyDialog::fillData(Uint32 armyset)
{
    // we don't want to update the screen every time we remove a button
    PG_Application::SetBulkMode(true);
    
    // first, clear the now useless buttons for the army icons
    while (!d_armies.empty())
    {
        delete *(d_armies.begin());
        d_armies.erase(d_armies.begin());
    }
    
    d_set = armyset;
    const Armysetlist* al = Armysetlist::getInstance();
    d_armies.resize(al->getSize(d_set));
    
    for (unsigned int i = 0; i < al->getSize(d_set); i++)
    {
        // a typical armyset should not have more than 15 armies, so arrange
        // them neatly on up to two rows; note that the id of the button equals the
        // id of the army in the armyset!
        PG_Rect r( 20 + (i%8)*44, 80 + (i/8)*44, 44, 44);
        d_armies[i] = new PG_Button(this, r, "", i);

        SDL_Surface* pic = GraphicsCache::getInstance()->getArmyPic(d_set, i,
                                        Playerlist::getInstance()->getNeutral(),
                                        1, 0);
        d_armies[i]->SetIcon(pic);
        d_armies[i]->sigClick.connect(slot(*this, &E_ArmyDialog::armySelected));
        d_armies[i]->SetToggle(true);
        d_armies[i]->Show();
    }

    // we are finished with the graphics
    PG_Application::SetBulkMode(false);
        
    // finally, select the first army
    armySelected(d_armies[0]);
    Update();
}

void E_ArmyDialog::updateStats(int index)
{
    d_index = index;
    const Army* a = Armysetlist::getInstance()->getArmy(d_set, index);

    char buffer[51]; buffer[50]='\0';

    // don't update the graphics every time we change a label
    PG_Application::SetBulkMode(true);

    // now fill all the data
    d_l_name->SetText(a->getName().c_str());

    snprintf(buffer, 50, "%i", a->getProduction());
    d_l_production->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getProductionCost());
    d_l_cost->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getUpkeep());
    d_l_upkeep->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getStat(Army::STRENGTH, false));
    d_l_strength->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getStat(Army::RANGED, false));
    d_l_ranged->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getStat(Army::SHOTS, false));
    d_l_shots->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getStat(Army::DEFENSE, false));
    d_l_defense->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getStat(Army::HP, false));
    d_l_hp->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getStat(Army::MOVES, false));
    d_l_moves->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getStat(Army::VITALITY, false));
    d_l_vitality->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getStat(Army::SIGHT, false));
    d_l_sight->SetText(buffer);

    // don't forget to reenable the automatic update
    PG_Application::SetBulkMode(false);
    Update();
}
