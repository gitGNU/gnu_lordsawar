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

#include "Server.h"
#include <iostream>

#ifdef WITH_GGZ
#include <ggz.h>
#endif

using namespace std;

Server::Server()
    : GGZGameServer()
{
	hostplaying = false;
}

Server::~Server()
{
}

#ifdef WITH_GGZ
// GGZ event
void Server::joinEvent(int player)
{
	std::cout << "JOIN: " << player << std::endl;

	ggz_write_int(fd(player), 42);
}

// GGZ event
void Server::leaveEvent(int player)
{
	std::cout << "LEAVE: " << player << std::endl;
}

// GGZ event
void Server::dataEvent(int player)
{
	std::cout << "DATA: " << player << std::endl;

	int op;
	ggz_read_int(fd(player), &op);
	if(op == 43)
	{
		std::cout << "he wants to play" << endl;
		if(player == 0) hostplaying = true;
		// cannot start if seats are still open
		if(open()) return;
		// for the time being, the game host's decision overrides all others
		if(hostplaying)
		{
			std::cout << "game is now playing" << endl;
			proceedToPlaying();
			// broadcast game start
			for(int i = 0; i < players(); i++)
			{
				if(fd(i) != -1)
				{
					std::cout << "inform player " << i << endl;
					ggz_write_int(fd(i), 44);
				}
			}
		}
	}
	else
	{
		std::cout << "a treacher!" << endl;
	}
}

// GGZ event
void Server::errorEvent()
{
	std::cerr << "ERROR!" << std::endl;
}
#endif

// End of file
