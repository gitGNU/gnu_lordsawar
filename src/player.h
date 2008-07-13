// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003 Marek Publicewicz
// Copyright (C) 2004 John Farrell
// Copyright (C) 2005 Bryan Duff
// Copyright (C) 2006, 2007, 2008 Ben Asselstine
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
class NetworkAction;
class History;
class NetworkHistory;
class City;
class Quest;
class Army;
class Ruin;
class Temple;
class MoveResult;
class FogMap;
class Fight;
class Reward;
class Signpost;
class VectoredUnit;

//! The abstract player class.
/** 
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
 * Finally, the current derivation scheme is as follows:
 * RealPlayer derives from Player.
 * NetworkPlayer derives from Player.
 * AI_Fast, AI_Dummy, and AI_Smart derive from RealPlayer.
 *
 * The Ids of players are very specific.  The first player (the White player)
 * must have the Id of 0, the next player (Green) must have the Id of 1, and 
 * so on.  The Neutral player must have the final player Id.
 * If the White player is not in the scenario, then the Id 0 must be skipped.
 * There can only be MAX_PLAYERS players in total.
 */

class Player: public sigc::trackable
{
    public:
        //! The available player types.
        enum Type {
	  //! Local human player.  See the RealPlayer class.
	  HUMAN = 0, 
	  //! Local computer player (Easy).  See the AI_Fast class.
	  AI_FAST = 1, 
	  //! Local computer player (Neutral).  See the AI_Dummy class.
	  AI_DUMMY = 2, 
	  //! Local computer player (Hard).  See the AI_Smart class.
	  AI_SMART = 4,
	  //! Remote player.  See the NetworkPlayer class.
	  NETWORKED = 8
	};

	//! Every player has a diplomatic state with every other player.
	enum DiplomaticState {
	  //! Can't attack opponent's stacks anywhere.
	  AT_PEACE = 1, 
	  //! Can't attack opponent's stacks in cities.
	  AT_WAR_IN_FIELD = 2, 
	  //! Can attack opponent's stack everywhere.
	  AT_WAR = 3
	};

	//! Every player has a diplomatic proposal to every other player.
	enum DiplomaticProposal {
	  //! Offer to keep the status-quo with the opponent.
	  NO_PROPOSAL = 0, 
	  //! Offer peace to an opponent.
	  PROPOSE_PEACE = 1, 
	  //! Offer limited war to an opponent (only kill stacks in the field).
	  PROPOSE_WAR_IN_FIELD = 2, 
	  //! Offer all-out war to an opponent.
	  PROPOSE_WAR = 3 
	};

	//! Every player keeps a tally of frags.
        enum TriumphType {
	  //! Kills we've made of an opponent's Hero army units.
	  TALLY_HERO = 0, 
	  //! Kills we've made of an opponent's awardable Army units.
	  TALLY_SPECIAL = 1, 
	  //! Kills we've made of an opponents other Army units.
	  TALLY_NORMAL = 2, 
	  //! Kills we've made of an opponent's Army units on the water.
	  TALLY_SHIP = 3, 
	  //! Kills we've made of opponent's Heroes who carry a standard Item.
	  TALLY_FLAG = 4
	};

        /** 
	 * Make a new player.  
	 * @note AI_Fast, AI_Dummy, AI_Smart and RealPlayer use this 
	 * constructor to make new Players.
         *
         * @param name         The name of the player.
         * @param armyset      The Id of the player's Armyset.
         * @param color        The player's colour.
	 * @param width        The width of the player's FogMap.
	 * @param height       The height of the player's FogMap.
	 * @param type         The kind of player (Player::Type).
	 * @param player_no    The Id of the player.  If this value is -1,
	 *                     the next free Id it used.
         */
	//! Default constructor.
        Player (std::string name, Uint32 armyset, SDL_Color color, int width,
		int height, Type type, int player_no = -1);

        //! Copy constructor
        Player(const Player&);

        //! Constructor for loading. See XML_Helper for documentation.
        Player(XML_Helper* helper);
        virtual ~Player();

        /** 
	 * Make a new player with the given parameters.
         * 
         * @note The neutral player must still be inserted as neutral player
         * manually!
         *
         * @param name     The name of the player.
         * @param armyset  The Id of the player's Armyset.
         * @param color    The player's colour.
         * @param width    The width of the player's FogMap.
         * @param height   The height of the player's FogMap.
         * @param type     The player's type (Player::Type).
         */
	//! Create a player.
        static Player* create(std::string name, Uint32 armyset, 
			      SDL_Color color, int width, int height, 
			      Type type);
        
        /** 
	 * Copies a player to a different type.
         * 
         * @note This method does not change ownerships! (e.g. of cities)
         *
         * @param player   The original player.
         * @param type     The type we want to get out (Player::Type).
         * @return A new player with the old player's data and the given type.
         */
	//! Create a new player from another player.
        static Player* create(Player* orig, Type type);

        //! Change the player's name.
        void setName(std::string name){d_name = name;}
        
        //! Change the player's Armyset.
        void setArmyset(Uint32 armyset){d_armyset = armyset;}

        //! Set the type of the player (to be used by derived classes only.)
        void setType(Type type) {d_type = type;}

        //! Change the player's colour.
        void setColor(SDL_Color c);

        //! Makes a player unable to die, even when having no units or cities.
        void setMortality(bool ismortal) {d_immortal = !ismortal;}
        
        //! Change the number of gold pieces of the player has.
        void setGold(int gold){d_gold = gold;}

        //! Add some gold pieces to the player's treasury.
        void addGold(int gold);

        //! Subtract gold pieces from the player's treasury.
        void withdrawGold(int gold);

        //! Remove every Action from the list of the player's actions.
        void clearActionlist();

        //! Remove every History element from the list of the player's events.
        void clearHistorylist();

        //! Add a Stack to the player's Stacklist.
        void addStack(Stack* stack);

        //! Remove a Stack from the player's Stacklist.
        bool deleteStack(Stack* stack);

        //! Returns the unique ID of the player.
        Uint32 getId() const {return d_id;}

	//! Returns a list of the player's actions to show in a report.
	std::list<Action *> getReportableActions();

        //! Returns the list of player's events. 
        std::list<History*>* getHistorylist() {return &d_history;}

        //! Return the Id of the player's Armyset.
        Uint32 getArmyset() const {return d_armyset;}

        //! Return whether or not the player has been killed.
        bool isDead() const {return d_dead;}

        //! Returns whether a player is immortal or not.
        bool isImmortal() const {return d_immortal;}

        //! Return the type of the player (Player::Type).
        Uint32 getType() {return d_type;}

        /**
	 * Return the amount of upkeep in gold pieces that the player spent 
	 * in the previous turn.  
	 */
	//! Return the upkeep.
        Uint32 getUpkeep() {return d_upkeep;}

	//! Return the income from all of the player's cities.
        Uint32 getIncome () {return d_income;}

	/**
	 * Perform a summation of the upkeep value for every Army in the 
	 * player's Stacklist.  This method sets d_upkeep.
	 * The upkeep value is in gold pieces.
	 */
	//! Calculates the upkeep.
	void calculateUpkeep();

	/**
	 * Perform a summation of the income value for every City in the 
	 * player's Citylist.  This method sets d_income.
	 * The income value is in gold pieces.
	 */
	//! Calculates the upkeep.
	void calculateIncome();

	//! Declare a new diplomatic state with respect to an opponent.
	void declareDiplomacy(DiplomaticState state, Player *player);

	//! Query the diplomatic state this player has with an opponent.
	DiplomaticState getDiplomaticState (Player *player);

	//! Query the diplomatic proposal we're making to an opponent.
	DiplomaticProposal getDiplomaticProposal (Player *player);

	//! Propose a new diplomatic state wrt another player
	void proposeDiplomacy (DiplomaticProposal proposal, Player *player);

	//! Set this player's rank in diplomatic matters.  Starts at 1.
	void setDiplomaticRank (Uint32 rank) {d_diplomatic_rank = rank;};

	//! What diplomatic rank does this player have?  Starts at 1.
	Uint32 getDiplomaticRank () {return d_diplomatic_rank;};

	//! What rank do we have?  As a name.
	std::string getDiplomaticTitle() {return d_diplomatic_title;};

	//! Set the rank as a name.
	void setDiplomaticTitle (std::string title) {d_diplomatic_title = title;};
	//! Negotiate diplomatic talks with an opponent, and return a new state.
	DiplomaticState negotiateDiplomacy (Player *player);

	//! Get the diplomatic score with respect to an opponent.
	Uint32 getDiplomaticScore (Player *p);

	/**
	 * Change the player's opinion of an opponent for the better.
	 *
	 * @param player    The player to improve our opinion by.
	 * @param amount    The amount to improve by.  The minimum value 
	 *                  is 1 and the maximum value is 15.
	 *
	 */
	//! Make your diplomatic view of another player increase.
	void improveDiplomaticRelationship (Player *p, Uint32 amount);

	/**
	 * Change all players opinion of you for the better, except for 
	 * possibly a single player.
	 *
	 * @param amount    The amount to improve.  The minimum value is 1
	 *                  and the maximum value is 15.
	 * @param except    Don't improve this player's view of the player.
	 *
	 * @note Pass except as NULL to not except a player.
	 */
	//! Make all other players diplomatic view of you increase.
	void improveDiplomaticRelationship (Uint32 amount, Player *except);

	/**
	 * Change the player's view of an opponent for the worse.
	 *
	 * @param player    The player to deteriorate our view of.
	 * @param amount    The amount to deteriorate by.  The minimum value 
	 *                  is 1 and the maximum value is 15.
	 *
	 */
	//! Make your diplomatic view of another player decrease.
	void deteriorateDiplomaticRelationship (Player *player, Uint32 amount);

	/**
	 * Change all players opinion of you for the worse.
	 *
	 * @param amount    The amount to deterioriate by.  The minimum value 
	 *                  is 1 and the maximum value is 15.
	 */
	//! Make all other players diplomatic view of you worsen
	void deteriorateDiplomaticRelationship (Uint32 amount);

	/**
	 * Change all players opinion of another player for the worse, 
	 * who happen to have a diplomatic state of state with you.
	 *
	 * @param player    The target player.
	 * @param amount    The amount to deterioriate by.  The minimum value 
	 *                  is 1 and the maximum value is 15.
	 * @param state     The state that an opponent has to be in with you,
	 *                  to make the deterioration happen.
	 */
	//! Make players you are at state with you think less of player.
	void deteriorateAlliesRelationship(Player *player, Uint32 amount, 
					   Player::DiplomaticState state);

	/**
	 * Change all players opinion of another player for the better, 
	 * who happen to have a diplomatic state of state with you.
	 *
	 * @param player    The target player.
	 * @param amount    The amount to improve by.  The minimum value 
	 *                  is 1 and the maximum value is 15.
	 * @param state     The state that an opponent has to be in with you,
	 *                  to make the improvement happen.
	 */
	//! Make players who are at STATE with PLAYER think better of you.
	void improveAlliesRelationship(Player *player, Uint32 amount, 
				       Player::DiplomaticState state);

        //! Returns the colour of the player.
        SDL_Color getColor() const {return d_color;}

        //! Returns the player's colour suitable for applying it to masks.
        SDL_Color getMaskColor() const;

        //! Returns the amount of gold pieces the player has.
        int getGold() const {return d_gold;}

        //! Returns the name of the player has.
        std::string getName(bool translate = true) const;

	//! Returns the player's current score.
        Uint32 getScore();

	/**
	 * The player's triumphs are tallied as opponent's armies die.
	 * This method gets a tally for certain kind of triumph.  
	 * See TriumphsDialog for a caller of this method.
	 *
	 * @param player      The player to obtain a tally for.
	 * @param type        The kind of kills to tally (Player::TriumphType).
	 *
	 * @return Zero or more number of armies killed.
	 */
	//! Returns a number of armies killed.
	Uint32 getTriumphTally(Player *player, TriumphType type) const
	  {return d_triumph[player->getId()][type];}

        //! Returns the list of stacks (Stacklist) owned by the player.
        Stacklist* getStacklist() const {return d_stacklist;}

        //! A shortcut for getting the Stack marked as active.
        Stack* getActivestack();

        //! Get the FogMap of the player.
        FogMap* getFogMap() const {return d_fogmap;}

        //! Get the fight order of the player.
	std::list<Uint32> getFightOrder() const {return d_fight_order;}

        //! Set the fight order of the player.
	void setFightOrder(std::list<Uint32> order);

        //! Show debugging information for the player's Action list.
        void dumpActionlist() const;

        //! Show debugging information for the player's History list.
        void dumpHistorylist() const;

        //! Mark the player as dead. Kills all Army units in the Stacklist.
        void kill();

        //! Revive the player, this does not provide any cities.
        void revive() {d_dead = false;}

        /** 
	 * Saves the player data to a file.
	 *
	 * @param helper     The opened saved-game file to write to.
         *
         * @note This function only saves basic data, it doesn't open/close the
         * player tags, this has to be done by the derived methods in 
	 * RealPlayer, AI_Fast, AI_Smart and AI_Dummy.
         */
	//! Save the player to a saved-game file.
        virtual bool save(XML_Helper* helper) const;

        /** 
	 * Loads a player from a file.
         *
         * This is a bit inconsistent with other classes, but with players you
         * have the problem that there are different types with different
         * classes. So we need a static member function which looks which
         * player type to load and calls the constructor of the appropriate
         * class.
         *
         * @param helper       the opened saved-game file to read from.
	 *
	 * @return The loaded Player instance.
         */
        static Player* loadPlayer(XML_Helper* helper);

        /** 
	 * This function is called when a player's turn starts. 
	 * For AI players this function should start the algorithm.
	 * Results in a History_StartTurn event going into the player's 
	 * Historylist.
         *
         * @return True if everything went well.
         */
	//! Callback to start a Player's turn.
        virtual bool startTurn() = 0;

        /** 
	 * This function is called before a player's turn starts.
         * The idea here is that it happens before heroes are recruited,
         * and before new army units show up in cities.
         */
	//! Initialise a Player's turn.
        void initTurn();

        virtual void endTurn() = 0;
        
        /** 
	 * Called so that the player can decide what to do with a newly
	 * conquered city.  For human players this method presents the dialog
	 * that asks the user what should be done (Razing, Pillaging, etc).
	 * For the computer players this method is for deciding what to do.
	 * The decision is made by emitting one of the following signals:
	 * srazingCity, spillagingCity, ssackingCity, soccupyingCity.
	 *
         * @param  city   The newly conquered city.
	 *
         * @return True if everything went well.
         */
	//! Decision callback for what to do if a city is invaded.
        virtual void invadeCity(City* city) = 0;

        /** 
	 * Called whenever a hero emerges in a city
	 *
         * @param  hero    The hero who has offered his or her service.
         * @param  city    The city where the hero is emerging.
         * @param  cost    The amount of gold pieces neccessary to recruit 
	 *                 the hero.
	 * 
         * @note Only change the name and gender attributes of the Hero.
         */
        void recruitHero(Hero* herotemplate, City *city, int cost, int alliesCount, const Army *ally);

	void rename (std::string name);
        /** 
	 * Called whenever a hero advances a level.
	 * For human players this method presents a dialog that allows the
	 * user to select an Army::STAT to improve (HP, MOVES, or SIGHT if
	 * a hidden map is in use).  For computer players this method is 
	 * used to decide which stat should be improved.
         * 
	 * This callback must result in an Action_Level element being 
	 * given to the addAction method.
	 *
         * @param army     The army to raise (is always a Hero.)
         */
	//! Callback to advance an Army's level.
        virtual void levelArmy(Army* a) = 0;
        
        /** 
	 * Called to split the stack in two stacks. You have to take great
         * care that one of the created stacks moves away after that because 
	 * there is code that assumes that there is only stack on a tile.
	 * The Army units that are grouped are the units in the new stack.
	 *
	 * This callback must result in an Action_Split element being 
	 * given to the addAction method.
         *
         * @param s        The stack to split.
	 *
         * @return False if there were any problems, else true.
         */
	//! Callback to split a Stack into two.
        bool stackSplit(Stack* s);

        /** 
	 * Called to merge two stacks into one.
	 *
	 * This callback must result in an Action_Join element being 
	 * given to the addAction method.
         *
         * @param receiver     The receiving stack.
         * @param joining      The joining stack, destroyed after the join.
         * @param grouped      Whether the armies of the other stacks are 
	 *                     selected automatically or not.
	 *
         * @return False if an error occured, else true.
         */
	//! Callback to merge two stacks into one.
        bool stackJoin(Stack* receiver, Stack* joining, bool grouped);

	/**
	 * Called to change the position of a Stack on the map.
	 * The new position is dictated by the last point of the Path of the 
	 * Stack.  This method can trigger many other actions.
	 *
	 * This callback must result in an Action_Move element being 
	 * given to the addAction method.
	 *
	 * @param s             The stack to be moved.
	 *
         * @return False if an error occured, else true.
	 */
        //! Callback to move a stack on the map.
        bool stackMove(Stack* s);

	//! Callback to take the armies from the stack that have at least
	//! enough moves to reach the end of the stack's path.
	bool stackSplitAndMove(Stack* s);
	bool stackSplitAndMoveToAttack(Stack* s);
	bool stackSplitAndMoveToJoin(Stack* s, Stack *join);

        /** 
	 * Called to move a Stack to a specified position.
         *
         * The Path is calculated on the fly unless follow is set to true. 
	 * In this case, an existing path is checked and iterated over.  
	 * This is useful if a stack didn't reach its target within one 
	 * round and should continue the movement.
         *
	 * This callback must result in an Action_Move element being 
	 * given to the addAction method.
	 *
         * @param s                The stack to be moved.
         * @param dest             The destination of the move.
         * @param follow           If set to false, calculate the path.
	 *
         * @return False on error, true otherwise.
         */
        //! Callback to move a stack on the map.
        MoveResult *stackMove(Stack* s, Vector<int> dest, bool follow);

        /** 
	 * Called to adjudicate a fight between two lists of stacks.
         * 
         * Note that all stacks next to the defending stack also take part in
         * the fight, if they belong either to the attacker's side or to the
         * defender.  If the attacker or the defender die in the course of 
	 * events, the pointers are set to 0.
         *
	 * This callback must result in an Action_Fight element being 
	 * given to the addAction method.
	 *
         * @param attacker         The list of attacking stacks.
         * @param defender         The list of defending stacks.
	 *
         * @return One of Fight::ATTACKER_WON, Fight::DEFENDER_WON, or
	 *         Fight::DRAW (Fight::Result).
         */
	//! Callback to adjudicate fights.
        Fight::Result stackFight(Stack** attacker, Stack** defender);
        
        /** 
	 * Called to adjudicate a fight between two lists of stacks in a ruin.
         *
         * @param attacker         The list of attacking stacks.  This list
	 *                         consists of a single Stack containing at
	 *                         least one Hero unit.
         * @param defender         The list of defending stacks.  This list
	 *                         consists of a single Army unit in a 
	 *                         single Stack.
	 *
         *  If the defender dies in the fight, the defender pointer is set 
	 *  to 0.
	 *  If the Hero loses the battle, only the Hero unit is removed
	 *  from the attacker's stack.
	 *
         * @return One of Fight::ATTACKER_WON, Fight::DEFENDER_WON, or
	 *         Fight::DRAW (Fight::Result).
         */
	//! Callback to adjudicate fights in ruins.
        Fight::Result stackRuinFight(Stack** attacker, Stack** defender);

	/** 
	 * Callback to decide if we perform treachery on a friendly player.
	 *
	 * For human players this method presents a dialog for the user
	 * to confirm if treachery should happen or not.  For computer
	 * players this method implements the decision to perform treachery
	 * or not.
	 *
	 * @param stack             My stack considering the treachery.
	 * @param player            The friendly player.
	 * @param pos               The place on the map being targetted.
	 * @param state             The state we end up in if we decide yes.
	 *
	 * @return True if we decided to be treacherous.  False otherwise.
	 */
	//! Decision callback for if we should perform trechery or not.
	bool treachery (Stack *stack, Player *player, Vector <int> pos);

        /** 
	 * A stack searches a ruin.  The stack must contain a hero.
	 *
	 * This callback must result in an Action_Ruin element being 
	 * given to the addAction method.
	 *
         * @param stack            The stack which searches the ruin.
         * @param ruin             The ruin to be searched.
	 *
         * @return reward          A pointer to the received Reward.  Return 
	 *                         NULL if the keeper could not be defeated.
         */
	//! Callback to have a stack visit a ruin.
        Reward* stackSearchRuin(Stack* stack, Ruin* ruin);

        /** 
	 * A stack visits a temple and becomes blessed. By blessing, the 
         * strength of all armies rises by 1.
         *
	 * This callback must result in an Action_Temple element being 
	 * given to the addAction method.
	 *
         * @param stack            The stack visiting the temple.
         * @param temple           The visited temple.
	 *
         * @return The number of blessed armies.
         */
	//! Callback to have a stack visit a temple.
        int stackVisitTemple(Stack* stack, Temple* temple);

        /** 
	 * A stack visits a temple and a Hero in the Stack receives a Quest 
	 * from the temple's priests.  If there is more than one hero in the 
	 * stack, the quest is assigned to the first hero without a quest.
         *
	 * This callback must result in an Action_Quest element being 
	 * given to the addAction method.
	 * This callback must result in a History_QuestStarted element being
	 * added to the player's History list (Player::d_history).
	 *
         * @param stack             The visiting stack.
         * @param temple            The visited temple.
         * @param except_raze       Don't give out a raze quest because it's
	 *                          impossible to raze a city in this 
	 *                          scenario.
	 *
         * @return The newly assigned Quest or 0 on error.
         */
	//! Callback to have a Hero get a new Quest from a temple.
        Quest* stackGetQuest(Stack* stack, Temple* temple, bool except_raze);
        
        /** 
	 * Called to ask the military advisor about what would happen 
	 * if the stack attacked the tile.
         *
         * @param stack    The stack to attack with.
	 * @param tile     The tile to attack (could be a city, or a stack).
	 * @param intense_combat  If the intense combat game option is on or 
	 *                        not.
	 *
         * @return The percent chance to win the fight.  The maximum value
	 *         is 100.0, and the minimum value is 0.0.
         */
	//! Callback to calculate the odds of winning a fight.
        float stackFightAdvise(Stack* stack, Vector<int> tile,
                               bool intense_combat);

        /** 
	 * Callback to have the active player occupy a given city.
	 * The player has defeated a City and now it has been decided
	 * that the player wishes to occupy this city.  The decision 
	 * happens in Player::invadeCity. Occupying means that the city 
	 * becomes owned by the ocuppying player.
         *
	 * This callback must result in an Action_Occupy element being 
	 * given to the addAction method.
	 *
         * @param city             The occupied city.
	 *
         * @return False on error, true otherwise.
         */
	//! Callback to occupy a city.
        void cityOccupy(City* city);

        /** 
	 * Pillage a city (trade in the best army type and get some gold.)
	 * The player has defeated a City and now it has been decided
	 * that the player wishes to pillage this city.  The decision to
	 * pillage happened in Player::invadeCity. Pillaging means that the 
	 * city becomes owned by the pillaging player, and that the strongest 
	 * Army unit type that the city can produce is traded-in for an 
	 * amount of gold pieces.
         *
	 * This callback must result in an Action_Pillage element being 
	 * given to the addAction method.
	 *
         * @param city             The city to be pillaged.
         * @param gold             Returns the amount of gold pillaged.
	 * @param pillaged_army_type The army type that is cashed in for gold.
	 *
         * @return False on error, true otherwise.
         */
	//! Callback to pillage a city.
        void cityPillage(City* city, int& gold, int *pillaged_army_type);

        /**
	 * Sack a city (trade in all army types except one and get some gold.)
	 * The player has defeated a City and now it has been decided
	 * that the player wishes to sack this city.  The decision to sack
	 * was made in Player::invadeCity.  Sacking entails that the city 
	 * becomes owned by the sacking player, and that all of the Army 
	 * units that the city produces are traded-in for gold pieces except 
	 * for the weakest Army unit.
         *
	 * The AI_Fast, AI_Dummy and AI_Smart classes use this method
	 * as defined in RealPlayer to sack cities.
         *
	 * This callback must result in an Action_Sack element being 
	 * given to the addAction method.
         *
         * @param city             The city to be sacked .
         * @param gold             Returns the amount of gold sacked.
	 * @param sacked_types     Returns the Army types that were cashed-in 
	 *                         for gold pieces.
	 *
         * @return False on error, true otherwise.
         */
	//! Callback to sack a city.
        void citySack(City* city, int& gold, std::list<Uint32> *sacked_types);

        /** 
	 * Raze (permanently destroy) a city.
	 * The player has defeated a City and now it has been decided
	 * that the player wishes to raze this city.  The decision to raze
	 * was made in Player::invadeCity.  Razing entails that the city 
	 * becomes burned, and is owned by nobody.  The city cannot produce
	 * Army units.  Other players find razing to be diplomatically 
	 * treacherous.
         *
	 * The AI_Fast, AI_Dummy and AI_Smart classes use this method
	 * as defined in RealPlayer to raze cities.
         *
	 * This callback must result in an Action_Raze element being 
	 * given to the addAction method.
         *
         * @param city             The city to be razed.
	 *
         * @return False on error, true otherwise.
         */
	//! Callback to raze a city.
        void cityRaze(City* city);

        /** 
	 * Add another production to a city.  
	 * The city has a set of Army units available to produce, but the
	 * Player deems this insufficent.  A new Army unit is purchased
	 * for an amount of gold pieces, so that the City can produce that
	 * Army unit.  Each Army unit type that can be produced is
	 * associated with one of 4 slots.  If the player purchases a new
	 * Army unit in a slot that already has an Army unit, it is 
	 * removed permanently.
         *
	 * This callback must result in an Action_Buy element being 
	 * given to the addAction method.
         *
         * @param city             The lucky city.
         * @param slot             The production slot of the city.  The 
	 *                         minimum value is 0 and the maximum value
	 *                         is 3.
         * @param armytype         The index of the army type to add.  This
	 *                         type relates to the Player's Armyset.
	 *
         * @return False on error, true otherwise.
         */
	//! Callback to purchase a new Army unit for production within a City.
        bool cityBuyProduction(City* city, int slot, int armytype);

        /** 
	 * Change the production of a city.
	 * The City has a set of Army units that it may produce.  There are
	 * up to 4 army units available for production in the City, and each
	 * sits in a slot.  The change of production is indicated by slot 
	 * number.  If the production is to stop altogether the slot number
	 * is -1.
	 * After a slot is selected and enough time passes, a new Army unit
	 * will arrive in the city that produced it.
         *
	 * This callback must result in an Action_Production element being 
	 * given to the addAction method.
         *
         * @param city             The affected city.
         * @param slot             The index of the selected production slot.
	 *                         The minimum value is -1 which means to 
	 *                         stop production in the City.  The other
	 *                         legal values are 0 through 3; one for
	 *                         each slot in the city.  If a slot does
	 *                         not contain an Army unit, then that slot
	 *                         number is an illegal value to this method.
	 *
         * @return False on error, true otherwise.
         */
	//! Callback to change the Army unit being produced within a City.
        bool cityChangeProduction(City* city, int slot);

	//! A player's city produces an army unit.
	/**
	 * @param city  The city that has produced an army unit.
         * @return False on error, true otherwise.
	 */
	bool cityProducesArmy(City *city);

	//! A player has a vectored army unit arrive somewhere.
	bool vectoredUnitArrives(VectoredUnit *unit);

        /** 
	 * This method gives the player the specified Reward.  There are 
	 * various possibilities when they player is being given a reward.
	 * It could be that the player has been given: some gold pieces, a 
	 * map that makes more of the map visible or information about the 
	 * location of a new ruin.  It could also be that a stack has been 
	 * given a number of powerful allies.  It could also be that a stack 
	 * contains a Hero, and the Reward is an Item for the Hero to carry.
	 *
	 * This callback must result in an Action_Reward element being 
	 * given to the addAction method.
         *
         * @param stack            The stack which has caused the reward.
         * @param reward           A pointer for storing the Reward being 
	 *                         given to the player.
	 *
         * @return False on error, true otherwise.
         */
	//! Callback to give a Reward to the Player or the player's Stack.
        bool giveReward (Stack *stack, Reward *reward);

	/**
	 * Disbanding a player's stack removes it from the game.  Disbanding
	 * stacks saves upkeep for unwanted Army units.
	 *
	 * This callback must result in an Action_Disband element being 
	 * given to the addAction method.
	 *
	 * @param stack            The stack to disband.
	 *
         * @return False on error, true otherwise.
	 */
        //! Callback to disband a player's stack.
        bool stackDisband(Stack* stack);

	/**
	 * Callback to drop an item at a particular position on the game map.
	 * The item is removed from the Hero's backback and placed in a bag
	 * at place on the map.
	 * 
	 * For this method to make sense, the Hero should be in a Stack
	 * that is co-located with the drop position.  E.g. Heroes should
	 * drop items here.
	 *
	 * This callback must result in an Action_Equip element being 
	 * given to the addAction method.
	 *
	 * @param hero             The Hero that holds the item.
	 * @param item             The Item to drop onto the ground.
	 * @param pos              The position of the tile on the game map to 
	 *                         drop the item onto.
	 *
         * @return False on error, true otherwise.
	 */
        //! Callback to have a Hero drop an Item.
        bool heroDropItem(Hero *hero, Item *item, Vector<int> pos);

	/**
	 * Callback to drop a all items at a particular position on the 
	 * game map.  All items in the Hero's backback are removed and placed 
	 * into a bag at place on the map.
	 *
	 * For this method to make sense, the Hero should be in a Stack
	 * that is co-located with the drop position.  E.g. Heroes should
	 * drop items here.
	 *
	 * This callback must result in one or more Action_Equip elements 
	 * being given to the addAction method.
	 *
	 * @param hero             The Hero that holds the items.
	 * @param pos              The position of the tile on the game map to 
	 *                         drop the item onto.
	 *
         * @return False on error, true otherwise.
	 */
        //! Callback to have a Hero drop all items.
        bool heroDropAllItems(Hero *hero, Vector<int> pos);

	/**
	 * Callback to pickup an Item at a particular position on the game 
	 * map.  The item is removed from a tile on the game map, and placed
	 * into the Hero's backback.
	 *
	 * For this method to make sense, the Hero should be in a Stack
	 * that is co-located with the pickup position.  E.g. Heroes should
	 * pickup items from the tile they are on.
	 *
	 * This callback must result in an Action_Equip element being 
	 * given to the addAction method.
	 *
	 * @param hero             The Hero that holds the item.
	 * @param item             The Item to pickup off of the ground.
	 * @param pos              The position of the tile on the game map to 
	 *                         pickup the item from.
	 *
         * @return False on error, true otherwise.
	 */
        //! Callback to have a Hero pick up an Item.
        bool heroPickupItem(Hero *hero, Item *item, Vector<int> pos);

	bool heroPickupAllItems(Hero *h, Vector<int> pos);

	/**
	 * Completing a Quest entails that the Hero is going to receive a
	 * reward, but that happens in Player::giveReward.
	 * The QuestsManager class handles removal of expired or completed 
	 * quests.
	 * This callback doesn't do much except record the event for
	 * posterity (see HistoryReportDialog).
	 *
	 * This callback must result in a History_QuestCompleted element being
	 * added to the player's History list (Player::d_history).
	 *
	 * @param hero             The Hero completing the Quest.
	 *
         * @return False on error, true otherwise.
	 */
        //! Callback to have a Hero complete a quest.
        bool heroCompletesQuest(Hero *hero);

	/**
	 * Callback to have the Player resign.  This entails disbanding
	 * all of the player's stacks and then razing all of the player's 
	 * remaining cities.  It also removes all of the gold pieces from 
	 * the player's treasury.
	 *
	 * This callback is called when a human player wants to surrender
	 * ungracefully.  Computer players do not currently consider
	 * calling this method to surrender, and they use a different
	 * mechanism to collectively surrender to a final human player.
	 *
	 * This callback must result in a Action_Resign element being
	 * given to the addAction method.
	 *
	 */
        //! Callback to disband all the player's stacks and raze all cities.
        void resign();

	/**
	 * Modifying a signpost entails changing the message on the sign.
	 * When playing in a hidden map, the hope is that we change the
	 * message on the sign before an opponent can read it.
	 *
	 * For this callback to make sense, you should only change
	 * Signposts for which we have a Stack co-located.
	 *
	 * This callback must result in a Action_ModifySignpost element being
	 * given to the addAction method.
	 *
	 * @param signpost         The signpost to modify.
	 * @param message          The new text to inscribe onto the sign.
	 *
         * @return False on error, true otherwise.
	 */
        //! Change the text on a signpost.
        bool signpostChange(Signpost *signpost, std::string message);

	/**
	 * Callback to have a Player rename a City.
	 *
	 * Only human players currently rename cities; computer players
	 * do not consider doing so.
	 *
	 * This callback must result in a Action_RenameCity element being
	 * given to the addAction method.
	 *
	 * @param city             The city to change the name of.
	 * @param name             The new name of the city.
	 *
         * @return False on error, true otherwise.
	 */
        //! Callback to rename a city.
        bool cityRename(City *city, std::string name);

        /** 
	 * Callback to initiate vectoring new units from a player's City to 
	 * a destination point on the game map.
	 *
	 * Computer players don't currently consider vectoring units, so
	 * only human players use this method.
	 *
	 * This callback must result in a Action_Vector element being
	 * given to the addAction method.
	 *
         * @param  city   The city to vector from.
         * @param  dest   The place on the map to vector the produced Army
	 *                units to.  If the  destination is -1,-1 it means 
	 *                to stop vectoring altogether.  The destination 
	 *                point should be co-located with a City or a 
	 *                planted standard Item.
	 *
         * @return False on error, true otherwise.
         */
	//! Callback to vector produced units from a city.
        bool vectorFromCity(City* city, Vector<int> dest);

        /** 
	 * Callback to change the vectoring destination for all of the
	 * player's cities that are vectoring to a particular city.
	 *
	 * Computer players don't currently consider vectoring units, so
	 * only human players use this method.
	 *
	 * This callback must result in one or more Action_Vector elements 
	 * being given to the addAction method.
	 *
	 * @param  city   The city being vectored to.
         * @param  dest   The place on the map to vector to.  The destination 
	 *                point should be co-located with a City or a 
	 *                planted standard Item.
	 *
         * @return False on error, true otherwise.
         */
	//! Callback to make a mass change to vectoring.
	bool changeVectorDestination(City *city, Vector<int> dest);

        /** 
	 * Callback to plant the Player's flag Item on the ground.
	 * Planting a standard entails taking the Item out of the Hero's
	 * backpack and putting it on the ground so that Army units can
	 * be vectored to that location.
	 *
	 * Computer players don't currently consider vectoring units, so
	 * only human players use this method.
	 *
	 * This callback must result in an Action_Plant element being added
	 * to the player's Action list (Player::d_actions).
	 *
	 * @param  stack  The Stack that contains the Hero who is holding
	 *                the plantable Item.  Every player has exactly one 
	 *                plantable Item.  The item is planted at the
	 *                position of the Stack on the map.
	 *
         * @return False on error, true otherwise.
         */
        //! Callback to plant a player's standard.
        bool heroPlantStandard(Stack *stack);

	/**
	 * @param city   The city being invaded.
	 * @param loot   The gold looted.
	 */
	//! Emitted when the player defeats a City.
        sigc::signal<void, City*, int> sinvadingCity;

	/**
	 * @param hero   The new hero that is emerging.
	 * @param city   The city in which the hero is emerging.
	 * @param gold   The amount of gold pieces the hero costs.
	 *
	 * @return True if we're accepting a hero, false if not.
	 */
        //! Emitted whenever a hero is recruited.
        sigc::signal<bool, Hero*, City *, int> srecruitingHero;

	/**
	 * @param army   The army that has gained a level.
	 *
	 * @return One of Army::Stat::STRENGTH, Army::Stat::MOVES, or 
	 *         Army::Stat::SIGHT.
	 */
        //! Emitted when an Army advances a level; returns stat to raise.
        sigc::signal<Army::Stat, Army*> snewLevelArmy;

	/**
	 * @param army   The army that has gotten a medal.
	 */
        //! Emitted whever a player's army gets a new medal.
        sigc::signal<void, Army*> snewMedalArmy;

	/**
	 * @param army         The army that has died.
	 * @param culprit_ids  A std::vector of enemy Army Ids to have 
	 *                     participated in the death of this player's Army.
	 */
        //! Emitted when a player's Army dies.
        sigc::signal<void, Army*, std::vector<Uint32> > sdyingArmy;

	/**
	 * @param ruin    The ruin being searched.
	 * @param stack   The stack doing the searching (must contain Hero).
	 * @param reward  The reward received.
	 */
        //! Emitted whenever the player successfully searched a ruin.
        sigc::signal<void, Ruin*, Stack*, Reward*> ssearchingRuin;

	/**
	 * @param temple  The temple being visited.
	 * @param stack   The stack that has gotten blessed.
	 */
        //! Emitted whenever the player visits a temple.
        sigc::signal<void, Temple*, Stack*> svisitingTemple;

	/**
	 * @param city   The city being occupied.
	 * @param stack  The stack doing the occupying.
	 */
	//! Emitted when the player occupies a City.
        sigc::signal<void, City*, Stack*> soccupyingCity;

	/**
	 * @param city        The city that has been pillaged.
	 * @param stack       The stack doing the pillaging.
	 * @param gold        The amount of gold pieces pillaged.
	 * @param army_types  The list of Army types traded-in for gold pieces.
	 */
        //! Emitted whenever the player pillages a city.
        sigc::signal<void, City*, Stack*, int, Uint32> spillagingCity;

	/**
	 * @param city        The city that has been sacked.
	 * @param stack       The stack doing the sacked.
	 * @param gold        The amount of gold pieces sacked.
	 * @param army_types  The list of Army types traded-in for gold pieces.
	 */
        //! Emitted whenever the player sacks a city.
        sigc::signal<void, City*, Stack*, int, std::list<Uint32> > ssackingCity;

	/**
	 * @param city        The city that has been razed.
	 * @param stack       The razing stack.
	 */
        //! Emitted whenever the player razes a city.
        sigc::signal<void, City*, Stack*> srazingCity;

	/**
	 * Emitted when the player's treasury has been changed.
	 */
        //! Emitted whenever a player's status changes.
        sigc::signal<void> schangingStatus;

	/**
	 * Emitted when the player's stack moves, is disbanded, gets blessed,
	 * searches a ruin, or is otherwise altered.
	 *
	 * @param stack    The stack that has been altered.
	 */
        //! Emitted whenever the stack's status has changed.
        sigc::signal<void, Stack*> supdatingStack;

	/**
	 * Emitted whenever a city is conquered or razed.
	 *
	 * @param city     The city that has been altered.
	 */
        //! Emitted whenever the status of a city has changed.
        sigc::signal<void, City*> supdatingCity;

	/**
	 * @param fight  The details of the upcoming fight.
	 */
	//! Emitted when a fight has started against a city or stack.
        sigc::signal<void, Fight &> fight_started;

	/**
	 * @param city     The city we attacked.
	 * @param result   If we won or not.
	 */
	//! Emitted after we attack a city.
        sigc::signal<void, City *, Fight::Result> cityfight_finished;
	
	/**
	 * @param attacker The player's attacking stack.
	 * @param keeper   The keeper of the ruin.
	 */
	//! Emitted when a fight in a ruin is started.
        sigc::signal<void, Stack *, Stack *> ruinfight_started;

	/**
	 * @param result   If we defeated the ruin's keeper or not.
	 */
	//! Emitted when a fight in a ruin has finished.
        sigc::signal<void, Fight::Result> ruinfight_finished;

	/**
	 * @param chance   The percent chance that we will prevail in battle.
	 */
	//! Emitted when a player asks for help from a military advisor.
        sigc::signal<void, float> advice_asked;

	//! Signal raised when a stack is considering an act of treachery.
        sigc::signal<bool, Stack *, Player *, Vector<int> > streacheryStack;

	//! Signal raised when a human player is deciding.
        sigc::signal<bool, Stack *, Player *, Vector<int> > streachery;

        //! Player would like to end the turn.
        sigc::signal<void> ending_turn;

        sigc::signal<void, int> hero_arrives_with_allies;

        sigc::signal<void, NetworkAction *> acting;
        sigc::signal<void, NetworkHistory *> history_written;
        
	//! is it safe to vector from the given city?
	static bool safeFromAttack(City *c, Uint32 safe_mp, Uint32 min_defenders);
	void addHistory(History *history);
    protected:
        // do some fight cleaning up, setting
        void cleanupAfterFight(std::list<Stack*> &attackers,
                               std::list<Stack*> &defenders);
        
        //! Move stack s one step forward on it's Path.
        bool stackMoveOneStep(Stack* s);

        void addAction(Action *action);

        // DATA
	//! The player's colour.
	/**
	 * Mask portions of images are shaded in this colour.
	 */
        SDL_Color d_color;

	//! The name of the Player.
        std::string d_name;

	//! The ArmySet of the Player.
        Uint32 d_armyset;

	//! The number of gold pieces the Player has in the treasury.
        int d_gold;

	//! Whether or not this player is dead.
        bool d_dead;

	//! Whether or not this player can be killed.
        bool d_immortal;

	//! The kind of Player (see Player::Type).
        Uint32 d_type;

	//! A unique numeric identifier identifying this Player.
        Uint32 d_id;

	//! A list of actions that this Player made this turn.
        std::list<Action*> d_actions;

	//! A list of "headlines" for this Player for the whole game.
        std::list<History*> d_history;

	//! A list of the Player's Stack objects.
        Stacklist* d_stacklist;

	//! What the player can see on the hidden map.
        FogMap* d_fogmap;

	//! The order in which this Player's army types fight in battle.
	/**
	 * @note This value is related to the Player's ArmySet.
	 */
	std::list<Uint32> d_fight_order; 

	//! A set of statistics for this Player.
	// 5 is max TriumphType + 1
	Uint32 d_triumph[MAX_PLAYERS][5]; 

	//! How many gold pieces the Player paid out in the last turn.
	Uint32 d_upkeep;

	//! How many gold pieces the Player made from taxes in the last turn.
	Uint32 d_income;

	//! The diplomatic view that this Player has of each other Player.
	DiplomaticState d_diplomatic_state[MAX_PLAYERS];

	//! The diplomatic rank this Player has among all other Players.
	Uint32 d_diplomatic_rank;

	//! The title that goes along with the diplomatic rank.
	std::string d_diplomatic_title;

	//! The proposals that this Player is making this turn.
	DiplomaticProposal d_diplomatic_proposal[MAX_PLAYERS];

	//! A quantification of how much this Player likes every other Player.
	Uint32 d_diplomatic_score[MAX_PLAYERS];

	//! assists in scorekeeping for diplomacy
	void alterDiplomaticRelationshipScore (Player *player, int amount);

        void conquerCity(City *city, Stack *stack);

        // return the new stack if split succeeded
        Stack *doStackSplit(Stack *s);
        void doStackJoin(Stack* receiver, Stack* joining, bool grouped);
        int doStackVisitTemple(Stack *s, Temple *t);
        void doCityOccupy(City *c);
        void doCityPillage(City *c, int& gold, int* pillaged_army_type);
        void doCitySack(City *c, int& gold, std::list<Uint32> *sacked_types);
        void doCityRaze(City *c);
        void doCityBuyProduction(City *c, int slot, int type);
        void doCityChangeProduction(City *c, int slot);
        void doGiveReward(Stack *s, Reward *reward);
        void doHeroDropItem(Hero *hero, Item *item, Vector<int> pos);
        void doHeroPickupItem(Hero *hero, Item *item, Vector<int> pos);
        void doLevelArmy(Army *army, Army::Stat stat);
        void doStackDisband(Stack *stack);
        void doSignpostChange(Signpost *signpost, std::string message);
        void doCityRename(City *c, std::string name);
        void doVectorFromCity(City * c, Vector<int> dest);
        void doSetFightOrder(std::list<Uint32> order);
        void doResign();
        void doHeroPlantStandard(Hero *hero, Item *item, Vector<int> pos);
        void doDeclareDiplomacy (DiplomaticState state, Player *player);
        void doProposeDiplomacy (DiplomaticProposal proposal, Player *player);
        void doConquerCity(City *city, Stack *stack);
        void doRecruitHero(Hero* herotemplate, City *city, int cost, int alliesCount, const Army *ally);
        void doRename(std::string name);
	void doCityProducesArmy(City *city);
	void doVectoredUnitArrives(VectoredUnit *unit);

	void AI_maybeBuyScout();

	//! Go to a temple if we're near enough.
	/**
	 * Helper method to take a stack on a mission to get blessed.
	 * If the method returns false initially, it means that the nearest 
	 * temple is unsuitable.
	 * @note The idea is that this method is called over subsequent turns, 
	 * until the blessed parameter gets filled with a value of true.
	 *
	 * @param s            The stack to visit a temple.
	 * @param dist         The maximum number of tiles that a temple
	 *                     can be away from the stack, and be considered
	 *                     for visiting.
	 * @param mp           The maximum number of movement points that a
	 *                     stack needs to have to reach the temple.
	 * @param percent_can_be_blessed  If the stack has this many army 
	 *                                units that have not been blessed
	 *                                at the temple (expressed as a
	 *                                percent), then the temple will be
	 *                                considered for visiting.
	 * @param blessed      Gets filled with false if the stack didn't get 
	 *                     blessed.  Gets filled with true if the stack 
	 *                     got blessed at the temple.
	 * @param stack_died   Gets filled with true if the stack got killed
	 *                     by an enemy stack on the same square as the
	 *                     temple.
	 *
	 * Returns true if the stack moved, false if it stayed still.
	 */
	bool AI_maybeVisitTempleForBlessing(Stack *s, int dist, int mp, 
					    double percent_can_be_blessed, 
					    bool &blessed, bool &stack_died);
	bool AI_maybePickUpItems (Stack *s, int dist, int mp, bool &picked_up,
				  bool &stack_died);



	bool AI_maybeVector(City *c, Uint32 safe_mp, Uint32 min_defenders,
			    City *target, City **vector_city = NULL);


	void AI_setupVectoring(Uint32 safe_mp, Uint32 min_defenders,
			       Uint32 mp_to_front);

	bool AI_maybeDisband(Stack *s, City *city, Uint32 min_defenders, 
			     int safe_mp, bool &stack_died);

    private:
        //! Loads the subdata of a player (actions and stacklist)
        bool load(std::string tag, XML_Helper* helper);

        /**
	 * Returns all heroes in the given list of stacks.
         *
         * @param stacks           the list of stacks which is searched.
         * @param heroes           Return a list of id's of the heroes found.
         */
	//! Get heroes.
        void getHeroes(const std::list<Stack*> stacks, 
		       std::vector<Uint32>& heroes);

        /** 
	 * Goes through a list of stacks and removes all armies with less
         * than 1 hitpoint.  It also removes empty stacks. 
         * This function also heals regenerating units at the end of combat. 
         *
         * @param stacks           The list searched for dead armies.
         * @param culprits         The list of heroes responsible for killing
	 *                         the armies.  This is needed for tracking
	 *                         the progress of a Quest.
         * @return The sum of the XP of the killed armies.
         */
	//! Remove dead Armies from a list of stacks after a fight.
        double removeDeadArmies(std::list<Stack*>& stacks,
                                std::vector<Uint32>& culprits);
        
        /** 
	 * Increases the number of experience points of a stack
         * the number of battles and checks if an army can get a medal
         *
         * This functions takes a number of experience points and distributes
         * them equally over all armies in the stack list. Therefore, the less
         * armies fight, the more experience the single armies get. It emits a
         * signal when a unit gains a level.
         *
         * @param stacks           A list of all stacks gaining experience.
         * @param xp_sum           The number of XP to distribute.
         */
	//! update Army state after a Fight.
        void updateArmyValues(std::list<Stack*>& stacks, double xp_sum);

        void adjustDiplomacyFromConqueringCity(City *city);
        // return how much gold we got out
        int lootCity(City *city);
        void takeCityInPossession(City* c);
	void tallyTriumph(Player *p, TriumphType type);
};

extern sigc::signal<void, Player::Type>  sendingTurn;

#endif // PLAYER_H

// End of file
