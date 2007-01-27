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

#ifndef QUESTEXPIREDDIALOG_H
#define QUESTEXPIREDDIALOG_H

#include <pgwindow.h>
#include <pgbutton.h>
#include <pglabel.h>
#include <queue>

class Quest;

/** 
    \brief Class implementing the dialog window, that show
           the message after a specific quest has been completed.
    
    The message consists of a standard header and the text lines
    provided by the quest object.
 */
class QuestExpiredDialog : public PG_Window
{ 
	public:

        /** \brief Constructor */
	QuestExpiredDialog(PG_Widget* parent, Quest *quest);
        /** \brief Destructor */
	~QuestExpiredDialog();

	/** \brief button close callback */
	bool b_closeClicked(PG_Button* btn);

	private:
        bool eventKeyDown(const SDL_KeyboardEvent* key);

        /** \brief close button */
	PG_Button* d_b_close;
        /** queue of PG_Label widgets */
        std::queue<PG_Label*> d_msgs;
};

#endif

// End of file
