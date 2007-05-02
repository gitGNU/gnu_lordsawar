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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.*

#include <pglineedit.h>
#include "MultiPlayerModeDialog.h"
#include <pglabel.h>
#include <pgfilearchive.h>
#include "PG_FileDialog.h"
#include "GameScenario.h"
#include "MapConfDialog.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

MultiPlayerModeDialog::MultiPlayerModeDialog(PG_Widget* parent, Rectangle rect)
	:PG_Window(parent, rect, "Choose Server or Client mode",PG_Window::MODAL), d_mode(true)
{
	d_b_server = new PG_Button(this,Rectangle(20, 35, 80, 25), "Server",0);
	d_b_server->SetToggle(true);
	d_b_server->SetPressed(true);
	d_b_client = new PG_Button(this, Rectangle(110, 35, 80, 25), "Client",1);
	d_b_client->SetToggle(true);
	d_b_start = new PG_Button(this, Rectangle(20, 150, Width() - 40, 30), "Start",2);
	d_b_close = new PG_Button(this, Rectangle(20, 185, Width() - 40, 30), "Close",3);
	d_edit = new PG_LineEdit(this, Rectangle(100, 100, Width() - 140, 30));
	d_help = new PG_Label(this, Rectangle(20, 70, Width() - 40, 30), ""); 
	d_label = new PG_Label(this, Rectangle(20, 100, 60, 30), "");

	d_b_client->sigClick.connect(slot(*this, &MultiPlayerModeDialog::clientClicked));
	d_b_server->sigClick.connect(slot(*this, &MultiPlayerModeDialog::serverClicked));
	d_b_start->sigClick.connect(slot(*this, &MultiPlayerModeDialog::startClicked));
	d_b_close->sigClick.connect(slot(*this, &MultiPlayerModeDialog::closeClicked));

	initGUI();
}

void MultiPlayerModeDialog::initGUI()
{
    // server mode
    if (d_mode)
    {
        d_edit->SetText("23456");
        d_help->SetText("The port number where clients shall be accepted!");
        d_label->SetText("Port:");
    }
    // client mode
    else
    {
        d_edit->SetText("127.0.0.1:23456");
        d_help->SetText("Host to connect to! (e.g. 80.108.27.42:7654)");
        d_label->SetText("Host:Port:");
    }
}

MultiPlayerModeDialog::~MultiPlayerModeDialog()
{
    delete d_b_client;
    delete d_b_server;
    delete d_b_start;
    delete d_help;
    delete d_label;
    delete d_edit;
}

bool MultiPlayerModeDialog::clientClicked(PG_Button* btn)
{
    d_mode = false;
    d_b_server->SetPressed(false);
    initGUI();
	return true;
}

bool MultiPlayerModeDialog::serverClicked(PG_Button* btn)
{
    d_mode = true;
    d_b_client->SetPressed(false);
    initGUI();
	return true;
}

bool MultiPlayerModeDialog::startClicked(PG_Button* btn)
{
    d_result = true;
    if (d_mode)
    {
        d_port = atoi(d_edit->GetText());
    }
    else
    {
        string text = d_edit->GetText();
        string::size_type idx = text.find(":");
        d_ip = text.substr(0, idx);
        d_port = atoi(text.substr(idx + 1, text.length() - idx - 1).c_str());
    }
    QuitModal();
    return true;
}

bool MultiPlayerModeDialog::closeClicked(PG_Button* btn)
{
    d_result = false;
    QuitModal();
    return true;
}

// End of file
