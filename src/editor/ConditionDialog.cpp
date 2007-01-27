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

#include "ConditionDialog.h"
#include "../events/CPlayer.h"
#include "../events/CCounter.h"
#include "../events/CLiving.h"
#include "../events/CDead.h"
#include "../events/CArmy.h"
#include "../playerlist.h"

E_ConditionDialog::E_ConditionDialog(PG_Widget* parent, PG_Rect rect, Event* ev)
    :PG_Window(parent, rect, _("Manage conditions"), PG_Window::MODAL),
    d_event(ev), d_reaction(0), d_index(-1)
{
    init();
}

E_ConditionDialog::E_ConditionDialog(PG_Widget* parent, PG_Rect rect, Reaction* r)
    :PG_Window(parent, rect, _("Manage conditions"), PG_Window::MODAL),
    d_event(0), d_reaction(r), d_index(-1)
{
    init();
}

E_ConditionDialog::~E_ConditionDialog()
{
}

bool E_ConditionDialog::itemSelected(PG_ListBoxBaseItem* item)
{
    // Paragui problem: if we edit the field and then select a new item,
    // first the new item is selected, then the editing ends. This can cause
    // strange problems, so let us end the editing here manually before doing
    // anything else.
    if (d_value->IsVisible())
        d_value->EditEnd();
    
    // get the index
    d_index = (long)item->GetUserData();

    fillData();
    checkButtons();
    return true;
}

bool E_ConditionDialog::typeChanged(PG_ListBoxBaseItem* item)
{
    // if no item is selected, ignore the call (should never happen anyway)
    if (d_index == -1)
        return true;
    

    // create a new condition, really evil cast! In fact, it is quite surprising
    // for me how difficult it is to overcome the error checks of the gcc :)
    Condition* c = 0;
    Condition::Type type = static_cast<Condition::Type>((long)item->GetUserData());
    
    if (type == getCondition()->getType())
        return true;

    switch (type)
    {
        case Condition::PLAYER:
            c = new CPlayer(Playerlist::getInstance()->getNeutral()->getId());
            break;
        case Condition::COUNTER:
            c = new CCounter(1);
            break;
        case Condition::LIVING:
            c = new CLiving(Playerlist::getInstance()->getNeutral()->getId());
            break;
        case Condition::DEAD:
            c = new CDead(Playerlist::getInstance()->getNeutral()->getId());
            break;
        case Condition::ARMY:
            c = new CArmy((Uint32)0);
    }

    // now replace the condition
    if (d_event)
    {
        d_event->removeCondition(d_index);
        d_event->addCondition(c, d_index);
    }
    else
    {
        d_reaction->removeCondition(d_index);
        d_reaction->addCondition(c, d_index);
    }

    // refill the list of all conditions and select the new one (hopefully
    // the index is right :))
    refillList();
    PG_ListBoxBaseItem* lbbi;
    lbbi = static_cast<PG_ListBoxBaseItem*>(d_condlist->FindWidget(d_index));
    d_condlist->SelectItem(lbbi, true);
    
    return true;
}

bool E_ConditionDialog::valueChanged(PG_LineEdit* edit)
{
    if (d_index == -1)
        return true;
    
    Condition* c = getCondition();
    Uint32 value = atoi(edit->GetText());

    switch (c->getType())
    {
        case Condition::PLAYER:
            dynamic_cast<CPlayer*>(c)->setPlayer(value);
            break;
        case Condition::COUNTER:
            dynamic_cast<CCounter*>(c)->setCounter(value);
            break;
        case Condition::LIVING:
            dynamic_cast<CLiving*>(c)->setPlayer(value);
            break;
        case Condition::DEAD:
            dynamic_cast<CDead*>(c)->setPlayer(value);
            break;
        case Condition::ARMY:
            dynamic_cast<CArmy*>(c)->setArmy(value);
    }

    return true;
}

bool E_ConditionDialog::addClicked(PG_Button* btn)
{
    // First, add a new standard condition to the list
    Condition* c = new CPlayer(Playerlist::getInstance()->getNeutral()->getId());

    if (d_event)
        d_event->addCondition(c);
    else
        d_reaction->addCondition(c);

    // Then, refill the listbox and select our new condition
    refillList();

    int index = 0;
    if (d_event)
        index = d_event->getNoOfConditions();
    else
        index = d_reaction->getNoOfConditions();

    // see typeChanged
    PG_ListBoxBaseItem* lbbi;
    lbbi = static_cast<PG_ListBoxBaseItem*>(d_condlist->FindWidget(index));
    d_condlist->SelectItem(lbbi, true);
    
    return true;
}

bool E_ConditionDialog::removeClicked(PG_Button* btn)
{
    if (d_index == -1)
        return true;

    // remove the present condition
    if (d_event)
        d_event->removeCondition(d_index);
    else
        d_reaction->removeCondition(d_index);

    // refill the list and select the first condition
    refillList();
    d_condlist->SelectFirstItem();
    checkButtons();
    
    return true;
}

bool E_ConditionDialog::okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

void E_ConditionDialog::init()
{
    // We need to setup here;
    //
    // 1. a listbox that shows all present conditions
    // 2. a dropdown for selecting the condition type
    // 3. a label and an edit field to edit the condition value
    // 4. Buttons for adding, removing conditions and ok
    //
    // When this is done, we need to do some setup:
    // 1. fill the listbox with all given conditions
    // 2. Select the first condition
    //
    // The rest should then be done by the other functions

    // Listbox
    PG_Rect rect(30, 40, 150, 200);
    d_condlist = new PG_ListBox(this, rect);
    d_condlist->SetMultiSelect(false);
    d_condlist->sigSelectItem.connect(slot(*this, &E_ConditionDialog::itemSelected));

    // Dropdown
    rect.SetRect(260, 40, 120, 20);
    d_type = new PG_DropDown(this, rect);
    d_type->AddItem(_("CPlayer"), (void*)Condition::PLAYER);
    d_type->AddItem(_("CCounter"), (void*)Condition::COUNTER);
    d_type->AddItem(_("CLiving"), (void*)Condition::LIVING);
    d_type->AddItem(_("CDead"), (void*)Condition::DEAD);
    d_type->AddItem(_("CArmy"), (void*)Condition::ARMY);
    d_type->SetEditable(false);
    d_type->sigSelectItem.connect(slot(*this, &E_ConditionDialog::typeChanged));

    // Label and edit field
    rect.SetRect(210, 100, 80, 20);
    d_l_value = new PG_Label(this, rect, "");

    rect.x += rect.w + 10;
    d_value = new PG_LineEdit(this, rect);
    d_value->SetValidKeys("0123456789");
    d_value->sigEditEnd.connect(slot(*this, &E_ConditionDialog::valueChanged));

    // Buttons
    rect.SetRect(260, 140, 80, 30);
    PG_Button* btn = new PG_Button(this, rect, _("Add"));
    btn->sigClick.connect(slot(*this, &E_ConditionDialog::addClicked));

    rect.y += 35;
    d_b_remove = new PG_Button(this, rect, _("Remove"));
    d_b_remove->sigClick.connect(slot(*this, &E_ConditionDialog::removeClicked));

    rect.y += 35;
    btn = new PG_Button(this, rect, _("OK"));
    btn->sigClick.connect(slot(*this, &E_ConditionDialog::okClicked));

    // rest setup
    refillList();
    d_condlist->SelectFirstItem();
}

void E_ConditionDialog::refillList()
{
    char buf[21]; buf[20] = '\0';
    
    // remove everything ...
    d_condlist->DeleteAll();

    // .. and refill the list; we don't care about the d_index and just hope
    // that the other code handles this. :)
    if (d_event)
        for (unsigned int i = 0; i < d_event->getNoOfConditions(); i++)
        {
            switch (d_event->getCondition(i)->getType())
            {
                case Condition::PLAYER:
                    snprintf(buf, 20, _("%i. CPlayer"), i);
                    break;
                case Condition::COUNTER:
                    snprintf(buf, 20, _("%i. CCounter"), i);
                    break;
                case Condition::LIVING:
                    snprintf(buf, 20, _("%i. CLiving"), i);
                    break;
                case Condition::DEAD:
                    snprintf(buf, 20, _("%i. CDead"), i);
                    break;
                case Condition::ARMY:
                    snprintf(buf, 20, _("%i. CArmy"), i);
            }
            new PG_ListBoxItem(d_condlist, 20, buf, 0, (void*)i);
        }
    else
        for (unsigned int i = 0; i < d_reaction->getNoOfConditions(); i++)
        {
            PG_ListBoxItem* item;
            item = new PG_ListBoxItem(d_condlist, 20, "", 0, (void*)i);
            switch (d_reaction->getCondition(i)->getType())
            {
                case Condition::PLAYER:
                    snprintf(buf, 20, _("%i. CPlayer"), i);
                    break;
                case Condition::COUNTER:
                    snprintf(buf, 20, _("%i. CCounter"), i);
                    break;
                case Condition::LIVING:
                    snprintf(buf, 20, _("%i. CLiving"), i);
                    break;
                case Condition::DEAD:
                    snprintf(buf, 20, _("%i. CDead"), i);
                    break;
                case Condition::ARMY:
                    snprintf(buf, 20, _("%i. CArmy"), i);
            }
            item->SetText(buf);
        }
}

void E_ConditionDialog::fillData()
{
    char buf[21]; buf[20] = '\0';
    Condition* c = getCondition();
    if (c == 0)
        return;
    
    switch (c->getType())
    {
        case Condition::PLAYER:
            d_type->SetText(_("CPlayer"));
            d_l_value->SetText(_("Player:"));
            snprintf(buf, 20, "%i", dynamic_cast<CPlayer*>(c)->getPlayer());
            break;
        case Condition::COUNTER:
            d_type->SetText(_("CCounter"));
            d_l_value->SetText(_("Counter:"));
            snprintf(buf, 20, "%i", dynamic_cast<CCounter*>(c)->getCounter());
            break;
        case Condition::LIVING:
            d_type->SetText(_("CLiving"));
            d_l_value->SetText(_("Player:"));
            snprintf(buf, 20, "%i", dynamic_cast<CLiving*>(c)->getPlayer());
            break;
        case Condition::DEAD:
            d_type->SetText(_("CDead"));
            d_l_value->SetText(_("Player:"));
            snprintf(buf, 20, "%i", dynamic_cast<CDead*>(c)->getPlayer());
            break;
        case Condition::ARMY:
            d_type->SetText(_("CArmy"));
            d_l_value->SetText(_("Army:"));
            snprintf(buf, 20, "%i", dynamic_cast<CArmy*>(c)->getArmy());
    }

    d_value->SetText(buf);
}

void E_ConditionDialog::checkButtons()
{
    if (d_index == -1)
    {
        d_type->EnableReceiver(false);
        d_value->EnableReceiver(false);
        d_b_remove->EnableReceiver(false);
    }
    else
    {
        d_type->EnableReceiver(true);
        d_value->EnableReceiver(true);
        d_b_remove->EnableReceiver(true);
    }
}

Condition* E_ConditionDialog::getCondition()
{
    if (d_index == -1)
        return 0;

    if (d_event)
        return d_event->getCondition(d_index);
    
    return d_reaction->getCondition(d_index);
}
