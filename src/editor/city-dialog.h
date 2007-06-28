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

#ifndef CITY_DIALOG_H
#define CITY_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>


class City;
class Army;

class CityDialog: public sigc::trackable
{
 public:
    CityDialog(City *city);

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::ComboBoxText *player_combobox;
    Gtk::CheckButton *capital_checkbutton;
    Gtk::Entry *name_entry;
    Gtk::SpinButton *income_spinbutton;
    Gtk::CheckButton *burned_checkbutton;

    Gtk::TreeView *army_treeview;
    
    class ArmyColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmyColumns()
	    { add(army); add(name);
	      add(strength); add(moves); add(hitpoints); add(upkeep);
	      add(duration); }

	Gtk::TreeModelColumn<const Army *> army;
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<int> strength, moves, hitpoints, upkeep, duration;
    };
    const ArmyColumns army_columns;
    Glib::RefPtr<Gtk::ListStore> army_list;
    Gtk::Button *add_button;
    Gtk::Button *remove_button;

    City *city;

    void on_add_clicked();
    void on_remove_clicked();
    void on_selection_changed();

    void add_army(const Army *a);
    void set_button_sensitivity();
};

#endif
