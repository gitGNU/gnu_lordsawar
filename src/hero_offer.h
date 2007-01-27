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

#ifndef HERO_OFFER_H
#define HERO_OFFER_H

#include <pgwindow.h>
#include <pglineedit.h>
#include <pglabel.h>
#include <pgbutton.h>

class Hero;

/** The hero offering dialog
  * 
  * This is a dialog with an image, a text telling that a hero offers his
  * services, a label to change the hero name and the ability to accept or
  * reject the offer.
  */

class Hero_offer : public PG_Window
{
    public:
        /** Default constructor
          * 
          * @param parent       the parent widget
          * @param hero         the hero who offers his services
          * @param gold         the amount of gold needed for this hero
          */
        Hero_offer(PG_Widget* parent, Hero* hero, int gold);
        ~Hero_offer();

        //! Return true if the player has accepted the offer, false if not
        bool getRetval() {return d_retval;}
        
        // Callbacks
	bool b_acceptClicked(PG_Button* btn);
	bool b_rejectClicked(PG_Button* btn);

    private:
        // DATA
        PG_Button* d_b_accept;
        PG_Button* d_b_reject;
        PG_LineEdit* d_e_name;
        PG_Label* d_l_pic;
        PG_Label* d_l_text;
        PG_Label* d_l_text2;
        int d_gold;
        SDL_Surface* d_recruitpic;
        bool d_retval;
};

#endif // HERO_OFFER_H
