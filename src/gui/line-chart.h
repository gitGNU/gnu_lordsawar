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

protected:
    //Override default signal handler:
    virtual bool on_expose_event(GdkEventExpose* event);
private:
    std::list<std::list<unsigned int> > d_lines;
    std::list<Gdk::Color> d_colours;
    unsigned int d_max_height_value;
};

#endif
