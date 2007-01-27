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

#include <sstream>
#include <pgmessagebox.h>
#include <pgapplication.h>
#include <pgbutton.h>
#include "cityinfo.h"
#include "army.h"
#include "armysetlist.h"
#include "player.h"
#include "city.h"
#include "GraphicsCache.h"
#include "VectorDialog.h"
#include "GameMap.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

CityInfo::CityInfo(City* city)
    :PG_Window(0, PG_Rect((PG_Application::GetScreenWidth()-510)/2,
                (PG_Application::GetScreenHeight()-460)/2, 510, 460),
    _("City info"), PG_Window::MODAL), d_city(city)
{
    char buffer[101];
    buffer[100] = '\0';

    // apply an appropriate prefix to the name
    switch (d_city->getDefenseLevel())
    {
        case 1:
            snprintf(buffer, 100, _("Village of %s"), d_city->getName().c_str());
            break;
        case 2:
            snprintf(buffer, 100, _("Town of %s"), d_city->getName().c_str());
            break;
        case 3:
            snprintf(buffer, 100, _("City of %s"), d_city->getName().c_str());
        case 4:
            snprintf(buffer, 100, _("Fortress of %s"), d_city->getName().c_str());
    }
    new PG_Label(this, PG_Rect(20, 25, my_width - 20, 30), buffer);

    snprintf(buffer, 100, _("Income: %i gold , Defense level : %i , Capital : %s"),
             d_city->getGold(), d_city->getDefenseLevel(),
             d_city->isCapital()?_("yes"):_("no"));
    d_l_defense= new PG_Label(this, PG_Rect(20, 55, 300, 20), buffer);
    d_l_defense->SizeWidget(d_l_defense->GetTextWidth(),20,true);

    // the basic production buttons
    d_l_basic = new PG_Label(this, PG_Rect(20, 80, 210, 20), _("Basic productions"));
    
    // add an empty button to the buttongroup for "NO PRODUCTION"
    d_b_no_production = new PG_Button(this, PG_Rect(20, 110, 60, 60), "N",0);
    d_b_no_production->sigClick.connect(slot(*this, &CityInfo::b_productionClicked));
    d_b_no_production->SetToggle(true);

    unsigned int as = Armysetlist::getInstance()->getStandardId();
    for (int i = 0; i < 4; i++)
    {
        int type = d_city->getArmytype(i, false);

        d_b_basic[i] = new PG_Button(this, PG_Rect(80 + i*60, 110, 60, 60),"",i+1);
        d_b_basic[i]->SetToggle(true);
        d_b_basic[i]->sigClick.connect(slot(*this, &CityInfo::b_productionClicked));

        if (i >= d_city->getNoOfBasicProd()) 
        {
            d_b_basic[i]->Hide();
            continue;
        }
        
        if (type != -1)
        {
            //use GraphicsCache to load army pics because of player-specific
            //colors
            SDL_Surface* armypic = GraphicsCache::getInstance()->getArmyPic(as, type,
                                                        d_city->getPlayer(),1,NULL);
            d_b_basic[i]->SetIcon(armypic, 0, 0);
        }
    }

    // set advanced production
    d_l_advanced = new PG_Label(this, PG_Rect(20, 195, 200, 20), _("Advanced productions"));

    as = d_city->getPlayer()->getArmyset();
    for (int i = 0; i < 3; i++)
    {
        int type = d_city->getArmytype(i, true);

        d_b_advanced[i] = new PG_Button(this, PG_Rect(64 + i*60, 215, 60, 60),"",i+10);
        d_b_advanced[i]->SetToggle(true);
        d_b_advanced[i]->sigClick.connect(slot(*this, &CityInfo::b_productionClicked));

        if (i >= d_city->getNoOfAdvancedProd())
        {
            d_b_advanced[i]->Hide();
            continue;
        }

        if (type != -1)
        {
            SDL_Surface* armypic = GraphicsCache::getInstance()->getArmyPic(as, type,
                                                        d_city->getPlayer(),1,NULL);
            d_b_advanced[i]->SetIcon(armypic, 0, 0);
        }
    }
    
    new PG_Label(this, PG_Rect(20, 300, 80, 20), _("Name:"));
    new PG_Label(this, PG_Rect(20, 320, 80, 20), _("Duration:"));
    new PG_Label(this, PG_Rect(20, 340, 80, 20), _("Strength:"));
    new PG_Label(this, PG_Rect(20, 360, 80, 20), _("Ranged:"));
    new PG_Label(this, PG_Rect(20, 380, 80, 20), _("Moves:"));
    new PG_Label(this, PG_Rect(20, 400, 80, 20), _("Upkeep:"));

    d_l_production = new PG_Label(this, PG_Rect(120, 300, 160, 20), "0");
    d_l_duration = new PG_Label(this, PG_Rect(120, 320, 160, 20), "0");
    d_l_strength = new PG_Label(this, PG_Rect(120, 340, 160, 20), "0");
    d_l_ranged = new PG_Label(this, PG_Rect(120, 360, 160, 20), "0");
    d_l_moves = new PG_Label(this, PG_Rect(120, 380, 160, 20), "0");
    d_l_upkeep = new PG_Label(this, PG_Rect(120, 400, 160, 20), "0");

    d_b_upgrade = new PG_Button(this, PG_Rect(my_width - 140, my_height - 180, 120, 30), _("Upgrade"),1);
    d_b_vectoring = new PG_Button(this, PG_Rect(my_width - 140, my_height - 145, 120, 30), _("Vectoring"),2);
    d_b_buy_basic = new PG_Button(this, PG_Rect(my_width - 140, my_height - 110, 120, 30), _("Buy Basic"),3);
    d_b_buy_advanced = new PG_Button(this, PG_Rect(my_width - 140, my_height - 75, 120, 30), _("Buy Advanced"),4);
    d_b_close = new PG_Button(this, PG_Rect(my_width - 140, my_height - 40, 120, 30), _("Close"),5);


    d_b_upgrade->sigClick.connect(slot(*this, &CityInfo::b_upgradeClicked));
    d_b_vectoring->sigClick.connect(slot(*this, &CityInfo::b_vectoringClicked));
    d_b_buy_basic->sigClick.connect(slot(*this, &CityInfo::b_buyBasicClicked));
    d_b_buy_advanced->sigClick.connect(slot(*this, &CityInfo::b_buyAdvancedClicked));
    d_b_close->sigClick.connect(slot(*this, &CityInfo::b_closeClicked));

    // if something is produced put button to DOWN    
    //if (d_city->getProduction() != -1) d_b_production[d_city->getProduction()]->SetPressed(true);

    updateProductionStats();
    checkButtons();
}

CityInfo::~CityInfo()
{
    delete d_l_production;
    delete d_l_duration;
    delete d_l_strength;
    delete d_l_defense;
    delete d_l_moves;
    delete d_l_upkeep;
    delete d_l_basic;
    delete d_l_advanced;

    delete d_b_no_production;
    for (int i = 0; i < 4; i++)
        delete d_b_basic[i];
    for (int i = 0; i < 3; i++)
        delete d_b_advanced[i];
    delete d_b_upgrade;
    delete d_b_buy_basic;
    delete d_b_buy_advanced;
    delete d_b_close; 
}

bool CityInfo::b_upgradeClicked(PG_Button* btn)
{
    int gold = neededGold();

    if ((gold < 0) || (d_city->getPlayer()->getGold() < gold))
    {
        char tmp[2];
        sprintf(tmp,"%d",CITY_LEVELS);
        string tmp1= string("Not enough gold or already reached ugrade level ")+ string(tmp) + string("!");

        PG_MessageBox mb(GetParent(), PG_Rect(200, 200, 200, 150),
                _("Upgrade City"),
                _(tmp1.c_str()),
                PG_Rect(60, 100, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
        return true;
    }

    char buffer[100]; buffer[99]='\0';

    snprintf(buffer, 99,
            ngettext("Do you want to upgrade the city level for %i gold?",
                     "Do you want to upgrade the city level for %i gold?", gold), gold);
    
    PG_Rect rect_1(40, 90, 80, 30);
    PG_Rect rect_2(130, 90, 80, 30);
    PG_MessageBox mb(GetParent(), PG_Rect(200, 200, 250, 130),
               _("Upgrade city"), buffer,
               rect_1, _("Yes"), rect_2, _("No"));
    mb.Show();

    if (mb.RunModal() == 1)
    {
        d_city->getPlayer()->cityUpgradeDefense(d_city);

        snprintf(buffer, 99, _("Income: %i gold coins , Defense level : %i , Capital : %s"),
                             d_city->getGold(), d_city->getDefenseLevel(),
                             d_city->isCapital()?_("yes"):_("no"));
        d_l_defense->SetText(buffer);
    }
    mb.Hide();

    checkButtons();

    return true;
}

bool CityInfo::b_vectoringClicked(PG_Button* btn)
{

    debug("production index=" << d_city->getProductionIndex())

    if (d_city->getProductionIndex()!=-1) 
    {
        PG_Rect r(0, 0, 2 * PG_Application::GetScreenWidth()/3, 2*PG_Application::GetScreenHeight()/3);
        r.x = r.w /4;
        r.y = r.h /4;
        VectorDialog dialog(d_city,GetParent(),r);
        dialog.Show();
        dialog.RunModal();
        dialog.Hide();
    }
    else 
    {
        PG_MessageBox mb(this, PG_Rect(my_width/2-100, my_height/2-87, 200, 150), _("Vectoring"),
                _("No production selected!"), PG_Rect(60, 110, 80, 30), _("OK"));
        mb.Show();
        mb.RunModal();
        mb.Hide();
    }

    return true;
}

bool CityInfo::b_buyBasicClicked(PG_Button* btn)
{
    D_Buy_Production dialog(d_city, false, GetParent(),
                            PG_Rect(my_width/2-225, my_height/2-150, 450, 300));
    dialog.Show();
    dialog.RunModal();
    dialog.Hide();

    int chosen_army = dialog.getChosenArmy();

    if (chosen_army == -1)
    {
        return true;
    }

    // Now look if there are free slots in the city, else we need to ask
    // the user if he wants to replace a production.
    int slot = getEmptySlot(false);
    
    if (slot == -1)
    {
        // try to get the selected unit
        slot = d_city->getProductionIndex();
        if ((slot == -1) || d_city->getAdvancedProd())
        {
            PG_MessageBox info(GetParent(), PG_Rect(200, 200, 400, 150),
                    _("No free production"),
                    _("There are no free production slots available. Select the one you want to have replaced or upgrade the city first."),
                    PG_Rect(160, 110, 80, 30), _("OK"));
            info.Show();
            info.RunModal();
            return true;
        }
        
        char buffer[101]; buffer[100]='\0';
        snprintf(buffer, 100, _("Do you really want to replace production \"%s\"?"),
                 d_city->getArmy(slot, false)->getName().c_str());

        PG_MessageBox mb(GetParent(), PG_Rect(200, 200, 250, 130),
                         _("No free production slot"), buffer,
                         PG_Rect(40, 90, 80, 30), _("Yes"),
                         PG_Rect(130, 90, 80, 30), _("No"));
        mb.Show();
        if (mb.RunModal() == 2)
            return true;
    }

    if (!d_city->getPlayer()->cityBuyProduction(d_city, slot, chosen_army, false))
        cerr <<_("Player::city_buyBasicProduction returned false where it shouldn't ");

    // And fill the new slots as well as update the graphics
    d_b_basic[slot]->SetIcon(GraphicsCache::getInstance()->getArmyPic(
                            Armysetlist::getInstance()->getStandardId(),
                            chosen_army, d_city->getPlayer(),1,NULL), 0, 0);
    d_b_basic[slot]->Show();
    d_b_basic[slot]->EnableReceiver(true);
    
    checkButtons();
    updateProductionStats();

    return true;
}

int CityInfo::getEmptySlot(bool advanced)
{
    if (advanced)
    {
        for (int i = 0; i < d_city->getNoOfAdvancedProd(); i++)
            if (d_city->getArmytype(i, true) == -1)
                return i;
    }
    else
    {
        for (int i = 0; i < d_city->getNoOfBasicProd(); i++)
            if (d_city->getArmytype(i, false) == -1)
                return i;
    }
       
    return -1;
}

bool CityInfo::b_productionClicked(PG_Button* btn)
{
    // id is the ID of the button that sent this event
    // id 0 is the "No production" button
    
    int id=btn->GetID();

    if (id / 10 != 0)
    {
        // advanced production
        d_city->getPlayer()->cityChangeProduction(d_city, id%10, true);
    }
    else
    {
        // basic production, the id scheme is a bit incoherent...
        d_city->getPlayer()->cityChangeProduction(d_city, id - 1, false);
    }
        
    updateProductionStats();
    
    return true;
}

void CityInfo::updateProductionStats()
{
    // we don't want to have every single action redrawn
    PG_Application::SetBulkMode(true);
    
    // unpress all buttons
    d_b_no_production->SetPressed(false);
    for (int i = 0; i < d_city->getNoOfBasicProd(); i++)
        d_b_basic[i]->SetPressed(false);
    for (int i = 0; i < d_city->getNoOfAdvancedProd(); i++)
        d_b_advanced[i]->SetPressed(false);

    // shortcuts
    int slot = d_city->getProductionIndex();
    bool advanced = d_city->getAdvancedProd();
    
    if (slot == -1)
    {
        // no production
        d_l_production->SetText(_("None"));
        d_l_duration->SetText(_("None"));
        d_l_strength->SetText(_("None"));
        d_l_ranged->SetText(_("None"));
        d_l_moves->SetText(_("None"));
        d_l_upkeep->SetText(_("None"));
    }
    else
    {
        const Army* a = d_city->getArmy(slot, advanced);

        // press the correct buttons
        if (advanced)
            d_b_advanced[slot]->SetPressed(true);
        else
            d_b_basic[slot]->SetPressed(true);

        // update the labels
        char buffer[41]; buffer[40]='\0';
        d_l_production->SetText(a->getName().c_str());
        snprintf(buffer, 40, "%i (%i)", a->getProduction(), d_city->getDuration());
        d_l_duration->SetText(buffer);
        snprintf(buffer, 40, "%i", a->getStat(Army::STRENGTH, false));
        d_l_strength->SetText(buffer);
        snprintf(buffer, 40, "%i", a->getStat(Army::RANGED, false));
        d_l_ranged->SetText(buffer);
        snprintf(buffer, 40, "%i", a->getStat(Army::MOVES, false));
        d_l_moves->SetText(buffer);
        snprintf(buffer, 40, "%i", a->getUpkeep());
        d_l_upkeep->SetText(buffer);
    }

    // get everything back to normal and redraw
    PG_Application::SetBulkMode(false);
    Redraw();
}

bool CityInfo::b_buyAdvancedClicked(PG_Button* btn)
{
    D_Buy_Production dialog(d_city, true, GetParent(),
                            PG_Rect(my_width/2-225, my_height/2-150, 450, 300));
    dialog.Show();
    dialog.RunModal();
    dialog.Hide();

    int chosen_army = dialog.getChosenArmy();

    if (chosen_army == -1)
    {
        return true;
    }

    // Now look if there are free slots in the city, else we need to ask
    // the user if he wants to replace a production.
    int slot = getEmptySlot(true);
    
    if (slot == -1)
    {
        // try to get the selected unit
        slot = d_city->getProductionIndex();
        if ((slot == -1) || !d_city->getAdvancedProd())
        {
            PG_MessageBox info(GetParent(), PG_Rect(200, 200, 400, 150),
                    _("No free production"),
                    _("There are no free production slots available. Select the one you want to have replaced or upgrade the city first."),
                    PG_Rect(160, 110, 80, 30), _("OK"));
            info.Show();
            info.RunModal();
            return true;
        }
        
        char buffer[101]; buffer[100]='\0';
        snprintf(buffer, 100, _("Do you really want to replace production \"%s\"?"),
                 d_city->getArmy(slot, true)->getName().c_str());

        PG_MessageBox mb(GetParent(), PG_Rect(200, 200, 250, 130),
                         _("No free production slot"), buffer,
                         PG_Rect(40, 90, 80, 30), _("Yes"),
                         PG_Rect(130, 90, 80, 30), _("No"));
        mb.Show();
        if (mb.RunModal() == 2)
            return true;
    }

    if (!d_city->getPlayer()->cityBuyProduction(d_city, slot, chosen_army, true))
        cerr <<_("Player::city_buyAdvancedProduction returned false where it shouldn't ");

    // And fill the new slots as well as update the graphics
    d_b_advanced[slot]->SetIcon(GraphicsCache::getInstance()->getArmyPic(
                            d_city->getPlayer()->getArmyset(), chosen_army,
                            d_city->getPlayer(),1,NULL), 0, 0);
    d_b_advanced[slot]->Show();
    d_b_advanced[slot]->EnableReceiver(true);
    
    checkButtons();
    updateProductionStats();

    return true;
}

int CityInfo::neededGold()
{
    if (d_city->getDefenseLevel() == 1) return 1000;
    else if (d_city->getDefenseLevel() == 2) return 2000;
    else if (d_city->getDefenseLevel() == 3) return 3000;
    return -1;
}

void CityInfo::checkButtons()
{
    // Upgrade button
    if ((d_city->getDefenseLevel() < (int)CITY_LEVELS && d_city->getPlayer()->getGold() >= neededGold()))
    {
        d_b_upgrade->Show();
    }
    else d_b_upgrade->Hide();

    // check the production slots
    for (int i = 0; i < 4; i++)
        if (i < d_city->getNoOfBasicProd())
            d_b_basic[i]->Show();

    for (int i = 0; i < 3; i++)
        if (i < d_city->getNoOfAdvancedProd())
            d_b_advanced[i]->Show();
}

bool CityInfo::b_closeClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

static unsigned int width  = 200;
static unsigned int height = 120;

CityInfoSmall::CityInfoSmall(City* city, int screen_x, int screen_y)
    :PG_ThemeWidget(0, PG_Rect(screen_x, screen_y, width, height)), d_city(city)
{
    char buf[100+1]; buf[100] = '\0';

    unsigned int y = 20;
    const unsigned int col1 = 15, col2 = 115, h = 15;
    const unsigned int step_y = h + 4;

    if (d_city->isCapital())
        snprintf(buf, 100, "%s (%s)", city->getName().c_str(), _("capital"));
    else
        snprintf(buf, 100, "%s", city->getName().c_str());
    new PG_Label(this, PG_Rect(col1, y, 150, h), buf); 

    y += step_y;
    new PG_Label(this, PG_Rect(col1, y, 90, h), _("Income:")); 
    snprintf(buf, 100, "%i", city->getGold());
    new PG_Label(this, PG_Rect(col2, y, 100, h), buf);

    y += step_y;
    new PG_Label(this, PG_Rect(col1, y, 90, h), _("Defense:")); 
    snprintf(buf, 100, "%i", city->getDefenseLevel());
    new PG_Label(this, PG_Rect(col2, y, 100, h), buf);

    if (city->isBurnt())
    {
        y += step_y;
        new PG_Label(this, PG_Rect(col1, y, 90, h), _("Status:")); 
        new PG_Label(this, PG_Rect(col2, y, 100, h), _("Razed!"));
    }

    snprintf(buf, 100, "%i", city->getDefenseLevel());
    new PG_Label(this, PG_Rect(col2, y, 100, h), buf);

    y += step_y;
    new PG_Label(this, PG_Rect(col1, y, 80, h), _("Position:")); 
    snprintf(buf, 100, "(%i,%i)", city->getPos().x, city->getPos().y);
    new PG_Label(this, PG_Rect(col2, y, 100, h), buf); 


    // SetCapture, so the cityinfosmall object can detect
    // the releasing of the
    // right mouse button outside the CityInfoSmall window
    SetCapture();
}

CityInfoSmall::~CityInfoSmall()
{
}

bool CityInfoSmall::eventMouseButtonUp(const SDL_MouseButtonEvent* event)
{
    if(event->button == SDL_BUTTON_RIGHT)
        QuitModal();
    return true;
}
// End of file
