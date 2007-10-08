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

class Player;
class Stack;
class Army;
class Hero;


/** A city on the game map
  * 
  * The City class includes the whole functionality of a city on the game map. A
  * city can produce armies, place produced armies on its fields and buy
  * production as well as be upgraded.
  *
  * Upgrading affects the city in several ways:
  * - more production can be bought
  * - the city produces more gold
  * - units defending the city get a defense bonus
  *
  * Some other things to mention are basic production and the system of
  * production slots.
  *
  * A production slot is a production capability of a city. If a city has 4
  * different production slots, it may choose its production from a maximum of 4
  * different units. The slots are divided into basic production slots (only
  * filled by units of the default armyset).
  */


class City : public Location
{
    public:
        /** Preferred constructor
          * 
          * @param pos          the location of the city
          * @param name         the name of the city
          * @param gold         the amount of gold the city produces each turn
          */
        City(Vector<int> pos, std::string name = "Noname", Uint32 gold = 20);

        //! The loading constructor. See XML_Helper for details.
        City(XML_Helper* helper);
        City(const City&);
        ~City();

        //! Save the city status. See XML_Helper for info.
        bool save(XML_Helper* helper) const;
        
        //! Set the player owning the city. Don't use this for conquering!!!
        void setPlayer(Player* player){d_player = player;}

        //! Set the gold the city produces each turn
        void setGold(Uint32 gold){d_gold = gold;}

        //! Set if the city is destroyed
        void setBurnt(bool burnt){d_burnt = burnt;}

        //! Sets whether the city is a capital
        void setCapital(bool capital) {d_capital = capital;}

        //! Sets whether the city is a capital
        void setCapitalOwner(Player *p) {d_capital_owner = p;}

        /** Set the production of the city
          * 
          * @param index    the index of the internal production slot, -1 for none
          */
        void setProduction(int index);

        
        //! Raise the defense level by one. Return true on success.
        bool raiseDefense();

        //! Lower the defense level by one. Return true on success.
        bool reduceDefense();

        /** Add a basic production slot
          *
          * Basic productions are those of the default armyset, which every
          * player can produce. Overwrites the production slot if neccessary.
          * 
          * @param index        the index of the production slot; if set to -1,
          *                     the city will try to find a free production slot
          *                     and overwrite an existing one if neccessary.
	  * @param army         the army to add
          * @return true on success, false on error
          */
        bool addBasicProd(int index, Army *army);

        //! Clears the basic production of a given slot
        void removeBasicProd(int index);
        
        //! Changes the owner of the city and prepares it for takeover
        void conquer(Player* newowner);
        
        //! Sets the production to random starting values
        void setRandomArmytypes();

        //! Produces the strongest army the city can produce
        void produceStrongestArmy();

        //! Produces the weakest army the city can produce
        void produceWeakestArmy();

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

        //! Get the player owning the city
        Player* getPlayer() const {return d_player;}

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

        //! Returns whether the player is a "friend" (==owner) of the city
        bool isFriend(Player* player) const;

        //! Set the point where the city will send the produced armies
        //(-1,-1) as argument will disable it
        void setVectoring(Vector<int> p);

        //! Get the point where the city will send the produced armies
        Vector<int> getVectoring() const {return d_vector;}

	//! Returns true if the city isn't accepting too many vectored armies
	bool canAcceptVectoredUnit();

    private:

        //! Produces the currently selected army
        Army * produceArmy();


        // DATA
        Player* d_player;           // Owner
        
        Army* d_basicprod[4];         // possible basic productions
	//std::vector<Army*> d_basicprod;
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
