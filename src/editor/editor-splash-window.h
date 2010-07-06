//  Copyright (C) 2010 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
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

#ifndef EDITOR_SPLASH_WINDOW_H
#define EDITOR_SPLASH_WINDOW_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>

class EditorSplashWindow: public sigc::trackable
{
 public:
    EditorSplashWindow();
    ~EditorSplashWindow();

    int run();

    void hide();
    
 private:
    Gtk::Window * window;
    Gtk::ProgressBar *progressbar;
};

#endif