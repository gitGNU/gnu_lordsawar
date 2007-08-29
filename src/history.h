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

#ifndef HISTORY_H
#define HISTORY_H

#include <string>
#include "vector.h"
#include <sigc++/trackable.h>

#include "fight.h"
#include "army.h"
#include "reward.h"

class XML_Helper;

/** The purpose of the past event classes is to keep track about what a 
  *  player has accomplished.
  * 
  * Each action item is derived from the abstract History class. It has to
  * contain three functions:
  *
  * - A loading constructor (which takes an XML_Helper parameter)
  * - a save function which saves the data
  * - a fillData function which takes some parameters and with these stores the
  *   data about what happened.
  */


class History
{
    public:
        enum Type {
                START_TURN = 1,
                FOUND_SAGE = 2,
                //GOLD_AMOUNT = 4,
                //WINNING_RANK = 5,
		//NEW_UNIT = 6 (army type, city)
		//DIPLOMATIC_PEACE (player)
		//DIPLOMATIC_WAR (player)
		//CONQUER_CITY (city name [hero])
		//HERO_COMPLETE_QUEST
		//HERO_KILLED (place name) (battle) (searching)
		//HERO_GETS_REWARD (reward)
		//HERO_RECEIVES_QUEST
		//CITY_RAZE
		//HERO_ARRIVES (city)
		//DIPLOMATIC_TREACHERY (player)
		//PLAYER_VANQUISHED
		//CONQUERED_CITY
        };
                
        
        History(Type type);
        virtual ~History();

        //! Returns debug information. Needs to be overwritten by derivatives
        virtual std::string dump() const = 0;

        //! Save function. See XML_Helper for information about saving.
        virtual bool save(XML_Helper* helper) const = 0;
        
        /** static load function (see XML_Helper)
          * 
          * Whenever an action item is loaded, this function is called. It
          * examines the stored id and calls the constructor of the appropriate
          * action class.
          *
          * @param helper       the XML_Helper instance for the savegame
          */
        static History* handle_load(XML_Helper* helper);

        //! Copies an action to a new one
        static History* copy(const History* a);

        //! Returns the id which identifies the type of past event
        Type getType() const {return d_type;}
        
    protected:
        Type d_type;
};

//-----------------------------------------------------------------------------

class History_StartTurn : public History
{
    public:
        History_StartTurn();
        History_StartTurn(XML_Helper* helper);
        ~History_StartTurn();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData();
    
    private:
};

//-----------------------------------------------------------------------------

class History_FoundSage : public History
{
    public:
        History_FoundSage();
        History_FoundSage(XML_Helper* helper);
        ~History_FoundSage();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(Hero *h);
    
    private:
        Uint32 d_hero;
};

#endif //HISTORY_H
