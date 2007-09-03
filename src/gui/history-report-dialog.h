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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef HISTORY_REPORT_DIALOG_H
#define HISTORY_REPORT_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>
#include <gtkmm/notebook.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <gtkmm/alignment.h>

#include "line-chart.h"

#include "../ObjectList.h"
#include "../historymap.h"
#include "../history.h"
#include "../player.h"
class Citylist;

struct SDL_Surface;
class Player;
// dialog for showing all ruins and temples
// the stack parameter is used as a starting position for showing ruins
class HistoryReportDialog: public sigc::trackable
{
 public:
    enum HistoryReportType {CITY = 0, EVENTS, GOLD, WINNING};
    HistoryReportDialog(Player *p, HistoryReportType type);

    void generatePastCitylists(); //data for map
    void generatePastCityCounts(); //data for chart
    void generatePastGoldCounts(); //data for chart
    void generatePastWinningCounts(); //data for chart
    void generatePastEventlists(); //data for treeview
    void set_parent_window(Gtk::Window &parent);

    void run();
    
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

    std::vector<ObjectList<City>* > past_citylists;
    LineChart *city_chart;
    std::vector<std::list<History *> > past_eventlists;
    std::list<std::list<Uint32> > past_citycounts;
    std::list<std::list<Uint32> > past_goldcounts;
    LineChart *gold_chart;
    std::list<std::list<Uint32> > past_rankcounts;
    LineChart *rank_chart;

    Gtk::Image *map_image;
  
    std::list<Gdk::Color> d_colours; //player colours
    
    Gtk::TreeView *events_treeview;


    class EventsColumns: public Gtk::TreeModelColumnRecord {
    public:
	EventsColumns() 
        { add(image); add(desc);}
	
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
	Gtk::TreeModelColumn<Glib::ustring> desc;
    };
    const EventsColumns events_columns;
    Glib::RefPtr<Gtk::ListStore> events_list;
    void addHistoryEvent(History *event);
    void on_close_button();
    void on_map_changed(SDL_Surface *map);
    void on_turn_changed(Gtk::Scale *scale);
    void fill_in_turn_info(Uint32 turn);
    void on_switch_page(GtkNotebookPage *page, guint number);

    bool closing;
};

#endif
