//  Copyright (C) 2007, 2008, Ole Laursen
//  Copyright (C) 2007, 2008 Ben Asselstine
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#ifndef FIGHT_WINDOW_H
#define FIGHT_WINDOW_H

#include <memory>
#include <vector>
#include <list>
#include <sigc++/trackable.h>
#include <glibmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/alignment.h>

#include "../game-parameters.h"
#include "../fight.h"

class Fight;
class Army;

// window for displaying the course of a fight
class FightWindow: public sigc::trackable
{
 public:
    FightWindow(Fight &fight);
    ~FightWindow();

    void set_parent_window(Gtk::Window &parent);

    void run(bool *quick);
    
 private:
    std::auto_ptr<Gtk::Window> window;
    static const int normal_round_speed = 5;//00; //milliseconds
    static const int max_cols = 8;
    static const int fast_round_speed = 2;//50; //after a key is pressed

    struct ArmyItem
    {
	Army *army;
	int hp;
	Gtk::ProgressBar *bar;
	Gtk::Image *image;
        bool exploding;
    };
    
    typedef std::vector<ArmyItem> army_items_type;
    army_items_type army_items;

    typedef std::list<FightItem> actions_type;
    actions_type actions;

    typedef std::vector<Army *> armies_type; // for convenience

    Glib::RefPtr<Glib::MainLoop> main_loop;
    
    int round;
    actions_type::iterator action_iterator;
    
    // determine the max no. of rows in each column
    int compute_max_rows(const armies_type &attackers,
			 const armies_type &defenders);
    
    // add an army to the window
    void add_army(Army *army, int initial_hp,
                  std::vector<Gtk::HBox *> &hboxes,
		  Gtk::VBox *vbox,
		  int current_no, int max_rows, Gtk::AlignmentEnum alignment);

    void on_key_release_event(GdkEventKey* event);

    bool do_round();
    bool d_quick;
};

#endif
