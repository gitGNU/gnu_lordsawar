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

#ifndef OptionsDialog_H
#define OptionsDialog_H

#include <pgwindow.h>
#include <pgcheckbutton.h>
#include <pglabel.h>
#include <pgbutton.h>
#include <pgfilearchive.h>
#include <pgapplication.h>
#include <pglistbox.h>
#include <pgdropdown.h>
#include "MapConfDialog.h"  // describes a slider with labels


/* Dialog for game options
 *
 */

class OptionsDialog : public PG_Window
{
    public:
        // CREATORS
        OptionsDialog(PG_Widget* parent, Rectangle rect);
        ~OptionsDialog();

        // CALLBACK FUNCTIONS

        //! Ok button has been clicked => apply changes
        bool okClicked(PG_Button* btn);

        //! Cancel button has been clicked => ignore changes
        bool cancelClicked(PG_Button* btn);

        //! Save button has been clicked => apply and save changes
        bool saveClicked(PG_Button* btn);

        //! General options type has been selected => update mode
        bool modeSelected(PG_ListBoxBaseItem* item);
        
    private:
        enum Mode {VIDEO, AUDIO, GENERAL};

        // WIDGETS
        PG_ListBox* d_submenu;

        PG_CheckButton* d_fullscreen;
        PG_DropDown* d_resolution;
        PG_CheckButton* d_smooth_scrolling;
        PG_CheckButton* d_show_next_player;
        PG_CheckButton* d_musicenable;
        TerrainConfig* d_musicvolume;
        TerrainConfig* d_speeddelay;

        PG_Button* d_b_ok;
        PG_Button* d_b_cancel;
        PG_Button* d_b_save;
};

#endif

// End of file
