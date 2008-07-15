//  Copyright (C) 2008 Ben Asselstine
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#ifndef TIMED_MESSAGE_DIALOG_H
#define TIMED_MESSAGE_DIALOG_H

#include <sigc++/trackable.h>
#include <glibmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/messagedialog.h>
#include <string>

// used for displaying a timed dialog that goes away after a period of time
class TimedMessageDialog: public sigc::trackable
{
 public:
    TimedMessageDialog(Gtk::Window &parent, std::string message, int timeout,
		       int grace = 30);
    ~TimedMessageDialog();

    void show_all();
    void run();
    void hide();
    
 private:
    Gtk::MessageDialog *window;
    Glib::RefPtr<Glib::MainLoop> main_loop;
    bool tick();
    void on_response(int id);
    int d_timeout;
    int d_timer_count;
    int d_grace;
};

#endif
