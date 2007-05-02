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

#ifndef ARMYDIALOG_H
#define ARMYDIALOG_H

#include <pgwindow.h>
#include <pgbutton.h>
#include <pglabel.h>
#include "army.h"

class PG_Button;
class PG_Label;
class Army;

/** \brief The class for displaying details of an army
  * 
  * An instance of this class is created and shown when you right-click
  * on an army icon at the bottom of the game screen. It features the
  * picture and details about the stats of this army. The class itself
  * is very simple. The whole initialisation is to be found in the
  * constructor, the rest of the functionality is inherited from the
  * PG_Window class.
  */

// FIXME: That this class is derived from PG_ThemeWidget is a hack; this widget
// can have an internal surface on which to draw the icons. Something better
// (for each stat a separate widget or use eventBlit instead of eventDraw) and
// the usage of PG_Window somehow seems better.

class ArmyDialog : public PG_ThemeWidget
{
    public:
        /** \brief The constructor
         * This is where the setup and arrangement of the data takes place
         */
        ArmyDialog(Army* army, PG_Widget* parent, Rectangle rect);

        /** \brief The destructor */
        ~ArmyDialog();

        /** \brief The function to be called when you press the OK button*/
        bool b_ok_clicked(PG_Button* btn);

        /** \brief The function to be called when you press the "Items" button
          * below the army picture in case it is a hero.
          */
        bool b_itemClicked(PG_Button* btn);
        
    private:
        // intercept keyboard shortcuts
        bool eventKeyDown(const SDL_KeyboardEvent* key);
        
        // derived to implement the icon drawing code
        void eventDraw(SDL_Surface* surface, const Rectangle& r);

        /** Places icons for the army's stats
          * 
          * Most army stats are displayed using icons (stored in d_icon). An
          * icon can have 4 meanings: normal (natural) stat, increased stat,
          * decreased stat and decreased additional stat (lost additional 
          * hitpoints, for example). The routine should find out which icons
          * to apply from the values for the normal, maximum and current stat.
          *
          * @param s        the internal surface to draw on
          * @param r        the rect to place the icons on
          * @param stat     the stat to use
          */
        void placeIcons(SDL_Surface* s, const Rectangle r, Army::Stat stat);
        
        // load the icons
        void loadIcons();

        // takes the unit description and adds some text about army and movement boni
        std::string assembleText();
        
        /** \brief several data, only to be used for displaying purposes*/
        Army* d_army;
        PG_Button* d_b_ok;
        PG_Button* d_b_item;

        PG_Label* d_l_shots;

        // 6 different stats, 4 icons each (normal, raised, lowered,
        // raised and lowered (e.g. lost additional hitpoints))
        SDL_Surface* d_icon[6][4];
};

#endif
