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

#ifndef LANG_DIALOG_H
#define LANG_DIALOG_H

#include <pgwindow.h>
#include <pgradiobutton.h>
#include <pgbutton.h>
#include <pglineedit.h>

/** Language selection dialog
  * 
  * This dialog asks the user for the language to use and sets the locale
  * appropriately. Currently, this is done using static PG_RadioButtons.

  * However, in the future it may be interesting to have this a bit more
  * dynamically.
  */

// TODO: more relaxed checking for fitting locales

class LangDialog : public PG_Window
{
    public:
        //! Standard constructor
        LangDialog(PG_Widget* parent, const Rectangle r);

    private:
        //! callback if another language is chosen
        bool langChosen(PG_RadioButton* btn, bool state);
        
        //! User has specified an own locale
        bool localeEdit();
        
        //! callback if OK button is clicked, set new language
        bool okClicked();

        //! Cancel button is clicked; ignore settings
        bool cancelClicked();
        

        // Data
        PG_RadioButton* d_b_english;
        PG_RadioButton* d_b_german;
        PG_RadioButton* d_b_italian;
        PG_RadioButton* d_b_polish;
        PG_RadioButton* d_b_other;
        PG_LineEdit* d_edit;

        PG_Button* d_b_ok;
        PG_Button* d_b_cancel;

        char d_origlang[21];        // should be sufficiently long :)
        char d_chosenlang[21];
};

#endif //LANG_DIALOG_H
