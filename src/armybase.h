// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2007, 2008 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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

#ifndef ARMY_BASE_H
#define ARMY_BASE_H

#include <SDL.h>
#include <string>

#include "defs.h"

class XML_Helper;

class ArmyBase
{
    public:

	//! The bitwise OR-able special bonus that the Army gives.
        enum Bonus {
	  //! Provides +1 strength to the Army when positioned in the open.
	  ADD1STRINOPEN      = 0x00000001,
	  //! Provides +2 strength to the Army when positioned in the open.
	  ADD2STRINOPEN      = 0x00000002,
	  //! Provides +1 strength to the Army when positioned in the forest.
	  ADD1STRINFOREST    = 0x00000004,
	  //! Provides +1 strength to the Army when positioned in the hills.
	  ADD1STRINHILLS     = 0x00000008, 
	  //! Provides +1 strength to the Army when positioned in a City.
	  ADD1STRINCITY      = 0x00000010,
	  //! Provides +2 strength to the Army when positioned in a City.
	  ADD2STRINCITY      = 0x00000020,
	  //! Provides +1 strength to the Stack when positioned in the hills.
	  ADD1STACKINHILLS   = 0x00000040,
	  //! Negate any City bonuses from an enemy Stack during a Fight.
	  SUBALLCITYBONUS    = 0x00000080,
	  //! Negates 1 strength point from an enemy Stack during a Fight.
	  SUB1ENEMYSTACK     = 0x00000100,
	  //! Provides +1 strength to all Army units in the Stack.
	  ADD1STACK          = 0x00000200,
	  //! Provides +2 strength to all Army units in the Stack.
	  ADD2STACK          = 0x00000400,
	  //! Negate all non-Hero bonuses in an enemy Stack during a Fight.
	  SUBALLNONHEROBONUS = 0x00000800,
	  //! Negate all Hero bonuses in an enemy Stack during a Fight.
	  SUBALLHEROBONUS    = 0x00001000, //0 enemy hero bonus
	  //! Provides a +1 strength to all Army units in a fortified Stack.
	  FORTIFY            = 0x00002000,
        };
	static Uint32 bonusFlagsFromString(const std::string str);
	static std::string bonusFlagsToString(const Uint32 bonus);
	static ArmyBase::Bonus bonusFlagFromString(const std::string str);
	static std::string bonusFlagToString(const ArmyBase::Bonus bonus);
        
	//! Various kinds of statistics that an instance of Army unit has.
	/**
	 * This enumeration assists in getting and setting of statistics in
	 * an instance of an Army unit.
	 */
        enum Stat {
	  //! How strong the Army unit is in battle.
	  STRENGTH = 0,
	  //! The maximum number of hitpoints that the Army unit can have.
	  HP = 3,
	  //! The maximum number of moves the Army unit has.
	  MOVES = 4,
	  //! The various Tile::Type that the Army moves efficiently in.
	  MOVE_BONUS = 5,
	  //! The special bonus the Army has (Army::Bonus).
	  ARMY_BONUS = 6,
	  //! How far the Army unit can see on a hidden map.
	  SIGHT = 7,
	  //! If the Army unit is in a boat or not.
	  SHIP = 8,
	  //! If the Army unit is having it's movement doubled/tripled or not.
	  MOVES_MULTIPLIER = 9,
        };

	static Uint32 moveFlagsFromString(const std::string str);
	static std::string moveFlagsToString(const Uint32 move_bonus);

	//! Copy constructor.
        ArmyBase(const ArmyBase& army);

	//! Loading constructor.
        ArmyBase(XML_Helper* helper);
        
	//! Create an empty army base.
	ArmyBase();

	//! Destructor.
        ~ArmyBase();

        // Set functions:
        
        //! Set how much gold this unit requires per turn.
        void setUpkeep(Uint32 upkeep){d_upkeep = upkeep;}
        
        //! Set the strength of the army.
        void setStrength(Uint32 strength) {d_strength = strength;}

        // Get functions
        
        //! Returns how many gold pieces this Army needs per turn.
        Uint32 getUpkeep() const {return d_upkeep;}
        

        //! Get the army bonus of the army.
        Uint32 getArmyBonus() const {return d_army_bonus;}

        //! Get the move bonus.
	/**
	 * Get which kinds of terrain tiles this Army moves efficiently 
	 * over top of.
	 *
	 * @return A bitwise OR-ing of the values in Tile::Type.
	 */
        Uint32 getMoveBonus() const {return d_move_bonus;}

        //! Get the move bonus of the army.
        Uint32 getMaxMoves() const {return d_max_moves;}

        //! Get the strength of the army.
        Uint32 getStrength() const {return d_strength;}

        //! Get the distance this army can see on a hidden map.
        Uint32 getSight() const {return d_sight;}

	std::string getArmyBonusDescription() const;

        //! Set how much XP this unit is worth when killed.
        void setXpReward(double xp_value){d_xp_value = xp_value;}

        //! Returns the number of XP that killing this Army garners it's killer.
        double getXpReward() const {return d_xp_value;}
    protected:

        //! Generic method for saving Army base data.
        bool saveData(XML_Helper* helper) const;

	//! How many turns the Army unit takes to be produced in a City.
	/**
	 * This value must be above 0.  Normal values for d_production are
	 * 1 through 4.
	 * This value does not change during gameplay.
	 */
        Uint32 d_production;

	//! The amount it costs to maintain this Army unit for this turn.
	/**
	 * @note The amount is in gold pieces.
	 *
	 * This value does not change during gameplay.
	 *
	 * @note Some special units have an upkeep of 0, but usually this
	 * value is more than zero.
	 */
        Uint32 d_upkeep;

	/**
	 * The strength of the Army unit is the prime factor when 
	 * calculating the outcome of a Fight.  This value should always
	 * be 1 or more, but not exceeding 15.
	 *
	 * This value can permanently increase when the Army unit increases
	 * it's level.
	 *
	 * Temporary increases due to the Army unit being on a certain kind 
	 * of terrain, or because another Army unit has conferred strength 
	 * on it (see Army::Bonus) are not reflected in d_strength.
	 *
	 * This value does not decrease during gameplay.
	 */
	//! The base strength of the Army unit.
        Uint32 d_strength;

	//! The maximum number of movement points that this Army unit has.
	/**
	 * This value must always be above 1.  Sane values are above 7.
	 *
	 * This value can be permanently increased when the Army unit 
	 * increases it's level.
	 *
	 * This value does not decrease during gameplay.
	 *
	 * @note When an Army unit is having it's movement doubled, or even
	 * tripled due to a Hero carrying an Item, this value does not 
	 * reflect that doubling or tripling.
	 */
        Uint32 d_max_moves;

	//! How far the Army unit can see on a hidden map.
	/**
	 * When a stack is moving on a hidden map, a certain number of
	 * tiles get illuminated or unshaded.  d_sight is the radius of
	 * tiles that this Army unit can illuminate.
	 *
	 * This value can be permanently increased when the Army unit 
	 * increases it's level.
	 *
	 * This value does not decrease during gameplay.
	 */
        Uint32 d_sight;

	//! The movement bonus of the Army unit.
	/**
	 * d_move_bonus represents the terrain tiles that the Army unit
	 * can travel efficiently over.  Traveling efficiently entails
	 * that it only costs 2 movement points to travel over that kind
	 * of terrain, no matter what the actual terrain movement value is.
	 *
	 * The movement bonus is a bitwise OR-ing of the values in 
	 * Tile::Type.
	 *
	 * When each of the members of Tile::Type are included in the
	 * d_move_bonus value, the Army unit is flying.
	 *
	 * This value does not change during gameplay.
	 */
        Uint32 d_move_bonus;

	/**
	 * d_army_bonus represents the special abilities this Army unit has.
	 * The special abilities are enumerated in Army::Bonus.
	 *
	 * The army bonus is a bitwise OR-ing of the values in Army::Bonus.
	 *
	 * This value does not change during gameplay.
	 */
	//! The special capbilities of the Army unit.
        Uint32 d_army_bonus;

	//! The gender of the Army unit.
	/**
	 * d_gender is one of the values contained in Army::Gender.
	 * This value does not change during gameplay (heh).
	 */
        Uint32 d_gender;

	//! The amount of XP this Army unit worth when killed by an assailant.
	/**
	 * When this Army unit is killed, d_xp_value is added to the killer's
	 * experience points.
	 *
	 * This value must be over 0.
	 *
	 * This value does not change during gameplay.
	 */
        double d_xp_value;


    private:
};

#endif // ARMY_BASE_H
