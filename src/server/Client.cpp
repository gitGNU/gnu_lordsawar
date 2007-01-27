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

#include "Client.h"
//#include "playerlist.h"
#include <iostream>

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl;}
//#define debug(x)

Client::Client(std::string host, int port)
{
    /*
    debug("Client(" << host << ", " << port << ")");
    CPPSocket::Address serverAddress(host, port);
    open(CPPSocket::Socket::TCP);
    connect(serverAddress);

	first = false;
	already_connected = false;
    */
}

Client::~Client()
{
}

void Client::sendPlayerInfo()
{
    debug("sendPlayerInfo()");
    //CPPSocket::StringBuffer data("PLAYER\nnewName\n");
    //send(data);
}

// End of file
