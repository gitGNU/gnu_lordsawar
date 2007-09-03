#include "line-chart.h"
#include <cairomm/context.h>

LineChart::LineChart(std::list<std::list<unsigned int> > lines, std::list<Gdk::Color> colours, unsigned int max_height_value)
{
  d_lines = lines;
  d_colours = colours;
  d_max_height_value = max_height_value;
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

    Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
    cr->set_line_width(1.0);

    // clip to the area indicated by the expose event so that we only redraw
    // the portion of the window that needs to be redrawn
    cr->rectangle(event->area.x, event->area.y,
            event->area.width, event->area.height);
    cr->clip();
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
	cr->move_to(origin_x, origin_y);
	for (; it != (*line).end(); it++, turn++)
	  {
	    cr->line_to(((float)turn / (float)max_turn) * width,
			(height - (((float)*it / (float)d_max_height_value) * height)));
	  }
	cr->stroke();
      }

  }

  //FIXME: add a horizontal axis, put some ticks on it, and label it.
  //FIXME: add a vertical axis, put some ticks on it, and label it.
  return true;
}
