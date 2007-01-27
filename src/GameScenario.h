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

#ifndef GAME_SCENARIO_H
#define GAME_SCENARIO_H

#include <pgmessageobject.h>
#include <string>
#include <sigc++/object_slot.h>
#include <list>

#include "player.h"
#include "events/Event.h"

class Server;
class Client;
class GamePreferencesDialog;

/** Class to hold several scenario options
  * 
  * This class has two functions. On the one hand side, it holds some data
  * about the current scenario being played (such as the name), on the other
  * hand it has a kind of supervisor function. Loading and saving works in
  * a hierarchical way with superior objects (such as the playerlist) saving
  * their data and then telling inferior objects (such as players) to save
  * their data as well. GameScenario is kind of the root of the saving or
  * loading process. For more information about the saving procedure, have
  * a look at XML_Helper.
  */

class GameScenario : public PG_MessageObject
{
    public:

        /** Initializes an "empty" scenario
          * 
          * @param name     the name of the scenario
          * @param comment  the comment for the scenario
          * @param turnmode the turnmode (see NextTurn for description)
          */
        GameScenario(std::string name, std::string comment, bool turnmode);
        
        /** Load the game scenario using a specified save game
          * 
          * @param savegame     the full name of the savegame to load
          * @param broken       set to true if something goes wrong
          * @param events       if set to true; initialize all events after
          *                     loading; set this to false to disable the event
          *                     mechanism (e.g. in the map editor)
          */
        GameScenario(std::string savegame, bool& broken, bool events);
        ~GameScenario();

        //! Returns the number of the current turn.
        Uint32 getRound() const {return d_round;}

        //! Returns the turn mode. See NextTurn for a description.
        bool getTurnmode() const {return d_turnmode;}

        //! Returns the name of the scenario.
        std::string getName(bool translate = true) const;

        //! Returns the comment for the scenario.
        std::string getComment(bool translate = true) const;

        //! Increments the turn number and does an autosave. Called by NextTurn
        //! via a signal.
        void nextRound();

        //! Sets the name of the scenario.
        void setName(std::string name) {d_name = name;}

        //! Sets the name of the scenario.
        void setComment(std::string comment) {d_comment = comment;}
        
        /** Saves the game. See XML_Helper for further explanations.
          * 
          * @param filename     the full name of the save game file
          * @return true if all went well, false otherwise
          */
        bool saveGame(std::string filename, std::string extension = "sav") const;

        //! Adds an event to the event list
        void addEvent(Event* event);

        //! removes an event from the list (used in the editor)
        void removeEvent(Event* ev);

        /** Deactivates all events. This becomes neccessary because in certain
          * circumstances several events may be triggered and cause the game to
          * finish. To prevent possible bugs, we deactivate all events on a
          * game end so that they are not triggered.
          */
        void deactivateEvents();

        //! Returns the internal list of events
        std::list<Event*> getEventlist() {return d_events;}

    private:
        /** Callback function for loading a game. See XML_Helper for details.
          *
          * @param tag      the tag name
          * @param helper   the helper for parsing the save game file
          * @return true if all went well, false otherwise.
          */
        bool load(std::string tag, XML_Helper* helper);

        // DATA
        Uint32 d_round;
        std::string d_name;
        std::string d_comment;
        bool d_turnmode; //see NextTurn for a description of this option

        std::list<Event*> d_events;
};

#endif // GAME_SCENARIO_H

// End of file
