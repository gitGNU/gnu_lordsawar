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
#ifndef BAR_CHART_H
#define BAR_CHART_H

#include <gtkmm.h>
#include <list>

class BarChart: public Gtk::DrawingArea
{
public:
    BarChart(std::list<unsigned int> bars, std::list<Gdk::Color> colours,
	     unsigned int max_value);
    virtual ~BarChart();

protected:
    //Override default signal handler:
    virtual bool on_expose_event(GdkEventExpose* event);
private:
    std::list<unsigned int> d_bars;
    std::list<Gdk::Color> d_colours;
    unsigned int d_max_value;
};

#endif
