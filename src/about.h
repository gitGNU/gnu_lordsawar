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

#ifndef ABOUT_H
#define ABOUT_H

#include "playerlist.h"
#include <pgwindow.h>
#include <pgtimerobject.h>
#include <pglabel.h>
#include <pgbutton.h>
#include <pgscrollarea.h>
#include <pgthemewidget.h>
#include <vector>
#include <string>

//! This one is a real simple dialog which shows the gold each player owns.
 
class AboutDialog :  public PG_Window,public PG_TimerObject
{
    public:
        AboutDialog(PG_Widget* parent, PG_Rect rect);
        ~AboutDialog();
	void initValues();
	bool b_okClicked(PG_Button* btn);

    private:
        bool eventKeyDown(const SDL_KeyboardEvent* key); 
        Uint32 eventTimer (ID id, Uint32 interval);

	std::vector<std::string> s_devel;
	std::vector<std::string> s_graphics;
	std::vector<std::string> s_acontributors;
	std::vector<std::string> s_icontributors;
	std::vector<std::string> s_exmembers;

	//WIDGETS
	PG_ScrollArea *d_s_area;
        Uint32 timer;
	Uint16 pos;
};

#endif
