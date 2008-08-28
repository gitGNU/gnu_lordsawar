// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2002 Mark L. Amidon
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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
class ArmyProdBase;
class ArmyProto;
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
	 * @param army         The Army production base to add.  Look at the
	 *                     Army class to find out what a production base is.
         */
        void addProductionBase(int index, ArmyProdBase *army);

        //! Clears the basic production of a given slot.
	/**
	 * @param index  The slot to remove the Army production base from.
	 *               This method deletes the Army production base object.  
	 *               This parameter must be a a value between 0 and 3.
	 */
        void removeProductionBase(int index);
        
        //! Changes the owner of the city and prepares it for takeover.
	/**
	 * @param newowner  The pointer to the Player in Playerlist who is the
	 *                  new owner of this City.
	 */
        void conquer(Player* newowner);
        
        //! Sets the production to random starting values
	/**
	 * @param produce_allies   Whether or not awardable Army units can
	 *                         be production bases in this city.
	 * @param likely  A value between 0 and 3 that represents how likely
	 *                more production bases are.
	 */
	void setRandomArmytypes(bool produce_allies, int likely);

        //! Produces one instance of the strongest Army the city can produce.
        void produceStrongestProductionBase();

        //! Produces one instance of the weakest Army the city can produce.
        void produceWeakestProductionBase();

        //! Produces a scout in the city.
	/**
	 * A Scout is defined as the first Army in the Armyset.
	 * It is used when neutral cities are "average".  
	 * It is also used as a fallback when a city must have one Army unit.
	 * @note The scout army doesn't have to be an Army production base in
	 * the city for this method to operate.
	 */
        void produceScout();

        //! Do everything neccessary for a new turn.
	/**
	 * Checks to see if an Army unit should be produced.
	 * Checks to see if a newly produced Army unit should be sent off to 
	 * the city's vector destination.
	 */
        void nextTurn();

        //! Returns whether or not the city can produce a given army type.
	/**
	 * This method scans the production slots of the city for the given 
	 * army prototype.
	 *
	 * @param type      The index of the Army prototype in the Armyset.
	 * @param armyset   The unique Id of the armyset for which to check
	 *                  if the given type is already a production base
	 *                  in this city.
	 * @return True if the given army prototype is already a production
	 *         base in the city.  Otherwise false.
	 */
        bool hasProductionBase(int type, Uint32 armyset) const;

        //! Returns true if the city already has bought this production type.
        bool hasProductionBase(const ArmyProto * army);

        //! Return the first slot that doesn't have a production base.
	int getFreeBasicSlot();

        //! Return the defense level of the city.
        int getDefenseLevel() const {return d_defense_level;}

	//! Returns the amount of gold needed to increase the defense level.
	/**
	 * @note This method isn't called.  Cities are not upgraded.
	 *
	 * Returns the number of gold pieces needed to increase the defense
	 *         level, or -1 if city can't be upgraded.
	 */
	int getGoldNeededForUpgrade() const; 
	
        //! Returns the maximum number of production bases of the city.
	/**
	 * The city has this many production slots in total.  This value 
	 * should always return 4.
	 *
	 * @return The maximum number of Army production bases that this city
	 *         can have.
	 */
        int getMaxNoOfProductionBases() const {return d_numprodbase;};

	void setMaxNoOfProductionBases(int max) {d_numprodbase = max;};

        //! Return the number of basic productions of the city.
	/**
	 * Scan the production slots and count how many are filled with an
	 * Army production base.
	 *
	 * @return The current number of used slots that the city has.
	 */
        int getNoOfProductionBases();

        //! Get the number of turns until current production base is finished.
        int getDuration() const {return d_duration;}

        //! Return the index of the active production slot.
	/**
	 * @return The index of the active production slot, or -1 if the City
	 *         isn't producing anything.
	 */
        int getActiveProductionSlot() const {return d_active_production_slot;}
        
	//! Set the active production base of the city.
        /**
	 * Make the Army production base in particular slot active, so that
	 * the Army starts being produced.
	 *
         * @param index  The index of the production slot to activate. 
	 *               -1 means no production at all.   This must be a value
	 *               between -1 and 3.
         */
        void setActiveProductionSlot(int index);
        
        //! Return the income of the city per turn.
        Uint32 getGold() const {return d_gold;}

        //! Return the index of the army in the given slot.
	/**
	 * @param slot  The production slot to return the army type for.  This
	 *              value ranges between 0 and 3.
	 *
	 * @return The index of the Army prototype unit within it's Armyset,
	 *         or -1 if no production base is allocated to that slot.
	 */
        int getArmytype(int slot) const;

        //! Return the army production base of the given slot.
        const ArmyProdBase * getProductionBase(int slot) const;
        
        //! Returns whether or not the city has been destroyed.
        bool isBurnt() const {return d_burnt;}

        //! Returns whether or not the city is a capital city.
        bool isCapital() const {return d_capital;}

        //! Returns the original owner of this capital city.
        Player *getCapitalOwner() const {return d_capital_owner;}

        //! Set the point where the city will send the produced armies.
        /**
	 * @note This method does not check to see if the destination point
	 * can receive yet another vectored unit.
	 *
	 * @param pos  The position on the map to send the produced units to.
	 *             The position must point to another city, or a planted
	 *             standard.  Set to (-1,-1) if not vectoring.
	 */
        void setVectoring(Vector<int> pos);

        //! Get the position where the city will send produced armies.
	/**
	 * @return The position on the map where the city will send it's newly
	 *         produced armies.  Returns (-1,-1) if the city is not
	 *         vectoring.
	 */
        Vector<int> getVectoring() const {return d_vector;}

	//! Returns true if the city isn't accepting too many vectored armies.
	/**
	 * Scans all of the vectoring units going to this city.  If vectoring
	 * to this city would accrue the count to 
	 * MAX_ARMIES_VECTORED_TO_ONE_CITY, this method returns false.
	 *
	 * @return True if the city can have another city vectoring to it.
	 *         Otherwise false.
	 */
	bool canAcceptVectoredUnit();

	//! Returns true if the city can accept a number of vectored armies.
	/**
	 * Instead of checking to see if one unit can be vectored here like in 
	 * City::canAcceptVectoredUnit, this method checks if the city can
	 * receive multiple units (from an equal number of cities, because a
	 * city can only produce and vector one army at a time).
	 *
	 * @param number_of_units  The number of vectored units to check to
	 *                         see if this city can receive.
	 *
	 * @return True if the city can have this many more cities vectoring 
	 *         to it.  Otherwise false.
	 */
	bool canAcceptVectoredUnits(Uint32 number_of_units);

	//! Changes the vectoring destination, but for en-route units too.
	/**
	 * This method acts like City::setVectoring but also changes the
	 * destination of the units that this city has already vectored.
	 *
	 * @note This method does not check to see if the destination point
	 * can receive yet another vectored unit.
	 *
	 * @param dest  The new destination position on the game map to vector 
	 *              newly produced Army units to.  The position must point
	 *              to a city or a planted standard.
	 *
	 * @return This method always returns true.
	 */
	bool changeVectorDestination(Vector<int> dest);

	//! Return how many armies are in the city.
	Uint32 countDefenders();

	Uint32 countCitiesVectoringToHere();

	//! This makes the army show up.  called when it's time.
	const Army *armyArrives();

    private:

        //! Produces the currently active Army production base.
        Army * produceArmy();

	//! Makes an Army production base be a little different than expected.
	/**
	 * This method is responsible for making Army production bases be
	 * a little bit different than the Army prototypes they derive from.
	 * This method will take a prototype (e.g. scouts), and maybe give it
	 * a strength of 2 (rather than 1), or a time of 2 (rather than 1).
	 * It also works the other way.  Elephants can be altered to take 5 
	 * turns instead of 4 turns, or have their strength decreased to 7.
	 */
	void randomlyImproveOrDegradeArmy(ArmyProdBase *army);

	//! Sort the Army production bases that this city produces by strength.
	/**
	 * @note Pnly use this prior to the start of game.
	 */
	void sortProduction();

        // DATA

	//! The production slots for this city.
	/**
	 * Each slot holds an Army production base that the City can 
	 * potentially produce Army units from.  When a slot is empty, it is 
	 * set to NULL.
	 */
        ArmyProdBase * d_prodbase[MAX_PRODUCTION_SLOTS_IN_A_CITY];

	//! The maximum number of slots.
	/**
	 * Equal to MAX_PRODUCTION_SLOTS_IN_A_CITY.
	 */
        int d_numprodbase;

	//! The active production slot.
	/**
	 * The Army production base in this slot is the Army unit that the
	 * city is currently busy creating.
	 */
        int d_active_production_slot;

	//! Number of turns until the next Army is produced.
	/**
	 *  Number of turns required to finish the current production.
	 *  When this value hits 0, the new Army unit is created.
	 */
        int d_duration;

	//! The City gives the Player this much gold per turn.
        Uint32 d_gold;

	//! The defense level of the city.
	/**
	 * This value is not taken into consideration for battles.
	 * It is just for show.
	 */
        int d_defense_level;
        
	//! Whether or not the city is destroyed or not.
	/**
	 * When City objects are razed they become uninhabitable and also 
	 * unconquerable.  They can still be used as a jumping off point
	 * into water, but they will not produce any more Army units or
	 * provide income to any players.
	 */
        bool d_burnt;

	//! Whether or not the city is vectoring units.
	/**
	 * Vectoring involves sending units to a destination other than this
	 * city.  The destination can be any other city also owned by the
	 * owner of this city, or to the planted standard belonging to the
	 * owner of this city.  It always takes two turns to get to the
	 * vectoring destination.  See VectoredUnitlist for more information.
	 *
	 * If this value is True, then the newly produced Army units are
	 * vectored to a destination determined by City::d_vector.
	 */
        bool d_vectoring;

	//! Where to send newly produced Army units to.
	/**
	 * A position on the game map to send the Army units that this City
	 * produces.  When vectoring is disabled, this value is (-1, -1).
	 * The position on the map must coincide with a City owned by the
	 * owner of this city, or the planted standard of the owner of this
	 * city.
	 */
        Vector<int> d_vector;

	//! Whether or not this is a capital city.
	/**
	 * Capital cities do not have a purpose other than bragging rights
	 * among players during gameplay.  Every Player starts with exactly
	 * one capital city.
	 *
	 * Conquering a capital city does not change the original owner of
	 * the capital city.
	 *
	 * If this value is True, this city is a capital city of the
	 * City::d_capital_owner player.  When false, it is not a capital city.
	 */
        bool d_capital;

	//! The original owner of this capital city.
	Player *d_capital_owner;
};

#endif // CITY_H
