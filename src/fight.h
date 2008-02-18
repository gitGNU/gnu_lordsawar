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

#ifndef FIGHT_H
#define FIGHT_H

#include <SDL_types.h>
#include <list>
#include <vector>

class Stack;
class Fighter;
class Hero;
class Army;

/** This is the structure that describes the course of the fight. It is later
  * read by a fight dialog to reconstruct what happened.
  */
struct FightItem
{
    int turn;       //!< fight round of this attack
    Uint32 id;      //!< id of the attacked army
    int damage;     //!< damage done to the army
};


/** This class is solely responsible for the _calculation_ of the fight.
  * It gets the participating stacks and damages the units within according
  * to the calculation. Furthermore, it creates a history of the fight, which
  * can later be used by the fight dialog to reconstruct the fight or be sent
  * over the network. For the graphical display, see the FightDialog class.
  *
  * Two things should be noted. First, the fight can include more than the
  * initial two stacks, since all stacks around the defender are considered
  * as potential "contributors". Second, irrespective of that, a fight is
  * always considered as "won", if the defending stack was destroyed and
  * "lost" if the attacking stack was crushed.
  */

class Fight
{
    public:
        //! The three possibilities how a fight can end
        enum Result {DRAW = 0, ATTACKER_WON = 1, DEFENDER_WON = 2};
        enum FightType {FOR_KICKS = 0, FOR_KEEPS = 1};

        /** Initializes a fight between two stacks
          * 
          * @param attacker         the attacking stack
          * @param defender         the defending stack
	  * @param type             optionally heal all stacks afterwards
          */
        Fight(Stack* attacker, Stack* defender, FightType type = FOR_KEEPS);
        ~Fight();

        
        //! Does the actual fight
        void battle(bool intense);


        //! Returns the result of the fight
        Result getResult() const {return d_result;}

        //! Returns the list of things that happened in chronological order
        std::list<FightItem> getCourseOfEvents() const {return d_actions;};
        
        //! Returns the participating attacker stacks
        std::list<Stack*> getAttackers() const {return d_attackers;}

        //! Returns the participating defender stacks
        std::list<Stack*> getDefenders() const {return d_defenders;}
        
	//! Get modified strength bonus
	Uint32 getModifiedStrengthBonus(Army *a);

        // CONSTANTS
        //! number of rounds the fight lasts
	//! if this is 0, then there is no maximum.
        static const int MAX_ROUNDS = 0;

        //! number of movement points needed to do a fight (TODO: needs to be used)
	//FIXME: use this constant
        static const int MOVES_FOR_FIGHT = 3;

	//! turn a list of stacks into an ordered list of armies
	//! this is used for calculation and display purposes
        static void orderArmies(std::list<Stack*> stacks, std::vector<Army*> &armies);

    private:
        /** Does one fight round.
          *
          * @return false if the maximum number of fight rounds has been
          * exceeded or one side has lost.
          */
        bool doRound();

        //! Calculates the attack/defense bonus of the armies
        void calculateBonus();
        void calculateBaseStrength(std::list<Fighter*> fighters);
        void calculateTerrainModifiers(std::list<Fighter*> fighters);
        void calculateModifiedStrengths (std::list<Fighter*>friendly, 
                                         std::list<Fighter*>enemy, 
                                         bool friendlyIsDefending,
                                         Hero *strongestHero);
        void calculateFinalStrengths (std::list<Fighter*> friendly, 
				      std::list<Fighter*> enemy);

        /** This function just has two armies fight against each other. It
          * applies the boni and several special boni to attacker and defender
          * and calculates (rather rolls) the result.
          *
          * @param attacker     the attacking army
          * @param defender     the defending army
          */
        void fightArmies(Fighter* attacker, Fighter* defender);

        //! removes a fighter from the fighting lists (d_att_close etc.)
        void remove(Fighter* f);
        // DATA
        std::list<Stack*> d_attackers;
        std::list<Stack*> d_defenders;
        
        std::list<Fighter*> d_att_close;
        std::list<Fighter*> d_def_close;
        
        std::list<FightItem> d_actions;
        
        int d_turn;
        Result d_result;
	FightType d_type;
	bool d_intense_combat;
};

#endif // FIGHT_H

