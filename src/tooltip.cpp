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

#include <iostream>
#include "tooltip.h"
#include "defs.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

ToolTip::ToolTip(PG_Widget* parent, Rectangle rect)
    :PG_ThemeWidget(parent, rect)
{
    debug("simple tooltip constructor")
    d_lab = new PG_Label(this, Rectangle(2,0,0,0), "");
    d_timer = 0;
}

ToolTip::~ToolTip()
{
   debug("Tooltip destructor!");
   delete d_lab;
}

void ToolTip::setData(const char* text, int screen_x, int screen_y, int max_x, int max_y)
{
    debug("data set")
    int pos_x=0;
    int pos_y=0;
    SDL_Color lightyellow = {252, 250, 222};
    SDL_Color black = {0, 0, 0};

    SetSizeByText(4,0,text);
    SetSimpleBackground(true);
    SetBackgroundColor(lightyellow); 
    d_lab->SetSizeByText(0,3,text);
    d_lab->SetFontColor(black);
    d_lab->SetText(text);

    if ((d_lab->GetTextWidth()+screen_x+4)>max_x)
    {
        pos_x=max_x-d_lab->GetTextWidth()-10;    
    }
    else
        pos_x=screen_x;
 
    if ((d_lab->GetTextHeight()+screen_y+2)>max_y)
    {
      pos_y=max_y-d_lab->GetTextHeight()-10;    
    }
    else pos_y=screen_y;

    MoveWidget(Rectangle(pos_x,pos_y,0,0));
    SetSizeByText(4,0,text);
    SetBorderSize(1);

    // Make a black border
    // TODO: a more generic way (this won't become available until a migration
    // to paragui-devel, but we can keep this in mind)
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            my_colorBorder[i][j] = black;
    
    // SetCapture, so the tooltip object can detect
    // the moving of the mouse 
    SetCapture();

    //We want the tooltip to be displayed after some time
    debug("Before Addtimer")
    d_timer=AddTimer(TOOLTIP_WAIT);
    d_movecounter=0;
}

bool ToolTip::eventMouseMotion(const SDL_MouseMotionEvent* event)
{  
    debug("mouse motion detected")
    if (d_movecounter>3) 
    {
        if (d_timer != 0)
        {
            RemoveTimer(d_timer);
            d_timer = 0;
        }
        ReleaseCapture();
        Hide();
    }
    else
        d_movecounter++;
    return true;
}

bool ToolTip::eventMouseButtonDown(const SDL_MouseButtonEvent* event)
{
    debug("mouse button detected")
    if (d_timer != 0)
    {
        RemoveTimer(d_timer);
        d_timer = 0;
    }
    ReleaseCapture();
    Hide();
    return false;
}

Uint32 ToolTip::eventTimer (ID id, Uint32 interval)
{
    debug("Tooltip Show!")
    if (d_timer != 0)
    {
        RemoveTimer(d_timer);
        d_timer = 0;
    }
    Show();
    return interval;
}
