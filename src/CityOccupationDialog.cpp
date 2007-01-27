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

#include "CityOccupationDialog.h"
#include <SDL_image.h>
#include <pgapplication.h>
#include <pgbutton.h>
#include <pglabel.h>
#include <pgmessagebox.h>
#include "citylist.h"
#include "playerlist.h"
#include "File.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

CityOccupationDialog::CityOccupationDialog(City* city)
    :PG_Window(0, PG_Rect((PG_Application::GetScreenWidth()-320)/2,
                (PG_Application::GetScreenHeight()-340)/2, 320, 340),
                _("Defeated city"), PG_Window::MODAL), d_b_pillage(0), d_city(city)
{
    d_background = File::getMiscPicture("city_occupied.jpg", false);
    SetBackground(d_background, 2);

    d_l_msg = new PG_Label(this, PG_Rect(10, 250, 280, 20), _("Choose your way!"));
    d_l_msg->SetAlignment(PG_Label::CENTER);

    // We look if the city can be pillaged at all
    bool maypillage = false;
    if ((city->getArmytype(0, false) != -1) || (city->getDefenseLevel() > 1))
    {
        maypillage = true;
    }

    d_b_occupy = new PG_Button(this, PG_Rect(20, my_height - 40, 80, 30), _("Occupy"),0);
    d_b_raze = new PG_Button(this, PG_Rect(220, my_height - 40, 80, 30), _("Raze"),0);
    d_b_occupy->sigClick.connect(slot(*this, &CityOccupationDialog::b_occupyClicked));
    d_b_raze->sigClick.connect(slot(*this, &CityOccupationDialog::b_razeClicked));

    if (maypillage)
    {
        d_b_pillage = new PG_Button(this, PG_Rect(120, my_height - 40, 80, 30), _("Pillage"),0);
	d_b_pillage->sigClick.connect(slot(*this, &CityOccupationDialog::b_pillageClicked));
    }
}

CityOccupationDialog::~CityOccupationDialog()
{
    delete d_b_occupy;
    delete d_b_pillage;
    delete d_b_raze;
    delete d_l_msg;
    SDL_FreeSurface(d_background);
}

bool CityOccupationDialog::b_occupyClicked(PG_Button* btn)
{
    Playerlist::getInstance()->getActiveplayer()->cityOccupy(d_city);
    QuitModal();
    return true;
}

bool CityOccupationDialog::b_pillageClicked(PG_Button* btn)
{
    int gold;
    char buf[101]; buf[100] = '\0';
    Playerlist::getInstance()->getActiveplayer()->cityPillage(d_city, gold);
    snprintf(buf,100,_("You have pillaged %d gold pieces."), gold);
    QuitModal();
    PG_MessageBox mb(this, PG_Rect(my_width/2-100, my_height/2-87, 200, 150), _("Pillage City"),
                buf, PG_Rect(60, 110, 80, 30), _("OK"));
    mb.Show();
    mb.RunModal();
    mb.Hide();
    return true;
}

bool CityOccupationDialog::b_razeClicked(PG_Button* btn)
{
    Playerlist::getInstance()->getActiveplayer()->cityRaze(d_city);
    QuitModal();
    return true;
}

// End of file
