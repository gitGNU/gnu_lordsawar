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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.*

#include <string>
#include <pglistboxitem.h>
#include "File.h"
#include "Configuration.h"
#include "VectorDialog.h"
#include "MainWindow.h"
#include "w_edit.h"
#include "defs.h"
#include "GameMap.h"
#include "vectormap.h"
#include "citylist.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

VectorDialog::VectorDialog(City *city,PG_Widget* parent, Rectangle rect)
    :PG_Window(parent, rect, _("Vectoring"),PG_Window::MODAL), d_city(city)
{
    debug("VectorDialog()");
    // The vector dialog consists of three parts:
    // - a game map (upper left) that takes a lrge portion, where you select the
    //   city's vector
    // - some buttons, coordinate labels etc. below the game map
    // - items to select the city to choose etc. (whatever we implement later :))
    //   on the right
    // When specifying sizes, we operate with a mixture of fixed and relative
    // sizes. The larger the dialog, the more space the vector map may e.g. take.


    // the largest part should be the vectormap. since it can scale the map
    // dynamically in x and y direction, we place it somehow fixed.

    // handy later on when we need to place everything relative to the vector map
    int px = 8;     // x position of the vectormap
    int py = 32;    // y position
    int width = my_width - 200;
    int height = my_height - 80;
    
    d_vectormap = new VectorMap(city, this, Rectangle(px, py, width, height));
    d_vectormap->smovVectMouse.connect(sigc::slot(*this, &VectorDialog::movingMouse));
    d_vectormap->sclickVectMouse.connect(sigc::slot(*this, &VectorDialog::clickedMouse));


    // Second part: place the buttons and labels below the vectormap

    Rectangle myrect(px, Height() - 40, 0, 30);
    d_b_ok = new PG_Button(this, myrect, _("OK"));
    d_b_ok->SizeWidget(d_b_ok->GetTextWidth()+40, 30, true);

    // The label should have the (-1,-1) default value
    // should be better than to have the max+1 size of the map
    myrect.x += d_b_ok->my_width + 30;
    d_l_tilepos = new PG_Label(this, myrect, "(-1,-1)");
    d_l_tilepos->SizeWidget(d_l_tilepos->GetTextWidth()+10,30,true);
    d_l_tilepos->SetText("(-1,-1)");

    // Here for the Vectoring label we want to display the correct value for the vectoring
    // the vectoring could be have been already set so we display its value 
    // Even with the select of cities this is ok because it will show
    // the vector for the selected city 
    myrect.x += d_l_tilepos->my_width + 30;
    char buffer[31]; buffer[30]='\0';
    d_vpos = city->getVectoring();
    snprintf(buffer, 30, "[%i,%i]", d_vpos.x, d_vpos.y);
    d_l_vectpos = new PG_Label(this, myrect,buffer);
    d_l_vectpos->SizeWidget(d_l_vectpos->GetTextWidth()+10,30,true);


    // Finally, set up the right panel
    myrect.x = px + width + 30;
    myrect.y = py;
    myrect.w = my_width - myrect.x - 30;
    myrect.h = my_height - myrect.y - 30;
    d_cities = new PG_ListBox(this, myrect);
    d_cities->SetMultiSelect(false);

    // and fill it
    Citylist* cl = Citylist::getInstance();
    for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    {
        if ((*it).getPlayer() != d_city->getPlayer())
            continue;
        
        PG_ListBoxItem* item = new PG_ListBoxItem(d_cities, 20, (*it).getName().c_str(),
                                                0, static_cast<void*>(&(*it)));
        
        // this works before connecting the listbox since we used the starting
        // city in the constructor of vectormap
        if (&(*it) == d_city)
            item->Select();
    }
        
    // last stuff
    d_b_ok->sigClick.connect(slot(*this, &VectorDialog::okClicked));
    d_cities->sigSelectItem.connect(slot(*this, &VectorDialog::citySelected));
}

VectorDialog::~VectorDialog()
{
}

bool VectorDialog::okClicked(PG_Button* btn)
{
    QuitModal();
    return true;
}

void VectorDialog::movingMouse(Vector<int> pos)
{
    if (pos.x < 0 || pos.y < 0)
    {
        d_l_tilepos->SetText("(-1,-1)");
        return;
    }
    
    char buffer[31]; buffer[30]='\0';
    snprintf(buffer, 30, "(%i,%i)", pos.x,pos.y);

    d_l_tilepos->SetText(buffer);
}

void VectorDialog::clickedMouse(Vector<int> pos)
{
    d_vpos=pos;
    if (pos.x < 0 || pos.y < 0)
    {
        d_l_vectpos->SetText("[-1,-1]");
        return;
    }
    
    char buffer[31]; buffer[30]='\0';
    snprintf(buffer, 30, "[%i,%i]", pos.x,pos.y);

    d_l_vectpos->SetText(buffer);
}

bool VectorDialog::citySelected(PG_ListBoxBaseItem* item)
{
    d_city = static_cast<City*>(item->GetUserData());
    d_vectormap->setCity(d_city);
    d_vectormap->Redraw();

    return true;
}

// End of file
