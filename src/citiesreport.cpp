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
#include <pgbutton.h>
#include <pglabel.h>
#include <pgapplication.h>

#include "citiesreport.h"
#include "armysetlist.h"
#include "playerlist.h"

//first comes the implementation of CityItem. A CityItem is a bar of ~420x80
//pixels which displays infos about a single city.
class CityItem : public PG_Widget
{
    public:
        CityItem(PG_Widget* parent, PG_Rect rect);
        ~CityItem();

        void readData(City* city, bool force_show);

        SigC::Signal1<void, City*> sclicked;

    private:
        bool eventMouseButtonDown(const SDL_MouseButtonEvent * event);

        PG_Label* d_l_name;
        PG_Label* d_l_owner;
        PG_Label* d_l_gold;
        PG_Label* d_l_production;
        PG_Label* d_l_finished;
        PG_Label* d_l_noinfo;
        PG_Button* d_b_production;
        City* d_city;
};

CityItem::CityItem(PG_Widget* parent, PG_Rect rect)
    :PG_Widget(parent, rect)
{
    d_l_name = new PG_Label(this, PG_Rect(10, 10, 150, 20), "");
    d_l_owner = new PG_Label(this, PG_Rect(10, 40, 150, 20), "");
    d_l_gold =new PG_Label(this, PG_Rect(10, 60, 100, 20), "");
    d_l_production = new PG_Label(this, PG_Rect(260, 20, 150, 20), "");
    d_l_finished = new PG_Label(this, PG_Rect(260, 50, 100, 20), "");
    d_b_production = new PG_Button(this, PG_Rect(190, 10, 50, 50), "",0);
    d_b_production->EnableReceiver(false);
    d_b_production->Hide();
    d_l_noinfo = new PG_Label(this,PG_Rect(190, 20, 200, 50), _("no information"));
    d_l_noinfo->Hide();
    d_l_noinfo->SetFontColor(PG_Color(200, 100, 100));
    d_l_noinfo->SetFontSize(16);
}

CityItem::~CityItem()
{
}

void CityItem::readData(City* city, bool force_detail)
{
    // TODO: this workaround becomes obsolete as soon as paragui supports
    // removal of a button image
    delete d_b_production;
    d_b_production = new PG_Button(this, PG_Rect(190, 10, 50, 50), "",0);
    d_b_production->EnableReceiver(false);
    

    d_city = city;
    
    d_l_name->SetText(city->getName().c_str());
    SDL_Color color = city->getPlayer()->getColor();
    d_l_name->SetFontColor(PG_Color(color.r, color.g, color.b));
    
    d_l_owner->SetText(city->getPlayer()->getName().c_str());
    d_l_gold->SetTextFormat(_("gold: %i"), city->getGold());

    if ((city->getPlayer() == Playerlist::getInstance()->getActiveplayer())
        || force_detail)
    {
        const Army* a = city->getArmy(city->getProductionIndex());
        if (a == 0)
        //i.e. no production selected
        {
            d_b_production->SetText("N");
            d_b_production->SetFontSize(20);
            d_l_production->SetText(_("None"));
            d_l_finished->SetText(_("n.a."));
        }
        else
        {
            d_b_production->SetText("");
            d_b_production->SetIcon(a->getPixmap(), 0, 0);
            d_l_production->SetText(a->getName().c_str());
            if (city->getProductionIndex() != -1) 
                d_l_finished->SetTextFormat("%i/%i",
                        city->getDuration(), a->getProduction());
            else
                d_l_finished->SetTextFormat(_("Idle"));
        }

        d_b_production->Show();
        d_l_production->Show();
        d_l_finished->Show();
        d_l_noinfo->Hide();
    }
    else    //city belongs to another player, so don't show info about it
    {
        d_b_production->Hide();
        d_l_production->Hide();
        d_l_finished->Hide();
        d_l_noinfo->Show();
    }
}

bool CityItem::eventMouseButtonDown(const SDL_MouseButtonEvent* event)
{
    if (!IsVisible())
        return true;

    sclicked.emit(d_city);
    return true;
}


//now comes the CitiesReport class
CitiesReport::CitiesReport(PG_Widget* parent, PG_Rect rect, bool showall)
    :PG_Window(parent, rect, _("city report"), PG_Window::MODAL), d_index(0),
    d_showall(showall)
{
    //I assume 500x400 pixels here as well
    d_b_ok = new PG_Button(this, PG_Rect(430, 370, 90, 25), _("OK"),2);
    d_b_ok->sigClick.connect(slot(*this, &CitiesReport::b_okClicked));

    d_b_up = new PG_Button(this, PG_Rect(430, 150, 90, 25), _("Prev."),0);
    d_b_up->sigClick.connect(slot(*this, &CitiesReport::b_upClicked));

    d_b_down = new PG_Button(this, PG_Rect(430, 250, 90, 25), _("Next"),1);
    d_b_down->sigClick.connect(slot(*this, &CitiesReport::b_downClicked));

    d_l_number = new PG_Label(this, PG_Rect(430, 35, 60, 20), "");

    d_items[0] = new CityItem(this, PG_Rect(10, 30, 420, 80));
    d_items[1] = new CityItem(this, PG_Rect(10, 120, 420, 80));
    d_items[2] = new CityItem(this, PG_Rect(10, 210, 420, 80));
    d_items[3] = new CityItem(this, PG_Rect(10, 300, 420, 80));

    for (int i = 0; i < 4; i++)
        d_items[i]->sclicked.connect(SigC::slot(*this, &CitiesReport::citySelected));
    
    //now get all cities sorted by player with the activeplayer first and
    //the cities of the neutral player last
    Citylist* cl = Citylist::getInstance();
    Playerlist* pl = Playerlist::getInstance();

    d_citylist.resize(cl->size());
    //first, cities of the active player

    for (Citylist::iterator cit = cl->begin(); cit != cl->end(); cit++)
    {
        if ((*cit).getPlayer() == Playerlist::getInstance()->getActiveplayer())
        {
            d_citylist[d_index] = &(*cit);
            d_index++;
        }
    }

    //now the other non-neutral players
    for (Playerlist::iterator pit = pl->begin(); pit != pl->end(); pit++)
    {
        if (((*pit) == pl->getActiveplayer()) || ((*pit) == pl->getNeutral()))
            continue;

        for (Citylist::iterator cit = cl->begin(); cit != cl->end(); cit++)
        {
            if ((*cit).getPlayer() == (*pit))
            {
                d_citylist[d_index] = &(*cit);
                d_index++;
            }
        }
    }

    //now the neutral player
    for (Citylist::iterator cit = cl->begin(); cit != cl->end(); cit++)
    {
        // This is needed for the editor when the active player 
        // at the beginning is the neutral player and we do not want to
        // add the same cities (this generates a segfault)

        if ((*cit).getPlayer()== pl->getActiveplayer())
	    continue;

        if ((*cit).getPlayer() == pl->getNeutral())
        {
            d_citylist[d_index] = &(*cit);
            d_index++;
        }
    }

    d_index = 0;

    fillCityItems();
}

CitiesReport::~CitiesReport()
{
    d_citylist.clear();
}

bool CitiesReport::b_okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

bool CitiesReport::b_upClicked(PG_Button* btn)
{
    // Andrea : If the number of cities is less than 4 there will be a Seg. fault.
    if (d_citylist.size() < 4 ) return true;

    d_index--;
    if(d_index < 0)
    d_index = 0;

    fillCityItems();

    return true;
}

bool CitiesReport::b_downClicked(PG_Button* btn)
{
    // Andrea : If the number of cities is less than 4 there will be a Seg. fault.
    if (d_citylist.size() < 4 ) return true;

    d_index++;
    if ((d_index+4) > (int) d_citylist.size())
        d_index = d_citylist.size() - 4;

    fillCityItems();

    return true;
}

void CitiesReport::citySelected(City* c)
{
    sselectingCity(c->getPos());
    Update();
}

void CitiesReport::fillCityItems()
{
    // disable redrawing of _everything_
    PG_Application::SetBulkMode(true);
    
    d_l_number->SetTextFormat("%i - %i", d_index+1, d_index + 4);

    // Andrea : If the number of cities is less than 4 there will be a Seg. fault.
    int value=4;
    if (d_citylist.size() < 4 ) value=(int) d_citylist.size();

    for (int i = 0; i < value; i++)
    {
        d_items[i]->readData(d_citylist[d_index+i], d_showall);
        d_items[i]->Redraw();
    }

    // and reenable it
    PG_Application::SetBulkMode(false);
    Update();
}

bool CitiesReport::eventKeyDown(const SDL_KeyboardEvent* key)
{
    switch(key->keysym.sym)
    {
        case SDLK_RETURN:
            b_okClicked(0);
            break;
        case SDLK_UP:
            b_upClicked(0);
            break;
        case SDLK_DOWN:
            b_downClicked(0);
            break;
        default:
            break;
    }
    return true;
}
