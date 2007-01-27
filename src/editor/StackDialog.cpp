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

#include <pgmessagebox.h>
#include <pgapplication.h>
#include <stdlib.h> //for INT_MAX and snprintf

#include "StackDialog.h"
#include "ArmyDialog.h"
#include "../defs.h"
#include "../playerlist.h"
#include "../stacklist.h"

E_StackDialog::E_StackDialog(PG_Widget* parent, PG_Rect rect, Stack* s, bool player)
    :PG_Window(parent, rect, _("Modify stack"), PG_Window::MODAL),
    d_stack(s), d_selected(INT_MAX)
{
    char buffer[21]; buffer[20] = '\0';

    // I divide the dialog into three parts:
    // - on the right side are the buttons for OK + adding/removing armies
    // - on the upper left side, all armies in the stack are listed + 
    //   general information
    // - on the lower left side, the stats and the buttons for changing them
    //   are displayed

    // a) right side
    PG_Rect r(my_width - 115, my_height - 150, 90, 30);
    d_b_add = new PG_Button(this, r, _("Add"));
    d_b_add->sigClick.connect(slot(*this, &E_StackDialog::armyAdded));

    r.y += 50;
    d_b_remove = new PG_Button(this, r, _("Remove"));
    d_b_remove->sigClick.connect(slot(*this, &E_StackDialog::armyRemoved));

    r.y += 50;
    d_b_ok = new PG_Button(this, r, _("OK"));
    d_b_ok->sigClick.connect(slot(*this, &E_StackDialog::okClicked));

    // b) upper left side
    r.SetRect(30, 40, 50, 20);
    new PG_Label(this, r, _("ID:"));
    r.x += 60;
    snprintf(buffer, 20, "%i", s->getId());
    new PG_Label(this, r, buffer);
    
    if (player)
    {
        r.x += 100;
        r.w = 150;
        PG_DropDown* drop = new PG_DropDown(this, r);
        drop->SetText(s->getPlayer()->getName().c_str());
        Playerlist* pl = Playerlist::getInstance();
        for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
            drop->AddItem((*it)->getName().c_str(), static_cast<void*>(*it));
        drop->sigSelectItem.connect(slot(*this, &E_StackDialog::playerChanged));
    }
    
    r.SetRect(30, 80, 44, 44);
    for (int i = 0; i < 8; i++)
    {
        d_b_army[i] = new PG_Button(this, r);
        d_b_army[i]->SetToggle(true);
        d_b_army[i]->sigClick.connect(slot(*this, &E_StackDialog::armySelected));
        r.x += 44;
    }

    // c) lower left side (the worst part)
    // for each stat, we need a label for the stat description, a label for the
    // value and a + and - button; for simplicity, each button gets the ID
    // (STAT * 2 + {0 for +, 1 for -}), this way statChanged can check which
    // stat is changed and if it is raised or not

    // place the naming tags...
    r.SetRect(30, 150, 130, 20);
    new PG_Label(this, r, _("Name:"));
    
    r.y += 40;
    new PG_Label(this, r, _("Strength:"));
    r.y += 20;
    new PG_Label(this, r, _("Ranged:"));
    r.y += 20;
    new PG_Label(this, r, _("Shots:"));
    r.y += 20;
    new PG_Label(this, r, _("Defense:"));
    r.y += 20;
    new PG_Label(this, r, _("Hitpoints:"));
    r.y += 20;
    new PG_Label(this, r, _("Vitality:"));
    r.y += 20;
    new PG_Label(this, r, _("Sight:"));
    r.y += 20;
    new PG_Label(this, r, _("Moves:"));
    r.y += 30;
    new PG_Label(this, r, _("ID:"));

    // ...the value labels...
    r.SetRect(170, 150, 150, 20);
    d_l_name = new PG_Label(this, r);
    r.w = 40;

    r.y += 40;
    d_l_strength = new PG_Label(this, r);
    r.y += 20;
    d_l_ranged = new PG_Label(this, r);
    r.y += 20;
    d_l_shots = new PG_Label(this, r);
    r.y += 20;
    d_l_defense = new PG_Label(this, r);
    r.y += 20;
    d_l_hp = new PG_Label(this, r);
    r.y += 20;
    d_l_vitality = new PG_Label(this, r);
    r.y += 20;
    d_l_sight = new PG_Label(this, r);
    r.y += 20;
    d_l_moves = new PG_Label(this, r);
    r.y += 30;
    d_l_id = new PG_Label(this, r);

    // ...and the buttons for raising/lowering the value
    r.SetRect(220, 191, 8, 8);
    PG_Button* btn = 0;
    
    btn = new PG_Button(this, r, "+", STRENGTH*2);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));
    r.y += 10;
    btn = new PG_Button(this, r, "-", STRENGTH*2+1);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));

    r.y += 10;
    btn = new PG_Button(this, r, "+", RANGED*2);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));
    r.y += 10;
    btn = new PG_Button(this, r, "-", RANGED*2+1);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));
    
    r.y += 10;
    btn = new PG_Button(this, r, "+", SHOTS*2);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));
    r.y += 10;
    btn = new PG_Button(this, r, "-", SHOTS*2+1);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));
    
    r.y += 10;
    btn = new PG_Button(this, r, "+", DEFENSE*2);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));
    r.y += 10;
    btn = new PG_Button(this, r, "-", DEFENSE*2+1);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));

    r.y += 10;
    btn = new PG_Button(this, r, "+", HP*2);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));
    r.y += 10;
    btn = new PG_Button(this, r, "-", HP*2+1);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));

    r.y += 10;
    btn = new PG_Button(this, r, "+", VITALITY*2);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));
    r.y += 10;
    btn = new PG_Button(this, r, "-", VITALITY*2+1);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));

    r.y += 10;
    btn = new PG_Button(this, r, "+", SIGHT*2);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));
    r.y += 10;
    btn = new PG_Button(this, r, "-", SIGHT*2+1);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));

    r.y += 10;
    btn = new PG_Button(this, r, "+", MOVES*2);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));
    r.y += 10;
    btn = new PG_Button(this, r, "-", MOVES*2+1);
    btn->sigClick.connect(slot(*this, &E_StackDialog::statChanged));

    // now initialize the dialog; if the stack is not empty, we can just
    // select the first army, else the initialization has to be done manually.
    updatePics();
    checkButtons();
    updateStats();

}

E_StackDialog::~E_StackDialog()
{
    while (!d_cache.empty())
    {
        delete (*d_cache.begin());
        d_cache.erase(d_cache.begin());
    }
}

bool E_StackDialog::okClicked(PG_Button* btn)
{
    // ask the user if he really wants to finish if the stack is empty
    if (d_stack->empty())
    {
        PG_Rect r(my_width/2 - 150, my_height/2 - 100, 300, 200);
        PG_MessageBox mb(this, r, _("Warning"), _("If you leave the dialog, the "
                      "empty stack will be destroyed. Do you still want to quit?"),
                      PG_Rect(50, 160, 40, 20), _("Yes"),
                      PG_Rect(210, 160, 40, 20), _("No"));
        mb.Show();
        // cancelled
        if (mb.RunModal() == 2)
            return true;
        mb.Hide();
    }

    QuitModal();
    return true;
}

bool E_StackDialog::armyAdded(PG_Button* btn)
{
    // no more place, should never happen BTW
    if (d_stack->size() == 8)
        return true;

    // fire up an army dialog and add the resulting army to the stack
    E_ArmyDialog d(this, PG_Rect(0, 0, my_width, my_height));
    d.Show();
    d.RunModal();
    d.Hide();

    if (!d.getSuccess())
        return true;

    // add the army to the stack
    Army* a = d.getSelection();
    a->setPlayer(d_stack->getPlayer());
    d_stack->push_back(a);

    // update everything
    updatePics();
    checkButtons();
    
    return true;
}

bool E_StackDialog::armyRemoved(PG_Button* btn)
{
    // nothing to do, should also never happen
    if (d_stack->empty() || (d_selected == INT_MAX))
        return true;

    Stack::iterator it = d_stack->begin();
    for (unsigned int i = 0; i < d_selected; i++, it++);
    d_stack->flErase(it);

    if (d_selected >= d_stack->size())
        d_selected = INT_MAX;

    updatePics();
    updateStats();
    checkButtons();
    
    return true;
}

bool E_StackDialog::armySelected(PG_Button* btn)
{
    unsigned int i;
    for (i = 0; i < 8; i++)
        if (btn == d_b_army[i])
            break;

    // Just being paranoid
    if (i >= d_stack->size())
    {
        std::cerr <<"ERROR in E_StackDialog::armySelected. This cannot be!!\n";
        return true;
    }
    
    d_selected = i;
    updateStats();
    checkButtons();
    
    return true;
}

bool E_StackDialog::statChanged(PG_Button* btn)
{
    if (d_selected == INT_MAX)
        return true;

    // get the army for future use
    Stack::iterator it = d_stack->begin();
    for (unsigned int i = 0; i < d_selected; i++, it++);
    Army* a = (*it);
    
    // see constructor for how to get this from the button id
    int diff = 1;
    if (btn->GetID() % 2 == 1)
        diff = -1;

    // this is rather dull code
    switch (btn->GetID() / 2)
    {
        case STRENGTH:
            a->setStat(Army::STRENGTH, a->getStat(Army::STRENGTH, false) + diff);
            if (a->getStat(Army::STRENGTH, false) < 1)
                a->setStat(Army::STRENGTH, 1);
            break;
        case RANGED:
            a->setStat(Army::RANGED, a->getStat(Army::RANGED, false) + diff);
            if (a->getStat(Army::RANGED, false) < 1)
                a->setStat(Army::RANGED, 1);
            break;
        case SHOTS:
            a->setStat(Army::SHOTS, a->getStat(Army::SHOTS, false) + diff);
            if (a->getStat(Army::SHOTS, false) < 1)
                a->setStat(Army::SHOTS, 1);
            break;
        case DEFENSE:
            a->setStat(Army::DEFENSE, a->getStat(Army::DEFENSE, false) + diff);
            if (a->getStat(Army::DEFENSE, false) < 1)
                a->setStat(Army::DEFENSE, 1);
            break;
        case HP:
            a->setStat(Army::HP, a->getStat(Army::HP, false) + diff);
            if (a->getStat(Army::HP, false) < 1)
                a->setStat(Army::HP, 1);
            // bring the hitpoints to the new maximum
            a->damage(a->getStat(Army::HP));
            a->heal(a->getStat(Army::HP));
            break;
        case VITALITY:
            a->setStat(Army::VITALITY, a->getStat(Army::VITALITY, false) + diff);
            if (a->getStat(Army::VITALITY, false) < 1)
                a->setStat(Army::VITALITY, 1);
            break;
        case SIGHT:
            a->setStat(Army::SIGHT, a->getStat(Army::SIGHT, false) + diff);
            if (a->getStat(Army::SIGHT, false) < 1)
                a->setStat(Army::SIGHT, 1);
            break;
        case MOVES:
            a->setStat(Army::MOVES, a->getStat(Army::MOVES, false) + diff);
            if (a->getStat(Army::MOVES, false) < 2)
                a->setStat(Army::MOVES, 2);
            a->resetMoves();
    }

    // update the data
    updateStats();
    return true;
}

bool E_StackDialog::playerChanged(PG_ListBoxBaseItem* item)
{
    // remove the stack from the old player's stacklist
    Stacklist* sl = d_stack->getPlayer()->getStacklist();
    Stacklist::iterator sit = find(sl->begin(), sl->end(), d_stack);
    if (sit != sl->end())
        sl->erase(sit);
    
    static_cast<Player*>(item->GetUserData())->addStack(d_stack);

    return true;
}

void E_StackDialog::updatePics()
{
    PG_Application::SetBulkMode(true);
    
    int i = 0;
    for (Stack::iterator it = d_stack->begin(); it != d_stack->end(); it++, i++)
    {
        Army* a = (*it);
        SDL_Surface* pic = a->getPixmap();
        pic = SDL_DisplayFormatAlpha(pic);

        d_cache.push_back(pic);
        d_b_army[i]->SetIcon(pic);
    }

    // remove the old images, they are not needed anymore
    while (d_cache.size() > 8)
    {
        SDL_FreeSurface(*d_cache.begin());
        d_cache.erase(d_cache.begin());
    }

    // In all cases, this function should be followed by a call to checkButtons,
    // so we don't update the screen here.
    PG_Application::SetBulkMode(false);
}

void E_StackDialog::updateStats()
{
    PG_Application::SetBulkMode(true);
    
    // If no army is selected, don't fill the fields
    if (d_selected == INT_MAX)
    {
        d_l_name->SetText("n.a.");
        d_l_strength->SetText("n.a.");
        d_l_ranged->SetText("n/a");
        d_l_shots->SetText("n/a");
        d_l_defense->SetText("n.a.");
        d_l_hp->SetText("n.a.");
        d_l_vitality->SetText("n.a.");
        d_l_sight->SetText("n.a.");
        d_l_moves->SetText("n.a.");
        d_l_id->SetText("");

        PG_Application::SetBulkMode(false);
        Update();
        return;
    }
        
    // else find the army that is selected and the army type
    Stack::iterator it = d_stack->begin();
    for (unsigned int i = 0; i < d_selected; i++, it++);
    Army* a = *it;

    char buffer[21]; buffer[20] = '\0';
    d_l_name->SetText(a->getName().c_str());

    snprintf(buffer, 20, "%i", a->getStat(Army::STRENGTH, false));
    d_l_strength->SetText(buffer);
    snprintf(buffer, 20, "%i", a->getStat(Army::RANGED, false));
    d_l_ranged->SetText(buffer);
    snprintf(buffer, 20, "%i", a->getStat(Army::SHOTS));
    d_l_shots->SetText(buffer);
    snprintf(buffer, 20, "%i", a->getStat(Army::DEFENSE, false));
    d_l_defense->SetText(buffer);
    snprintf(buffer, 20, "%i", a->getStat(Army::HP, false));
    d_l_hp->SetText(buffer);
    snprintf(buffer, 20, "%i", a->getStat(Army::VITALITY, false));
    d_l_vitality->SetText(buffer);
    snprintf(buffer, 20, "%i", a->getStat(Army::SIGHT, false));
    d_l_sight->SetText(buffer);
    snprintf(buffer, 20, "%i", a->getStat(Army::MOVES, false));
    d_l_moves->SetText(buffer);
    snprintf(buffer, 20, "%i", a->getId());
    d_l_id->SetText(buffer);

    PG_Application::SetBulkMode(false);
    Update();
}

void E_StackDialog::checkButtons()
{
    PG_Application::SetBulkMode(true);
    
    // What we do here is: Check the army buttons and the (add/remove) button
    
    // First, hide all army buttons and show those that correspond to an army
    for (int i = 0; i < 8; i++)
    {
        d_b_army[i]->SetPressed(false);
        d_b_army[i]->Hide();
    }
    
    for (unsigned int i = 0; i < d_stack->size(); i++)
        d_b_army[i]->Show();
    
    if (d_selected != INT_MAX)
        d_b_army[d_selected]->SetPressed(true);

    // Now enable/disable the buttons for adding/removing an army
    d_b_add->EnableReceiver(true);
    if (d_stack->size() == 8)
        d_b_add->EnableReceiver(false);

    d_b_remove->EnableReceiver(true);
    if (d_selected == INT_MAX)
        d_b_remove->EnableReceiver(false);

    PG_Application::SetBulkMode(false);
    Update();
}
