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

#include <pglabel.h>
#include <pgbutton.h>
#include "d_buy_production.h"
#include "armysetlist.h"
#include "city.h"
#include "player.h"
#include "GraphicsCache.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
#define debug(x)

D_Buy_Production::D_Buy_Production(City* city, bool advanced, 
                                   PG_Widget* parent, PG_Rect rect)
    :PG_Window(parent, rect, _("Buy Production"),PG_Window::MODAL),
    d_city(city), d_chosenArmy(-1)
{
    const Armysetlist* al = Armysetlist::getInstance();
    
    if (advanced)
        d_armyset = city->getPlayer()->getArmyset();
    else
        d_armyset = al->getStandardId();
    d_gold = city->getPlayer()->getGold();

    unsigned int ci;        // current index (== current index in the armyset)

    for (unsigned int i = 0; i <= (al->getSize(d_armyset)/5) ; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            ci = i*5 + j;

            if (ci >= al->getSize(d_armyset))
                continue;
            
            SDL_Surface* armyPic = GraphicsCache::getInstance()->getArmyPic(
                                    d_armyset, ci, d_city->getPlayer(),1,NULL);
            d_b_production[ci] = new PG_Button(this, PG_Rect(20 + j*60, i*60 + 30, 60, 60),"",ci);
            d_b_production[ci]->SetToggle(true);
            d_b_production[ci]->SetIcon(armyPic, 0, 0);
            d_b_production[ci]->sigClick.connect(slot(*this, &D_Buy_Production::b_productionClicked));
        }
    }

    d_l_production = new PG_Label(this, PG_Rect(20, 180, 190, 20), _("Name:"));
    d_l_duration = new PG_Label(this, PG_Rect(20, 200, 190, 20), _("Duration:"));
    d_l_upkeep = new PG_Label(this, PG_Rect(20, 220, 190, 20), _("Upkeep:"));
    d_l_moves = new PG_Label(this, PG_Rect(20, 240, 190, 20), _("Moves:"));
    d_l_productionCost = new PG_Label(this, PG_Rect(20, 260, 180, 20), _("Production cost:"));
    
    d_l_strength = new PG_Label(this, PG_Rect(220, 180, 140, 20), _("Strength:"));
    d_l_ranged = new PG_Label(this, PG_Rect(220, 200, 140, 20), _("Ranged:"));
    d_l_shots = new PG_Label(this, PG_Rect(220, 220, 140, 20), _("Shots:"));
    d_l_defense = new PG_Label(this, PG_Rect(220, 240, 140, 20), _("Defense:"));
    d_l_hp = new PG_Label(this, PG_Rect(220, 260, 140, 20), _("Hitpoints:"));
    
    d_b_buy = new PG_Button(this, PG_Rect(350, 30, 80, 30), _("Buy"),11);
    d_b_cancel = new PG_Button(this, PG_Rect(350, 70, 80, 30), _("Cancel"),11);

    d_b_buy->EnableReceiver(false);
    d_b_buy->sigClick.connect(slot(*this, &D_Buy_Production::b_buyClicked));
    d_b_cancel->sigClick.connect(slot(*this, &D_Buy_Production::b_cancelClicked));
}

D_Buy_Production::~D_Buy_Production()
{
    delete d_l_moves;
    delete d_l_upkeep;
    delete d_l_production;
    delete d_l_duration;
    delete d_l_strength;
    delete d_l_ranged;
    delete d_l_shots;
    delete d_l_defense;
    delete d_l_hp;
    delete d_l_productionCost;
    delete d_b_buy;
    delete d_b_cancel;

    const Armysetlist* al = Armysetlist::getInstance();
    unsigned int ci;        // current index (== current index in the armyset)

    for (unsigned int i = 0; i <= (al->getSize(d_armyset)/5) ; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            ci = i*5 + j;

            if (ci >= al->getSize(d_armyset))
                continue;

            if (d_b_production[ci]!=NULL) 
            {
  	       debug("pointer buyprod=" << d_b_production[ci])
               delete d_b_production[ci];
	    }
	}
    }
}

bool D_Buy_Production::b_buyClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool D_Buy_Production::b_cancelClicked(PG_Button* btn)
{
    d_chosenArmy = -1;
    QuitModal();
    return true;
}

bool D_Buy_Production::b_productionClicked(PG_Button* btn)
{
    int id = btn->GetID();
    d_chosenArmy = id;

    const Armysetlist* al = Armysetlist::getInstance();
    // deselect all other buttons
    for (unsigned int i = 0; i < al->getSize(d_armyset); i++)
    {
        if ((int)i != id)
            d_b_production[i]->SetPressed(false);
    }

    // selected armytype
    const Army* a = al->getArmy(d_armyset, d_chosenArmy);
    string name = "Name: " + a->getName();

    d_l_production->SetText(name.c_str());
    char buffer[41]; buffer[40] = '\0';
    snprintf(buffer, 40, _("Duration: %i"), a->getProduction());
    d_l_duration->SetText(buffer);
    snprintf(buffer, 40, _("Strength: %i"), a->getStat(Army::STRENGTH, false));
    d_l_strength->SetText(buffer);
    snprintf(buffer, 40, _("Ranged: %i"), a->getStat(Army::RANGED, false));
    d_l_ranged->SetText(buffer);
    snprintf(buffer, 40, _("Shots: %i"), a->getStat(Army::SHOTS, false));
    d_l_shots->SetText(buffer);
    snprintf(buffer, 40, _("Defense: %i"), a->getStat(Army::DEFENSE, false));
    d_l_defense->SetText(buffer);
    snprintf(buffer, 40, _("Hitpoints: %i"), a->getStat(Army::HP, false));
    d_l_hp->SetText(buffer);
    snprintf(buffer, 40, _("Moves: %i"), a->getStat(Army::MOVES, false));
    d_l_moves->SetText(buffer);
    snprintf(buffer, 40, _("Upkeep: %i"), a->getUpkeep());
    d_l_upkeep->SetText(buffer);
    snprintf(buffer, 40, _("Production cost: %i"), a->getProductionCost());
    d_l_productionCost->SetText(buffer);

    // enough money to buy this production-type or have it already?
    if ((int) a->getProductionCost() > d_gold ||
        d_city->hasProduction(d_chosenArmy, d_armyset))
    {
        d_b_buy->EnableReceiver(false);
    }
    else
    {
        // allow to press buy button
        d_b_buy->EnableReceiver(true);
    }
    return true;
}
