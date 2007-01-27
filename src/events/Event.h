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

#ifndef EVENT_H
#define EVENT_H

#include <SDL_types.h>
#include <sigc++/sigc++.h>
#include <string>
#include <vector>

#include "../xmlhelper.h"

class Reaction;
class Condition;

/**
  * Class for events which do something during a scenario.
  *
  * This is the base class for events.
  * There are different events for different occassions. Each event connects to
  * one of the several signals which are raised when something happens. Such an
  * event can trigger actions which can be anything in the range from winning
  * the game to gaining or loosing some troops.
  *
  * The correct type of the constructor etc. can only be determined in the
  * derived classes, since they will want to be fed with different data.
  *
  * The classes up to now are:
  * EPlayerDead:    raised when the specified player dies
  * EKillAll:       raised when all players except one have died
  * ECityConq:      raised when the specified city is conquered by a player
  * EArmyKilled:    raised when a specified army has been killed.
  * ERound:         raised at the beginning of a specific round
  * ERuinSearch:    raised when a specific ruin has been searched
  * ETemple:        raised when a specific temple has been searched
  * EDummy:         never raised, can be useful in some constructions
  * ENextTurn:      when activated, it is raised at the next possible turn
  * EStackKilled:   like EArmyKilled, but here a stack must be destroyed
  * EStackMove:     raised when a stack moves to (a) certain position(s)
  *
  * Furthermore, an event can have one or more conditions which determine if
  * it is triggered. See Condition.h for an explanation what they do.
  */

class Event : public SigC::Object
{
    public:
        enum Type {KILLALL=0, PLAYERDEAD=1, CITYCONQUERED=2, ARMYKILLED=3,
                    ROUND=4, RUINSEARCH=5, TEMPLESEARCH=6, DUMMY=7,
                    NEXTTURN=8, STACKKILLED=9, STACKMOVE=10};
    
        //! Creates an event with type as its, well, type
        Event(Type type);

        //! Does the basic loading functionality, especially loading of reactions
        Event(XML_Helper* helper);

        virtual ~Event();

        /** Load an event
          * 
          * @param helper the XML_Helper object representing the savegame
          *
          * This function examines the event type and loads the correct event
          * with this information.
          */
        static Event* loadEvent(XML_Helper* helper);

        /** Saves the event data. The base class only saves the common data!
          * Opening tags and such has to be done by the derived classes!
          */
        virtual bool save(XML_Helper* helper) const;

        /** Initialises the event. This is neccessary because events are loaded
          * first, but usually hook up to signals of other classes which are
          * loaded only after the events have been loaded.
          *
          * The init function is called automatically after a savegame has been
          * loaded by GameScenario.
          */
        virtual void init() {}

        //! checks if all conditions are fulfilled and triggers all reactions
        void raise();


        //! Returns the type of the event
        Type getType() const {return d_type;}
        
        //! Returns false if the event is processed and can be deleted.
        bool getActive() const {return d_active;}

        //! activate/deactivate an event
        void setActive(bool active) {d_active = active;}

        //! get the id of the event
        Uint32 getId() {return d_id;}

        
        //! Add a reaction to be triggered when this event is raised, if
        //! index != -1, insert the reaction before this position.
        bool addReaction(Reaction* r, int index = -1);

        //! remove the ith reaction from the list
        bool removeReaction(Uint32 index);

        //! Get the ith reaction or zero if this one doesn't exist
        Reaction* getReaction(Uint32 index);

        //! Returns the number of reactions
        Uint32 getNoOfReactions() const {return d_reactions.size();}
        

        //! add a condition to the list
        bool addCondition(Condition* c, int index = -1);

        //! remove the ith condition from the list
        bool removeCondition(Uint32 index);

        //! get the ith condition or zero if it doesn't exist
        Condition* getCondition(Uint32 index);

        //! Returns the number of conditions
        Uint32 getNoOfConditions() const {return d_conditions.size();}


        /** Sets a comment for the event
          *
          * The idea behind this is to give the event a description so that it
          * is easier to see what this event is for. I hope this proves to be
          * useful in the editor.
          */
        void setComment(std::string comment) {d_comment = comment;}

        //! Returns the comment string
        std::string getComment() const {return d_comment;}

    protected:
        //Data
        std::vector<Reaction*> d_reactions;
        std::vector<Condition*> d_conditions;
        Type d_type;
        bool d_active;
        std::string d_comment;
        Uint32 d_id;

    private:
        //! Callback for loading of the reactions. See XML_Helper for details
        bool loadData(std::string tag, XML_Helper* helper);
};

#endif //EVENT_H
