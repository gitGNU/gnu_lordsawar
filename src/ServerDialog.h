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

#ifndef SERVERDIALOG_H
#define SERVERDIALOG_H

#include <string>
#include <pgwindow.h>
#include <pgbutton.h>

class Server;
class PG_Button;
//! This class is currently unused.

class ServerDialog : public PG_Window
{
	public:
		// CREATORS
		ServerDialog(PG_Widget* parent, const Rectangle& rect);
		~ServerDialog();

		// MANIPULATORS
		void startGame();

		// CALLBACKS
		bool hostGameClicked(PG_Button* btn);
		bool startClicked(PG_Button* btn);
		bool closeClicked(PG_Button* btn);
        /*
		PARAGUI_CALLBACK(save_game);
		PARAGUI_CALLBACK(check_buttons);
		void new_client_connected(int socket);
		void start_server();
		void add_playerstatus(int socket, std::string name);
		void del_playerstatus(int socket);
        */

	private:
		Server* server;
		//??? QMultiLineEdit* me_output;
		PG_Button* d_b_hostGame;
		PG_Button* d_b_start;
		PG_Button* d_b_close;
		bool server_started;
};

#endif

// End of file
