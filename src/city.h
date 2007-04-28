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
  * - more production (basic and advanced) can be bought
  * - the city produces more gold
  * - units defending the city get a defense bonus
  *
  * Some other things to mention are basic/advanced production and the system of
  * production slots.
  *
  * A city can have two totally different productions, basic
  * and advanced ones. Basic productions are only relatively simple units of the
  * default armyset which are kept if the city is taken over by another player
  * (who can also buy basic productions only from the default armyset). Advanced
  * productions are productions of the player armyset, usually better and more
  * expensive. Since they are bound to a player, they are removed whenever the
  * ownership of the city changes.
  *
  * A production slot is a production capability of a city. If a city has 4
  * different production slots, it may choose its production from a maximum of 4
  * different units. The slots are divided into basic production slots (only
  * filled by units of the default armyset) and advanced slots (only filled by
  * units of the owner's armyset).
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
        City(PG_Point pos, std::string name = "Noname", Uint32 gold = 20);

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

        /** Set the production of the city
          * 
          * @param index    the index of the internal production slot, -1 for none
          * @param advanced if false, produce a basic armytype, else an advanced
          */
        void setProduction(int index, bool advanced);

        
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
          * @param armytype     the index of the army within the armyset
          * @return true on success, false on error
          */
        bool addBasicProd(int index, int armytype);

        //! Clears the basic production of a given slot
        void removeBasicProd(int index);
        
        /** Add an advanced production
          * 
          * Advanced productions are those of the player's armyset. Production
          * slots are overwritten if neccessary.
          * 
          * @param index        the index of the production slot
          * @param armytype     the index of the army within the armyset
          * @return true on success, false on error
          */
        bool addAdvancedProd(int index, int armytype);

        //! Clears the advanced production of a given slot
        void removeAdvancedProd(int index);
        
        
        //! Changes the owner of the city and prepares it for takeover
        void conquer(Player* newowner);
        
        //! Sets the production to random starting values
        void setRandomArmytypes();

        //! Produces the strongest army the city can produce
        void produceLargestArmy();

        //! Add a hero to the stacks within the city
        void addHero(Hero* hero) const;

        //! Do everything neccessary for a new turn
        void nextTurn();

        //! Returns whether the city can produce a given army type
        bool hasProduction(int type, Uint32 armyset) const;

        //! Get the first free slot of basic production for the AI
	int getFreeBasicSlot();

        //! Get the first free slot of advanced production for the AI
        int getFreeAdvancedSlot();

        //! returns true if the city already has bought this production type
        bool isAlreadyBought(const Army * army, bool isadvanced);

        //! Get the defense level of the city
        int getDefenseLevel() const {return d_defense_level;}

        //! Get the maximum number of basic productions of the city
        int getMaxNoOfBasicProd() const {return d_numbasic;};

        //! Get the number of basic productions of the city
        int getNoOfBasicProd();

        //! Get the maximum number of advanced productions of the city
        int getMaxNoOfAdvancedProd() const {return d_numadv;};

        //! Get the player owning the city
        Player* getPlayer() const {return d_player;}

        //! Get the duration till the current production is finished
        int getDuration() const {return d_duration;}

        //! Get the index of the current internal production slot
        int getProductionIndex() const {return d_production;}
        
        //! Do we currently produce an advanced army type?
        bool getAdvancedProd() const {return d_adv_prod;};

        //! Get the income of the city per turn
        Uint32 getGold() const {return d_gold;}

        //! Get the index of the army in the given slot
        int getArmytype(int slot, bool advanced) const;

        //! Get the army description of the given slot (what about a better name?)
        const Army* getArmy(int slot, bool advanced) const;
        
        //! Returns whether city was razed (i.e. destroyed)
        bool isBurnt() const {return d_burnt;}

        //! Returns whether the city is a capital
        bool isCapital() const {return d_capital;}

        //! Returns whether the player is a "friend" (==owner) of the city
        bool isFriend(Player* player) const;

        //! Set the point where the city will send the produced armies
        //(-1,-1) as argument will disable it
        void setVectoring(PG_Point p);

        //! Get the point where the city will send the produced armies
        PG_Point getVectoring() const {return d_vector;}

    private:
        //! Returns a non-full stack in the city or creates a new one
        Stack* getFreeStack() const;

        //! Produces the currently selected army
        void produceArmy();


        // DATA
        Player* d_player;           // Owner
        
        int d_basicprod[4];         // possible basic productions
        int d_advprod[3];           // possible advanced productions
        int d_numbasic;             // max number of possible basic productions
        int d_numadv;               // max number of poss. advanced productions

        int d_production;           // number of produced armytype
        bool d_adv_prod;            // advanced (true) or basic production
        int d_duration;             // needed turns to finish current production
        Uint32 d_gold;                 // gold produced by city each turn
        int d_defense_level;        // defense of the city
        
        bool d_burnt;               // is city burnt down?
        bool d_vectoring;           // is vectoring active? 
        PG_Point d_vector;          // where to send produced armies 
        bool d_capital;
};

#endif // CITY_H
