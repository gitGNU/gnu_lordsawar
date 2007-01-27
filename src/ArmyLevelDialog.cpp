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

#include <stdio.h>
#include <pgapplication.h>
#include "ArmyLevelDialog.h"
#include "defs.h"

ArmyLevelDialog::ArmyLevelDialog(Army* army, PG_Widget* parent, PG_Rect rect)
    :PG_Window(parent, rect, _("Your unit advanced a level!"), PG_Window::MODAL), d_army(army),
    d_l_ranged(0), d_result(Army::STRENGTH)
{
    char buffer[31];
    buffer[30] = '\0';

    //I assume a size of 400*300 pixels here

    new PG_Label(this, PG_Rect(15, 220, 270, 20),_("Choose a characteristic to raise."));
    d_b_ok = new PG_Button(this, PG_Rect(300, 210, 115, 30),
                            _("OK"),1);
    d_b_ok->sigClick.connect(slot(*this, &ArmyLevelDialog::b_ok_clicked));

    //the army pic
    d_b_pic = new PG_Button(this, PG_Rect(15, 40, 50, 50), "",2);
    d_b_pic->EnableReceiver(false);
    d_b_pic->SetIcon(d_army->getPixmap(), 0, 0);

    //the name of the army
    new PG_Label(this, PG_Rect(15, 100, 200, 20), d_army->getName().c_str());

    snprintf(buffer, 30, _("xp: %.2f"), d_army->getXP());
    new PG_Label(this, PG_Rect(15, 130, 95, 20), buffer);

    snprintf(buffer, 30, "%i", d_army->getLevel());
    new PG_Label(this, PG_Rect(15, 150, 45, 20),_("level: "));
    PG_Label* l = new PG_Label(this, PG_Rect(65, 150, 30, 20), buffer);
    l->SetFontColor(PG_Color(50, 255, 50));

    PG_Rect r(220, 60, 40, 20);
    PG_RadioButton* rb1, *rb;
    
    rb1 = new PG_RadioButton(this, r, "", NULL, Army::STRENGTH);
    rb1->sigClick.connect(slot(*this, &ArmyLevelDialog::stat_chosen));
    rb1->SetPressed();
    r.y += 20;
    
    // only allow to increase ranged attack if the unit already has one
    if (d_army->getStat(Army::RANGED, false) > 0)
    {
        rb = new PG_RadioButton(this, r, "", rb1, Army::RANGED);
        rb->sigClick.connect(slot(*this, &ArmyLevelDialog::stat_chosen));
        r.y += 20;
    }
    
    rb = new PG_RadioButton(this, r, "", rb1, Army::DEFENSE);
    rb->sigClick.connect(slot(*this, &ArmyLevelDialog::stat_chosen));
    r.y += 20;
    rb = new PG_RadioButton(this, r, "", rb1, Army::VITALITY);
    rb->sigClick.connect(slot(*this, &ArmyLevelDialog::stat_chosen));
    r.y += 20;
    rb = new PG_RadioButton(this, r, "", rb1, Army::SIGHT);
    rb->sigClick.connect(slot(*this, &ArmyLevelDialog::stat_chosen));
    r.y += 20;
    rb = new PG_RadioButton(this, r, "", rb1, Army::HP);
    rb->sigClick.connect(slot(*this, &ArmyLevelDialog::stat_chosen));
    r.y += 20;
    rb = new PG_RadioButton(this, r, "", rb1, Army::MOVES);
    rb->sigClick.connect(slot(*this, &ArmyLevelDialog::stat_chosen));
    r.y += 20;


    //the head of the table for the single values
    l = new PG_Label(this, PG_Rect(140, 40, 110, 20), _("Stats"));
    l->SetFontColor(PG_Color(0, 0, 0));

    //and the values themselves
    r.SetRect(140, 60, 95, 20);

    new PG_Label(this, r, _("strength"));
    r.y += 20;
    if (d_army->getStat(Army::RANGED, false) > 0)
        new PG_Label(this, r, _("ranged"));
    else
        r.y -= 20;
    r.y += 20;
    new PG_Label(this, r, _("defense"));
    r.y += 20;
    new PG_Label(this, r, _("vitality"));
    r.y += 20;
    new PG_Label(this, r, _("sight"));
    r.y += 20;
    new PG_Label(this, r, _("hp"));
    r.y += 20;
    new PG_Label(this, r, _("moves"));
    
    
    r.SetRect(280, 60, 45, 20);

    d_l_strength = new PG_Label(this, r);
    r.y += 20;
    if (d_army->getStat(Army::RANGED, false) > 0)
    {
        d_l_ranged = new PG_Label(this, r);
        r.y += 20;
    }
    d_l_defense = new PG_Label(this, r);
    r.y += 20;
    d_l_vitality = new PG_Label(this, r);
    r.y += 20;
    d_l_sight = new PG_Label(this, r);
    r.y += 20;
    d_l_hp = new PG_Label(this, r);
    r.y += 20;
    d_l_moves = new PG_Label(this, r);

    initStats();
}

ArmyLevelDialog::~ArmyLevelDialog()
{
}

void ArmyLevelDialog::initStats()
{
    char buffer[31];
    buffer[30] = '\0';

    // First, display all the unchanged values
    //
    // Since blitting will else become _very_ slow, enable bulk updating.
    // In our case, no blits are performed, then
    PG_Application::SetBulkMode(true);

    snprintf(buffer, 30, "%i", d_army->getStat(Army::STRENGTH, false));
    d_l_strength->SetText(buffer);
    d_l_strength->SetFontColor(PG_Color(255, 255, 255));

    if (d_l_ranged)
    {
        snprintf(buffer, 30, "%i", d_army->getStat(Army::RANGED, false));
        d_l_ranged->SetText(buffer);
        d_l_ranged->SetFontColor(PG_Color(255, 255, 255));
    }

    snprintf(buffer, 30, "%i", d_army->getStat(Army::DEFENSE, false));
    d_l_defense->SetText(buffer);
    d_l_defense->SetFontColor(PG_Color(255, 255, 255));

    snprintf(buffer, 30, "%i", d_army->getStat(Army::VITALITY, false));
    d_l_vitality->SetText(buffer);
    d_l_vitality->SetFontColor(PG_Color(255, 255, 255));

    snprintf(buffer, 30, "%i", d_army->getStat(Army::SIGHT, false));
    d_l_sight->SetText(buffer);
    d_l_sight->SetFontColor(PG_Color(255, 255, 255));

    snprintf(buffer, 30, "%i", d_army->getStat(Army::HP, false));
    d_l_hp->SetText(buffer);
    d_l_hp->SetFontColor(PG_Color(255, 255, 255));

    snprintf(buffer, 30, "%i", d_army->getStat(Army::MOVES, false));
    d_l_moves->SetText(buffer);
    d_l_moves->SetFontColor(PG_Color(255, 255, 255));

    // Now find out which button is pressed and change the label of
    // the appropriate stat
    int diff = d_army->gainLevel(d_result, true);
    switch (d_result)
    {
        case Army::STRENGTH:
            snprintf(buffer, 30, "%i", d_army->getStat(Army::STRENGTH, false) + diff);
            d_l_strength->SetText(buffer);
            d_l_strength->SetFontColor(PG_Color(50, 255, 50));
            break;
        case Army::RANGED:
            snprintf(buffer, 30, "%i", d_army->getStat(Army::RANGED, false) + diff);
            d_l_ranged->SetText(buffer);
            d_l_ranged->SetFontColor(PG_Color(50, 255, 50));
            break;
        case Army::DEFENSE:
            snprintf(buffer, 30, "%i", d_army->getStat(Army::DEFENSE, false) + diff);
            d_l_defense->SetText(buffer);
            d_l_defense->SetFontColor(PG_Color(50, 255, 50));
            break;
        case Army::VITALITY:
            snprintf(buffer, 30, "%i", d_army->getStat(Army::VITALITY, false) + diff);
            d_l_vitality->SetText(buffer);
            d_l_vitality->SetFontColor(PG_Color(50, 255, 50));
            break;
        case Army::SIGHT:
            snprintf(buffer, 30, "%i", d_army->getStat(Army::SIGHT, false) + diff);
            d_l_sight->SetText(buffer);
            d_l_sight->SetFontColor(PG_Color(50, 255, 50));
            break;
        case Army::HP:
            snprintf(buffer, 30, "%i", d_army->getStat(Army::HP, false) + diff);
            d_l_hp->SetText(buffer);
            d_l_hp->SetFontColor(PG_Color(50, 255, 50));
            break;
        case Army::MOVES:
            snprintf(buffer, 30, "%i", d_army->getStat(Army::MOVES, false) + diff);
            d_l_moves->SetText(buffer);
            d_l_moves->SetFontColor(PG_Color(50, 255, 50));
            break;
        default:
            break;
    }

    // Now disable bulk mode to have our changes take effect
    PG_Application::SetBulkMode(false);
    Update();
}

bool ArmyLevelDialog::stat_chosen(PG_RadioButton* widget, bool state)
{
    d_result = static_cast<Army::Stat>(widget->GetID());
    initStats();
    return true;
}

bool ArmyLevelDialog::b_ok_clicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool ArmyLevelDialog::eventKeyDown(const SDL_KeyboardEvent* key)
{
    switch(key->keysym.sym)
    {
        case SDLK_RETURN:
            b_ok_clicked(0);
            break;
        default:
            break;
    }
    return true;
}
