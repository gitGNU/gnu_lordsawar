//  Copyright (C) 2007, 2008 Ben Asselstine
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#ifndef HISTORY_H
#define HISTORY_H

#include <string>
#include "vector.h"
#include <sigc++/trackable.h>

class XML_Helper;

class Hero;
class HeroProto;
class City;
#include "army.h"

//! A permanent record of an accomplishment during gameplay.
/** 
 * The purpose of the history classes is to keep track about what a 
 *  player has accomplished.  This list is retained for the duration of 
 *  the game.
 * 
 * Each history item is derived from the abstract History class. It has to
 * contain three functions; A loading constructor (which takes an XML_Helper 
 * parameter), a save function which saves the data, a fillData function 
 * which takes some parameters and with these stores the data about what 
 * happened.
 *
 */
class History
{
    public:
	//! A History can be one of the following kinds.
        enum Type {
	  //! The player started a turn.
	  START_TURN = 1,
	  //! The player has searched a Ruin and found a sage.
	  FOUND_SAGE = 2,
	  //! The player has accrued a certain amount of gold in the treasury.
	  GOLD_TOTAL = 3,
	  //! A hero has emerged.
	  HERO_EMERGES = 4,
	  //! A City has been taken.
	  CITY_WON = 5,
	  //! A City has been razed.
	  CITY_RAZED = 6,
	  //! A Hero has inititiated a Quest.
	  HERO_QUEST_STARTED = 7,
	  //! A Hero has completed a Quest.
	  HERO_QUEST_COMPLETED = 8,
	  //! A Hero was killed in battle at a City.
	  HERO_KILLED_IN_CITY = 9,
	  //! A Hero was killed in battle in the field.
	  HERO_KILLED_IN_BATTLE = 10,
	  //! A Hero was killed searching a Ruin.
	  HERO_KILLED_SEARCHING = 11,
	  //! A Hero was involved in taking a City.
	  HERO_CITY_WON = 12,
	  //! The player has this score.
	  SCORE = 13,
	  //! The player has been utterly defeated.
	  PLAYER_VANQUISHED = 14,
	  //! The player has achieved peace with an opponent.
	  DIPLOMATIC_PEACE = 15,
	  //! The player has started a war with an opponent.
	  DIPLOMATIC_WAR = 16,
	  //! The player has been treacherous towards an opponent.
	  DIPLOMATIC_TREACHERY = 17,
	  //! A Hero finds some powerful allies.
	  HERO_FINDS_ALLIES = 18,
	  //! The player has finished a turn.
	  END_TURN = 19
        };
	static std::string historyTypeToString(const History::Type type);
	static History::Type historyTypeFromString(const std::string str);
                
	//! Default constructor.
        History(Type type);

	//! Loading from XML constructor.
	History (XML_Helper *helper);

	//! Destructor.
        virtual ~History();

        //! Returns debug information. Needs to be overwritten by derivatives
        virtual std::string dump() const = 0;

        /** 
	 * static load function (see XML_Helper)
         * 
         * Whenever a History item is loaded, this function is called. It
         * examines the stored History::Type and calls the constructor of 
	 * the appropriate History class.
         *
         * @param helper       The opened saved-game file to read from.
         */
	//! Load a History from an opened saved-game file.
        static History* handle_load(XML_Helper* helper);

        //! Copies a history into a new one.
        static History* copy(const History* a);

        //! Returns the id which identifies the type of History event.
        Type getType() const {return d_type;}
        
	bool save(XML_Helper* helper) const;
	bool saveContents(XML_Helper* helper) const;

    protected:
        virtual bool doSave(XML_Helper* helper) const = 0;
        Type d_type;
};

//-----------------------------------------------------------------------------

//! A permanent record of a player starting a turn.
class History_StartTurn : public History
{
    public:
	//! Default constructor.
        History_StartTurn();
	//! Copy constructor.
	History_StartTurn(const History_StartTurn &history);
	//! Load the historical event from an opened saved-game file.
        History_StartTurn(XML_Helper* helper);
	//! Destructor.
        ~History_StartTurn();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! This method doesn't need to be called for History_StartTurn.
        bool fillData();
    
    private:
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero searching a Ruin and finding a sage.
class History_FoundSage : public History
{
    public:
	//! Default constructor.
        History_FoundSage();
	//! Copy constructor.
	History_FoundSage(const History_FoundSage &history);
	//! Load the historical event from an opened saved-game file.
        History_FoundSage(XML_Helper* helper);
	//! Destructor.
        ~History_FoundSage();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with the Hero who found the sage.
        bool fillData(Hero *hero);

	//! Get the name of the Hero who found the sage.
	std::string getHeroName() const {return d_hero;}
    
    private:
	//! The name of the Hero.
	std::string d_hero;
};

//-----------------------------------------------------------------------------

//! A permanent record of the amount of gold pieces a player has.
class History_GoldTotal : public History
{
    public:
	//! Default constructor.
        History_GoldTotal();
	//! Copy constructor.
	History_GoldTotal(const History_GoldTotal &history);
	//! Load the historical event from an opened saved-game file.
        History_GoldTotal(XML_Helper* helper);
	//! Destructor.
        ~History_GoldTotal();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with the amount of gold pieces the Player has.
        bool fillData(int gold);

	//! Get the amount of gold associated with this event.
	int getGold() const {return d_gold;}
    
    private:
	//! The amount of gold pieces the player has at a point in time.
        int d_gold;
};

//-----------------------------------------------------------------------------

//! A permanent record of a new Hero emerging in a City.
class History_HeroEmerges : public History
{
    public:
	//! Default constructor.
        History_HeroEmerges();
	//! Copy constructor.
	History_HeroEmerges(const History_HeroEmerges &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroEmerges(XML_Helper* helper);
	//! Destructor.
        ~History_HeroEmerges();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with pertinent data.
	/**
	 * Populate the event with the new Hero who has showed up and 
	 * the City where it has appeared.
	 *
	 * @param hero    The Hero that has emergerd.
	 * @param city    The City where the Hero has emerged.
	 */
        bool fillData(HeroProto *hero, City *city);

	//! Get the name of the Hero who appeared.
	std::string getHeroName() const {return d_hero;}

	//! Get the name of the City where the Hero has emerged.
	std::string getCityName() const {return d_city;}
    
    private:
	//! The name of the Hero who emerged.
	std::string d_hero;

	//! The name of the City where the Hero emerged.
	std::string d_city;
};

//-----------------------------------------------------------------------------

//! A permanent record of an enemy city being defeated.
class History_CityWon : public History
{
    public:
	//! Default constructor.
        History_CityWon();
	//! Copy constructor.
	History_CityWon(const History_CityWon &history);
	//! Load the historical event from an opened saved-game file.
        History_CityWon(XML_Helper* helper);
	//! Destructor.
        ~History_CityWon();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the record with the City that the Player defeated.
        bool fillData(City *city);

	//! Get the Id of the City object that was defeated.
	Uint32 getCityId() const {return d_city;}
    
    private:
	//! The Id of the City object that was defeated.
	Uint32 d_city;
};

//-----------------------------------------------------------------------------

//! A permanent record of an enemy city being defeated by a Hero.
class History_HeroCityWon: public History
{
    public:
	//! Default constructor.
        History_HeroCityWon();
	//! Copy constructor.
	History_HeroCityWon(const History_HeroCityWon &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroCityWon(XML_Helper* helper);
	//! Destructor.
        ~History_HeroCityWon();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with pertinent data.
	/**
	 * Populate the event with the City that was defeated and the
	 * Hero that assisted in the conquering.
	 *
	 * @param city    The City that was conquered.
	 * @param hero    The Hero who lead the conquering of the City.
	 */
        bool fillData(Hero *hero, City *city);

	//! Get the name of the Hero who conquered the City.
	std::string getHeroName() const {return d_hero;}

	//! Get the name of the City that was conquered.
	std::string getCityName() const {return d_city;}
    
    private:
	//! The name of the Hero who helped in conquering the City.
	std::string d_hero;

	//! The name of the City that was conquered.
	std::string d_city;
};

//-----------------------------------------------------------------------------

//! A permanent record of an enemy city being razed.
class History_CityRazed : public History
{
    public:
	//! Default constructor.
        History_CityRazed();
	//! Copy constructor.
	History_CityRazed(const History_CityRazed &history);
	//! Load the historical event from an opened saved-game file.
        History_CityRazed(XML_Helper* helper);
	//! Destructor.
        ~History_CityRazed();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with the City that was razed.
        bool fillData(City *city);

	//! Get the Id of the City object that was razed.
	Uint32 getCityId() const {return d_city;}
    
    private:
	//! The Id of the City that was razed.
        Uint32 d_city;
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero initiating a Quest.
class History_HeroQuestStarted : public History
{
    public:
	//! Default constructor.
        History_HeroQuestStarted();
	//! Copy constructor.
	History_HeroQuestStarted(const History_HeroQuestStarted &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroQuestStarted(XML_Helper* helper);
	//! Destructor.
        ~History_HeroQuestStarted();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with the Hero who initiated a new Quest.
        bool fillData(Hero *hero);

	//! Get the name of the Hero who started a Quest.
	std::string getHeroName() const {return d_hero;}
    
    private:
	//! The name of the Hero who started the Quest.
	std::string d_hero;
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero completing a Quest.
class History_HeroQuestCompleted: public History
{
    public:
	//! Default constructor.
        History_HeroQuestCompleted();
	//! Copy constructor.
	History_HeroQuestCompleted(const History_HeroQuestCompleted &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroQuestCompleted(XML_Helper* helper);
	//! Destructor.
        ~History_HeroQuestCompleted();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with the Hero who finished a Quest.
        bool fillData(Hero *hero);

	//! Get the name of the Hero who finished a Quest.
	std::string getHeroName() const {return d_hero;}
    
    private:
	//! The name of the Hero who completed the Quest.
	std::string d_hero;
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero killed in the defense or attack of a City.
class History_HeroKilledInCity : public History
{
    public:
	//! Default constructor.
        History_HeroKilledInCity();
	//! Copy constructor.
	History_HeroKilledInCity(const History_HeroKilledInCity &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroKilledInCity(XML_Helper* helper);
	//! Destructor.
        ~History_HeroKilledInCity();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with pertinent data.
	/**
	 * Populate the event with the Hero who was killed and the City where
	 * it happened.
	 *
	 * @param hero    The Hero who died.
	 * @param city    The City that houses the Hero's broken corpse.
	 */
        bool fillData(Hero *hero, City *city);

	//! Get the name of the Hero who died.
	std::string getHeroName() const {return d_hero;}

	//! Get the name of the City where the Hero died.
	std::string getCityName() const {return d_city;}
    
    private:
	//! Get the name of the Hero who was killed.
	std::string d_hero;

	//! Get the name of the City where the Hero was killed.
	std::string d_city;
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero killed in battle outside of a City.
class History_HeroKilledInBattle: public History
{
    public:
	//! Default constructor.
        History_HeroKilledInBattle();
	//! Copy constructor.
	History_HeroKilledInBattle(const History_HeroKilledInBattle &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroKilledInBattle(XML_Helper* helper);
	//! Destructor.
        ~History_HeroKilledInBattle();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with the Hero who was killed in battle.
        bool fillData(Hero *hero);

	//! Get the name of the Hero who died in battle outside of a City.
	std::string getHeroName() const {return d_hero;}
    
    private:
	//! The name of the Hero who died in battle outside of a City.
	std::string d_hero;
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero killed while searching a Ruin.
class History_HeroKilledSearching: public History
{
    public:
	//! Default constructor.
        History_HeroKilledSearching();
	//! Copy constructor.
	History_HeroKilledSearching(const History_HeroKilledSearching &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroKilledSearching(XML_Helper* helper);
	//! Destructor.
        ~History_HeroKilledSearching();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with the Hero who was killed while searching.
        bool fillData(Hero *hero);

	//! Get the name of the Hero who died while searching a Ruin.
	std::string getHeroName() const {return d_hero;}
    
    private:
	//! The name of the Hero who died while searching a Ruin.
	std::string d_hero;
};

//-----------------------------------------------------------------------------

//! A permanent record of the player's score.
class History_Score: public History
{
    public:
	//! Default constructor.
        History_Score();
	//! Copy constructor.
	History_Score(const History_Score &history);
	//! Load the historical event from an opened saved-game file.
        History_Score(XML_Helper* helper);
	//! Destructor.
        ~History_Score();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with the player's score for this turn.
        bool fillData(Uint32 score);

	//! Get the player's score for this turn.
	Uint32 getScore() const {return d_score;}
    
    private:
	//! The player's score.
        int d_score;
};

//-----------------------------------------------------------------------------

//! A permanent record of the player being utterly defeated.
class History_PlayerVanquished: public History
{
    public:
	//! Default constructor.
        History_PlayerVanquished();
	//! Copy constructor.
	History_PlayerVanquished(const History_PlayerVanquished &history);
	//! Load the historical event from an opened saved-game file.
        History_PlayerVanquished(XML_Helper* helper);
	//! Destructor.
        ~History_PlayerVanquished();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

};

//-----------------------------------------------------------------------------

//! A permanent record of the player making peace with an opponent.
class History_DiplomacyPeace : public History
{
    public:
	//! Default constructor.
        History_DiplomacyPeace();
	//! Copy constructor.
	History_DiplomacyPeace(const History_DiplomacyPeace &history);
	//! Load the historical event from an opened saved-game file.
        History_DiplomacyPeace(XML_Helper* helper);
	//! Destructor.
        ~History_DiplomacyPeace();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with the Player who we are at peace with.
        bool fillData(Player *opponent);

	//! Get the Id of the Player object we are at peace with.
	Uint32 getOpponentId() const {return d_opponent_id;}
    
    private:
	//! The Id of the Player object we are at peace with.
	Uint32 d_opponent_id;
};

//-----------------------------------------------------------------------------

//! A permanent record of the player going to war with an opponent.
class History_DiplomacyWar: public History
{
    public:
	//! Default constructor.
        History_DiplomacyWar();
	//! Copy constructor.
	History_DiplomacyWar(const History_DiplomacyWar &history);
	//! Load the historical event from an opened saved-game file.
        History_DiplomacyWar(XML_Helper* helper);
	//! Destructor.
        ~History_DiplomacyWar();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with the Player who we are at war with.
        bool fillData(Player *opponent);

	//! Get the Id of the Player object we are at war with.
	Uint32 getOpponentId() const {return d_opponent_id;}
    
    private:
	// The Id of the Player object we are at war with.
	Uint32 d_opponent_id;
};

//-----------------------------------------------------------------------------

//! A permanent record of the player being treacherous to an opponent.
class History_DiplomacyTreachery: public History
{
    public:
	//! Default constructor.
        History_DiplomacyTreachery();
	//! Copy constructor.
	History_DiplomacyTreachery(const History_DiplomacyTreachery &history);
	//! Load the historical event from an opened saved-game file.
        History_DiplomacyTreachery(XML_Helper* helper);
	//! Destructor.
        ~History_DiplomacyTreachery();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with the Player we performed treachery on.
        bool fillData(Player *opponent);

	//! Get the Id of the Player object that we peformed treachery on.
	Uint32 getOpponentId() const {return d_opponent_id;}
    
    private:
	//! The Id of the Player object that we peformed treachery on.
	Uint32 d_opponent_id;
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero finding powerful allies.
class History_HeroFindsAllies : public History
{
    public:
	//! Default constructor.
        History_HeroFindsAllies();
	//! Copy constructor.
	History_HeroFindsAllies(const History_HeroFindsAllies &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroFindsAllies(XML_Helper* helper);
	//! Destructor.
        ~History_HeroFindsAllies();

	//! Return some debug information about this historical event.
        std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Populate the event with the name of the Hero who found allies.
        bool fillData(Hero *hero);

	//! Get the name of the Hero who found powerful allies.
	std::string getHeroName() const {return d_hero;}
    
    private:
	//! The name of the Hero who found powerful allies at a Ruin.
	std::string d_hero;
};

//-----------------------------------------------------------------------------
//! A permanent record of a player ending a turn.
class History_EndTurn : public History
{
    public:
	//! Default constructor.
	History_EndTurn();
	//! Copy constructor.
	History_EndTurn(const History_EndTurn &history);
	//! Load the historical event from an opened saved-game file.
	History_EndTurn(XML_Helper* helper);
	//! Destructor.
	~History_EndTurn();

	//! Return some debug information about this historical event.
	std::string dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! This method doesn't need to be called for History_EndTurn.
	bool fillData();
    
    private:
};


#endif //HISTORY_H
