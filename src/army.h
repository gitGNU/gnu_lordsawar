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

#ifndef ARMY_H
#define ARMY_H

#include <SDL.h>
#include <string>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include "defs.h"

class Player;
class XML_Helper;

 /*
  * Description of a single army type
  * 
  * This class is the atom of every army. It contains all data related to
  * a single army type of an armyset, such as strength, defense, movement points
  * and so on.
  */

class Army : public sigc::trackable
{
    public:

        //! used only for heroes (e.g. to decide about the recruitment image)
        enum Gender {NONE = 0, MALE = 1, FEMALE = 2};

        enum Bonus {
	      LEADER      = 0x00000004, //+1 str to stack, may search
	      CAVALRY     = 0x00000008, //+1 str in open
	      ANTICAVALRY = 0x00000010, //*2 str vs cavalry
	      CRITICAL    = 0x00000040, //can instakill

	      ADD1STRINOPEN      = 0x00000080,
	      ADD2STRINOPEN      = 0x00000100,
	      ADD1STRINFOREST    = 0x00000200,
	      ADD1STRINHILLS     = 0x00000400,
	      ADD1STRINCITY      = 0x00000800,
	      ADD2STRINCITY      = 0x00001000,
	      ADD1STACKINHILLS   = 0x00002000,
	      SUBALLCITYBONUS    = 0x00004000,
	      SUB1ENEMYSTACK     = 0x00008000,
	      ADD1STACK          = 0x00010000,
	      ADD1SPECIAL        = 0x00020000,
	      ADD2SPECIAL        = 0x00040000,
	      SUBALLNONHEROBONUS = 0x00080000,
	      SUBALLHEROBONUS    = 0x00100000,

        };
        
        enum Stat {
            STRENGTH=0,
            HP=3,
            MOVES=4,
            MOVE_BONUS=5,
            ARMY_BONUS=6,
            SIGHT=7,
	    SHIP=8
        };

        /** Multiple-purpose constructor
          * 
          * This constructor is used for:
          *
          * - creating a new army out of an army prototype (army prototypes have
          *   an id of 0; the constructor behaves a bit differently then)
          * - copying armies within the game (sometimes neccessary)
          */
        Army(const Army& a, Player* p = 0);

        /** Loading constructor. See XML_Helper for more details.
          *
          * The constructor has to care for two cases. Sometimes, an army
          * prototype is loaded, from which other units are cloned, sometimes
          * actual armies have to be loaded (in Armysetlist)
          *
          * @param helper       the XML_Helper instance of the savegame
          * @param prototype    if set to false, load some additional values
          *                     instead of setting defaults (e.g. id, level)
          */
        Army(XML_Helper* helper, bool prototype = false);
        
        virtual ~Army();


        // Set functions:
        
        //! Set the armyset of the army
        void setArmyset(Uint32 armyset, Uint32 type);

	//! Get the armyset of the army
        Uint32 getArmyset() const {return d_armyset;}
        
        //! Set the name of the unit type
        void setName(std::string name){d_name = name;}

        //! Set the basic image of the unit type
        void setPixmap(SDL_Surface* pixmap){d_pixmap = pixmap;}

        //! Set the mask of the unit type (for player colors)
        void setMask(SDL_Surface* mask){d_mask = mask;}

        //! Set how much XP this unit is worth when killed
        void setXpReward(double xp_value){d_xp_value = xp_value;}

        //! All-purpose function for setting the stats of the army
        void setStat(Stat stat, Uint32 value);

        //! sets the descriptive text for this army
        void setDescription(std::string text) {d_description = text;};
        
        //! Set how much gold this unit needs per turn
        void setUpkeep(Uint32 upkeep){d_upkeep = upkeep;}
        
        //! Set the player owning this army
        void setPlayer(Player* p) {d_player = p;}

        //! Set the number of hitpoints of this army
        void setHP(Uint32 hp) {d_hp = hp;}

        //! Set how many turns this unit type needs to be produced
        void setProduction(Uint32 production){d_production = production;}

        //! Set the amount of gold needed to buy a city this production
        void setProductionCost(Uint32 production_cost){d_production_cost = production_cost;}

        //! Set the sex of the army type
        void setGender(Gender gender){d_gender = gender;}

        /** Set the grouping information of this army.
          * 
          * Background: When you select a stack on the map, you can toggle the
          * armies which will be moved when you click somewhere else on the map
          * then. Grouped==true means that this army will be among the selected
          * armies.
          */
        void setGrouped(bool grouped){d_grouped = grouped;}

        
        // Get functions
        
        //! Returns the name of the unit type
        std::string getName() const {return __(d_name);}

        //! Get the unique id of this army
        Uint32 getId() const {return d_id;}

        //! Get the type of this army
        Uint32 getType() const {return d_type;}

        //! Get the player owning this army
        Player* getPlayer() const {return d_player;}

        //! Get the image of the army. Internally, this refers to GraphicsCache.
        SDL_Surface* getPixmap() const;

        //! Returns the mask (read-only!!) for player colors
        SDL_Surface* getMask() const {return d_mask;}

        //! Returns the descriptive text of this army
        std::string getDescription() const {return d_description;}

        //! Returns how much gold this unit type needs per turn
        Uint32 getUpkeep() const {return d_upkeep;}
        
        //! returns how many turns this unit type needs to be produced
        Uint32 getProduction() const {return d_production;}

        //! Returns how much gold setting up the production costs
	//! If this number is over zero, then the army can be
	//! purchased.
        Uint32 getProductionCost() const {return d_production_cost;}

        //! Returns how much XP killing a unit of this type is worth
        double getXpReward() const {return d_xp_value;}

        //! Return the gender of the armytype
        Uint32 getGender() const {return d_gender;}


        /** Returns a teh value of a certain stat of the unit.
          * 
          * If modified is set to false, you get the raw, inherent value of
          * the army. Set it to true to get the modified one. This is not
          * important for generic armies, but heroes can have their stats
          * modified by wearing items.
          */
        virtual Uint32 getStat(Stat stat, bool modified=true) const;

        //! Get the current number of hitpoints (in opposite to getMaxHp)
        Uint32 getHP() const {return d_hp;}

        //! Get the current number of moves (in opposite to getMaxMoves)
        Uint32 getMoves() const {return d_moves;}

        //! Get the move bonus
        Uint32 getMoveBonus() const {return d_move_bonus;}

        //! Get the number of experience points of the unit
        double getXP() const {return d_xp;}

        //! Get the level of the army
        Uint32 getLevel() const {return d_level;}

        //! Returns grouping information
        bool isGrouped() const {return d_grouped;}

        //! Returns whether the army is a hero
        virtual bool isHero() const {return false;}

        /** Heal the unit
          * 
          * @param hp       hitpoints to heal. Set to zero for "natural" healing.
          */
        void heal(Uint32 hp = 0);

        /** Damage this unit
          * 
          * @param damageDone       how much damage the unit suffers
          * @return true if the unit has died, else otherwise
          */
        bool damage(Uint32 damageDone);

        /** Reduce the number of moves
          * 
          * @param moves            how many movement points have been spent
          */
        void decrementMoves(Uint32 moves);

        //! Resets the number of movement points to maximum
        void resetMoves();

        //! Blesses the unit (currently: strength+1)
	//! Returns whether or not the army was blessed.
        bool bless();

        //! Increases the XP of the unit
        void gainXp(double n);

        //! Checks whether unit can advance a level
        bool canGainLevel() const;

        /** Increases the unit's level
          * 
          * @return how much the stat increases or -1 on error (e.g. because
          *         the XP is not enough)
          * @param stat     the stat to increase, only
          */
        int gainLevel(Stat stat);

	// return how much the stat would be boosted by gaining a level
        int computeLevelGain(Stat stat);

        //! get all medal bonuses (if medals or not) at once
        bool* getMedalBonuses() const {return (bool*)&d_medal_bonus;}

        //! get a specific medal bonus
        bool getMedalBonus(Uint32 index) const {return d_medal_bonus[index];}

        //! Sets a specific medal bonus
        void setMedalBonus(Uint32 index, bool value) {d_medal_bonus[index]=value;}

        //! Returns the number of battles the unit took part in
        Uint32 getBattlesNumber() const {return d_battles_number;}

        //! Changes the number of battles the unit prticipated in
        void setBattlesNumber(Uint32 value) {d_battles_number=value;}
         
        //! Returns the number of blows the units has scored
        double getNumberHasHit() const {return d_number_hashit;}

        //! Sets the number of hits this unit has scored
        void setNumberHasHit(double value) {d_number_hashit=value;}

        //! Returns the number of hits this unit has suffered
        double getNumberHasBeenHit() const {return d_number_hasbeenhit;}

        //! Sets the number of hits this unit has suffered
        void setNumberHasBeenHit(double value) {d_number_hasbeenhit=value;}

        void printAllDebugInfo() const;

        //! Saves the unit information (see XML_Helper for further info)
        virtual bool save(XML_Helper* helper) const;
        
        //! This signal is raised when the army dies; it is static because
        //! sometimes the army doesn't exist yet when the signal is connected
        static sigc::signal<void, Army*> sdying;

	//! Sets whether or not this army type can found in a ruin.
	void setDefendsRuins(bool defends) {d_defends_ruins = defends; }
	//! Gets whether or not this army type can found in a ruin.
	bool getDefendsRuins() const {return d_defends_ruins; }

	//! Sets whether or not this army can be a reward for completing a 
	//! quest, or for having a new hero arrive.
	void setAwardable (bool awardable) {d_awardable = awardable; }
	//! Gets whether or not this army can be a reward for completing a 
	//! quest, or for having a new hero arrive.
	bool getAwardable() const {return d_awardable; }

	//! Sets this army as being in a ship or not
	void setInShip (bool s);

    protected:
        //! Generic function for saving the army data. Useful for the hero class,
        //  which doesn't need to repeat the save code.
        bool saveData(XML_Helper* helper) const;

        //! Copies the generic data from the original prototype army (used for loading)
        void copyVals(const Army* a);
        
        Uint32 d_type;
        Uint32 d_armyset;
        SDL_Surface* d_pixmap;
        SDL_Surface* d_mask;
        
        std::string d_name;
        std::string d_description;
        Uint32 d_production;
        Uint32 d_production_cost;
        Uint32 d_upkeep;
        Uint32 d_strength;
        Uint32 d_max_hp;
        Uint32 d_max_moves;
        Uint32 d_sight;
        double d_xp_value;
        Uint32 d_move_bonus;
        Uint32 d_army_bonus;
	bool d_ship;

        Uint32 d_gender;

        Player* d_player;
        Uint32 d_id;
        Uint32 d_hp;
        Uint32 d_moves;
        double d_xp;            // experience points
        Uint32 d_level;
        bool d_grouped;

        //! Medal bonuses : we have 3 medals
        //!  0 _ medal for being extremely merciless (gives an extra +1 to strength)
        //!      a unit gets this medal if in a combat it scores more than 90% of hits
        //!  1 _ medal for being very good in defense (gives an extra +1 to defense)
        //!      a unit gets this medal if in a combat is never hit
        //!  2 _ medal for being very good in combat  (gives an extra +4 to Hit Points)
        //!      a unit gets this medal if it survives a lot of battles (for now 10 battles)
        bool d_medal_bonus[3];

        //! number of fought battles 
        Uint32 d_battles_number;
        //! wighted number of hits per battle
        double d_number_hashit;
        //! weighted number of times the army has been hit in a battle
        double d_number_hasbeenhit;
	//! this army type can defend a ruin or not
	bool d_defends_ruins;
	//! this army type can be a reward from quest
	bool d_awardable;
        std::list<Uint32> d_visitedTemples;
};

#endif // ARMY_H
