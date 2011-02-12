//  Copyright (C) 2008, 2011 Ben Asselstine
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

#ifndef DECORATED_H
#define DECORATED_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include "GraphicsCache.h"

class Decorated: public sigc::trackable
{
 public:
    Decorated();
    virtual ~Decorated();

    void decorate(Gtk::Window *window, GraphicsCache::BackgroundType back = GraphicsCache::GAME_BACKGROUND, int alpha = 255);

    void decorate_border(Gtk::Viewport *container, int alpha = 255);

    void set_title(std::string title);

    sigc::signal<void> window_closed;

 private:
    Gtk::Window *window;
    Gtk::Label *title;

    bool on_mouse_motion_event(GdkEventMotion *e);

    void on_maximize();
    void on_hide();
  
    bool maximized;
    Gtk::Button *maximize_button;
};

#endif
