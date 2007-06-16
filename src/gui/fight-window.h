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

#ifndef FIGHT_WINDOW_H
#define FIGHT_WINDOW_H

#include <memory>
#include <vector>
#include <list>
#include <sigc++/trackable.h>
#include <glibmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/label.h>
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

    void run();
    
 private:
    std::auto_ptr<Gtk::Window> window;
    Gtk::Label *rounds_label;
    static const int max_cols = 8;

    struct ArmyItem
    {
	Army *army;
	int hp;
	Gtk::ProgressBar *bar;
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
    void add_army(Army *army, std::vector<Gtk::HBox *> &hboxes,
		  Gtk::VBox *vbox,
		  int current_no, int max_rows, Gtk::AlignmentEnum alignment);

    bool do_round();
};

#endif
