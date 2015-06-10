//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015 Ben Asselstine
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

#ifndef CITY_WINDOW_H
#define CITY_WINDOW_H

#include <vector>
#include <gtkmm.h>
#include "vectormap.h"
#include "army-info-tip.h"
#include "lw-dialog.h"

class City;

// dialog for displaying a friendly city
class CityWindow: public LwDialog
{
 public:
    CityWindow(Gtk::Window &parent, City *city, bool razing_possible, bool see_opponents_production);

    ~CityWindow();

    void run();
    void hide();
    
    static bool on_raze_clicked (City *city, Gtk::Dialog *parent);
    
 private:
    VectorMap* prodmap;
    ArmyInfoTip* army_info_tip;
    Gtk::Image *map_image;
    Glib::RefPtr<Gdk::Pixbuf> map_pixbuf;
    Gtk::Label *city_label;
    Gtk::Label *status_label;
    Gtk::Label *production_info_label1;
    Gtk::Label *production_info_label2;
    Gtk::Button *buy_button;
    Gtk::Button *on_hold_button;
    Gtk::Button *rename_button;
    Gtk::Button *destination_button;
    Gtk::Button *raze_button;
    Gtk::Label *turns_left_label;
    Gtk::Image *current_image;
    Gtk::Label *current_label;

    City *city;

    bool d_razing_possible;
    bool d_see_all;


    Gtk::Box *production_toggles_hbox;
    std::vector<Gtk::ToggleButton *> production_toggles;
    bool ignore_toggles;

    void fill_in_city_info();
    void fill_in_production_toggles();
    void fill_in_production_info();

    void on_production_toggled(Gtk::ToggleButton *toggle);
    bool on_production_button_event(GdkEventButton *e, Gtk::ToggleButton *toggle);
    void on_on_hold_clicked();
    void on_buy_clicked();
    void on_destination_clicked();
    void on_map_changed(Cairo::RefPtr<Cairo::Surface> map);
    bool on_map_mouse_button_event(GdkEventButton *e);
    void on_rename_clicked ();
    void on_raze_clicked ();

    void update_toggle_picture(int slot);
};

#endif
