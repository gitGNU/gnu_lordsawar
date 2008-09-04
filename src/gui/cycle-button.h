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

#ifndef CYCLE_BUTTON_H
#define CYCLE_BUTTON_H

#include <sigc++/trackable.h>
#include <gtkmm/button.h>
#include <gtkmm/notebook.h>
#include <glibmm/ustring.h>
#include <sigc++/signal.h>
#include <string>
#include <list>

// used for displaying a timed dialog that goes away after a period of time
class CycleButton
{
 public:
    CycleButton(std::list<Gtk::Button*>states);
    ~CycleButton();

    Gtk::Widget *get_widget(){return d_notebook;};
    void show();
    void set_active(int row_number);
    Glib::ustring get_active_text();
    int get_active();
    int get_active_row_number(){return get_active();};
    void set_sensitive(bool sensitive) {d_notebook->set_sensitive(sensitive);};

    sigc::signal<void> signal_changed;
 private:
    Gtk::Notebook *d_notebook;
    void on_button_clicked();
};

#endif
