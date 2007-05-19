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

#ifndef CITY_WINDOW_H
#define CITY_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <gtkmm/togglebutton.h>
#include "../vectormap.h"


class City;

// dialog for displaying a friendly city
class CityWindow: public sigc::trackable
{
 public:
    CityWindow(City *city);

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    std::auto_ptr<VectorMap> prodmap;
    Gtk::Image *map_image;
    Gtk::Label *city_label;
    Gtk::Label *status_label;
    Gtk::Label *production_info_label1;
    Gtk::Label *production_info_label2;
    Gtk::Button *buy_button;
    Gtk::Button *on_hold_button;

    City *city;

    std::vector<Gtk::ToggleButton *> production_toggles;
    bool ignore_toggles;

    void fill_in_city_info();
    void fill_in_production_toggles();
    void fill_in_production_info();
    void set_buy_button_state();

    void on_production_toggled(Gtk::ToggleButton *toggle);
    void on_on_hold_clicked();
    void on_buy_clicked();
    void on_destination_clicked();
    void on_map_changed(SDL_Surface *map);
};

#endif
