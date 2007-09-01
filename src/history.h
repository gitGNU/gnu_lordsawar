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

class XML_Helper;

class Hero;
class City;
#include "army.h"
/** The purpose of the past event classes is to keep track about what a 
  *  player has accomplished.  This list is retained for the duration of 
  *  the game.
  * 
  * Each history item is derived from the abstract History class. It has to
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
                GOLD_TOTAL = 3,
		HERO_EMERGES = 4,
		CITY_WON = 5,
		CITY_RAZED = 6,
		HERO_QUEST_STARTED = 7,
		HERO_QUEST_COMPLETED = 8,
		HERO_KILLED_IN_CITY = 9,
		HERO_KILLED_IN_BATTLE = 10,
		HERO_KILLED_SEARCHING = 11,
                //WINNING_RANK = 5,
		//DIPLOMATIC_PEACE (player)
		//DIPLOMATIC_WAR (player)
		//HERO_GETS_REWARD (reward)
		//DIPLOMATIC_TREACHERY (player)
		//PLAYER_VANQUISHED
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

        bool fillData(Hero *hero);
    
    private:
        Uint32 d_hero;
};

//-----------------------------------------------------------------------------

class History_GoldTotal : public History
{
    public:
        History_GoldTotal();
        History_GoldTotal(XML_Helper* helper);
        ~History_GoldTotal();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(int gold);
    
    private:
        int d_gold;
};

//-----------------------------------------------------------------------------

class History_HeroEmerges : public History
{
    public:
        History_HeroEmerges();
        History_HeroEmerges(XML_Helper* helper);
        ~History_HeroEmerges();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(Hero *hero, City *city);
    
    private:
        Uint32 d_hero;
        Uint32 d_city;
};

//-----------------------------------------------------------------------------

class History_CityWon : public History
{
    public:
        History_CityWon();
        History_CityWon(XML_Helper* helper);
        ~History_CityWon();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

	Uint32 getCityId() const {return d_city;}

        bool fillData(City *city, Hero *hero = NULL);
    
    private:
        Uint32 d_city;
        Uint32 d_hero;
};

//-----------------------------------------------------------------------------

class History_CityRazed : public History
{
    public:
        History_CityRazed();
        History_CityRazed(XML_Helper* helper);
        ~History_CityRazed();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(City *city);
    
    private:
        Uint32 d_city;
};

//-----------------------------------------------------------------------------

class History_HeroQuestStarted : public History
{
    public:
        History_HeroQuestStarted();
        History_HeroQuestStarted(XML_Helper* helper);
        ~History_HeroQuestStarted();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(Hero *hero);
    
    private:
        Uint32 d_hero;
};

//-----------------------------------------------------------------------------

class History_HeroQuestCompleted: public History
{
    public:
        History_HeroQuestCompleted();
        History_HeroQuestCompleted(XML_Helper* helper);
        ~History_HeroQuestCompleted();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(Hero *hero);
    
    private:
        Uint32 d_hero;
};

//-----------------------------------------------------------------------------

class History_HeroKilledInCity : public History
{
    public:
        History_HeroKilledInCity();
        History_HeroKilledInCity(XML_Helper* helper);
        ~History_HeroKilledInCity();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(Hero *hero, City *city);
    
    private:
        Uint32 d_hero;
	Uint32 d_city;
};

//-----------------------------------------------------------------------------

class History_HeroKilledInBattle: public History
{
    public:
        History_HeroKilledInBattle();
        History_HeroKilledInBattle(XML_Helper* helper);
        ~History_HeroKilledInBattle();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(Hero *hero);
    
    private:
        Uint32 d_hero;
};

//-----------------------------------------------------------------------------

class History_HeroKilledSearching: public History
{
    public:
        History_HeroKilledSearching();
        History_HeroKilledSearching(XML_Helper* helper);
        ~History_HeroKilledSearching();

        std::string dump() const;
        bool save(XML_Helper* helper) const;

        bool fillData(Hero *hero);
    
    private:
        Uint32 d_hero;
};


#endif //HISTORY_H
