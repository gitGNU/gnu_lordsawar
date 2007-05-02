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

#include <assert.h>
#include <pglabel.h>
#include <pgapplication.h>

#include "ArmyInfo.h"
#include "army.h"
#include "ArmyDialog.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<std::flush;}
#define debug(x)

ArmyInfo::ArmyInfo(Army* army, bool pressable, PG_Widget* parent, Rectangle rect)
	:PG_ThemeWidget(parent, rect, true), d_army(army), d_pressed(false),
        d_pressable(pressable)
{
	d_pressed = d_army->isGrouped();

	// show number of moves
	char buffer[80];
	sprintf(buffer, "%i", d_army->getMoves());
	d_l_moves = new PG_Label(this, Rectangle(20, 57, 20, 20), buffer);
	d_l_moves->SetFontColor (PG_Color (0, 0, 0));

	Show();
}

ArmyInfo::~ArmyInfo()
{
	delete d_l_moves;
}

void ArmyInfo::eventDraw(SDL_Surface* surface, const Rectangle& rect)
{
	debug("eventDraw()");
	PG_ThemeWidget::eventDraw(surface, rect);

	assert(d_army);

	DrawBorder(rect, 1, !d_pressed);

	// show pic of army in the button
	Rectangle r(2, 2, 54, 54);
	SDL_BlitSurface(d_army->getPixmap(), NULL, surface, &r);

	// draw the hitpoints-bar
	SDL_Color red = {255, 0, 0};
	SDL_Color green = {0, 255, 0};

    float factor = (float)d_army->getHP() / (float)d_army->getStat(Army::HP) * 52;
	DrawLine(4, 78, 52, 78, red, 5);
	DrawLine(4, 78, static_cast<unsigned int>(factor), 78, green, 5);
}

bool ArmyInfo::eventMouseButtonDown(const SDL_MouseButtonEvent* event)
{
	debug("eventMouseButtonDown()");
    
    if (d_army == 0)
        return true;

    if (event->button == SDL_BUTTON_RIGHT)
    {
        //pop up an army information dialog
        Rectangle r(PG_Application::GetScreenWidth()/2 - 300, PG_Application::GetScreenHeight()/2 - 250,
                                                600, 500);
        ArmyDialog dialog(d_army, 0, r);

        dialog.Show();
        dialog.RunModal();
        dialog.Hide();
        Redraw();

        return true;
    }

    if (!d_pressable)
        return true;

	if (d_pressed) 
	{
		d_pressed = false;
		d_army->setGrouped(false);
	}
	else
	{
		d_pressed = true;
		d_army->setGrouped(true);
	}

	Redraw();
	return true;
}
