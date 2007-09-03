#ifndef BAR_CHART_H
#define BAR_CHART_H

#include <gtkmm/drawingarea.h>
#include <list>

class BarChart: public Gtk::DrawingArea
{
public:
    BarChart(std::list<unsigned int> bars, std::list<Gdk::Color> colours);
    virtual ~BarChart();

protected:
    //Override default signal handler:
    virtual bool on_expose_event(GdkEventExpose* event);
private:
    std::list<unsigned int> d_bars;
    std::list<Gdk::Color> d_colours;
};

#endif
