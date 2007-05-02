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

#ifndef MULTIPLAYERMODEDIALOG_H
#define MULTIPLAYERMODEDIALOG_H

#include <pgwindow.h>
#include <pglineedit.h>
#include "GameScenario.h"
#include <string>

class PG_LineEdit;
//! This class is not in use; ignore it for now

class MultiPlayerModeDialog : public PG_Window
{
	public:
	// CREATORS
	MultiPlayerModeDialog(PG_Widget* parent, Rectangle rect);
	~MultiPlayerModeDialog();

        // MANIPULATORS
        void initGUI();

	// CALLBACK FUNCTIONS
	bool clientClicked(PG_Button* btn);
	bool serverClicked(PG_Button* btn);
	bool startClicked(PG_Button* btn);
	bool closeClicked(PG_Button* btn);

	// ACCESSORS
	bool getMode(){return d_mode;}
        bool getResult() {return d_result;}
        std::string getIP() {return d_ip;}
        int getPort() {return d_port;}

	private:
	// WIDGETS
	PG_Button* d_b_client;
	PG_Button* d_b_server;
	PG_Button* d_b_start;
	PG_Button* d_b_close;
	PG_Label* d_help;
	PG_Label* d_label;
	PG_LineEdit* d_edit;

	// DATA
        std::string d_ip;
        int d_port;
	bool d_mode;
        bool d_result;    // closed = false, ok = true
};

#endif

// End of file
