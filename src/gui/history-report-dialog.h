//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#ifndef HISTORY_REPORT_DIALOG_H
#define HISTORY_REPORT_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "line-chart.h"

#include "LocationList.h"
#include "historymap.h"
#include "history.h"
#include "player.h"
class Citylist;

class Player;
#include "decorated.h"
// dialog for showing all ruins and temples
// the stack parameter is used as a starting position for showing ruins
class HistoryReportDialog: public Decorated
{
 public:
    enum HistoryReportType {CITY = 0, EVENTS, GOLD, WINNING};
    HistoryReportDialog(Player *p, HistoryReportType type);
    ~HistoryReportDialog();

    void generatePastCitylists(); //data for map
    void generatePastCityCounts(); //data for chart
    void generatePastGoldCounts(); //data for chart
    void generatePastWinningCounts(); //data for chart
    void generatePastEventlists(); //data for treeview
    void set_parent_window(Gtk::Window &parent);

    void run();
    void hide();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    std::auto_ptr<HistoryMap> historymap;

    Player *d_player;
    Gtk::Scale *turn_scale;
    Gtk::Notebook *history_notebook;
    Gtk::Label *city_label;
    Gtk::Label *gold_label;
    Gtk::Label *winner_label;
    Gtk::Alignment *city_alignment;
    Gtk::Alignment *gold_alignment;
    Gtk::Alignment *winner_alignment;

    std::vector<LocationList<City*>* > past_citylists;
    LineChart *city_chart;
    std::vector<std::list<NetworkHistory *> > past_eventlists;
    std::list<std::list<guint32> > past_citycounts;
    std::list<std::list<guint32> > past_goldcounts;
    LineChart *gold_chart;
    std::list<std::list<guint32> > past_rankcounts;
    LineChart *rank_chart;

    Gtk::Image *map_image;
  
    std::list<Gdk::Color> d_colours; //player colours
    
    Gtk::TreeView *events_treeview;


    class EventsColumns: public Gtk::TreeModelColumnRecord {
    public:
	EventsColumns() 
        { add(image); add(desc);}
	
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixmap> > image;
	Gtk::TreeModelColumn<Glib::ustring> desc;
    };
    const EventsColumns events_columns;
    Glib::RefPtr<Gtk::ListStore> events_list;
    void addHistoryEvent(NetworkHistory *event);
    void on_close_button();
    void on_map_changed(Glib::RefPtr<Gdk::Pixmap> map);
    void on_turn_changed(Gtk::Scale *scale);
    void fill_in_turn_info(guint32 turn);
    void on_switch_page(GtkNotebookPage *page, guint number);
    void update_window_title();

    bool closing;
};

#endif
