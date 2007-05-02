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

#ifndef GOLD_REPORT_H
#define GOLD_REPORT_H

#include "playerlist.h"
#include <pgwindow.h>

//! This one is a real simple dialog which shows the gold each player owns.
 
class GoldReport : public PG_Window
{
    public:
        GoldReport(PG_Widget* parent, Rectangle rect);
        ~GoldReport();
	bool b_okClicked(PG_Button* btn);


    private:
        bool eventKeyDown(const SDL_KeyboardEvent* key);
};

#endif
