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

#ifndef REPORT_DIALOG_H
#define REPORT_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "LocationList.h"
#include "vectormap.h"
#include "citymap.h"
#include "armymap.h"
#include "player.h"

class Action;
class BarChart;

#include "decorated.h"

//
//
class ReportDialog: public Decorated
{
 public:
    enum ReportType {ARMY = 0, CITY, GOLD, PRODUCTION, WINNING};

    ReportDialog(Player *player, ReportType type);

    void set_parent_window(Gtk::Window &parent);

    static std::string calculateRank(std::list<guint32> scores, guint32 score);
    void run();
    void hide();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    std::auto_ptr<VectorMap> vectormap;
    std::auto_ptr<ArmyMap> armymap;
    std::auto_ptr<CityMap> citymap;

    Gtk::Image *map_image;
    
    Gtk::Label *army_label;
    Gtk::Label *city_label;
    Gtk::Label *gold_label;
    Gtk::Label *production_label;
    Gtk::Label *winning_label;
    Gtk::Notebook *report_notebook;
    Gtk::Alignment *army_alignment;
    BarChart *army_chart;
    Gtk::Alignment *city_alignment;
    BarChart *city_chart;
    Gtk::Alignment *gold_alignment;
    BarChart *gold_chart;
    Gtk::Alignment *winning_alignment;
    BarChart *winning_chart;

    Player *d_player;
    void on_army_map_changed(Glib::RefPtr<Gdk::Pixmap> map);
    void on_city_map_changed(Glib::RefPtr<Gdk::Pixmap> map);
    void on_vector_map_changed(Glib::RefPtr<Gdk::Pixmap> map);
    void on_switch_page(GtkNotebookPage *page, guint number);

    Gtk::TreeView *armies_treeview;

    class ArmiesColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmiesColumns() 
        { add(image); add(desc);}
	
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
	Gtk::TreeModelColumn<Glib::ustring> desc;
    };
    const ArmiesColumns armies_columns;
    Glib::RefPtr<Gtk::ListStore> armies_list;
    void addProduction(const Action *action);
    void on_close_button();
    bool closing;
    void fill_in_info();
    void updateArmyChart();
    void updateCityChart();
    void updateGoldChart();
    void updateWinningChart();
};

#endif
