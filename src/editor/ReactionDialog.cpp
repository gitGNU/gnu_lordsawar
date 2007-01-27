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
#include <pglistboxitem.h>
#include <pgapplication.h>

#include "ReactionDialog.h"
#include "StackDialog.h"
#include "SelectionDialog.h"
#include "../defs.h"
#include "../GameMap.h"
#include "../playerlist.h"
#include "../events/RMessage.h"
#include "../events/RAddGold.h"
#include "../events/RAddUnit.h"
#include "../events/RDelUnit.h"
#include "../events/RUpdate.h"
#include "../events/RCenter.h"
#include "../events/RCenterObj.h"
#include "../events/RCreateItem.h"
#include "../events/RWinGame.h"
#include "../events/RLoseGame.h"
#include "../events/RRaiseEvent.h"
#include "../events/RActEvent.h"
#include "../events/RRevivePlayer.h"
#include "../events/RKillPlayer.h"
#include "../events/RTransferCity.h"


E_ReactionDialog::E_ReactionDialog(PG_Widget* parent, PG_Rect rect, Event* ev)
    :PG_Window(parent, rect, _("Manage reactions"), PG_Window::MODAL),
    d_event(ev), d_index(-1)
{
    // We first create our items:
    // - a listbox to show all reactions
    // - three labels and edit widgets for editing up to two values
    // - one multilineedit for messages
    // - five buttons: modify stack, add/remove reaction, ok, conditions
    //
    // Then we select the first item in the list (if applicable)
    
    // First, listbox
    PG_Rect r(30, 40, 200, 350);
    d_reactions = new PG_ListBox(this, r);
    d_reactions->SetMultiSelect(false);
    d_reactions->sigSelectItem.connect(slot(*this, &E_ReactionDialog::itemSelected));

    // labels and edits
    r.SetRect(270, 40, 80, 20);
    d_l_val1 = new PG_Label(this, r, "");
    r.y += 25;
    d_l_val2 = new PG_Label(this, r, "");
    r.y +=25;
    d_l_val3 = new PG_Label(this, r, "");

    r.SetRect(360, 40, 80, 20);
    d_val1 = new PG_LineEdit(this, r);
    d_val1->sigEditEnd.connect(slot(*this, &E_ReactionDialog::valueChanged));
    r.y += 25;
    d_val2 = new PG_LineEdit(this, r);
    d_val2->sigEditEnd.connect(slot(*this, &E_ReactionDialog::valueChanged));
    r.y += 25;
    d_val3 = new PG_LineEdit(this, r);
    d_val3->sigEditEnd.connect(slot(*this, &E_ReactionDialog::valueChanged));

    // multilineedit, this can cover the second value and the stack button
    // without problems (they don't interfere)
    r.SetRect(270, 60, 200, 120);
//    d_text = new PG_MultiLineEdit(this, r);
    d_text = new PG_LineEdit(this, r);
    d_text->sigEditEnd.connect(slot(*this, &E_ReactionDialog::valueChanged));
    
    // buttons
    r.SetRect(300, 170, 80, 30);
    d_b_stack = new PG_Button(this, r, _("Stack"));
    d_b_stack->sigClick.connect(slot(*this, &E_ReactionDialog::stackClicked));
    r.y += 40;
    d_b_condition = new PG_Button(this, r, _("Conditions"));
    d_b_condition->sigClick.connect(slot(*this, &E_ReactionDialog::conditionsClicked));
    r.y += 40;
    d_b_remove = new PG_Button(this, r, _("Remove"));
    d_b_remove->sigClick.connect(slot(*this, &E_ReactionDialog::removeClicked));
    r.y += 40;
    PG_Button* btn = new PG_Button(this, r, _("Add"));
    btn->sigClick.connect(slot(*this, &E_ReactionDialog::addClicked));
    r.y += 40;
    btn = new PG_Button(this, r, _("OK"));
    btn->sigClick.connect(slot(*this, &E_ReactionDialog::okClicked));

    // now initialize
    fillReactions();
    fillData();     // in case there are no reactions, hide the values all, first
    d_reactions->SelectFirstItem();
}

E_ReactionDialog::~E_ReactionDialog()
{
}

bool E_ReactionDialog::itemSelected(PG_ListBoxBaseItem* item)
{
    // paragui problem: When editing a line edit and selecting a new item
    // from the listbox, first the new item is selected, then the edit ends.
    // This can create strange bugs, therefore we stop the editing here
    // manually.
    if (d_text->IsVisible())
        d_text->EditEnd();
    if (d_val1->IsVisible())
        d_val1->EditEnd();
    if (d_val2->IsVisible())
        d_val2->EditEnd();
    if (d_val3->IsVisible())
        d_val3->EditEnd();
    
    // fillData also cares for setting up the correct index
    fillData();
    return true;
}

bool E_ReactionDialog::valueChanged(PG_LineEdit* edit)
{
    if (d_index == -1)
        return true;
    
    Reaction* r = d_event->getReaction(d_index);
    PG_Point pos;
    
    switch(d_event->getReaction(d_index)->getType())
    {
        case Reaction::MESSAGE:
            dynamic_cast<RMessage*>(r)->setMessage(d_text->GetText());
            break;
            
        case Reaction::ADDGOLD:
            if (edit == d_val1)
                dynamic_cast<RAddGold*>(r)->setPlayer(atoi(edit->GetText()));
            else
                dynamic_cast<RAddGold*>(r)->setGold(atoi(edit->GetText()));
            break;
            
        case Reaction::ADDUNIT:
            pos = dynamic_cast<RAddUnit*>(r)->getPos();
            if (edit == d_val1)
                dynamic_cast<RAddUnit*>(r)->setPlayer(atoi(edit->GetText()));
            else if (edit == d_val2)
                pos.x = atoi(edit->GetText());
            else
                pos.y = atoi(edit->GetText());

            // some error checking: don't allow values outside of range
            if (pos.x >= GameMap::getWidth() || pos.y >= GameMap::getHeight())
                fillData();
            else
                dynamic_cast<RAddUnit*>(r)->setPos(pos);
            break;

        case Reaction::DELUNIT:
            dynamic_cast<RDelUnit*>(r)->setArmy(atoi(edit->GetText()));
            break;
            
        case Reaction::CENTER:
            pos = dynamic_cast<RCenter*>(r)->getPos();
            if (edit == d_val1)
                pos.x = atoi(edit->GetText());
            else
                pos.y = atoi(edit->GetText());

            // some error checking: don't allow values outside of range
            if (pos.x >= GameMap::getWidth() || pos.y >= GameMap::getHeight())
                fillData();
            else
                dynamic_cast<RCenter*>(r)->setPos(pos);
            break;
            
        case Reaction::CENTEROBJ:
            dynamic_cast<RCenterObj*>(r)->setObject(atoi(edit->GetText()));
            break;

        case Reaction::CREATEITEM:
            pos = dynamic_cast<RCreateItem*>(r)->getPos();
            if (edit == d_val1)
                dynamic_cast<RCreateItem*>(r)->setItem(atoi(edit->GetText()));
            else if (edit == d_val2)
                pos.x = atoi(edit->GetText());
            else
                pos.y = atoi(edit->GetText());
            
            // some error checking: don't allow values outside of range
            if (pos.x >= GameMap::getWidth() || pos.y >= GameMap::getHeight())
                fillData();
            else
                dynamic_cast<RCreateItem*>(r)->setPos(pos);
            break;
            
        case Reaction::WINGAME:
            dynamic_cast<RWinGame*>(r)->setStatus(atoi(edit->GetText()));
            break;
            
        case Reaction::LOSEGAME:
            dynamic_cast<RLoseGame*>(r)->setStatus(atoi(edit->GetText()));
            break;
            
        case Reaction::RAISEEVENT:
            dynamic_cast<RRaiseEvent*>(r)->setEvent(atoi(edit->GetText()));
            break;
            
        case Reaction::ACTEVENT:
            if (edit == d_val1)
                dynamic_cast<RActEvent*>(r)->setEvent(atoi(edit->GetText()));
            else
                dynamic_cast<RActEvent*>(r)->setActivate((bool)atoi(edit->GetText()));
            break;
            
        case Reaction::REVIVEPLAYER:
            dynamic_cast<RRevivePlayer*>(r)->setPlayer(atoi(edit->GetText()));
            break;
            
        case Reaction::KILLPLAYER:
            dynamic_cast<RKillPlayer*>(r)->setPlayer(atoi(edit->GetText()));
            break;
            
        case Reaction::TRANSFERCITY:
            if (edit == d_val1)
                dynamic_cast<RTransferCity*>(r)->setCity(atoi(edit->GetText()));
            else
                dynamic_cast<RTransferCity*>(r)->setPlayer(atoi(edit->GetText()));
            break;
            
        default:
            // do nothing
            break;
    }

    return true;
}

bool E_ReactionDialog::stackClicked(PG_Button* btn)
{
    if (d_index == -1)
        return true;

    Reaction* r = d_event->getReaction(d_index);
    // this button should not be activated if the type is wrong
    if (r->getType() != Reaction::ADDUNIT)
        return true;

    // Show the StackDialog
    RAddUnit* reac = dynamic_cast<RAddUnit*>(r);
    Stack* s = reac->getStack();
    if (s == 0)
        s = new Stack(Playerlist::getInstance()->getPlayer(reac->getPlayer()),
                      reac->getPos());

    E_StackDialog d(this, PG_Rect(0, 0, my_width, my_height), s, false);
    d.Show();
    d.RunModal();
    d.Hide();


    // Replace the stack in the reaction + if the stack is empty, remove it.
    // The if-clauses need some thinking, but they should be OK.
    if (reac->getStack() && s->empty())
    {
        reac->setStack(0);
        delete s;
    }
    else if (s->empty())
        delete s;
    else if (!reac->getStack())
        reac->setStack(s);
    
    return true;
}

bool E_ReactionDialog::conditionsClicked(PG_Button* btn)
{
    if (d_index == -1)
        return true;
    
    E_ConditionDialog d(this, PG_Rect(0, 0, my_width, my_height),
                        d_event->getReaction(d_index));
    d.Show();
    d.RunModal();
    d.Hide();

    return true;
}

bool E_ReactionDialog::addClicked(PG_Button* btn)
{
    // Query the user what type the reaction should have; first create a list
    // of strings
    std::list<std::string> types;
    types.push_back(_("Message"));      types.push_back(_("AddGold"));
    types.push_back(_("AddUnit"));      types.push_back(_("DelUnit"));
    types.push_back(_("Update"));       types.push_back(_("Center"));
    types.push_back(_("CenterObj"));    types.push_back(_("CreateItem"));
    types.push_back(_("WinGame"));      types.push_back(_("LoseGame"));
    types.push_back(_("RaiseEvent"));   types.push_back(_("ActEvent"));
    types.push_back(_("RevivePlayer")); types.push_back(_("KillPlayer"));
    types.push_back(_("TransferCity"));

    PG_Rect rect(PG_Application::GetScreenWidth()/2 - 100,
                 PG_Application::GetScreenHeight()/2 - 225, 200, 450);
    E_SelectionDialog d(0, rect, types);
    d.Show();
    d.RunModal();
    d.Hide();
    
    // Dialog was cancelled
    if (d.getSelection() == -1)
        return true;
    
    // now decide which reaction to create
    Reaction* r = 0;
    Player* neutral = Playerlist::getInstance()->getNeutral();
    switch (static_cast<Reaction::Type>(d.getSelection()))
    {
        case Reaction::MESSAGE:
            r = new RMessage("");
            break;
        case Reaction::ADDGOLD:
            r = new RAddGold(neutral->getId(), 0);
            break;
        case Reaction::ADDUNIT:
            r = new RAddUnit(0, neutral->getId(), PG_Point(0,0));
            break;
        case Reaction::DELUNIT:
            r = new RDelUnit(1);
            break;
        case Reaction::UPDATE:
            r = new RUpdate();
            break;
        case Reaction::CENTER:
            r = new RCenter(PG_Point(0,0));
            break;
        case Reaction::CENTEROBJ:
            r = new RCenterObj(1);
            break;
        case Reaction::CREATEITEM:
            r = new RCreateItem(0, PG_Point(0,0));
            break;
        case Reaction::WINGAME:
            r = new RWinGame(1);
            break;
        case Reaction::LOSEGAME:
            r = new RLoseGame(2);
            break;
        case Reaction::RAISEEVENT:
            r = new RRaiseEvent(1);
            break;
        case Reaction::ACTEVENT:
            r = new RActEvent(0, true);
            break;
        case Reaction::REVIVEPLAYER:
            r = new RRevivePlayer(neutral->getId());
            break;
        case Reaction::KILLPLAYER:
            r = new RKillPlayer(neutral->getId());
            break;
        case Reaction::TRANSFERCITY:
            r = new RTransferCity(0, neutral->getId());
            break;
    }

    // now insert the reaction and refill the list
    d_event->addReaction(r, d_index);
    fillReactions();
    fillData();
    
    return true;
}

bool E_ReactionDialog::removeClicked(PG_Button* btn)
{
    if (d_index == -1)
        return true;

    d_event->removeReaction(d_index);
    fillReactions();
    fillData();
    
    return true;
}

bool E_ReactionDialog::okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

void E_ReactionDialog::fillReactions()
{
    char buf[31]; buf[30]='\0';
    
    // first, remove the old entries
    d_reactions->DeleteAll();
    
    // insert all new entries with ordering number
    for (int i = 0; i < (int)d_event->getNoOfReactions(); i++)
    {
        switch (d_event->getReaction(i)->getType())
        {
            case Reaction::MESSAGE:
                snprintf(buf, 30, _("%i. Message"), i);
                break;
            case Reaction::ADDGOLD:
                snprintf(buf, 30, _("%i. AddGold"), i);
                break;
            case Reaction::ADDUNIT:
                snprintf(buf, 30, _("%i. AddUnit"), i);
                break;
            case Reaction::DELUNIT:
                snprintf(buf, 30, _("%i. DelUnit"), i);
                break;
            case Reaction::UPDATE:
                snprintf(buf, 30, _("%i. Update"), i);
                break;
            case Reaction::CENTER:
                snprintf(buf, 30, _("%i. Center"), i);
                break;
            case Reaction::CENTEROBJ:
                snprintf(buf, 30, _("%i. CenterObject"), i);
                break;
            case Reaction::CREATEITEM:
                snprintf(buf, 30, _("%i. CreateItem"), i);
                break;
            case Reaction::WINGAME:
                snprintf(buf, 30, _("%i. WinGame"), i);
                break;
            case Reaction::LOSEGAME:
                snprintf(buf, 30, _("%i. LoseGame"), i);
                break;
            case Reaction::RAISEEVENT:
                snprintf(buf, 30, _("%i. RaiseEvent"), i);
                break;
            case Reaction::ACTEVENT:
                snprintf(buf, 30, _("%i. ActivateEvent"), i);
                break;
            case Reaction::REVIVEPLAYER:
                snprintf(buf, 30, _("%i. RevivePlayer"), i);
                break;
            case Reaction::KILLPLAYER:
                snprintf(buf, 30, _("%i. KillPlayer"), i);
                break;
            case Reaction::TRANSFERCITY:
                snprintf(buf, 30, _("%i. TransferCity"), i);
                break;
        }

        new PG_ListBoxItem(d_reactions, 20, buf, 0, (void*)i);
    }
    
    // Don't forget to add a virtual "last reaction" with id -1. Since new
    // reaction are appended before the selected item, we need a mechanism
    // to append new reactions at the end => add a virtual end
    new PG_ListBoxItem(d_reactions, 20, _("--- End ---"), 0, (void*)-1);
}

void E_ReactionDialog::fillData()
{
    // first, find the index
    d_index = -1;

    if (d_reactions->GetSelectedItem())
        d_index = (long)d_reactions->GetSelectedItem()->GetUserData();

    // now, disable everything for now, we reactivate them in the huge switch
    // statement later, disable paragui updates for now
    PG_Application::SetBulkMode(true);
    
    d_b_stack->Hide();
    d_l_val1->Hide();
    d_l_val2->Hide();
    d_l_val3->Hide();
    d_val1->Hide();
    d_val2->Hide();
    d_val3->Hide();
    d_text->Hide();

    // quit if no or the last item has been selected
    if (d_index == -1)
    {
        d_b_remove->EnableReceiver(false);
        d_b_condition->EnableReceiver(false);
        PG_Application::SetBulkMode(false);
        Update();
        return;
    }

    d_b_remove->EnableReceiver(true);
    d_b_condition->EnableReceiver(true);

    // now enable and fill what is needed
    char buf[21]; buf[20] = '\0';
    Reaction* r = d_event->getReaction(d_index);
    
    switch (r->getType())
    {
        case Reaction::MESSAGE:
            // just a multilineedit to change the message
            d_text->Show();
            d_text->SetText(dynamic_cast<RMessage*>(r)->getMessage(false).c_str());
            break;
            
        case Reaction::ADDGOLD:
            // first edit is player id, second amount of gold
            d_l_val1->SetText(_("Player:"));
            d_l_val1->Show();
            d_l_val2->SetText(_("Gold:"));
            d_l_val2->Show();
            
            snprintf(buf, 20, "%i", dynamic_cast<RAddGold*>(r)->getPlayer());
            d_val1->SetText(buf);
            d_val1->Show();
            d_val1->SetValidKeys("0123456789");

            snprintf(buf, 20, "%i", dynamic_cast<RAddGold*>(r)->getGold());
            d_val2->SetText(buf);
            d_val2->Show();
            d_val2->SetValidKeys("-0123456789");
            break;

        case Reaction::ADDUNIT:
            // first edit is player, second xpos, third ypos, + stack button
            d_l_val1->SetText(_("Player:"));
            d_l_val1->Show();
            d_l_val2->SetText(_("X:"));
            d_l_val2->Show();
            d_l_val3->SetText(_("Y:"));
            d_l_val3->Show();

            snprintf(buf, 20, "%i", dynamic_cast<RAddUnit*>(r)->getPlayer());
            d_val1->SetText(buf);
            d_val1->Show();
            d_val1->SetValidKeys("0123456789");
            
            snprintf(buf, 20, "%i", dynamic_cast<RAddUnit*>(r)->getPos().x);
            d_val2->SetText(buf);
            d_val2->Show();
            d_val2->SetValidKeys("0123456789");

            snprintf(buf, 20, "%i", dynamic_cast<RAddUnit*>(r)->getPos().y);
            d_val3->SetText(buf);
            d_val3->Show();
            d_val3->SetValidKeys("0123456789");

            d_b_stack->Show();
            break;
            
        case Reaction::DELUNIT:
            // first edit is the army id
            d_l_val1->SetText(_("Army:"));
            d_l_val1->Show();

            snprintf(buf, 20, "%i", dynamic_cast<RDelUnit*>(r)->getArmyId());
            d_val1->SetText(buf);
            d_val1->Show();
            d_val1->SetValidKeys("0123456789");
            break;

        case Reaction::UPDATE:
            // do nothing
            break;

        case Reaction::CENTER:
            // first edit is xpos, second ypos
            d_l_val1->SetText(_("X:"));
            d_l_val1->Show();
            d_l_val2->SetText(_("Y:"));
            d_l_val2->Show();

            snprintf(buf, 20, "%i", dynamic_cast<RCenter*>(r)->getPos().x);
            d_val1->SetText(buf);
            d_val1->Show();
            d_val1->SetValidKeys("0123456789");

            snprintf(buf, 20, "%i", dynamic_cast<RCenter*>(r)->getPos().y);
            d_val2->SetText(buf);
            d_val2->Show();
            d_val2->SetValidKeys("0123456789");
            break;

        case Reaction::CENTEROBJ:
            //first edit is the id of the object
            d_l_val1->SetText(_("Object:"));
            d_l_val1->Show();

            snprintf(buf, 20, "%i", dynamic_cast<RCenterObj*>(r)->getObject());
            d_val1->SetText(buf);
            d_val1->Show();
            d_val1->SetValidKeys("0123456789");
            break;

        case Reaction::CREATEITEM:
            // first edit is list id of the item, second xpos, third ypos
            d_l_val1->SetText(_("Item:"));
            d_l_val1->Show();
            d_l_val2->SetText(_("X:"));
            d_l_val2->Show();
            d_l_val3->SetText(_("Y:"));
            d_l_val3->Show();

            snprintf(buf, 20, "%i", dynamic_cast<RCreateItem*>(r)->getItemIndex());
            d_val1->SetText(buf);
            d_val1->Show();
            d_val1->SetValidKeys("0123456789");

            snprintf(buf, 20, "%i", dynamic_cast<RCreateItem*>(r)->getPos().x);
            d_val2->SetText(buf);
            d_val2->Show();
            d_val2->SetValidKeys("0123456789");

            snprintf(buf, 20, "%i", dynamic_cast<RCreateItem*>(r)->getPos().y);
            d_val3->SetText(buf);
            d_val3->Show();
            d_val3->SetValidKeys("0123456789");
            break;

        case Reaction::WINGAME:
            // first edit is the status
            d_l_val1->SetText(_("Status:"));
            d_l_val1->Show();

            snprintf(buf, 20, "%i", dynamic_cast<RWinGame*>(r)->getStatus());
            d_val1->SetText(buf);
            d_val1->Show();
            d_val1->SetValidKeys("0123456789");
            break;

        case Reaction::LOSEGAME:
            // first edit is the status
            d_l_val1->SetText(_("Status:"));
            d_l_val1->Show();

            snprintf(buf, 20, "%i", dynamic_cast<RLoseGame*>(r)->getStatus());
            d_val1->SetText(buf);
            d_val1->Show();
            d_val1->SetValidKeys("0123456789");
            break;

        case Reaction::RAISEEVENT:
            // first edit is event id
            d_l_val1->SetText(_("Event:"));
            d_l_val1->Show();

            snprintf(buf, 20, "%i", dynamic_cast<RRaiseEvent*>(r)->getEvent());
            d_val1->SetText(buf);
            d_val1->Show();
            d_val1->SetValidKeys("0123456789");
            break;

        case Reaction::ACTEVENT:
            // first edit is event id, second edit is 0/1 for true/false
            d_l_val1->SetText(_("Event:"));
            d_l_val1->Show();
            d_l_val2->SetText(_("Activate:"));
            d_l_val2->Show();

            snprintf(buf, 20, "%i", dynamic_cast<RActEvent*>(r)->getEvent());
            d_val1->SetText(buf);
            d_val1->Show();
            d_val1->SetValidKeys("0123456789");

            snprintf(buf, 20, "%i", (int)dynamic_cast<RActEvent*>(r)->getActivate());
            d_val2->SetText(buf);
            d_val2->Show();
            d_val2->SetValidKeys("01");
            break;

        case Reaction::REVIVEPLAYER:
            // first edit is player id
            d_l_val1->SetText(_("Player:"));
            d_l_val1->Show();

            snprintf(buf, 20, "%i", dynamic_cast<RRevivePlayer*>(r)->getPlayer());
            d_val1->SetText(buf);
            d_val1->Show();
            d_val1->SetValidKeys("0123456789");
            break;

        case Reaction::KILLPLAYER:
            // first edit is player id
            d_l_val1->SetText(_("Player:"));
            d_l_val1->Show();

            snprintf(buf, 20, "%i", dynamic_cast<RKillPlayer*>(r)->getPlayer());
            d_val1->SetText(buf);
            d_val1->Show();
            d_val1->SetValidKeys("0123456789");
            break;

        case Reaction::TRANSFERCITY:
            // first edit is city id, second player id
            d_l_val1->SetText(_("City:"));
            d_l_val1->Show();
            d_l_val2->SetText(_("Player:"));
            d_l_val2->Show();

            snprintf(buf, 20, "%i", dynamic_cast<RTransferCity*>(r)->getCity());
            d_val1->SetText(buf);
            d_val1->Show();
            d_val1->SetValidKeys("0123456789");

            snprintf(buf, 20, "%i", dynamic_cast<RTransferCity*>(r)->getPlayer());
            d_val2->SetText(buf);
            d_val2->Show();
            d_val2->SetValidKeys("0123456789");
            break;
    }

    // reenable paragui updates
    PG_Application::SetBulkMode(false);
    Update();
}
