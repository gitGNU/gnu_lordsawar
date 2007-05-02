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

#include <iostream>
#include <pgbutton.h>
#include "ServerDialog.h"
// TBD - port to SDL_net
//#include "cppsocket/exception.h"
//#include "playerstatus.h"

using namespace std;

#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
//#define debug(x)

ServerDialog::ServerDialog(PG_Widget* parent, const Rectangle& rect)
	:PG_Window(parent, rect, "LordsAWar server 0.1", PG_Window::MODAL), server(0)
{
    d_b_hostGame = new PG_Button(this, Rectangle(20, 100, 100, 30), "Host game",0);
    d_b_start = new PG_Button(this, Rectangle(20, 135, 100, 30), "Start",1);
    d_b_close = new PG_Button(this, Rectangle(20, 170, 100, 30), "Close",2);

    d_b_hostGame->sigClick.connect(slot(*this, &ServerDialog::hostGameClicked));
    d_b_start->sigClick.connect(slot(*this, &ServerDialog::startClicked));
    d_b_close->sigClick.connect(slot(*this, &ServerDialog::closeClicked));

    /* TBD
	file->insertItem("&Load game", this, SLOT(load_game()), 0, 1);
	file->insertItem("&Save game", this, SLOT(save_game()), 0, 2);
	
	mb->insertItem("&File", file);
	
	me_output = new QMultiLineEdit(vbox);
	me_output->setReadOnly(TRUE);
    */
    // TBD check_menues
}

ServerDialog::~ServerDialog()
{
}

// Create a new server
bool ServerDialog::hostGameClicked(PG_Button* btn)
{
    /*
       connect(server, SIGNAL(new_connection(int)), this, SLOT(new_client_connected(int)));
       connect(server, SIGNAL(playerinfo_sent(int, QString)), this, SLOT(add_playerstatus(int, QString)));
       connect(server, SIGNAL(closed_connection(int)), this, SLOT(del_playerstatus(int)));
   */


    // Write output string to output widget
    //me_output->insertLine(temp);

    //b_start->setEnabled(true);
    //b_old_map->setEnabled(true);
    return true;
}

/*
// enabled/disable buttons depending on the current program state
void ServerDialog::checkGui()
{
	if (server) 
	{
		file->setItemEnabled(0, true);
		file->setItemEnabled(1, false);
		file->setItemEnabled(2, false);
		file->setItemEnabled(3, true);				
	}
	else file->setItemEnabled(0, false);
}

void ServerDialog::new_client_connected(int socket)
{
	// create output string containing socket number of new client
	QString temp;
	temp.setNum(socket);
	temp.prepend("New client connected at: ");
	
	// output line
	me_output->insertLine(temp);
}
*/

// start the previously created server
bool ServerDialog::startClicked(PG_Button* btn)
{
    debug("start()");
	
	//TBD create a new map
	//server->send_map();			    // send map to all players
	//server->send_playerorder();		// send order to all players

    return true;
}
/*

void ServerDialog::add_playerstatus(int socket, QString name)
{
	//cerr << "add_playerstatus()\n";
	playerlist.append(new PlayerStatus(socket, name, vbox_playerstatus));
}

void ServerDialog::del_playerstatus(int socket)
{
	//cerr << "del_playerstatus()\n";
	PlayerStatus* ps;
	
	for (ps = playerlist.first(); ps != 0; ps = playerlist.next())
	{
		if (ps->get_socket() == socket)	break;
	}
		
	playerlist.remove(ps);
}
*/

bool ServerDialog::closeClicked(PG_Button* btn)
{
    /* TBD
    server->stopAcceptingClients();
    */
    QuitModal();
    return true;
}
