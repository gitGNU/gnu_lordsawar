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

#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <sigc++/sigc++.h>

class Client// : public SigC::Object
{
	public:
		Client(std::string host, int port);
		~Client();

		void send_turninfo();
		void wait_for_map();
		void wait_for_turninfo();
		void wait_for_playerorder();

		bool is_first(){return first;}

        // SLOTS
		void sendPlayerInfo();
		
	//TBD signals:
		void got_turninfo();

	//TBD public slots:
		void incoming_data();

	private:
		void read_turninfo();

		bool first;	
		bool already_connected;
};

#endif

// End of file
