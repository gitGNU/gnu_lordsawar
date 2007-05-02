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
#include "stacklist.h"
#include "File.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

CityOccupationDialog::CityOccupationDialog(City* city)
    :PG_Window(0, Rectangle((PG_Application::GetScreenWidth()-320)/2,
                (PG_Application::GetScreenHeight()-340)/2, 320, 340),
                _("Victory!"), PG_Window::MODAL), d_b_pillage(0), d_city(city)
{
    char *s1;
    char buf[101]; buf[100] = '\0';
    d_background = File::getMiscPicture("city_occupied.jpg", false);
    SetBackground(d_background, 2);

    switch (rand() % 4)
    {
      case 0: s1 = "triumphed"; break;
      case 1: s1 = "claimed victory"; break;
      case 2: s1 = "shown no mercy"; break;
      case 3: s1 = "slain the foe"; break;
    }
    Playerlist::getInstance()->getActiveplayer()->cityOccupy(d_city);
    Army *h = Playerlist::getActiveplayer()->getStacklist()->getActivestack()->getFirstHero();
    if (h == NULL)
      snprintf(buf,100,_("%s you have %s"), Playerlist::getInstance()->getActiveplayer()->getName(false).c_str(), s1);
    else
      snprintf(buf,100,_("%s you have %s"), h->getName().c_str(), s1);
    d_l_msg = new PG_Label(this, PG_Rect(15, 60, 280, 20), buf);
    d_l_msg->SetAlignment(PG_Label::CENTER);
    snprintf(buf,100,_("in the battle of %s"), city->getName().c_str());
    d_l_msg = new PG_Label(this, PG_Rect(15, 80, 280, 20), buf);
    d_l_msg->SetAlignment(PG_Label::CENTER);
    d_l_msg = new PG_Label(this, PG_Rect(15, 100, 280, 20), _("The city is yours!"));
    d_l_msg->SetAlignment(PG_Label::CENTER);
    d_l_msg = new PG_Label(this, PG_Rect(15, 120, 280, 20), _("Will you..."));
    d_l_msg->SetAlignment(PG_Label::CENTER);

    // We look if the city can be pillaged or sacked at all
    bool maypillage = false;
    bool maysack = false;
    if (city->getNoOfBasicProd() > 0)
    {
      maypillage = true;
    }
    if (city->getNoOfBasicProd() > 1)
    {
      maysack = true;
    }

    d_b_occupy = new PG_Button(this, PG_Rect(25, my_height - 40, 60, 30), _("Occupy"),0);
    d_b_raze = new PG_Button(this, PG_Rect(235, my_height - 40, 60, 30), _("Raze"),0);
    d_b_occupy->sigClick.connect(slot(*this, &CityOccupationDialog::b_occupyClicked));
    d_b_raze->sigClick.connect(slot(*this, &CityOccupationDialog::b_razeClicked));

    if (maypillage)
    {
      d_b_pillage = new PG_Button(this, PG_Rect(95, my_height - 40, 60, 30), _("Pillage"),0);
      d_b_pillage->sigClick.connect(slot(*this, &CityOccupationDialog::b_pillageClicked));
    }
    if (maysack)
    {
      d_b_sack = new PG_Button(this, PG_Rect(165, my_height - 40, 60, 30), _("Sack"),0);
      d_b_sack->sigClick.connect(slot(*this, &CityOccupationDialog::b_sackClicked));
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

bool CityOccupationDialog::b_sackClicked(PG_Button* btn)
{
    int gold;
    char buf[101]; buf[100] = '\0';
    Playerlist::getInstance()->getActiveplayer()->citySack(d_city, gold);
    snprintf(buf,100,_("You have sacked %d gold pieces."), gold);
    QuitModal();
    PG_MessageBox mb(this, PG_Rect(my_width/2-100, my_height/2-87, 200, 150), _("Sack City"),
                buf, PG_Rect(60, 110, 80, 30), _("OK"));
    mb.Show();
    mb.RunModal();
    mb.Hide();
    return true;
}

bool CityOccupationDialog::b_pillageClicked(PG_Button* btn)
{
    int gold;
    char buf[101]; buf[100] = '\0';
    Playerlist::getInstance()->getActiveplayer()->cityPillage(d_city, gold);
    snprintf(buf,100,_("You have pillaged %d gold pieces."), gold);
    QuitModal();
    PG_MessageBox mb(this, Rectangle(my_width/2-100, my_height/2-87, 200, 150), _("Pillage City"),
                buf, Rectangle(60, 110, 80, 30), _("OK"));
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
