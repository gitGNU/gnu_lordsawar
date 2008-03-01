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

#ifndef CITY_H
#define CITY_H

#include <list>
#include <string>
#include "Location.h"
#include "Ownable.h"
#include "Renamable.h"

class Player;
class Stack;
class Army;
class Hero;

#define DEFAULT_CITY_NAME "Noname"
#define DEFAULT_CITY_INCOME 20

//! A City on the game map.
/**
 * Players vie for control of City objects on the game map.  The main goal
 * of the game is to conquer these City objects.  Cities can be also be razed,
 * making them uninhabitable and unconquerable.
 *
 * A city can produce armies, provide income, place produced armies on it's 
 * tiles and buy production as well as have it's name changed.  Cities can
 * also vector their produced units to another position on the map.
 *
 * A City has 4 production slots.  A production slot is a production 
 * capability of a city.  Every one of the slots can be filled with an 
 * Army production base.  Cities can be assigned a random set of Army
 * production bases.  The name of a City can also be randomly set from a 
 * list of potential City names.
 *
 * Some City objects are capital cities.  Every player has a single capital
 * city.  Conquering another player's capital city doesn't give any bonus
 * except for bragging rights.
 */
class City : public Ownable, public Location, public Renamable
{
    public:
	//! Default constructor.
        /** 
          * Make a new city object.
	  *
          * @param pos       The location of the city on the game map.
          * @param name      The name of the city.
          * @param gold      The amount of gold the city produces each turn.
          */
        City(Vector<int> pos, std::string name = DEFAULT_CITY_NAME, 
	     Uint32 gold = DEFAULT_CITY_INCOME);
	//! Copy constructor.
        City(const City&);
        //! Loading constructor.
	/**
	 * Make a new city object by reading it from a saved-game file.
	 *
	 * @param helper The opened saved-game file to load the City from.
	 */
        City(XML_Helper* helper);
	//! Destructor.
        ~City();

        //! Save the city to an opened saved-game file.
        bool save(XML_Helper* helper) const;
        
        //! Set the gold the city produces each turn.
        void setGold(Uint32 gold){d_gold = gold;}

        //! Set whether or not the city is destroyed.
        void setBurnt(bool burnt){d_burnt = burnt;}

        //! Sets whether the city is a capital.
        void setCapital(bool capital) {d_capital = capital;}

        //! Sets whether the city is a capital.
        void setCapitalOwner(Player *p) {d_capital_owner = p;}

	//! Set the production of the city.
        /**
	 * Make the Army production base in particular slot active, so that
	 * the Army starts being produced.
	 *
         * @param index  The index of the production slot to activate. 
	 *               -1 means no production at all.   This must be a value
	 *               between -1 and 3.
         */
        void setProduction(int index);

        
        //! Raise the defense level by one. Return true on success.
        bool raiseDefense();

        //! Lower the defense level by one. Return true on success.
	/**
	 * This method is not used.
	 */
        bool reduceDefense();

	//! Add an Army production base to a production slot.
        /**
         * @note This method overwrites the production slot if neccessary.
         * 
         * @param index        The index of the production slot; if set to -1,
         *                     the city will try to find a free production slot.
	 *                     This must be a value between -1 and 3.
	 * @param army         The Army production base to add.
	 *
         * @return True on success, false on error.
         */
        bool addBasicProd(int index, Army *army);

        //! Clears the basic production of a given slot.
	/**
	 * @param index  The slot to remove the Army production base from.
	 *               This method deletes the Army production base object.  
	 *               This parameter must be a a value between 0 and 3.
	 */
        void removeBasicProd(int index);
        
        //! Changes the owner of the city and prepares it for takeover.
	/**
	 * @param newowner  The pointer to the Player in Playerlist who is the
	 *                  new owner of this City.
	 */
        void conquer(Player* newowner);
        
        //! Sets the production to random starting values
	//! @param likely = 0, 1, 2, 3.  3 is more armies, more powerful.
	void setRandomArmytypes(bool produce_allies, int likely);

        //! Produces the strongest army the city can produce
        void produceStrongestArmy();

        //! Produces the weakest army the city can produce
        void produceWeakestArmy();

        //! Produces a weak army for exploring (used when neutral cities are
	// "average".  The scout army doesn't have to be a buildable army
	// by the city for this method to operate.
        void produceScout();

        //! Do everything neccessary for a new turn
        void nextTurn();

        //! Returns whether the city can produce a given army type
        bool hasProduction(int type, Uint32 armyset) const;

        //! Get the first free slot of basic production for the AI
	int getFreeBasicSlot();

        //! returns true if the city already has bought this production type
        bool isAlreadyBought(const Army * army);

        //! Get the defense level of the city
        int getDefenseLevel() const {return d_defense_level;}

	// returns -1 if city can't be upgraded
	int getGoldNeededForUpgrade() const; 
	
        //! Get the maximum number of basic productions of the city
        int getMaxNoOfBasicProd() const {return d_numbasic;};

        //! Get the number of basic productions of the city
        int getNoOfBasicProd();

        //! Get the duration till the current production is finished
        int getDuration() const {return d_duration;}

        //! Get the index of the current internal production slot
        int getProductionIndex() const {return d_production;}
        
        //! Get the income of the city per turn
        Uint32 getGold() const {return d_gold;}

        //! Get the index of the army in the given slot
        int getArmytype(int slot) const;

        //! Get the army description of the given slot (what about a better name?)
        const Army* getArmy(int slot) const;
        
        //! Returns whether city was razed (i.e. destroyed)
        bool isBurnt() const {return d_burnt;}

        //! Returns whether the city is a capital
        bool isCapital() const {return d_capital;}

        //! Returns the original owner of this capital city
        Player *getCapitalOwner() const {return d_capital_owner;}

        //! Set the point where the city will send the produced armies
        //(-1,-1) as argument will disable it
        void setVectoring(Vector<int> p);

        //! Get the point where the city will send the produced armies
        Vector<int> getVectoring() const {return d_vector;}

	//! Returns true if the city isn't accepting too many vectored armies
	bool canAcceptVectoredUnit();

	//! Returns true if the city can accept a number of vectored armies 
	bool canAcceptVectoredUnits(Uint32 number_of_units);

	//! sets the point where the city will send the produced armies to.
	//Also changes the vectored units that are "en route" from this city.
	bool changeVectorDestination(Vector<int> dest);

    private:

        //! Produces the currently selected army
        Army * produceArmy();

	void randomlyImproveOrDegradeArmy(Army *army);

	//! sort the armies that this city produces by strength
	//! Note: only use this prior to the start of game.
	void sortProduction();

        // DATA
        Army* d_basicprod[4];         // possible basic productions
        int d_numbasic;             // max number of possible basic productions

        int d_production;           // number of produced armytype
        int d_duration;             // needed turns to finish current production
        Uint32 d_gold;                 // gold produced by city each turn
        int d_defense_level;        // defense of the city
        
        bool d_burnt;               // is city burnt down?
        bool d_vectoring;           // is vectoring active? 
        Vector<int> d_vector;          // where to send produced armies 
        bool d_capital;
	Player *d_capital_owner;    // the original owner of the capital city
};

#endif // CITY_H
