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

#ifndef REACTION_H
#define REACTION_H

#include <SDL_types.h>
#include <sigc++/sigc++.h>
#include <vector>

#include "../xmlhelper.h"
#include "Condition.h"

/** Defines an action if a certain event occurs
  * 
  * When an event is triggered, something happens. We implement this by
  * defining an event class and a reaction class. While the event class
  * just hooks up to a signal, the real action is done by a reaction class
  * (such as winning/losing a game, getting reinforcements etc.). All
  * reaction classes have the function trigger which, well, triggers the
  * action.
  *
  * Reactions implemented so far:
  * RMessage:       display a message box with some text
  * RAddGold:       adds some gold to a specific player
  * RAddUnit:       adds an army to a player
  * RDelUnit:       removes a specific unit
  * RUpdate:        updates the Game Screen (to reflect changes e.g. with RAddUnit)
  * RCenter:        centers the game screen on a given point
  * RCenterObj:     centers the game screen on a given object(city, stack, temple...)
  * RCreateItem:    creates a new item at a given location
  * RWinGame:       ends the game with a "win" dialog
  * RLoseGame:     ends the game with a "loose" dialog
  * RRaiseEvent:    raises a given event
  * RActEvent:      activates/deactivates a given event
  * RRevivePlayer:  brings a player back to live again
  * RKillPlayer:    kills a player
  * RTransferCity:  changes the ownership of a given city
  *
  * Furthermore, a reaction can have conditions which determine if it is triggered
  * or not. See Condition.h for details.
  */

class Reaction : public SigC::Object
{
    public:
        enum Type {MESSAGE = 0, ADDGOLD = 1, ADDUNIT = 2, DELUNIT = 3,
                   UPDATE = 4, CENTER = 5, CENTEROBJ = 6, CREATEITEM = 7,
                   WINGAME = 8, LOSEGAME = 9, RAISEEVENT = 10,
                   ACTEVENT = 11, REVIVEPLAYER = 12, KILLPLAYER = 13,
                   TRANSFERCITY = 14};
        
        Reaction(Type type);
        Reaction(XML_Helper* helper);
        virtual ~Reaction();

        /** Load a reaction
          * 
          * @param helper   an XML_Helper instance of a savegame
          *
          * This function determines the reaction type and loads the appropriate
          * class.
          */
        static Reaction* loadReaction(XML_Helper* helper);

        //! Saves the reaction data
        virtual bool save(XML_Helper* helper) const;

        //! Triggers the action
        virtual bool trigger() const = 0;
        

        Type getType() const {return d_type;}

        //! add a condition to the reaction
        bool addCondition(Condition* c, int index = -1);
        
        //! remove a condition from the reaction
        bool removeCondition(Uint32 index);

        //! get the ith condition or 0 if it doesn't exist
        Condition* getCondition(Uint32 index);

        //! Returns the number of conditions
        Uint32 getNoOfConditions() const {return d_conditions.size();}

    protected:
        //! shortcut: check through all conditions if they return true
        bool check() const;
        
        Type d_type;
        std::vector<Condition*> d_conditions;

    private:
        //! callback for loading a condition during loading process
        bool loadCondition(std::string tag, XML_Helper* helper);
};


#endif //REACTION_H
