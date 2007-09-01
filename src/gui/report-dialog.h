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

#ifndef REPORT_DIALOG_H
#define REPORT_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/notebook.h>
#include <gtkmm/button.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>

#include "../ObjectList.h"
#include "../vectormap.h"
#include "../citymap.h"
#include "../armymap.h"
#include "../player.h"

class Action;
struct SDL_Surface;

//
//
class ReportDialog: public sigc::trackable
{
 public:
    enum ReportType {ARMY = 0, CITY, GOLD, PRODUCTION, WINNING};

    ReportDialog(Player *player, ReportType type);

    void set_parent_window(Gtk::Window &parent);

    void run();
    
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

    Player *d_player;
    void on_army_map_changed(SDL_Surface *map);
    void on_city_map_changed(SDL_Surface *map);
    void on_vector_map_changed(SDL_Surface *map);
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
};

#endif
