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

#ifndef QUESTS_REPORT_H
#define QUESTS_REPORT_H

#include <sigc++/sigc++.h>
#include <pgwindow.h>
#include <pgbutton.h>
#include <pglabel.h>
#include "Quest.h"
#include "hero.h"

class QuestItem;
class PG_Label;
class PG_Button;

/**
 * \brief Class implementing the dialog window, that shows
          all pending quests for the active player.
 */
class QuestsReport : public PG_Window
{
    public:

        /** \brief Constructor */
        QuestsReport(PG_Widget* parent, PG_Rect rect);
        ~QuestsReport();

        bool b_okClicked(PG_Button* btn);
        bool b_upClicked(PG_Button* btn);
        bool b_downClicked(PG_Button* btn);

    private:

        /** \brief Fill the rows with the quests' data */
        void fillQuestItems();

        bool eventKeyDown(const SDL_KeyboardEvent* key);


        /** \brief The rows of the window */
        QuestItem *d_items[3];

        /** \brief quests for this player */
        std::vector<Quest*> d_questlist;

        /** \brief and the heroes who do them */
        std::vector<Hero*> d_heroes;

        /** \brief close button */
        PG_Button* d_b_ok;
        /** \brief scroll-up button */
        PG_Button* d_b_up;
        /** \brief scroll-down button */
        PG_Button* d_b_down;
        /** \brief label showing the range of displayed records */
        PG_Label* d_l_number;
        /** \brief No-quests message */
        PG_Label* d_l_noquests;
        /** \brief index of the quest corresponding to the first 
                   visible row */
        int d_index;
};

#endif /* QUESTS_REPORT_H */
