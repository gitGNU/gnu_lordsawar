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

#ifndef FIGHTDIALOG_H
#define FIGHTDIALOG_H

#include <list>
#include <pgthemewidget.h>
#include <pglabel.h>
#include "fight.h"

class DialogItem;
class Stack;

/** \brief FightDialog implements the graphical part of a fight
  * 
  * The code for a fight is splitted over two classes: FightDialog and fight.
  * This class only contains the graphical portion of the code (drawing armies
  * and such), the fight class implements the "logic".
  * 
  * The class is given an instance of Fight and takes the course of the fight
  * from that object.
  */

class FightDialog : public PG_ThemeWidget
{ 
    public:
        /** Standard constructor
          *
          * This constructor cares for setting up the fight itself! This is
          * just a convenience feature. Internally, the fight is completely
          * separated from the dialog.
          * 
          * @param attacker     the attacking stack
          * @param defender     the defending stack
          * @param duel         whether this is a duel situation (e.g. ruin
          *                     search) or a fight on open terrain.
          */
        FightDialog(Stack* attacker, Stack* defender, bool duel);

        /** Alternative constructor
          * 
          * With network mode not being too far away, we have to handle cases
          * where a fight takes place on a remote computer and just the result
          * has to be displayed. To use this constructor, feed a fight with the
          * information about attacking/defending stack and supply it together
          * with the list of actions.
          * 
          * @param attackers    list of attacking stacks
          * @param defenders    list of defending stacks
          * @param actions      the list that describes what happened in the fight
          *
          * @note This constructor assumes that the armies HAVE NOT BEEN TOUCHED
          * yet, i.e. they are still on the status before the fight took place
          * (this affects mostly the hitpoints). Not concerning this may lead
          * to strange behaviour!
          */
        FightDialog(std::list<Stack*> attackers, std::list<Stack*> defenders,
                    const std::list<FightItem>& actions);

        ~FightDialog();

        //! Start the display of the battle
        void battle();

        //! Return the internal fight object or 0 if we didn't have one
        const Fight* getFight() const {return d_fight;}

    private:

        //! Initiate the dialog; sets the size and places the army images
        void initScreen();
       
        //! Inherited from PG_Widget
        void eventDraw(SDL_Surface* surface, const PG_Rect& rect);

        /** Draw an army at a given pixel position with a hitpoints bar
          * 
          * @param surface  the surface to draw to
          * @param unit     the unit to draw
          */
        void drawArmy (SDL_Surface* surface, const DialogItem& unit);

        //! Finds the entry in d_(attackers/defenders) for the given army id
        DialogItem* findArmy(Uint32 id);


        
        // DATA
        std::list<DialogItem> d_attackers;
        std::list<DialogItem> d_defenders;
        std::list<FightItem> d_actions;
        
        Fight* d_fight;
        PG_Label* d_l_turn;
        PG_Rect d_line;

        static const int ROUND_DELAY = 500;
};

#endif // FIGHTDIALOG_H
