//  Copyright (C) 2011 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#ifndef HOSTED_GAME_H
#define HOSTED_GAME_H

#include <gtkmm.h>
#include <string>


#include <sys/time.h>

class XML_Helper;
class AdvertisedGame;

//! A single game in the gamelist, either advertised or hosted.
/**
 *
 */
class HostedGame
{
    public:

        static std::string d_tag;
	//! Make a new hosted game entry.
	HostedGame(AdvertisedGame *advertised_game);

	//! Load a new hosted game entry from an opened file.
	HostedGame(XML_Helper *helper);

	//! Destroy a hosted game entry.
	~HostedGame();


	// Get Methods
        AdvertisedGame * getAdvertisedGame() const {return d_advertised_game;};

	// Methods that operate on the class data but do not modify it.

	//! Save the hosted game entry to an opened file.
        bool save(XML_Helper* helper) const;

	// Methods that operate on the class data and modify it.


    private:

	// DATA
        
        bool loadAdvertisedGame(std::string tag, XML_Helper *helper);

        AdvertisedGame *d_advertised_game;
        guint32 d_pid;
	
};

#endif // HOSTED_GAME_H
