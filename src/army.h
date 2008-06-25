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

#ifndef ARMY_H
#define ARMY_H

#include <SDL.h>
#include <string>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "defs.h"
#include "Ownable.h"

class Player;
class XML_Helper;

//! An instance of an Army unit, an Army unit type, or an Army production base.
 /**
  * This class is the atom of every army.  It contains values such as
  * strength, movement points, upkeep, and so on.
  *
  * The purpose of the Army class is three-fold; an Army class can hold a
  * an Army prototype, or a production base, or an Army instance.
  *
  * The Army instance is the most frequently used purpose of the Army class.
  * An Army instance has a unique Id among all other game objects, has an 
  * owner (Player), and is included in a Stack.
  *
  * The Army unit prototype purpose is the second-most frequently used 
  * purpose of the Army class.  These types originate from a configuration 
  * file; for example: army/default/default.xml.  Every Army unit type has an 
  * type value that makes it unique among all other Army unit types in an 
  * Armyset.
  *
  * The production base purpose refers to the Army units that are included
  * in the City class as potential Army units that the City can produce.
  * This purpose is exactly the same as the Army unit prototype purpose, 
  * except it knows which Armyset it comes from, and it knows which Army
  * unit type it derives from.
  *
  * Maybe these three purposes will be split up into three or more classes 
  * in the future.
  */
class Army : public Ownable, public sigc::trackable
{
    public:

	//! The purpose of this Army object.
	enum ArmyContents {
	  //! A prototype of an Army. e.g. Scouts.
	  TYPE = 0, 
	  //! A description of a kind of Army that a City can produce.
	  PRODUCTION_BASE = 1, 
	  //! An instance of an Army that is moving around the map in a Stack.
	  INSTANCE = 2
	};

        //! The different genders an Army unit can have.
	/**
	 * The purpose of this enumeration is to show the correct 
	 * recruitment picture for Hero units when they emerge in a 
	 * City, and the Player has to decide if they want it or not.
	 */
        enum Gender {
	  //! The Army unit has no gender (Not used).
	  NONE = 0, 
	  //! The Army unit is male.
	  MALE = 1, 
	  //! The Army unit is female.
	  FEMALE = 2
	};

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

        /** 
	 * Make a new instance of an Army unit by copying it from another.
         * 
	 * This constructor is used to create a new army from an 
	 * Army prototype.  Army prototypes have an Army::d_id of 0; 
	 * and this constructor behaves a bit differently when this is the 
	 * case.  This constructor also is used to copy Army instances
	 * during gameplay.
	 *
	 * @param army    The Army unit to create the new one from.
	 * @param owner   The Player who owns this Army.
	 * @param for_template   Whether the army should get and id
         */
	//! Copy constructor.
        Army(const Army& army, Player* owner = 0, bool for_template = false);

        /** 
	 * Load an Army from an opened saved-game file or from an opened
	 * armyset configuration file.
         *
         * The constructor has to care for all three cases. Sometimes, an army
         * prototype is loaded, from which other units are cloned, sometimes
         * actual Army instances have to be loaded, and sometimes we load
	 * up a production base when loading the Army units that a City
	 * can produce.
         *
         * @param helper       The opened saved-game file to load from.
         * @param contents     Which purpose this Army unit has.
         */
	//! Loading constructor.
        Army(XML_Helper* helper, enum ArmyContents contents = INSTANCE);
        
	/**  
	 * Creates an empty prototype Army unit.  This constructor is only
	 * used in the ArmySetWindow (the Armyset editor).
	 */
	//! Create an empty army.
	Army();

	//! Destructor.
        virtual ~Army();

        // Set functions:
        
        //! Set the Id of Armyset and type that this Army belongs to.
        void setArmyset(Uint32 armyset, Uint32 type);

        //! Set the name of the Army.
        void setName(std::string name){d_name = name;}

        //! Set the basic image of the Army.
        void setPixmap(SDL_Surface* pixmap);

        //! Set the image mask of the unit type (for player colours).
        void setMask(SDL_Surface* mask);

        //! Set how much XP this unit is worth when killed.
        void setXpReward(double xp_value){d_xp_value = xp_value;}

        //! Set an Army statistic.
        void setStat(Stat stat, Uint32 value);

        //! Sets the descriptive text for this Army.
        void setDescription(std::string text) {d_description = text;};
        
        //! Set how much gold this unit requires per turn.
        void setUpkeep(Uint32 upkeep){d_upkeep = upkeep;}
        
        //! Set the current number of hitpoints of this army.
        void setHP(Uint32 hp) {d_hp = hp;}

        //! Set how many turns this unit type needs to be produced.
        void setProduction(Uint32 production){d_production = production;}

        //! Set the gold pieces needed to add this Army to a city's production.
        void setProductionCost(Uint32 production_cost)
	  {d_production_cost = production_cost;}

        //! Set the gender of the army type.
        void setGender(Gender gender){d_gender = gender;}

        /** 
         * When you select a stack on the map, you can toggle the armies 
	 * which will be moved when you click somewhere else on the map.
         * Grouped==true means that this army will be among the selected
         * armies.
         */
	//!Set the grouped state of this Army.
        void setGrouped(bool grouped){d_grouped = grouped;}

	//! Sets whether or not the Army is a Hero.
	void setHero(bool hero) {d_hero = hero;}

        //! Sets whether or not the Army has a particular medal.
        void setMedalBonus(Uint32 index, bool value) 
	  {d_medal_bonus[index]=value;}
        
        //! Sets the number of battles the Army unit participated in.
        void setBattlesNumber(Uint32 value) {d_battles_number=value;}
         
        //! Sets the number of hits this Army unit has scored against a foe.
        void setNumberHasHit(double value) {d_number_hashit=value;}

        //! Sets the number of hits this Army unit has suffered against a foe.
        void setNumberHasBeenHit(double value) {d_number_hasbeenhit=value;}

	//! Sets whether or not this Army prototype can found in a ruin.
	void setDefendsRuins(bool defends) {d_defends_ruins = defends; }

	/**
	 * Sets whether or not this Army prototype can be a reward for
	 * Quest, or if Army units of this kind can accompany a new
	 * Hero when one emerges in a City.
	 */
	//! Sets the awardable state of an Army prototype.
	void setAwardable (bool awardable) {d_awardable = awardable; }

	//! Sets whether or not this Army unit is in a boat.
	void setInShip (bool s);

	//! Sets whether or not this Army unit is in a tower.
	void setFortified (bool f);

        // Get functions
        
	//! Get the Id of the Armyset to which the Army's type belongs.
        Uint32 getArmyset() const {return d_armyset;}
        
        //! Returns the name of the Army.
        std::string getName() const {return d_name;}

        //! Return the unique id of this army.
	/**
	 * A unique Id for this Army unit instance among all other objects
	 * in the game.
	 * @note This value is 0 for Army prototypes.
	 */
        Uint32 getId() const {return d_id;}

        //! Get the type of this army.
	/**
	 * The type of the Army is the index of it's type in the Armyset.
	 */
        Uint32 getType() const {return d_type;}

        //! Get the image of the army. 
	/**
	 * Getting the image for the Army unit involves the GraphicsCache.
	 * Pixmaps of Army instances are coloured in the Player's colour,
	 * and may have medals drawn on their image.
	 * Pixmaps of Army types are not coloured.
	 */
        SDL_Surface* getPixmap() const;

        //! Returns the mask (read-only) for player colors.
        SDL_Surface* getMask() const {return d_mask;}

	//! Returns the basename of the picture's filename
	/**
	 * Returns the filename that holds the image for this Army.
	 * The filename does not have a path, and the filename does
	 * not have an extension (e.g. .png).
	 */
	std::string getImageName() const {return d_image;}

	//! Sets the filename of the image.
	void setImageName(std::string image) {d_image = image;}

        //! Returns the descriptive text of this Army.
        std::string getDescription() const {return d_description;}

        //! Returns how many gold pieces this Army needs per turn.
        Uint32 getUpkeep() const {return d_upkeep;}
        
        //! Returns how many turns this Army needs to be produced.
        Uint32 getProduction() const {return d_production;}

        //! Returns how much gold setting up the production costs
	/**
	 * @return The amount of gold pieces required to add this Army
	 *         into the City's suite of 4 production slots.
	 */
        Uint32 getProductionCost() const {return d_production_cost;}

        //! Returns the number of XP that killing this Army garners it's killer.
        double getXpReward() const {return d_xp_value;}

        //! Return the gender of the Army.
        Uint32 getGender() const {return d_gender;}

        /** 
         * If modified is set to false, you get the raw, inherent value of
         * the army. Set it to true to get the modified one. This is not
         * important for generic armies, but heroes can have their stats
         * modified by wearing items.
	 *
	 * @param stat     The statistic to get the value of.
	 * @param modified Whether or not we get the modified stat value.
	 *
	 * @return The value of the statistic.
         */
	//! Returns the value of the given stat for the Army.
        virtual Uint32 getStat(Stat stat, bool modified=true) const;

        //! Get the current number of hitpoints that the Army has.
        Uint32 getHP() const {return d_hp;}

        //! Get the current number of movement points that the Army has.
        Uint32 getMoves() const {return d_moves;}

        //! Get the move bonus.
	/**
	 * Get which kinds of terrain tiles this Army moves efficiently 
	 * over top of.
	 *
	 * @return A bitwise OR-ing of the values in Tile::Type.
	 */
        Uint32 getMoveBonus() const {return d_move_bonus;}

        //! Get the current number of experience points that the Army unit has.
        double getXP() const {return d_xp;}

        //! Get the current level of the Army.
        Uint32 getLevel() const {return d_level;}

        //! Returns the grouped state of the Army within a Stack.
        bool isGrouped() const {return d_grouped;}

        //! Returns whether or not the Army is a Hero.
        bool isHero() const {return d_hero;}

        //! Return which medals this Army unit has.
        bool* getMedalBonuses() const {return (bool*)&d_medal_bonus;}

        //! Return whether or not the Army has a particular medal.
        bool getMedalBonus(Uint32 index) const {return d_medal_bonus[index];}

        //! Returns the number of battles the Army unit participated in.
        Uint32 getBattlesNumber() const {return d_battles_number;}

        //! Returns the number of blows the Army unit has scored against a foe.
        double getNumberHasHit() const {return d_number_hashit;}

        //! Returns the number of hits this Army unit has suffered.
        double getNumberHasBeenHit() const {return d_number_hasbeenhit;}

	//! Gets whether or not this army type can found in a ruin.
	bool getDefendsRuins() const {return d_defends_ruins; }

	/**
	 * Gets whether or not this army can be a reward for completing a 
	 * Quest, or if an Army unit of this type can accompany a new
	 * Hero when one emerges in a City.
	 */
	//! Gets the awardable state of the Army.
	bool getAwardable() const {return d_awardable; }

	//! Return whether or not the Army is in a tower.
	bool getFortified ();

	//! Returns how many experience points the next level requires.
        Uint32 getXpNeededForNextLevel() const;

	//! Generate a string that describes what bonuses this Army has.
	std::string getArmyBonusDescription() const;

        /** 
	 * Regenerate an amount of the Army unit's hitpoints but not 
	 * exceeding the maximum number of hitpoints.
	 *
         * @param hp       The amount of hitpoints to heal.  Set to zero for 
	 *                 "natural" healing -- but this feature is not 
	 *                 currently used because wounded army units are always 
	 *                 fully healed after battle.
         */
	//! Heal the Army unit.
        void heal(Uint32 hp = 0);

        /** 
	 * Decrease the number of hitpoints that this Army has.
         * 
         * @param damageDone       The amount of damage that the Army suffers.
	 *
         * @return True if the Army unit has died, otherwise false.
         */
	//! Damage the Army.
        bool damage(Uint32 damageDone);

        /** 
	 * Reduce the number of moves that the Army unit has.
	 * @note This method doesn't reduce the maximum number of movement 
	 * points; it reduces the current number of movement points, which 
	 * get restored at the start of the next turn.
         * 
         * @param moves      The number of movement points to consume.
         */
	//! Consume some movement points.
        void decrementMoves(Uint32 moves);

        //! Restores the number of movement points to the maximum level.
        void resetMoves();

	/**
	 * Add 1 to the strength of the Army unit if it has not already
	 * visited the Temple at which it's parent stack is currently
	 * sitting on.
	 *
	 * @return True if the Army unit was blessed, otherwise false.
	 */
	//! Bless the Army unit if it hasn't already visited this Temple.
        bool bless();

        //! Increases the experience points of the Army by the given amount.
        void gainXp(double n);

        //! Checks whether or not the Army unit can advance a level.
        bool canGainLevel() const;

        /** 
	 * Increase the Army unit's level, and increase one of three stats;
	 * Stat::STRENGTH, Stat::MOVES, or Stat::SIGHT.
	 *
         * @param stat     The stat to increase.
	 *
         * @return How much the statistic increases or -1 upon error 
	 *         (e.g. because the XP is not enough).
         */
	//! Increase the Army's level, and increase a given stat.
        int gainLevel(Stat stat);

	/**
	 * Calculate how much a stat is increased because the Army unit
	 * has increased it's level.
	 *
	 * @param stat  One of Stat::STRENGTH, Stat::MOVES, or Stat::SIGHT.
	 *
	 * @return The new value of the stat after it is increased.
	 */
	//! Return how much the stat would be boosted by gaining a level.
        int computeLevelGain(Stat stat);

        void printAllDebugInfo() const;

        //! Saves the Army to an opened saved-game file.
	/**
	 * @param contents  What the purpose of this Army is.
	 */
        virtual bool save(XML_Helper* helper, 
			  enum ArmyContents contents = INSTANCE) const;
        
	/**
	 * @note This signal is static because sometimes the army doesn't 
	 * exist yet when the signal is connected.
	 *
	 * @param army  The army that has died.
	 */
	//! Emitted when an Army has died.
        static sigc::signal<void, Army*> sdying;

    protected:
        //! Generic method for saving Army data.  Useful to the Hero class.
        bool saveData(XML_Helper* helper, 
		      enum ArmyContents contents = INSTANCE) const;

        //! Copies the generic data from the Army prototype.
        void copyVals(const Army* a);
        
	//! The index of the Army unit's type in it's Armyset.
        Uint32 d_type;

	//! The Id of the Armyset that the Army prototype belongs to.
        Uint32 d_armyset;

	//! The picture of the Army prototype.
        SDL_Surface* d_pixmap;

	//! The mask portion of the Army prototype picture.
        SDL_Surface* d_mask;
        
	//! The name of the Army unit.  e.g. Scouts.
        std::string d_name;

	//! The description of the Army unit.
        std::string d_description;

	//! How many turns the Army unit takes to be produced in a City.
	/**
	 * This value must be above 0.  Normal values for d_production are
	 * 1 through 4.
	 * This value does not change during gameplay.
	 */
        Uint32 d_production;

        //! How many gold pieces needed to add this Army to a city's production.
	/**
	 * If d_production_cost is over zero, then the Army can be purchased.
	 * If not, then the Army unit cannot be incorporated into a 
	 * City's production at any price.
	 *
	 * This value does not change during gameplay.
	 */
        Uint32 d_production_cost;

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

	/**
	 * The maximum number of hitpoints is the secondmost important
	 * factor when calculating the outgoing of a Fight.
	 *
	 * This value should always be 2.
	 *
	 * This value does not change during gameplay.
	 */
	//! The maximum number of hitpoints this Army unit has.
        Uint32 d_max_hp;

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

	//! Movement point multiplier of the Army unit.
	/**
	 * If an Army unit is being affected by a Hero unit's Item that
	 * confers the Item::DOUBLEMOVESTACK Item::Bonus, the effect is
	 * stored in d_max_moves_multiplier.
	 *
	 * This value must be 1 or more.
	 *
	 * This value typically changes from 1 to 2, and back to 1 during
	 * gameplay.
	 */
	Uint32 d_max_moves_multiplier;

	//! Movement point bonus due to resting.
	/**
	 * When an army unit doesn't use all of it's movement points in a
	 * turn, some of those points get held-over until the following turn.
	 * If a unit has 3 movement points remaining, the bonus is 2.  If the
	 * unit has 2 movement points remaining, the bonus is 2.  If the unit
	 * has 1 movement point remaining, the bonus is 1.
	 *
	 * This value is a number between 0 and 2.
	 */
	Uint32 d_max_moves_rest_bonus;

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

	/**
	 * Being in a ship affects the Army's strength in battle.
	 * Every army has a strength of 4 when fighting on a boat.
	 * It also affects the number of moves that the Army has.
	 * See MAX_BOAT_MOVES.
	 *
	 * This value gets set and unset as Army unit's stack goes in
	 * and out of the water.
	 */
	//! Whether or not this Army unit is afloat in a boat.
	bool d_ship;

	//! The gender of the Army unit.
	/**
	 * d_gender is one of the values contained in Army::Gender.
	 * This value does not change during gameplay (heh).
	 */
        Uint32 d_gender;

	//! The Id of the Army unit.
	/**
	 * d_id is a unique value among all other game objects.  If it is 0,
	 * the Army is a prototype.
	 *
	 * This value does not change during gameplay.
	 */
        Uint32 d_id;

	//! The current number of hitpoints that the Army unit has.
	/**
	 * When this value is 0 it means the Army unit is dead.
	 * 
	 * During a Fight this value gets decremented as the Army unit
	 * suffers attacks by enemy Army units.
	 *
	 * After a Fight, this value gets restored to d_max_hp.
	 *
	 * d_hp does not exceed d_max_hp.
	 */
        Uint32 d_hp;

	//! The current number of movement points that the Army unit has.
	/**
	 * As the Army unit moves around in a Stack on the GameMap, it travels
	 * over certain terrain tiles.  As the Army unit moves over a
	 * particular terrain Tile, d_moves dwindles in value.
	 *
	 * At the end of a round, this value gets restored to d_max_moves.
	 *
	 * d_moves does not exceed d_moves_hp.
	 */
        Uint32 d_moves;

	//! The current level of experience points the Army unit has.
	/**
	 * This value increases as the Army unit assists in killing enemy
	 * Army units.  This value does not decrease during gameplay.
	 *
	 * This value affects what d_level the Army unit is.
	 */
        double d_xp;

	//! The experience level the Army unit has attained.
	/**
	 * This value increases as the Army unit increases it's experience
	 * points.  d_level increases when a multiple of Army::xp_per_level 
	 * is surpassed.  d_level does not decrease during gameplay.
	 *
	 * d_level is not factored into any calculations that affect the
	 * outcome of the game.  It's just for show.
	 *
	 * @note Only Hero units advance in levels.
	 */
        Uint32 d_level;

	//! Whether or not the Army unit is grouped into it's parent Stack.
	/**
	 * When splitting stacks, it is necessary to know which Army units
	 * are staying in the original Stack, and which ones are going into
	 * the new Stack.
	 *
	 * If an Army is grouped, it is going into the new Stack.
	 *
	 * When attacking, only the grouped Army units in the Stack attack.
	 */
        bool d_grouped;

	/**
	 * There are three different medals that an Army unit can win.
	 *
	 * Merciless Medal: medal for being extremely merciless.  A unit 
	 *                  gets this medal if in a combat it scores more 
	 *                  than 90% of hits.
	 * Defender Medal: medal for being very good in defense.  A unit 
	 *                 gets this medal if in a combat is never hit.
	 * Veteran Medal: medal for being very good in combat.  A unit gets 
	 *                this medal if it survives 10 battles.
	 *
	 * Medals do not affect the game outcome and are just for show.
	 */
	//! The medals that the Army unit has been given.
        bool d_medal_bonus[3];

        //! The total number of battles that this Army unit has fought in.
	/**
	 * d_battles_number is a counter that is used to know when to
	 * award the Veteran's medal to this Army unit.
	 *
	 * This value does not decrease during gameplay.
	 */
        Uint32 d_battles_number;

        //! The weighted number of hits per battle for this Army unit.
	/**
	 * d_number_hashit is a counter that is used to know when to award
	 * the Merciless medal to this Army unit.
	 *
	 * This value does not decrease during gameplay.
	 */
        double d_number_hashit;

        //! The weighted number of times the Army unit has been hit in a battle.
	/**
	 * d_number_hasbeenhit is a counter that is used to know when to award
	 * the Defender medal to this Army unit.
	 *
	 * This value does not decrease during gameplay.
	 */
        double d_number_hasbeenhit;

	//! Whether or not the Army prototype can defend a Ruin.
	/**
	 * Some Army unit can be the guardian of a Ruin.  Hero units fight
	 * a single Army unit of this kind when they search a Ruin.  
	 * d_defends_ruin indicates whether this Army unit can defend a Ruin 
	 * or not.
	 *
	 * This value does not change during gameplay.
	 */
	bool d_defends_ruins;

	//! The awardable status of the Army prototype.
	/**
	 * Whether or not this Army prototype can be a reward for a Quest, 
	 * or if Army units of this kind can accompany a new Hero when one 
	 * emerges in a City.
	 *
	 * This value does not change during gameplay.
	 */
	bool d_awardable;

	//! A list of the Ids of Temples the Army unit has visited.
	/**
	 * As the Army unit gets blessed at various Temple objects, this
	 * list grows with unique Temple Ids.
	 * The purpose of the list is to prevent the Army unit from being
	 * blessed at the same Temple more than once.
	 *
	 * The length of the list does not decrease during gameplay.
	 */
        std::list<Uint32> d_visitedTemples;

	//! Whether or not this Army unit is a Hero.
	bool d_hero;

	//! The number of experience points per experience level.
	/**
	 * When an Army unit's d_xp surpasses a multiple of xp_per_level,
	 * it increases it's d_level by 1.
	 */
        static const int xp_per_level = 10;

	//! The basename of the file containing the image for this Army unit.
	/**
	 * This value does not contain a path, and does not contain an
	 * extension (e.g. .png).
	 */
	std::string d_image;
};

#endif // ARMY_H
