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

#ifndef SERVER_H
#define SERVER_H

#include "config.h"
#include "ggzgameserver.h"

class Server : public GGZGameServer
{
	public:
		Server();
		~Server();
 
#ifdef WITH_GGZ
		void joinEvent(int player);
		void leaveEvent(int player);
		void dataEvent(int player);
		void errorEvent();
#endif

		// send the map to all clients
		//void send_map();
		// tell the player that he starts first
		//void send_playerorder();
	private:
		bool hostplaying;
};

#endif

// End of file
