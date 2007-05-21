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

#ifndef DESTINATION_DIALOG_H
#define DESTINATION_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/togglebutton.h>

#include "../vectormap.h"

struct SDL_Surface;

// dialog for choosing the destination of the production of a city
class DestinationDialog: public sigc::trackable
{
 public:
    DestinationDialog(City *city);

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    std::auto_ptr<VectorMap> vectormap;

    Gtk::Image *map_image;
    Gtk::ToggleButton *see_all_toggle;
    Gtk::ToggleButton *vector_toggle;
    Gtk::ToggleButton *change_toggle;
    Gtk::Label *current_label;
    Gtk::Image *current_image;
    Gtk::Label *turns_label;
    Gtk::Image *one_turn_away_image;
    Gtk::Image *two_turns_away_image;
    Gtk::Image *next_turn_1_image;
    Gtk::Image *next_turn_2_image;
    Gtk::Image *next_turn_3_image;
    Gtk::Image *next_turn_4_image;
    Gtk::Image *turn_after_1_image;
    Gtk::Image *turn_after_2_image;
    Gtk::Image *turn_after_3_image;
    Gtk::Image *turn_after_4_image;
    
    City *city;
    
    void on_map_changed(SDL_Surface *map);
    bool on_map_mouse_button_event(GdkEventButton *e);
    void on_see_all_toggled(Gtk::ToggleButton *toggle);
    void on_vector_toggled(Gtk::ToggleButton *toggle);
    void on_change_toggled(Gtk::ToggleButton *toggle);
    void fill_in_vectoring_info();
};

#endif
