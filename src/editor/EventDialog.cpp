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
#include <pgapplication.h>
#include <pgmessagebox.h>
#include <pglistboxitem.h>

#include "EventDialog.h"
#include "SelectionDialog.h"
#include "../defs.h"
#include "../playerlist.h"
#include "../GameMap.h"
#include "../events/EKillAll.h"
#include "../events/EPlayerDead.h"
#include "../events/ECityConq.h"
#include "../events/EArmyKilled.h"
#include "../events/ERound.h"
#include "../events/ERuinSearch.h"
#include "../events/ETempleSearch.h"
#include "../events/EDummy.h"
#include "../events/ENextTurn.h"
#include "../events/EStackKilled.h"
#include "../events/EStackMove.h"


E_EventDialog::E_EventDialog(PG_Widget* parent, PG_Rect rect, GameScenario* sc)
    :PG_Window(parent, rect, _("Manage events"), PG_Window::MODAL),
    d_scenario(sc), d_active(0)
{
    // As told in the other dialog, we need the following items:
    // - a listbox for selecting the item
    // - an id label; a label + edit for the value (max 1 value exists)
    // - a mulilineedit for comments
    // - two buttons to change reactions and conditions
    // - buttons for adding, removing events and quitting the dialog
    //
    // Afterwards, we initialize everything

    // the listbox
    PG_Rect r(30, 40, 200, 350);
    d_events = new PG_ListBox(this, r);
    d_events->SetMultiSelect(false);
    d_events->sigSelectItem.connect(slot(*this, &E_EventDialog::eventSelected));

    // the id label
    r.SetRect(270, 40, 80, 20);
    new PG_Label(this, r, _("ID:"));
    r.x += r.w + 10;
    d_l_id = new PG_Label(this, r, "");
    
    // a dropdown for selecting if the event starts active or deactivated
    r.SetRect(270, 70, 130, 20);
    d_activate = new PG_DropDown(this, r);
    d_activate->AddItem(_("Activate"));
    d_activate->AddItem(_("Deactivate"));
    d_activate->SetEditable(false);
    d_activate->sigSelectItem.connect(slot(*this, &E_EventDialog::activationChanged));
    
    // the label and edit for the values
    r.SetRect(270, 100, 80, 20);
    d_l_val1 = new PG_Label(this, r, "");
    r.x += r.w + 10;
    d_val1 = new PG_LineEdit(this, r);
    d_val1->SetValidKeys("0123456789");
    d_val1->sigEditEnd.connect(slot(*this, &E_EventDialog::valueChanged));

    r.SetRect(270, 120, 80, 20);
    d_l_val2 = new PG_Label(this, r, "");
    r.x += r.w + 10;
    d_val2 = new PG_LineEdit(this, r);
    d_val2->SetValidKeys("0123456789");
    d_val2->sigEditEnd.connect(slot(*this, &E_EventDialog::valueChanged));

    // the comment edit
    r.SetRect(270, 150, 100, 20);
    d_l_comment = new PG_Label(this, r, _("Comment:"));
    r.SetRect(270, 170, 200, 100);
//    d_comment = new PG_MultiLineEdit(this, r);
    d_comment = new PG_LineEdit(this, r);
    d_comment->sigEditEnd.connect(slot(*this, &E_EventDialog::commentChanged));

    // the buttons to modify reactions and conditions
    r.SetRect(270, 290, 100, 30);
    d_b_reaction = new PG_Button(this, r, _("Reactions"));
    d_b_reaction->sigClick.connect(slot(*this, &E_EventDialog::reactionsClicked));
    r.x += r.w + 10;
    d_b_condition = new PG_Button(this, r, _("Conditions"));
    d_b_condition->sigClick.connect(slot(*this, &E_EventDialog::conditionsClicked));

    // buttons for adding, removing
    r.SetRect(270, 325, 100, 30);
    d_b_remove = new PG_Button(this, r, _("Remove"));
    d_b_remove->sigClick.connect(slot(*this, &E_EventDialog::removeClicked));
    r.x += r.w + 10;
    PG_Button* btn = new PG_Button(this, r, _("Add"));
    btn->sigClick.connect(slot(*this, &E_EventDialog::addClicked));

    // OK button and button for point management
    r.SetRect(380, 360, 100, 30);
    btn = new PG_Button(this, r, _("OK"));
    btn->sigClick.connect(slot(*this, &E_EventDialog::okClicked));

    
    // rest of the setup; try to select the first item
    fillData();
    fillEvents();
    d_events->SelectFirstItem();    
}

E_EventDialog::~E_EventDialog()
{
}

bool E_EventDialog::eventSelected(PG_ListBoxBaseItem* item)
{
    // paragui problem: When editing a line edit and selecting a new item
    // from the listbox, first the new item is selected, then the edit ends.
    // This can create strange bugs, therefore we stop the editing here
    // manually.
    if (d_val1->IsVisible())
        d_val1->EditEnd();
    if (d_val2->IsVisible())
        d_val2->EditEnd();
    if (d_comment->IsVisible())
        d_comment->EditEnd();
    
    fillData();
    return true;
}

bool E_EventDialog::activationChanged(PG_ListBoxBaseItem* item)
{
    if (!d_active)
        return true;

    if (std::string(item->GetText()) == _("Activate"))
        d_active->setActive(true);
    else
        d_active->setActive(false);

    return true;
}

bool E_EventDialog::valueChanged(PG_LineEdit* edit)
{
    if (!d_active)
        return true;

    PG_Point pos;
    switch (d_active->getType())
    {
        case Event::PLAYERDEAD:
            dynamic_cast<EPlayerDead*>(d_active)->setPlayer(atoi(edit->GetText()));
            break;
        case Event::CITYCONQUERED:
            dynamic_cast<ECityConq*>(d_active)->setCity(atoi(edit->GetText()));
            break;
        case Event::ARMYKILLED:
            dynamic_cast<EArmyKilled*>(d_active)->setArmy(atoi(edit->GetText()));
            break;
        case Event::ROUND:
            dynamic_cast<ERound*>(d_active)->setRound(atoi(edit->GetText()));
            break;
        case Event::RUINSEARCH:
            dynamic_cast<ERuinSearch*>(d_active)->setRuin(atoi(edit->GetText()));
            break;
        case Event::TEMPLESEARCH:
            dynamic_cast<ETempleSearch*>(d_active)->setTemple(atoi(edit->GetText()));
            break;
        case Event::STACKKILLED:
            dynamic_cast<EStackKilled*>(d_active)->setStack(atoi(edit->GetText()));
            break;
        case Event::STACKMOVE:
            pos = dynamic_cast<EStackMove*>(d_active)->getPos();
            if (edit == d_val1)
                pos.x = atoi(edit->GetText());
            else
                pos.y = atoi(edit->GetText());
            
            if (pos.x >= GameMap::getWidth() || pos.y >= GameMap::getHeight())
                break;
            dynamic_cast<EStackMove*>(d_active)->setPos(pos);
        case Event::KILLALL:
        case Event::DUMMY:
        case Event::NEXTTURN:
            break;
    }
    
    return true;
}

bool E_EventDialog::commentChanged(PG_LineEdit* edit)
{
    if (!d_active)
        return true;

    d_active->setComment(edit->GetText());
    return true;
}

bool E_EventDialog::reactionsClicked(PG_Button* btn)
{
    if (!d_active)
        return true;
    
    E_ReactionDialog d(0, PG_Rect(my_xpos, my_ypos, my_width, my_height), d_active);
    d.Show();
    d.RunModal();
    d.Hide();
    
    return true;
}

bool E_EventDialog::conditionsClicked(PG_Button* btn)
{
    if (!d_active)
        return true;
    
    PG_Rect r(my_width/2 - 200, my_height/2 - 150, 400, 300);
    E_ConditionDialog d(this, r, d_active);
    d.Show();
    d.RunModal();
    d.Hide();
    
    return true;
}

bool E_EventDialog::removeClicked(PG_Button* btn)
{
    if (!d_active)
        return true;

    // This is a very complex change, so we better ask the user
    PG_Rect r(my_width/2 - 150, my_height/2 - 100, 300, 200);
    PG_MessageBox mb(this, r, _("Warning"), _("Do you really want to remove "
                  "this event?"),
                  PG_Rect(50, 160, 40, 20), _("Yes"),
                  PG_Rect(210, 160, 40, 20), _("No"));
    mb.Show();
    // cancelled
    if (mb.RunModal() == 2)
        return true;
    mb.Hide();

    
    // remove the event
    d_scenario->removeEvent(d_active);

    //setup the event list
    fillEvents();
    fillData();
    d_events->SelectFirstItem();

    return true;
}

bool E_EventDialog::addClicked(PG_Button* btn)
{
    std::list<std::string> types;
    types.push_back(_("KillAll"));      types.push_back(_("PlayerDead"));
    types.push_back(_("CityConquered"));types.push_back(_("ArmyKilled"));
    types.push_back(_("Round"));        types.push_back(_("RuinSearch"));
    types.push_back(_("TempleSearch")); types.push_back(_("Dummy"));
    types.push_back(_("NextTurn"));     types.push_back(_("StackKilled"));
    types.push_back(_("StackMove"));
    
    // query the user about the event type
    PG_Rect rect(PG_Application::GetScreenWidth()/2 - 100,
                 PG_Application::GetScreenHeight()/2 - 175, 200, 350);
    E_SelectionDialog d(0, rect, types);
    d.Show();
    d.RunModal();
    d.Hide();

    if (d.getSelection() == -1)
        return true;
    
    // create the new event according to the selection
    Event::Type retval = static_cast<Event::Type>(d.getSelection());
    Event* ev = 0;
    Player* neutral = Playerlist::getInstance()->getNeutral();
    switch (retval)
    {
        case Event::KILLALL:
            ev = new EKillAll();
            break;
        case Event::PLAYERDEAD:
            if (neutral)
                ev = new EPlayerDead(neutral->getId());
            else
                ev = new EPlayerDead((Uint32)0);
            break;
        case Event::CITYCONQUERED:
            ev = new ECityConq(1);
            break;
        case Event::ARMYKILLED:
            ev = new EArmyKilled(1);
            break;
        case Event::ROUND:
            ev = new ERound(1);
            break;
        case Event::RUINSEARCH:
            ev = new ERuinSearch(1);
            break;
        case Event::TEMPLESEARCH:
            ev = new ETempleSearch(1);
            break;
        case Event::DUMMY:
            ev = new EDummy();
            break;
        case Event::NEXTTURN:
            ev = new ENextTurn();
            break;
        case Event::STACKKILLED:
            ev = new EStackKilled(1);
            break;
        case Event::STACKMOVE:
            ev = new EStackMove(PG_Point(0,0));
    }

    // insert the event and setup everything, later we may select the last
    // item, but I don't like it yet (PG_ListBox misses a function here)
    d_scenario->addEvent(ev);
    fillEvents();
    fillData();
    
    return true;
}

bool E_EventDialog::okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

void E_EventDialog::fillEvents()
{
    // first, clear the listbox from all the filth
    d_events->DeleteAll();

    // now insert the events one by one
    std::list<Event*> el = d_scenario->getEventlist();
    for (std::list<Event*>::iterator it = el.begin(); it != el.end(); it++)
    {
        PG_ListBoxItem* item = new PG_ListBoxItem(d_events, 20, "", 0, (void*)(*it));
        
        switch((*it)->getType())
        {
            case Event::KILLALL:
                item->SetText(_("EKillAll"));
                break;
            case Event::PLAYERDEAD:
                item->SetText(_("EPlayerDead"));
                break;
            case Event::CITYCONQUERED:
                item->SetText(_("ECityConquered"));
                break;
            case Event::ARMYKILLED:
                item->SetText(_("EArmyKilled"));
                break;
            case Event::ROUND:
                item->SetText(_("ERound"));
                break;
            case Event::RUINSEARCH:
                item->SetText(_("ERuinSearch"));
                break;
            case Event::TEMPLESEARCH:
                item->SetText(_("ETempleSearch"));
                break;
            case Event::DUMMY:
                item->SetText(_("EDummy"));
                break;
            case Event::NEXTTURN:
                item->SetText(_("ENextTurn"));
                break;
            case Event::STACKKILLED:
                item->SetText(_("EStackKilled"));
                break;
            case Event::STACKMOVE:
                item->SetText(_("EStackMove"));
                break;
        }
    }   
}

void E_EventDialog::fillData()
{
    // first, set d_active to something useful and exit if no event is selected
    d_active = 0;
    
    if (d_events->GetSelectedItem())
        d_active = (Event*)d_events->GetSelectedItem()->GetUserData();

    // now first disable paragui updates ...
    PG_Application::SetBulkMode(true);

    // ... and all the buttons and edit fields etc. that may change
    d_l_id->Hide();
    d_activate->Hide();
    d_l_val1->Hide();
    d_val1->Hide();
    d_l_val2->Hide();
    d_val2->Hide();
    d_l_comment->Hide();
    d_comment->Hide();
    d_b_reaction->EnableReceiver(false);
    d_b_condition->EnableReceiver(false);
    d_b_remove->EnableReceiver(false);
    
    // if no event is selected, we leave here.
    if (!d_active)
    {
        PG_Application::SetBulkMode(false);
        Update();
        return;
    }

    // else reenable what is neccessary ...
    char buf[21]; buf[20]='\0';
    
    // ... first the stuff that is the same for all events ...
    snprintf(buf, 20, "%i", d_active->getId());
    d_l_id->SetText(buf);
    d_l_id->Show();

    d_activate->Show();
    if (d_active->getActive())
        d_activate->SelectItem(0);
    else
        d_activate->SelectItem(1);

    d_l_comment->Show();
    d_comment->SetText(d_active->getComment().c_str());
    d_comment->Show();

    d_b_reaction->EnableReceiver(true);
    d_b_condition->EnableReceiver(true);
    d_b_remove->EnableReceiver(true);

    // ... then the different things
    switch (d_active->getType())
    {
        case Event::KILLALL:
            // nothing to do here...
            break;

        case Event::PLAYERDEAD:
            // edit is the player id
            d_l_val1->SetText(_("Player:"));
            d_l_val1->Show();
            
            snprintf(buf, 20, "%i", dynamic_cast<EPlayerDead*>(d_active)->getPlayer());
            d_val1->SetText(buf);
            d_val1->Show();
            break;

        case Event::CITYCONQUERED:
            // edit is city id
            d_l_val1->SetText(_("City:"));
            d_l_val1->Show();

            snprintf(buf, 20, "%i", dynamic_cast<ECityConq*>(d_active)->getCity());
            d_val1->SetText(buf);
            d_val1->Show();
            break;

        case Event::ARMYKILLED:
            // edit is the army id
            d_l_val1->SetText(_("Army:"));
            d_l_val1->Show();

            snprintf(buf, 20, "%i", dynamic_cast<EArmyKilled*>(d_active)->getArmyId());
            d_val1->SetText(buf);
            d_val1->Show();
            break;

        case Event::ROUND:
            // edit is round number
            d_l_val1->SetText(_("Round:"));
            d_l_val1->Show();

            snprintf(buf, 20, "%i", dynamic_cast<ERound*>(d_active)->getRound());
            d_val1->SetText(buf);
            d_val1->Show();
            break;

        case Event::RUINSEARCH:
            // edit is ruin id
            d_l_val1->SetText(_("Ruin:"));
            d_l_val1->Show();

            snprintf(buf, 20, "%i", dynamic_cast<ERuinSearch*>(d_active)->getRuin());
            d_val1->SetText(buf);
            d_val1->Show();
            break;
            
        case Event::TEMPLESEARCH:
            // edit is temple id
            d_l_val1->SetText(_("Temple:"));
            d_l_val1->Show();
            
            snprintf(buf, 20, "%i", dynamic_cast<ETempleSearch*>(d_active)->getTemple());
            d_val1->SetText(buf);
            d_val1->Show();
            break;

        case Event::DUMMY:
        case Event::NEXTTURN:
            break;

        case Event::STACKKILLED:
            // edit is stack id
            d_l_val1->SetText(_("Stack:"));
            d_l_val1->Show();
            
            snprintf(buf, 20, "%i", dynamic_cast<EStackKilled*>(d_active)->getStackId());
            d_val1->SetText(buf);
            d_val1->Show();
            break;

        case Event::STACKMOVE:
            // first edit is xpos, second ypos
            d_l_val1->SetText(_("X:"));
            d_l_val1->Show();
            d_l_val2->SetText(_("Y:"));
            d_l_val2->Show();

            snprintf(buf, 20, "%i", dynamic_cast<EStackMove*>(d_active)->getPos().x);
            d_val1->SetText(buf);
            d_val1->Show();
            snprintf(buf, 20, "%i", dynamic_cast<EStackMove*>(d_active)->getPos().y);
            d_val2->SetText(buf);
            d_val2->Show();
    }

    // reenable paragui updates at the end
    PG_Application::SetBulkMode(false);
    Update();
}
