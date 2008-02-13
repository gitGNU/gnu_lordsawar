//  Copyright (C) 2007, 2008 Ben Asselstine
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

#ifndef LINE_CHART_H
#define LINE_CHART_H

#include <gtkmm/drawingarea.h>
#include <list>

//the first parameter to the construct might seem a bit weird.
//the outer list contains a list of numbers belonging to a player
//the inner list is just a list of numbers forming the y component of the graph.
//the x component is taken from the turn number, which is just that number's position in the list.
class LineChart: public Gtk::DrawingArea
{
public:
    LineChart(std::list<std::list<unsigned int> > lines, std::list<Gdk::Color> colours, unsigned int max_height_value);
    virtual ~LineChart();

    void set_x_indicator(int x);

protected:
    //Override default signal handler:
    virtual bool on_expose_event(GdkEventExpose* event);
private:
    std::list<std::list<unsigned int> > d_lines;
    std::list<Gdk::Color> d_colours;
    unsigned int d_max_height_value;
    int d_x_indicator;
};

#endif
