//  Copyright (C) 2007, 2008, 2009, 2012, 2014 Ben Asselstine
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

#include <vector>
#include <gtkmm.h>

#include "line-chart.h"

#include "LocationList.h"
#include "historymap.h"
#include "history.h"
#include "player.h"
#include "lw-dialog.h"

class Citylist;
class City;
class Player;

class HistoryReportDialog: public LwDialog
{
 public:
    enum HistoryReportType {CITY = 0, RUIN, EVENTS, GOLD, WINNING};
    HistoryReportDialog(Gtk::Window &parent, Player *p, HistoryReportType type);
    ~HistoryReportDialog();

    void generatePastCitylists(); //data for map
    void generatePastCityCounts(); //data for chart
    void generatePastRuinlists(); //data for map
    void generatePastRuinCounts(); //data for chart
    void generatePastGoldCounts(); //data for chart
    void generatePastWinningCounts(); //data for chart
    void generatePastEventlists(); //data for events list

    void run();
    void hide();
    
 private:
    HistoryMap* historymap;

    Player *d_player;
    Gtk::Scale *turn_scale;
    Gtk::Notebook *history_notebook;
    Gtk::Label *city_label;
    Gtk::Label *ruin_label;
    Gtk::Label *gold_label;
    Gtk::Label *winner_label;
    Gtk::Alignment *city_alignment;
    Gtk::Alignment *ruin_alignment;
    Gtk::Alignment *gold_alignment;
    Gtk::Alignment *winner_alignment;

    std::vector<LocationList<City*>* > past_citylists;
    LineChart *city_chart;
    std::vector<std::list<NetworkHistory *> > past_eventlists;
    std::list<std::list<guint32> > past_citycounts;
    std::vector<LocationList<Ruin*>* > past_ruinlists;
    LineChart *ruin_chart;
    std::list<std::list<guint32> > past_ruincounts;
    std::list<std::list<guint32> > past_goldcounts;
    LineChart *gold_chart;
    std::list<std::list<guint32> > past_rankcounts;
    LineChart *rank_chart;

    Gtk::Image *map_image;
  
    std::list<Gdk::RGBA> d_colours; //player colours
    
    Gtk::VBox *events_list_box;

    void addHistoryEvent(NetworkHistory *event);
    void on_map_changed(Cairo::RefPtr<Cairo::Surface> map);
    void on_turn_changed(Gtk::Scale *scale);
    void fill_in_turn_info(guint32 turn);
    void on_switch_page(Gtk::Widget *page, guint number);
    void update_window_title();
};

#endif
