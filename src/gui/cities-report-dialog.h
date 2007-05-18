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

#ifndef CITIES_REPORT_DIALOG_H
#define CITIES_REPORT_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>

#include "../vector.h"

class City;
class Player;

// dialog for showing the cities, emits city_selected when one is selected by
// the user (but keeps the dialog open until "close" is pressed)
class CitiesReportDialog: public sigc::trackable
{
 public:
    CitiesReportDialog(Player *player);

    void set_parent_window(Gtk::Window &parent);

    void run();

    sigc::signal<void, City *> city_selected;
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;

    Player *player;
    Gtk::TreeView *cities_treeview;

    class CitiesColumns: public Gtk::TreeModelColumnRecord {
    public:
	CitiesColumns() 
        { add(name); add(income); add(player);
	  add(production_image); add(production); add(production_progress);
	  add(city); }
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Glib::ustring> income;
	Gtk::TreeModelColumn<Glib::ustring> player;
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > production_image;
	Gtk::TreeModelColumn<Glib::ustring> production;
	Gtk::TreeModelColumn<Glib::ustring> production_progress;
	Gtk::TreeModelColumn<City *> city;
    };
    const CitiesColumns cities_columns;
    Glib::RefPtr<Gtk::ListStore> cities_list;

    void on_selection_changed();

    void add_city(City &city);
};

#endif
