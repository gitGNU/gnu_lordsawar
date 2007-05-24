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

#ifndef LOAD_SCENARIOS_DIALOG_H
#define LOAD_SCENARIOS_DIALOG_H

#include <memory>
#include <string>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>

class XML_Helper;

// dialog for choosing a scenario
class LoadScenarioDialog: public sigc::trackable
{
 public:
    LoadScenarioDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();

    std::string get_scenario_filename();

 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::Button *load_button;
    Gtk::Label *name_label;
    Gtk::Label *description_label;
    Gtk::TreeView *scenarios_treeview;

    class ScenariosColumns: public Gtk::TreeModelColumnRecord {
    public:
	ScenariosColumns() 
        { add(name); add(filename); }
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<std::string> filename;
    };
    const ScenariosColumns scenarios_columns;
    Glib::RefPtr<Gtk::ListStore> scenarios_list;
    
    std::string selected_filename;
    
    void on_selection_changed();
    bool scan_scenario(std::string tag, XML_Helper* helper);
    void add_scenario(std::string filename);
};

#endif
