#include "bar-chart.h"
#include <cairomm/context.h>

BarChart::BarChart(std::list<unsigned int> bars, std::list<Gdk::Color> colours)
{
  d_bars = bars;
  d_colours = colours;
}

BarChart::~BarChart()
{
}

bool BarChart::on_expose_event(GdkEventExpose* event)
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

    unsigned int lw = 10;
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
    cr->set_line_width((double)lw);

    unsigned int max = 0;
    std::list<unsigned int>::iterator bit = d_bars.begin();
    for (; bit != d_bars.end(); bit++)
      {
	if (*bit > max)
	  max = *bit;
      }

    unsigned int offs = 15;
    unsigned int d = 10;
    cr->move_to(0, 0);
    bit = d_bars.begin();
    std::list<Gdk::Color>::iterator cit = d_colours.begin();
    unsigned int i = 0;
    for (; bit != d_bars.end(), cit != d_colours.end(); bit++, cit++, i+=(lw+d))
      {
        cr->move_to(0, i + lw + offs);
	double red = (double)(*cit).get_red() /65535.0;
	double green = (double)(*cit).get_green() /65535.0;
	double blue = (double)(*cit).get_blue() /65535.0;
	cr->set_source_rgb(red, green, blue);
	cr->set_source_rgb(red, green, blue);
	cr->line_to(((float) *bit / (float)max) * width, i + lw + offs);
	cr->stroke();
      }
  }

  //FIXME: add a horizontal axis, put some ticks on it, and label it.
  return true;
}
