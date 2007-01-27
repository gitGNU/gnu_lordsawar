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

#include "about.h"
#include "defs.h"
#include "File.h"
#include <iostream>
#include <pgapplication.h>

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

AboutDialog::AboutDialog(PG_Widget* parent, PG_Rect rect)
  :PG_Window(parent, rect, _("About"), PG_Window::MODAL),timer(0),pos(0)
{
    initValues();

    int posx=10;
    int posy=110;
    int viewsize=150;

    SetBackground(File::getMiscPicture("about_screen.jpg", false),BKMODE_STRETCH);
 
    d_s_area =new PG_ScrollArea(this, PG_Rect(2,posy,320,viewsize));
    d_s_area->SetAreaHeight(s_devel.size()*15+s_graphics.size()*15+s_acontributors.size()*15+s_icontributors.size()*15+s_exmembers.size()*15+viewsize+290);

    posy+=10;

    PG_Label* label = new PG_Label(d_s_area, PG_Rect(posx,posy,200, 20),_("Creator of the project:"));
    label->SetFontColor(PG_Color(64, 137, 182));
    posy+=20;
    label = new PG_Label(d_s_area, PG_Rect(posx+25, posy, 130, 15),"Michael Bartl");    

    posy+=20;
    label = new PG_Label(d_s_area, PG_Rect(posx,posy,200, 20),_("Development:"));
    label->SetFontColor(PG_Color(64, 137, 182));
    
    posy+=5;
    for(unsigned int i=0;i<s_devel.size();i++)
    {
        posy+=15; 
        label = new PG_Label(d_s_area, PG_Rect(posx+25, posy, 130, 15),s_devel[i].c_str());    
    }
    posy+=20;

    label = new PG_Label(d_s_area, PG_Rect(posx, posy, 200, 20),_("Graphics:"));    
    label->SetFontColor(PG_Color(64, 137, 182));

    posy+=5;
    for(unsigned int i=0;i<s_graphics.size();i++)
    {
        posy+=15;
        label = new PG_Label(d_s_area, PG_Rect(posx+25, posy, 130, 15),s_graphics[i].c_str());    
    }
    posy+=20;
    
    label = new PG_Label(d_s_area, PG_Rect(posx, posy, 200, 20),_("Active Contributors:"));
    label->SetFontColor(PG_Color(64, 137, 182));

    posy+=5;
    for(unsigned int i=0;i<s_acontributors.size();i++)
    {
        posy+=15;
        label = new PG_Label(d_s_area, PG_Rect(posx+25, posy, 130, 15),s_acontributors[i].c_str());    
    }
    posy+=20;

    label = new PG_Label(d_s_area, PG_Rect(posx, posy, 230, 20),_("Inactive Members/Contributors:"));
    label->SetFontColor(PG_Color(64, 137, 182));

    posy+=5;
    for(unsigned int i=0;i<s_icontributors.size();i++)
    {
        posy+=15;
        label = new PG_Label(d_s_area, PG_Rect(posx+25, posy, 130, 15),s_icontributors[i].c_str());    
    }
    posy+=20;

    label = new PG_Label(d_s_area, PG_Rect(posx, posy, 200, 20),_("Ex Members:"));
    label->SetFontColor(PG_Color(64, 137, 182));

    posy+=5;
    for(unsigned int i=0;i<s_exmembers.size();i++)
    {
        posy+=15;
        label = new PG_Label(d_s_area, PG_Rect(posx+25, posy, 130, 15),s_exmembers[i].c_str());    
    }
    posy+=21;

    char tmp[20]; 
    sprintf(tmp,_("Version %s"),FL_VERSION);

    label = new PG_Label(d_s_area, PG_Rect(posx+35, posy, 120, 20),tmp);
    //label->SetFontColor(PG_Color(50, 150, 50));

    PG_Button* b_ok = new PG_Button(this, PG_Rect((Width() - 45),
                    (Height() - 25), 40, 20), _("OK"),1);

    b_ok->sigClick.connect(slot(*this, &AboutDialog::b_okClicked));
 
    timer=AddTimer(40);
}

AboutDialog::~AboutDialog()
{
    d_s_area->DeleteAll();
    delete d_s_area;
}

void AboutDialog::initValues()
{
    s_devel.push_back("Ulf Lorenz");
    s_devel.push_back("Andrea Paternesi");
    s_devel.push_back("Josef Spillner");
    s_devel.push_back("Vibhu Rishi");

    s_graphics.push_back("James Andrews");
    s_graphics.push_back("Tiziano Ottaviani");

    s_acontributors.push_back("Michael Scherer");
    s_acontributors.push_back("David Barnsdale");
    s_acontributors.push_back("Brian Duff");
    s_acontributors.push_back("Sauro Fabi");
    s_acontributors.push_back("Gunnar Lindholm");
    s_acontributors.push_back("Laszlo Toth");
    s_acontributors.push_back("Israel Lopez");
    s_acontributors.push_back("Fady Hossam");
    s_acontributors.push_back("Sean Rinehart");
    s_acontributors.push_back("Rene Saucedo");
    s_acontributors.push_back("Bogdan Czaplinski");

    s_icontributors.push_back("John Farrell");
    s_icontributors.push_back("Regis Leroy");
    s_icontributors.push_back("Richard Johnson");
    s_icontributors.push_back("Taylor Rolison");
    s_icontributors.push_back("Mark L. Amidon");
    s_icontributors.push_back("Thomas Plonka");

    s_exmembers.push_back("Tobias Mathes");
    s_exmembers.push_back("Chris Slater");
    s_exmembers.push_back("David Sterba");
    s_exmembers.push_back("Daniel Nilsson");
    s_exmembers.push_back("Marek Publicewicz");
    s_exmembers.push_back("Filip Kroczak");
    s_exmembers.push_back("Daniel Rigos");
    s_exmembers.push_back("Jimmy Chin");
}

Uint32 AboutDialog::eventTimer (ID id, Uint32 interval)
{
    debug("Scroll window! -- " << pos)

    if (pos<=d_s_area->GetAreaHeight()) 
    {
        // the bulk mode is a workaround as long as it is not implemented
        // in paragui itself
        PG_Application::SetBulkMode(true);
        pos++;
        d_s_area->ScrollTo(0,pos);
        PG_Application::SetBulkMode(false);
    }
    else 
    {
       RemoveTimer(timer);
    }
    
    Redraw();
    return interval;
}

bool AboutDialog::eventKeyDown(const SDL_KeyboardEvent* key)
{	
    switch (key->keysym.sym)
    {		
	case SDLK_RETURN:
	  b_okClicked(0);
	  break;
        default:
          break;
    }
	
    return true;
}

bool AboutDialog::b_okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}


//End of file
