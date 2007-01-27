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

#ifndef PLAYER_PREFERENCES_H
#define PLAYER_PREFERENCES_H

#include <string>
#include <pgwidget.h>
#include <pgbutton.h>
#include <pglineedit.h>
#include <pgdropdown.h>
#include <pglabel.h>
#include <sigc++/sigc++.h>

class PG_Button;
class PG_LineEdit;
class PG_DropDown;
class PG_ListBoxBaseItem;

class Player_preferences : public PG_Widget                         
{
    public:
    enum Type{HUMAN, HUMAN_OR_COMPUTER, ANY};

        // CREATORS
        Player_preferences(Type type, std::string name, PG_Widget* parent,
        PG_Rect rect);
        ~Player_preferences();

        // ACCESSORS
        bool isActive();
        bool isComputer();
        bool isEasy();
        std::string getName(){return d_e_name->GetText();}
        unsigned int getArmyset();
        void setName(std::string name){d_e_name->SetText(name.c_str());}

        // SIGNAL
        SigC::Signal0<void> playerDataChanged;

        bool b_typeChanged(PG_ListBoxBaseItem * cb);

    private:
        // WIDGETS
        PG_DropDown* d_cb_type;
        PG_LineEdit* d_e_name;
        PG_DropDown* d_cb_armyset;
};

#endif // PLAYER_PREFERENCES_H

// End of file
