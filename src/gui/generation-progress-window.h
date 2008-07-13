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

#ifndef GENERATION_PROGRESS_WINDOW_H
#define GENERATION_PROGRESS_WINDOW_H

#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/statusbar.h>
#include <string>
#include <glibmm/main.h>
class MapGenerator;

// used for displaying a timed dialog that goes away after a period of time
class GenerationProgressWindow: public sigc::trackable
{
 public:
    GenerationProgressWindow();
    ~GenerationProgressWindow();

    void setGenerator(MapGenerator *generator);
    void update_progress(double fraction, std::string status);
    void show_all();
    
 private:
    std::auto_ptr<Gtk::Dialog> window;
    Gtk::ProgressBar *progressbar;
    Gtk::Statusbar *statusbar;
    Glib::RefPtr<Glib::MainLoop> main_loop;

    //bool on_delete_event(GdkEventAny *e);
};

#endif
