//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include <config.h>

#include <gtkmm.h>

#include "stack-info-tip.h"

#include "glade-helpers.h"
#include "image-helpers.h"

#include "ucompose.hpp"
#include "vector.h"
#include "defs.h"
#include "stack.h"
#include "GraphicsCache.h"
#include "map-tip-position.h"
#include "decorated.h"
#include "File.h"

StackInfoTip::StackInfoTip(Gtk::Widget *target, MapTipPosition mpos, const Stack *stack)
{
    GraphicsCache *gc = GraphicsCache::getInstance();
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/stack-info-window.ui");

    Gtk::Window *w = 0;
    xml->get_widget("window", w);
    window.reset(w);
    Decorated decorator;
    decorator.decorate(window.get(),File::getMiscFile("various/background.png"), 200);

    xml->get_widget("image_hbox", image_hbox);

    //fill up the hbox with images of the armies in the stack
    for (Stack::const_iterator it = stack->begin(); it != stack->end(); it++)
      image_hbox->add(*manage(new Gtk::Image(gc->getArmyPic(*it))));

    image_hbox->show_all();

    // move into correct position
    window->get_child()->show();
    Vector<int> p(0, 0);
    target->get_window()->get_origin(p.x, p.y);
    if (target->has_no_window())
    {
	Gtk::Allocation a = target->get_allocation();
	p.x += a.get_x();
	p.y += a.get_y();
    }
    Vector<int> size(0, 0);
    window->get_size(size.x, size.y);
    switch (mpos.justification)
    {
    case MapTipPosition::LEFT:
	window->set_gravity(Gdk::GRAVITY_NORTH_WEST);
	break;
    case MapTipPosition::RIGHT:
	window->set_gravity(Gdk::GRAVITY_NORTH_EAST);
	p.x -= size.x;
	break;
    case MapTipPosition::TOP:
	window->set_gravity(Gdk::GRAVITY_NORTH_WEST);
	break;
    case MapTipPosition::BOTTOM:
	window->set_gravity(Gdk::GRAVITY_SOUTH_WEST);
	p.y -= size.y;
	break;
    }

    p += mpos.pos;
	
    window->move(p.x, p.y);
    window->show();
}
