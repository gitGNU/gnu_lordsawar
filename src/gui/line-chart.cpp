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

#include "line-chart.h"
#include <cairomm/context.h>

LineChart::LineChart(std::list<std::list<unsigned int> > lines, std::list<Gdk::Color> colours, unsigned int max_height_value)
{
  d_lines = lines;
  d_colours = colours;
  d_max_height_value = max_height_value;
  d_x_indicator = -1;
}

LineChart::~LineChart()
{
}

bool LineChart::on_expose_event(GdkEventExpose* event)
{
  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if(window)
  {
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();

    // coordinates for the center of the window
    int xc, yc;
    xc = width / 2;
    yc = height / 2;
    int origin_x = 0;
    int origin_y = height;
    unsigned int hoffs = 30;
    unsigned int voffs = 30;

    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
      
    // clip to the area indicated by the expose event so that we only redraw
    // the portion of the window that needs to be redrawn
    cr->rectangle(event->area.x, event->area.y,
            event->area.width, event->area.height);
    cr->clip();
    cr->set_source_rgb (0.8, 0.8, 0.8);
    cr->set_line_width(1000.0);
    cr->move_to(0,0);
    cr->line_to(width,height);
    cr->stroke();

    cr->set_line_width(1.0);
    //loop through the outer list, and operate on the inner lists
    unsigned int max_turn = 0;
    std::list<std::list<unsigned int> >::iterator line = d_lines.begin();
    for (; line!= d_lines.end(); line++)
      {
	if ((*line).size() > max_turn)
	  max_turn = (*line).size();
      }

    if (d_max_height_value == 0)
      {
	line = d_lines.begin();
	for (; line!= d_lines.end(); line++)
	  {
	    std::list<unsigned int>::iterator it = (*line).begin();
	    for (; it != (*line).end(); it++)
	      {
		if (*it > d_max_height_value)
		  d_max_height_value = *it;
	      }
	  }
      }
    // ensure the border is big enough for the label.
    Glib::RefPtr<Pango::Layout> layout = Glib::wrap (pango_cairo_create_layout (cr->cobj ()));
    std::string text_font = "Sans 8";
    Pango::FontDescription font_desc (text_font);
    layout->set_font_description (font_desc);
    char buf[15];
    snprintf (buf, sizeof (buf), "%d", d_max_height_value);
    layout->set_text(buf);
    int w, h;
    layout->get_pixel_size (w, h);
    if (w  > hoffs)
      hoffs = w;

    std::list<Gdk::Color>::iterator cit = d_colours.begin();
    line = d_lines.begin();
    for (; line!= d_lines.end(), cit != d_colours.end(); line++, cit++)
      {
	//okay, here's my line and it's colour,
	double red = (double)(*cit).get_red() /65535.0;
	double green = (double)(*cit).get_green() /65535.0;
	double blue = (double)(*cit).get_blue() /65535.0;
	cr->set_source_rgb(red, green, blue);
	std::list<unsigned int>::iterator it = (*line).begin();
	unsigned int turn = 1;
	cr->move_to(origin_x + hoffs, origin_y - voffs);
	for (; it != (*line).end(); it++, turn++)
	  {
	    cr->line_to((((float)turn / (float)max_turn) * (width - (hoffs * 2))) + hoffs,
	    (((float)*it / (float)d_max_height_value) * (height - (voffs * 2))) + voffs);
			//(height + voffs - (((float)*it / (float)d_max_height_value) * height - (voffs * 2) )));
	  }
	cr->stroke();
      }

    //draw horizontal axis
    cr->set_source_rgb(0.3, 0.3, 0.3);
    cr->move_to(origin_x + hoffs, origin_y - voffs);
    cr->line_to((width - (hoffs * 2)) + hoffs, origin_y - voffs);
    cr->stroke();

    //draw ticks on the horizontal axis
    cr->set_source_rgb(0.3, 0.3, 0.3);
    cr->move_to(origin_x + hoffs, origin_y - voffs + 1);
    cr->line_to(origin_x + hoffs, origin_y - voffs + (voffs / 4) + 1);
    cr->stroke();

    cr->move_to((width - (hoffs * 2)) + hoffs - 1, origin_y - voffs + 1);
    cr->line_to((width - (hoffs * 2)) + hoffs - 1, 
		origin_y - voffs + (voffs / 4) + 1);
    cr->stroke();

    //draw vertical axis
    cr->set_source_rgb(0.3, 0.3, 0.3);
    cr->move_to(origin_x + hoffs, origin_y - voffs);
    cr->line_to(origin_x + hoffs, voffs);
    cr->stroke();

    //draw ticks on the vertical axis
    cr->move_to(origin_x + hoffs - 1, origin_y - voffs);
    cr->line_to(origin_x + hoffs - (hoffs / 4) - 1, origin_y - voffs);
    cr->stroke();
    cr->move_to(origin_x + hoffs - 1, voffs + 1);
    cr->line_to(origin_x + hoffs - (hoffs / 4) - 1, voffs + 1);
    cr->stroke();

    // draw the labels on the horizontal axis
    layout->set_font_description (font_desc);
    layout->set_text("0");
    layout->get_pixel_size (w, h);
    cr->move_to(hoffs - (w / 2), origin_y - voffs + (voffs / 4));
    cr->set_source_rgb (0.0, 0.0, 0.0);
    cr->set_operator (Cairo::OPERATOR_ATOP);
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());

    snprintf (buf, sizeof (buf), "%d", max_turn);
    layout->set_text(buf);
    layout->get_pixel_size (w, h);
    cr->move_to((width - (hoffs * 2)) + hoffs - (w / 2), 
		origin_y - voffs + (voffs / 4) + 1);
    cr->set_source_rgb (0.0, 0.0, 0.0);
    cr->set_operator (Cairo::OPERATOR_ATOP);
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());

    layout->set_text("Turns");
    layout->get_pixel_size (w, h);
    cr->move_to((width / 2 - (hoffs * 1)) + hoffs - (w / 2), 
		origin_y - voffs + (voffs / 2) + 1);
    cr->set_source_rgb (0.0, 0.0, 0.0);
    cr->set_operator (Cairo::OPERATOR_ATOP);
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());

    // draw the labels on the vertical axis
    layout->set_font_description (font_desc);
    layout->set_text("0");
    layout->get_pixel_size (w, h);
    cr->move_to(origin_x + hoffs - (hoffs / 4) - 1 - w, origin_y - voffs - (h/2));
    cr->set_source_rgb (0.0, 0.0, 0.0);
    cr->set_operator (Cairo::OPERATOR_ATOP);
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());

    snprintf (buf, sizeof (buf), "%d", d_max_height_value);
    layout->set_text(buf);
    layout->get_pixel_size (w, h);
    cr->move_to(origin_x + hoffs - (hoffs / 4) - 1 - w, voffs + 1 - (h/2));
    cr->set_source_rgb (0.0, 0.0, 0.0);
    cr->set_operator (Cairo::OPERATOR_ATOP);
    pango_cairo_show_layout (cr->cobj (), layout->gobj ());

    //draw the indicator line
    if (d_x_indicator > -1 && d_x_indicator <= max_turn)
      {
	//draw a line at turn x
	cr->set_source_rgb(0.0, 0.0, 0.0);
	cr->move_to((((float)d_x_indicator/ (float)max_turn) * (width - (hoffs * 2))) + hoffs, voffs);
	cr->line_to((((float)d_x_indicator/ (float)max_turn) * (width - (hoffs * 2))) + hoffs, height - voffs);
	cr->stroke();
      }
  }

  //FIXME: add a horizontal axis, put some ticks on it, and label it.
  //FIXME: add a vertical axis, put some ticks on it, and label it.
  return true;
}

void LineChart::set_x_indicator(int x)
{
  d_x_indicator = x;
  Glib::RefPtr<Gdk::Window> window = get_window();
  if(window)
    {
      Gtk::Allocation allocation = get_allocation();
      const int width = allocation.get_width();
      const int height = allocation.get_height();
      GdkEventExpose event;
      event.area.x = 0;
      event.area.y = 0;
      event.area.height = height;
      event.area.width = width;
      on_expose_event(&event);
    }
}
