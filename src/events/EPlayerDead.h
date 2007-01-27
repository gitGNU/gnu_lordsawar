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

#ifndef EPLAYERDEAD_H
#define EPLAYERDEAD_H

#include "Event.h"

class Player;

/** This specialized event is triggered when a player dies. It connects to the
  * splayerDead signal of the playerlist.
  */

class EPlayerDead : public Event
{
    public:
        /** Constructor
          * 
          * @param player   the id of the player whose death triggers the event
          */
        EPlayerDead(Uint32 player);

        //! Loads the event from a savegame descriped by the XML_Helper.
        EPlayerDead(XML_Helper* helper);
        ~EPlayerDead();

        //! Saves the event data.
        bool save(XML_Helper* helper) const;

        //! Initialises the event
        void init();

        
        //! Returns the player whose death we wait for.
        Uint32 getPlayer() const {return d_player;}

        //! Changes the player
        void setPlayer(Uint32 player) {d_player = player;}

    private:
        Uint32 d_player;

        //! Callback which triggers the event.
        void trigger(Player* dead);
};

#endif //EPLAYERDEAD_H
