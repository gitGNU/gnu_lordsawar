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

#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <list>
#include <vector>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <SDL_types.h>
#include <SDL_video.h>

#include "vector.h"
#include "fight.h"
#include "army.h"
#include "reward.h"

class Stacklist;
class XML_Helper;
class Hero;
class Action;
class City;
class Quest;
class Army;
class Ruin;
class Temple;
class MoveResult;
class FogMap;
class Fight;
class Reward;
class FogMap;
class Signpost;

/** The abstract player class.
  *
  * This class does not yet implement an actual player type. Its purpose is to
  * provide an abstract class design which every player implementation has to
  * fulfill. Of each player class we demand the following functionality:
  *
  * 1. functions for _every_ action a player can do
  * 2. some kind of callback functions if the player has a choice (e.g. if he has
  *    conquered a city, he may choose between razing, pillaging and occupying)
  * 3. signals which are raised whenever something important happens
  * 4. an actionlist which keeps track of _everything_ a player has done
  *
  * The fourth point allows us an easy network playing scheme. After a player
  * has finished, all he has to do is sending his actionlist to the over network
  * players. Since every item which the player can touch/kill etc. (cities,
  * armies, ruins,...) has a unique id, the remote game instances can then
  * simply apply these action lists to their own situation.
  *
  * The third point allows us to dock other classes to every possible event. One
  * example are ruin searching quests which must be informed whenever any
  * player searches a ruin. Or the bigmap can be informed when a stack moves,
  * so that it may update its contents.
  *
  * The second item allows an easy coexistence of AI and human players.
  * Basically, an AI player just follows some kind of routine while a human
  * player uses gui interaction. However, this becomes a bit problematic
  * whenever the player may decide something. One solution is providing
  * callbacks for these cases. The human player then opens a dialog somehow
  * while the AI player overwrites the default behaviour.
  *
  * The first point makes a nice derivation scheme possible. It is possible to
  * divide the player types into a local player, who implements all these
  * functions and a networked player. The latter one simply overwrites these
  * functions so that the game status is updated for each item of the actionlist
  * he has been sent. Furthermore, with a local player having been implemented,
  * it is extremely easy to write an AI player. All you have to do is overwrite
  * the startTurn function with your own code. For every action you already know
  * there is an implementation in a superior class which takes off the burden
  * for the actual work.
  *
  *
  * Finally, the current derivation scheme is as follows
  *
  * Player
  *   |
  *   +--- NetworkPlayer (not implemented yet)
  *   |
  *   +--- RealPlayer (a local player, human players use instances of this class)
  *          +
  *          |
  *          +--- AI_Dummy (AI for the neutral player, does nothing)
  *          |
  *          +--- AI_Fast (dumb AI, tries to conquer the nearest city)
  *          |
  *          +--- AI_Smart (quite smart AI)
  */

class Player: public sigc::trackable
{
    public:
        //! The available player types. Needed when loading a player.
        enum Type {HUMAN = 0, AI_FAST = 1, AI_DUMMY = 2, AI_SMART = 4 };

        /** Default constructor
          *
          * @param name         the name of the player
          * @param armyset      the player's armyset
          * @param color        the player's main color
          */
        Player (std::string name, Uint32 armyset, SDL_Color color, Type type,
		int player_no = -1);

        //! Copy constructor
        Player(const Player&);

        //! Constructor for loading. See XML_Helper for documentation.
        Player(XML_Helper* helper);
        virtual ~Player();

        /** creates the correct player
          * 
          * @note The neutral player must still be inserted as neutral player
          * manually!
          *
          * @param name     the name of the player
          * @param armyset  the armyset of the player
          * @param color    the color of the player
          * @param type     the player's type
          */
        static Player* create(std::string name, Uint32, SDL_Color color, Type type);
        
        /** copies a player to a different type
          * 
          * @note this does not change ownerships e.g. of cities!
          *
          * @param player   the original player
          * @param type     the type we want to get out
          * @return a new player with the old player's data and the given type
          */
        static Player* create(Player* orig, Type type);

	// get standard color for player no
	static SDL_Color get_color_for_no(int player_no);

	// get standard color for neutral player
	static SDL_Color get_color_for_neutral();

        //! Change the player's name
        void setName(std::string name){d_name = name;}
        
        //! Change the player's armyset
        void setArmyset(Uint32 armyset){d_armyset = armyset;}

        //! Set the type of the player (to be used by derived classes only)
        void setType(Type type) {d_type = type;}

        //! Change the gold of the player. Try to avoid this.
        void setGold(int gold){d_gold = gold;}

        //! Change the player's color
        void setColor(SDL_Color c);

        //! Makes a player immortal, even if he has no units or cities.
        void setMortality(bool ismortal) {d_immortal = !ismortal;}
        

        //! Add gold to the player's treasure
        void addGold(int gold);

        //! Subtract gold from the player's treasure
        void withdrawGold(int gold);

        //! Remove all items from the list of player's actions
        void clearActionlist();

        //! Add a stack to the player's list of stacks
        void addStack(Stack* stack);

        //! Remove a stack from the player's stacklist
        bool deleteStack(Stack* stack);


        //! Returns the unique ID of the player
        Uint32 getId() const {return d_id;}

        //! Returns the list of player's actions. 
        std::list<Action*>* getActionlist() {return &d_actions;}

        //! Return the player's armyset
        Uint32 getArmyset() const {return d_armyset;}

        //! Return whether the player has been killed
        bool isDead() const {return d_dead;}

        //! Returns whether a player is immortal (not killable) or not
        bool isImmortal() const {return d_immortal;}

        //! Return the type of the player
        Uint32 getType() {return d_type;}

        //! Returns the main color of the player
        SDL_Color getColor() const {return d_color;}

        //! Returns the player's color suitable for applying it to masks.
        SDL_Color getMaskColor() const;

        //! Returns the gold of the player
        int getGold() const {return d_gold;}

        //! Returns the name of the player
        std::string getName(bool translate = true) const;

        //! Returns the list of stacks owned by the player
        Stacklist* getStacklist() const {return d_stacklist;}

        //! A shortcut for getting the stack marked as active
        Stack* getActivestack();

        //! get the fog map of the player
        FogMap* getFogMap() const {return d_fogmap;}

        //! get the fight order of the player
	std::list<Uint32> getFightOrder() const {return d_fight_order;}

        //! Dumps the items in the actionlist on stderr
        void dumpActionlist() const;

        //! Mark the player as dead; this kills all his armies etc.
        void kill();

        //! revive the player, this does not give him any cities!!!
        void revive() {d_dead = false;}

        /** Saves the player data
          *
          * @note This function only saves basic data, it doesn't open/close the
          * player tags, this has to be done by derived functions!
          */
        virtual bool save(XML_Helper* helper) const;

        /** Loads a player
          *
          * This is a bit inconsistent with other classes, but with players you
          * have the problem that there are different types with different
          * classes. So we need a static member function which looks which
          * player type to load and calls the constructor of the appropriate
          * class.
          *
          * @param helper       the XML_Helper instance of the savegame
          */
        static Player* loadPlayer(XML_Helper* helper);


        /** This function is called when a player's turn starts. For AI players,
          * this function should start the algorithm.
          *
          * @return true if wverything went well
          */
        virtual bool startTurn() = 0;

        /** Called so that the player can decide what to do with an
            occupied city as soon as he has occupied it (mainly for AI)

            @return always returns true (unused feature)
            @param  city   the conquered city
          */
        virtual bool invadeCity(City* c) = 0;

        /** Called whenever a hero decides to join you, so the player can
            decide what to do with the hero.

            @return true if the player accepts the hero, false otherwise
            @param  hero    the hero who has offered his service
            @param  cost    the amount of gold neccessary to recruit the
                            hero

            @note Don't touch the hero except for naming purposes etc.
                  The hero is integrated elsewhere!
          */
        virtual bool recruitHero(Hero* hero, City *city, int cost) = 0;

        /** Callback whenever a unit advances a level.
          * 
          * @return true on success, false on error
          * @param army     army to raise
          */
        virtual bool levelArmy(Army* a) = 0;
        
        /** Called to split the stack s in two stacks. You have to take great
          * care that one of the created stacks moves away after that. Some code
          * parts assume that each tile is occupied by just one stack!
          * You can change which armies are in the new stack by setting their
          * "grouped" value.
          *
          * @param s        the stack to split
          * @return false if there were any problems, else true
          */
        virtual bool stackSplit(Stack* s) =0;

        /** Called to merge two stacks into one.
          *
          * @param receiver         the receiving stack
          * @param joining          the joining stack, destroyed after the join
          * @param grouped          whether the armies of the other stacks are selected
          *                         automatically or not.
          * @return false if an error occured, else true
          */
        virtual bool stackJoin(Stack* receiver, Stack* joining, bool grouped)=0;

        //! A shortcut for stackMove(s, <last path item>, true)
        virtual bool stackMove(Stack* s) =0;

        /** Move a stack to a specified position
          *
          * Moves a stack. The path is calculated on the fly unless you set
          * follow to true. In this case, an existing path is checked and
          * iterated over. This is useful if a stack didn't reach its target
          * within one round and should continue the movement.
          *
          * @param s                the stack to be moved
          * @param dest             the destination of the move
          * @param follow           if set to false, calculate the path
          * @return false on error, true otherwise
          */
        virtual MoveResult *stackMove(Stack* s, Vector<int> dest, bool follow)=0;

        /** Two stacks fight each other.
          * 
          * Note that all stacks next to the defending stack also take part in
          * the fight, if they belong either to the attacker's side or to the
          * defender. If the attacker or the defender die in the course of events,
          * the pointers are set to 0.
          *
          * @param attacker         the attacking stack
          * @param defender         the defending stack
          * @param ruin             true if the fight is caused by searching a ruin
          * @return one of Fight::ATTACKER_WON, Fight::DEFENDER_WON, Fight::NONE
          */
        virtual Fight::Result stackFight(Stack** attacker, Stack** defender, 
                                         bool ruin) =0;
        virtual Fight::Result stackRuinFight(Stack** attacker, Stack** defender) =0;

        /** A stack searches a ruin. The stack should contain a hero.
          *
          * @param s                the stack which searches the ruin
          * @param r                the ruin to be searched
          * @param reward           a pointer to the received reward
          * @return true if the ruin was searched (keeper defeated)
          */
        virtual Reward* stackSearchRuin(Stack* s, Ruin* r) =0;

        /** A stack visits a temple and becomes blessed. By blessing, the attack
          * strength of all armies rises by 1.
          *
          * @param s                the stack visiting the temple
          * @param t                the visited temple
          * @return the number of blessed armies
          */
        virtual int stackVisitTemple(Stack* s, Temple* t) = 0;

        /** A stack visits a temple and receives a random quest from the
         * temple's priests. If more than one heroes are in the stack, the
         * quest is assigned to the first hero without quest.
         *
         * @param s                 the visiting stack
         * @param t                 the visited temple
         * @return teh quest we got or 0 on error
         */
        virtual Quest* stackGetQuest(Stack* s, Temple* t) = 0;
        
        /** Occupy a city (i.e. change the owner to yourself)
          *
          * @param c                the occupied city
          * @return false on error, true otherwise
          */
        virtual bool cityOccupy(City* c) =0;

        /** Pillage a city (trade in the best army type and get some gold)
          *
          * @param c                the city to be pillaged
          * @param gold             returns the amount of gold pillaged
	  * @param pillaged_army_type the army type that is cashed in for gold
          * @return false on error, true otherwise
          */
        virtual bool cityPillage(City* c, int& gold, int& pillaged_army_type)=0;

        /** Sack a city (trade in all army types except one and get some gold)
          *
          * @param c                the city to be sacked 
          * @param gold             returns the amount of gold sacked
	  * @param sacked_types     the army types that were cashed in for gold
          * @return false on error, true otherwise
          */
        virtual bool citySack(City* c, int& gold, std::list<Uint32> *sacked_types) = 0;

        /** Raze (permanently destroy) a city
          *
          * @param c                the city to be razed
          * @return false on error, true otherwise
          */
        virtual bool cityRaze(City* c) =0;

        /** Add another production to a city
          *
          * @param c                the lucky city
          * @param slot             the production slot of the city
          * @param armytype         the index of the army type to add
          */
        virtual bool cityBuyProduction(City* c, int slot, int type) =0;

        /** Change the production of a city
          *
          * @param c                the affected city
          * @param slot             the index of the selected production slot
          * @return false on error, true otherwise
          */
        virtual bool cityChangeProduction(City* c, int slot) =0;

        /** Gives the player some random reward
          *
          * @param level            the quality of the reward
          * @param s                the stack which has caused the reward
          * @param reward           a pointer for storing the type of the reward
          * @return false on error, true otherwise
          */
        virtual bool giveReward (Stack *stack, Reward *reward) = 0;

        //! Disband a player's stack
        virtual bool stackDisband(Stack* s) =0;

        //! Change the text on the signpost of the square we're sitting on
        virtual bool signpostChange(Signpost * s, std::string message) =0;

        /** Signal raised when a city is conquered. This signal is solely
          * for internal use. Don't use it, you may break stability (Background:
          * libsigc++ doesn't guarantee order of execution and this signal may
          * usually end the game which can have unwanted side effects).
          */
        sigc::signal<void, City*> sinvadingCity;

        //! Signal raised whenever a player has conquered a city. This is the
        //! signal you should use for further actions.
        sigc::signal<void, City*, Stack*> soccupyingCity;

        
        //! Signal raised when a hero is recruited
        sigc::signal<bool, Hero*, City *, int>         srecruitingHero;
        //! Signal raised when an army advances a level; may return stat to raise
        sigc::signal<Army::Stat, Army*>        snewLevelArmy;
        //! Signal raised when an army gets a new medal
        sigc::signal<void, Army*>              snewMedalArmy;
        //! Signal raised when an army dies
        sigc::signal<void, Army*, std::vector<Uint32> >    sdyingArmy;
        //! Signal raised when a stack dies
        sigc::signal<void, Stack*>             sdyingStack;
        
        //! Signal raised whenever the player successfully searched a ruin
        sigc::signal<void, Ruin*, Stack*, Reward*>      ssearchingRuin;
        //! Signal raised whenever the player visits a temple
        sigc::signal<void, Temple*, Stack*>    svisitingTemple;
        //! Signal raised whenever the player moves a stack
        sigc::signal<void, Stack*>             smovingStack;
        //! Signal raised whenever the player pillages a city 
        sigc::signal<void, City*, Stack*, int, std::list<Uint32> >      spillagingCity;
        //! Signal raised whenever the player sacks a city 
        sigc::signal<void, City*, Stack*, int, std::list<Uint32> >      ssackingCity;
        //! Signal raised whenever the player razes a city 
        sigc::signal<void, City*, Stack*>      srazingCity;

        //! Signal raised whenever a player's status (e.g. gold) changes
        sigc::signal<void> schangingStatus;
        //! Signal raised whenever the stack's staus has changed
        sigc::signal<void, Stack*> supdatingStack;
        //! Signal raised whenever the status of a city has changed
        sigc::signal<void, City*> supdatingCity;

	// emitted when a fight is started, parameters are in the fight object,
	// so should the results be
        sigc::signal<void, Fight &> fight_started;
	
	// emitted when a fight in a ruin is started
        sigc::signal<void, Stack *, Stack *> ruinfight_started;
        sigc::signal<void, Fight::Result> ruinfight_finished;

    protected:
        //! Move stack s one step forward on his stored path
        virtual bool stackMoveOneStep(Stack* s) = 0;

        // DATA
        SDL_Color d_color;
        std::string d_name;
        Uint32 d_armyset;
        int d_gold;
        bool d_dead;            // is player already dead?
        bool d_immortal;
        Uint32 d_type;
        Uint32 d_id;
        std::list<Action*> d_actions; //list of actions done by the player
        Stacklist* d_stacklist;
        FogMap* d_fogmap;
	std::list<Uint32> d_fight_order; //for each army in armyset, a number

    private:
        //! Loads the subdata of a player (actions and stacklist)
        bool load(std::string tag, XML_Helper* helper);
};

extern sigc::signal<void, Player::Type>  sendingTurn;

#endif // PLAYER_H

// End of file
