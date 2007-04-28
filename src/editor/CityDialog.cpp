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
#include <pgmessagebox.h>
#include <pgapplication.h>

#include "CityDialog.h"
#include "../defs.h"
#include "../playerlist.h"
#include "../armysetlist.h"
#include "../citylist.h"
#include "../File.h"
#include "../GraphicsCache.h"


E_CityDialog::E_CityDialog(PG_Widget* parent, PG_Rect rect, City* c)
    :PG_Window(parent, rect, _("City"), PG_Window::MODAL), d_city(c)
{
    d_blanksurf = File::getEditorPic("button_blank");
    
    char buffer[51]; buffer[50]='\0';
    PG_LineEdit* edit = 0;
    
    snprintf(buffer, 50, _("City at (%i,%i)"), c->getPos().x, c->getPos().y);
    SetTitle(buffer);

    snprintf(buffer, 50, _("ID: %i"), c->getId());
    new PG_Label(this, PG_Rect(30, 40, 100, 20), buffer);

    // place edit and label for the name of the city
    PG_Rect r(210, 40, 80, 20);
    new PG_Label(this, r, _("Name:"));
    
    r.x += r.w + 10;
    r.w = 150;
    edit = new PG_LineEdit(this, r);
    edit->sigEditEnd.connect(slot(*this, &E_CityDialog::nameChanged));
    edit->SetText(c->getName().c_str());

    // place label and edit for the income
    r.SetRect(30, 65, 50, 20);
    new PG_Label(this, r, _("Gold:"));

    r.x += r.w + 10;
    edit = new PG_LineEdit(this, r, "LineEdit", 3);
    edit->SetValidKeys("0123456789");
    edit->sigEditEnd.connect(slot(*this, &E_CityDialog::goldChanged));
    snprintf(buffer, 50, "%i", c->getGold());
    edit->SetText(buffer);

    // create a dropdown to select the owner and fill it with all players
    r.SetRect(210, 65, 80, 20);
    new PG_Label(this, r, _("Owner:"));
    
    r.x += r.w + 10;
    r.w = 150;
    PG_DropDown* drop = new PG_DropDown(this, r);
    drop->sigSelectItem.connect(slot(*this, &E_CityDialog::playerChanged));
     
    Playerlist* pl = Playerlist::getInstance();
    for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
        drop->AddItem((*it)->getName().c_str(), static_cast<void*>(*it));
    drop->SetText(d_city->getPlayer()->getName().c_str());

    // create a label for the level and two buttons to raise/lower it.
    new PG_Label(this, PG_Rect(210, 90, 80, 20), _("Level:"));
     
    snprintf(buffer, 50, "%i", d_city->getDefenseLevel());
    d_l_level = new PG_Label(this, PG_Rect(300, 90, 30, 20), buffer);

    // raise_level button
    d_b_up = new PG_Button(this, PG_Rect(340, 90, 10, 8), "+");
    d_b_up->sigClick.connect(slot(*this, &E_CityDialog::levelChanged));
    d_b_up->SetFontSize(8);

    // lower_level button
    d_b_down = new PG_Button(this, PG_Rect(340, 102, 10, 8), "-");
    d_b_down->sigClick.connect(slot(*this, &E_CityDialog::levelChanged));
    d_b_down->SetFontSize(8);

    // Create a dropdown to select if the city is burnt down or not
    drop = new PG_DropDown(this, PG_Rect(300, 120, 150, 20));
    drop->AddItem(_("Untouched"));
    drop->AddItem(_("Burnt down"));
    drop->SetEditable(false);
    if (d_city->isBurnt())
        drop->SetText(_("Burnt down"));
    else
        drop->SetText(_("Untouched"));
    drop->sigSelectItem.connect(slot(*this, &E_CityDialog::statusChanged));

    // create a label for the captial checkbox
    d_cb_capital = new PG_CheckButton (this, PG_Rect(210, 145, 120, 20),_("Capital City"),0);
    if (d_city->isCapital())
        d_cb_capital->SetPressed();
    else
        d_cb_capital->SetUnpressed();
    d_cb_capital->sigClick.connect(slot(*this,&E_CityDialog::capitalToggled));

    // Create the labels for the army stats
    r.SetRect(50, 280, 100, 20);
    new PG_Label(this, r, _("Name:"));
    r.y += 20;
    new PG_Label(this, r, _("Strength:"));
    r.y += 20;
    new PG_Label(this, r, _("Ranged:"));
    r.y += 20;
    new PG_Label(this, r, _("Defense:"));
    r.y += 20;
    new PG_Label(this, r, _("Duration:"));
    r.y += 20;
    new PG_Label(this, r, _("Upkeep:"));

    r.SetRect(150, 280, 150, 20);
    d_l_name = new PG_Label(this, r);
    r.y += 20;
    d_l_strength = new PG_Label(this, r);
    r.y += 20;
    d_l_ranged = new PG_Label(this, r);
    r.y += 20;
    d_l_defense = new PG_Label(this, r);
    r.y += 20;
    d_l_production = new PG_Label(this, r);
    r.y += 20;
    d_l_upkeep = new PG_Label(this, r);

    // create the buttons for the production, 5 basic (including "no prod") 
    // and three advanced ones
    d_basic.resize(5);
    for (int i = 0; i < 5; i++)
    {
        d_basic[i] = new PG_Button(this, PG_Rect(30+i*44, 160, 44, 44));
        d_basic[i]->sigClick.connect(slot(*this, &E_CityDialog::productionSet));
        d_basic[i]->SetToggle(true);
    }
    
    d_advanced.resize(3);
    for (int i = 0; i < 3;i++)
    {
        d_advanced[i] = new PG_Button(this, PG_Rect(74+i*44, 210, 44, 44));
        d_advanced[i]->sigClick.connect(slot(*this, &E_CityDialog::productionSet));
        d_advanced[i]->SetToggle(true);
    }

    // add a button to buy new basic production
    r.SetRect(my_width-180, my_height-180, 140, 30);
    PG_Button* btn = new PG_Button(this, r, _("Buy Basic"));
    btn->sigClick.connect(slot(*this, &E_CityDialog::buyBasic));
   
    //! add a button to buy new advanced production
    r.y += 40;
    btn = new PG_Button(this, r, _("Buy Advanced"));
    btn->sigClick.connect(slot(*this, &E_CityDialog::buyAdvanced));
    
    // add a button to remove a production
    r.y += 40;
    btn = new PG_Button(this, r, _("Remove Production"));
    btn->sigClick.connect(slot(*this, &E_CityDialog::removeClicked));
    
    // not to forget: the OK button
    r.y += 40;
    btn = new PG_Button(this, r, _("OK"));
    btn->sigClick.connect(slot(*this, &E_CityDialog::okClicked));

    // checkButtons will initialize all the stuff
    updatePics();
    checkButtons();
}

E_CityDialog::~E_CityDialog()
{
    // remove all cached images
    while (!d_cache.empty())
    {
        SDL_FreeSurface(*(d_cache.begin()));
        d_cache.erase(d_cache.begin());
    }
    SDL_FreeSurface(d_blanksurf);
}

bool E_CityDialog::nameChanged(PG_LineEdit* edit)
{
    d_city->setName(std::string(edit->GetText()));
    return true;
}

bool E_CityDialog::goldChanged(PG_LineEdit* edit)
{
    d_city->setGold(atoi(edit->GetText()));
    return true;
}

bool E_CityDialog::levelChanged(PG_Button* btn)
{
    int gold = d_city->getGold();
    
    if (btn == d_b_up)
        d_city->raiseDefense();
    else
        d_city->reduceDefense();

    char buffer[51]; buffer[50]='\0';
    snprintf(buffer, 50, "%i", d_city->getDefenseLevel());
    d_l_level->SetText(buffer);

    // reset the city gold
    d_city->setGold(gold);
    
    checkButtons();
    return true;
}

bool E_CityDialog::playerChanged(PG_ListBoxBaseItem* item)
{
    Player* p = static_cast<Player*>(item->GetUserData());

    // change the city's owner; if the player has another armyset than the
    // previous one, remove the advanced productions
    if (d_city->getPlayer()->getArmyset() != p->getArmyset())
    {
        d_city->removeAdvancedProd(0);
        d_city->removeAdvancedProd(1);
        updatePics();
    }

    d_city->setPlayer(p);
    
    return true;
}

bool E_CityDialog::statusChanged(PG_ListBoxBaseItem* item)
{
    if (std::string(item->GetText()) == _("Burnt down"))
        d_city->setBurnt(true);
    else
        d_city->setBurnt(false);

    return true;
}

bool E_CityDialog::capitalToggled()
{
    // note: When quitting the dialog, we have to ensure that the owner
    // of the city has at most 1 capital!
    d_city->setCapital(!d_city->isCapital());
    return true;
}

bool E_CityDialog::productionSet(PG_Button* btn)
{
    bool isadvanced = false;
    int selected = -1;
    
    for (unsigned int i = 0; i < d_basic.size(); i++)
    {
        d_basic[i]->SetPressed(false);
        if (d_basic[i] == btn)
            selected = i-1;
    }

    for (unsigned int i = 0; i < d_advanced.size(); i++)
    {
        d_advanced[i]->SetPressed(false);
        if (d_advanced[i] == btn)
        {
            isadvanced = true;
            selected = i;
        }
    }

    btn->SetPressed(true);
    d_city->setProduction(selected, isadvanced);
    updateStats();
            
    return true;
}

bool E_CityDialog::buyBasic(PG_Button* btn)
{
    // This is not the best solution for the interface, but the easiest one to
    // code. If all production slots are full, ask the user to remove one,
    // first.
    if (d_city->getArmytype(d_city->getMaxNoOfBasicProd()-1, false) != -1)
    {
        PG_MessageBox mb(this, PG_Rect(my_width/2-150, my_height/2-75, 300, 150),
                        _("Error"), _("No free productions, maybe remove one first!"),
                        PG_Rect(120, 110, 60, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
        return true;
    }
    
    Uint32 def = Armysetlist::getInstance()->getStandardId();
    E_ArmyDialog d(this, PG_Rect(0, 0, my_width, my_height), def);
    d.Show();
    d.RunModal();
    d.Hide();

    // add the selected production unless the selection was cancelled
    if (!d.getSuccess())
        return true;

    d_city->addBasicProd(-1, d.getIndex());

    updatePics();
    updateStats();
    checkButtons();

    return true;
}

bool E_CityDialog::buyAdvanced(PG_Button* btn)
{
    // same as buyBasic, but with advanced productions
    if ((d_city->getArmytype(d_city->getMaxNoOfAdvancedProd()-1, true) != -1)
        || (d_city->getMaxNoOfAdvancedProd() == 0))
    {
        PG_MessageBox mb(this, PG_Rect(my_width/2-150, my_height/2-75, 300, 150),
                        _("Error"), _("No free productions, maybe remove one first!"),
                        PG_Rect(120, 110, 60, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
        return true;
    }
    
    E_ArmyDialog d(this, PG_Rect(0, 0, my_width, my_height),
                    d_city->getPlayer()->getArmyset());
    d.Show();
    d.RunModal();
    d.Hide();

    // add the selected production unless the selection was cancelled
    if (!d.getSuccess())
        return true;

    d_city->addAdvancedProd(-1, d.getIndex());

    updatePics();
    updateStats();
    checkButtons();
    
    return true;
}

bool E_CityDialog::removeClicked(PG_Button* btn)
{
    if (d_city->getProductionIndex() == -1)
        return true;

    // Show a message box querying the user and remove the production on
    // confirmation
    PG_MessageBox mb(this, PG_Rect(my_width/2-100, my_height/2-75, 200, 150),
                    _("Confirmation"), _("Do you really want to remove this production?"),
                    PG_Rect(40, 120, 40, 20), _("Yes"),
                    PG_Rect(120, 120, 40, 20), _("No"));
    mb.Show();
    if (mb.RunModal() == 2)
        return true;

    if (d_city->getAdvancedProd())
        d_city->removeAdvancedProd(d_city->getProductionIndex());
    else
        d_city->removeBasicProd(d_city->getProductionIndex());
    
    updatePics();
    updateStats();
    checkButtons();
    
    return true;
}

bool E_CityDialog::okClicked(PG_Button* btn)
{
    QuitModal();

    // make sure we have not more than 1 capital.
    if (d_city->isCapital())
    {
        Citylist* cl = Citylist::getInstance();
        for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
            if ((*it).isCapital() && &(*it) != d_city &&
			    (*it).getPlayer() == d_city->getPlayer())
                (*it).setCapital(false);
    }
    return true;
}

void E_CityDialog::checkButtons()
{
    // we do three things:
    // 1. check the level of the city and show/hide some production slots
    // 2. press the button whose production has been selected
    // 3. enable/disable the upgrade/downgrade buttons

    // we disable automatic updating for now
    PG_Application::SetBulkMode(true);
    
    // 1+2
    int selected = d_city->getProductionIndex();
    bool advanced = d_city->getAdvancedProd();
    
    for (unsigned int i = 0; i < d_basic.size(); i++)
    {
        d_basic[i]->Hide();
        d_basic[i]->SetPressed(false);
        d_basic[i]->EnableReceiver(true);
    }
    for (unsigned int i = 0; i < d_advanced.size(); i++)
    {
        d_advanced[i]->Hide();
        d_advanced[i]->SetPressed(false);
        d_advanced[i]->EnableReceiver(true);
    }

    for (int i = -1; i < d_city->getMaxNoOfBasicProd(); i++)
    {
        d_basic[i+1]->Show();
        if (!advanced && (i == selected))
            d_basic[i+1]->SetPressed(true);
        // make button inaccessible if it has no production
        if (d_city->getArmytype(i, false) == -1 && i != -1)
            d_basic[i+1]->EnableReceiver(false);
    }
    for (int i = 0; i < d_city->getMaxNoOfAdvancedProd(); i++)
    {
        d_advanced[i]->Show();
        if (advanced && (i == selected))
            d_advanced[i]->SetPressed(true);
        if (d_city->getArmytype(i, true) == -1)
            d_advanced[i]->EnableReceiver(false);
    }
    
    // 3
    d_b_up->EnableReceiver(true);
    d_b_down->EnableReceiver(true);

    if (d_city->getDefenseLevel() == 4)
        d_b_up->EnableReceiver(false);
    if (d_city->getDefenseLevel() == 1)
        d_b_down->EnableReceiver(false);

    // finally, some updating
    updateStats();
    PG_Application::SetBulkMode(false);
    Update();
}

void E_CityDialog::updateStats()
{
    PG_Application::SetBulkMode(true);
    
    // retrieve the current production
    const Army* a = 0;

    if (d_city->getProductionIndex() == -1)
    {
        d_l_name->SetText("n/a");
        d_l_strength->SetText("n/a");
        d_l_ranged->SetText("n/a");
        d_l_defense->SetText("n/a");
        d_l_production->SetText("n/a");
        d_l_upkeep->SetText("n/a");
        PG_Application::SetBulkMode(false);
        Update();
        return;
    }

    // from here on we can assume that a valid production is set
    a = d_city->getArmy(d_city->getProductionIndex(), d_city->getAdvancedProd());
    
    // fill the data, disable some updating stuff
    char buffer[51]; buffer[50] = '\0';

    PG_Application::SetBulkMode(true);
    
    d_l_name->SetText(a->getName().c_str());

    snprintf(buffer, 50, "%i", a->getStat(Army::STRENGTH, false));
    d_l_strength->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getStat(Army::RANGED, false));
    d_l_ranged->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getStat(Army::DEFENSE, false));
    d_l_defense->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getProduction());
    d_l_production->SetText(buffer);

    snprintf(buffer, 50, "%i", a->getUpkeep());
    d_l_upkeep->SetText(buffer);

    PG_Application::SetBulkMode(false);
    Update();
}

void E_CityDialog::updatePics()
{
    // as usual, disable blitting for the beginning
    PG_Application::SetBulkMode(true);

    // now for each production button, create an appropriate image and
    // assign this to the button; don't forget to store it in the cache
    // (we need the pointer for erasing lateron)
    unsigned int nbasic = d_city->getMaxNoOfBasicProd();
    unsigned int nadvanced = d_city->getMaxNoOfAdvancedProd();
    
    d_basic[0]->SetText("N");
    for (unsigned int i = 0; i < nbasic; i++)
    {
        int index = d_city->getArmytype(i, false);
        if (index == -1)
        {
            // production is not set; since this can happen because a production
            // is removed, it is neccessary to give the PG_Button another icon
            // which is 100% transparent.
            d_basic[i+1]->SetIcon(d_blanksurf);
            continue;
        }

        // copy the image for the production, assign it to the button and store
        // it in the cache 
        SDL_Surface* pic = GraphicsCache::getInstance()->getArmyPic(
                            Armysetlist::getInstance()->getStandardId(),
                            index, d_city->getPlayer(), 1, 0);
        pic = SDL_DisplayFormatAlpha(pic);
        d_basic[i+1]->SetIcon(pic);
        d_cache.push_back(pic);
    }

    for (unsigned int i = 0; i < nadvanced; i++)
    {
        // the procedure is almost the same
        int index = d_city->getArmytype(i, true);
        if (index == -1)
        {
            // production is not set; since this can happen because a production
            // is removed, it is neccessary to give the PG_Button another icon
            // which is 100% transparent.
            d_advanced[i]->SetIcon(d_blanksurf);
            continue;
        }

        // copy the image for the production, assign it to the button and store
        // it in the cache 
        SDL_Surface* pic = GraphicsCache::getInstance()->getArmyPic(
                            d_city->getPlayer()->getArmyset(),
                            index, d_city->getPlayer(), 1, 0);
        pic = SDL_DisplayFormatAlpha(pic);
        d_advanced[i]->SetIcon(pic);
        d_cache.push_back(pic);
    }

    // Erase the oldest items, they aren't needed anymore
    while (d_cache.size() > 7)
    {
        SDL_FreeSurface(*d_cache.begin());
        d_cache.erase(d_cache.begin());
    }

    PG_Application::SetBulkMode(false);
    Update();
}
