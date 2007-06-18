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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.*

#include <config.h>

#include <assert.h>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <vector>
#include <libglademm/xml.h>
#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/alignment.h>

#include "fight-window.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "../ucompose.hpp"
#include "../timing.h"
#include "../File.h"
#include "../defs.h"
#include "../player.h"
#include "../playerlist.h"
#include "../stack.h"
#include "../army.h"
#include "../GraphicsCache.h"

namespace 
{
    int const FIGHT_ROUND_INTERVAL = 500; // milliseconds
}


FightWindow::FightWindow(Fight &fight)
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/fight-window.glade");

    Gtk::Window *d = 0;
    xml->get_widget("window", d);
    window.reset(d);
    
    Gtk::VBox *attacker_close_vbox;
    Gtk::VBox *defender_close_vbox;
    xml->get_widget("attacker_close_vbox", attacker_close_vbox);
    xml->get_widget("defender_close_vbox", defender_close_vbox);

    xml->get_widget("rounds_label", rounds_label);

    // extract attackers and defenders
    armies_type attackers, defenders;

    Player *attacker = NULL; /* for some reason attackers->getFront()->getPlayer() doesn't always yeild a player.  so we fill this variable instead */
    Player *defender = NULL;
    std::list<Stack *> l;
    l = fight.getAttackers();
    for (std::list<Stack *>::iterator i = l.begin(); i != l.end(); ++i)
        for (Stack::const_iterator si = (*i)->begin(); si != (*i)->end(); ++si)
          {
            if (!attacker && (*si)->getPlayer())
              attacker = (*si)->getPlayer();
	    attackers.push_back(*si);
          }
    
    l = fight.getDefenders();
    for (std::list<Stack *>::iterator i = l.begin(); i != l.end(); ++i)
        for (Stack::const_iterator si = (*i)->begin(); si != (*i)->end(); ++si)
          {
            if (!defender && (*si)->getPlayer())
              defender = (*si)->getPlayer();
	    defenders.push_back(*si);
          }

    int rows = compute_max_rows(attackers, defenders);
    
    // add the armies
    std::vector<Gtk::HBox *> close_hboxes;
    int close;

    // ... attackers
    close = 0;
    for (armies_type::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
      add_army(*i, close_hboxes, attacker_close_vbox, close++, rows,
	       Gtk::ALIGN_LEFT);
    }

    close_hboxes.clear();
    
    // ... defenders
    close = 0;
    for (armies_type::iterator i = defenders.begin(); i != defenders.end(); ++i)
    {
	add_army(*i, close_hboxes, defender_close_vbox, close++, rows,
		 Gtk::ALIGN_LEFT);
    }
    
    // fill in shield pictures
    GraphicsCache *gc = GraphicsCache::getInstance();
    Player* p;

    Gtk::Image *defender_shield_image;
    p = defenders.front()->getPlayer();
    xml->get_widget("defender_shield_image", defender_shield_image);
    defender_shield_image->property_pixbuf()=to_pixbuf(gc->getShieldPic(2, p));

    Gtk::Image *attacker_shield_image;
    p = attackers.front()->getPlayer();
    xml->get_widget("attacker_shield_image", attacker_shield_image);
    attacker_shield_image->property_pixbuf()=to_pixbuf(gc->getShieldPic(2, p));

    rounds_label->set_text(String::ucompose("%1", 0));

    fight.battle();
    actions = fight.getCourseOfEvents();
}

FightWindow::~FightWindow()
{
}

void FightWindow::set_parent_window(Gtk::Window &parent)
{
    window->set_transient_for(parent);
    window->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void FightWindow::run()
{
    round = 0;
    action_iterator = actions.begin();
    
    Timing::instance().register_timer(
	sigc::mem_fun(this, &FightWindow::do_round), FIGHT_ROUND_INTERVAL);
    
    window->show_all();
    main_loop = Glib::MainLoop::create();
    main_loop->run();
}

int FightWindow::compute_max_rows(const armies_type &attackers,
				  const armies_type &defenders)
{
    assert(!attackers.empty() || !defenders.empty());
    
    if (attackers.size() > defenders.size())
      return (attackers.size() / max_cols) + 
              (attackers.size() % max_cols == 0 ? 0 : 1);
    else
      return (defenders.size() / max_cols) + 
              (defenders.size() % max_cols == 0 ? 0 : 1);
    // Find out how to distribute the close range and long range attackers and
    // defenders, assuming that close range units are put in front.

    std::vector<int> counts(2, 0);

    // count the number of melee/ranged units
    for (armies_type::const_iterator i = attackers.begin(), end = attackers.end();
	 i != end; ++i)
    {
        ++counts[0];
    }

    for (armies_type::const_iterator i = defenders.begin(), end = defenders.end();
	 i != end; ++i)
    {
        ++counts[1];
    }

    // now find the max number of rows
    std::vector<int> heights(counts.begin(), counts.end());
    std::vector<int> widths(2, 0);
    for (int i = 0; i < 2; ++i)
	if (counts[i] > 0)
	    widths[i] = 1;

    double const wanted_ratio = 4.0 / 3;
    double old_dist = 10000;
    int old_height = *std::max_element(heights.begin(), heights.end());
    while (true) {
	int width = std::accumulate(widths.begin(), widths.end(), 0);
	int height = *std::max_element(heights.begin(), heights.end());

	double ratio = double(width) / height;
#if 0
	std::cerr << "WIDTH " << width
		  << " HEIGHT " << height
		  << " RATIO " << ratio << std::endl;
#endif

	double dist = std::abs(wanted_ratio - ratio);
	if (dist >= old_dist)
	    break;		// we passed the optimal point
	else
	{
	    old_dist = dist;
	    old_height = height;
	    
	    // compute new heights and widths
	    int max_height = height - 1;
	    if (max_height < 1)
		break;
	    
	    for (int i = 0; i < 2; ++i)
		if (heights[i] > max_height)
		{
		    heights[i] = max_height;
		    // round the division up
		    widths[i] = (counts[i] + max_height - 1) / max_height;
		}
	}
    }

#if 0
    std::cerr << "used height " << old_height << std::endl;
#endif
    // use old because the algorithm goes one step too far before it stops
    return old_height;
}

void FightWindow::add_army(Army *army,
			   std::vector<Gtk::HBox *> &hboxes,
			   Gtk::VBox *vbox,
			   int current_no, int max_rows,
			   Gtk::AlignmentEnum alignment)
{
    // construct the army box
    Gtk::VBox *army_box = manage(new Gtk::VBox);
	
    // image
    army_box->add(*manage(new Gtk::Image(to_pixbuf(army->getPixmap()))));
    
    // hit points graph
    Gtk::ProgressBar *progress = manage(new Gtk::ProgressBar);
    progress->set_fraction(double(army->getHP()) / army->getStat(Army::HP));
    progress->property_width_request() = army->getPixmap()->w;
    progress->property_height_request() = 12;
    army_box->pack_start(*progress, Gtk::PACK_SHRINK, 4);

    // then add it to the right hbox
    int current_row = (current_no / max_cols);
    if (current_row >= int(hboxes.size()))
    {
	// add an hbox
	Gtk::HBox *hbox = manage(new Gtk::HBox);
	hbox->set_spacing(6);
	hboxes.push_back(hbox);

	Gtk::Alignment *a = manage(new Gtk::Alignment(alignment));
	a->property_xscale() = 0;
	a->add(*hbox);
	vbox->pack_start(*a, Gtk::PACK_SHRINK);
    }

    Gtk::HBox *hbox = hboxes[current_row];
    hbox->pack_start(*army_box, Gtk::PACK_SHRINK);

    // finally add an entry for later use
    ArmyItem item;
    item.army = army;
    item.hp = army->getHP();
    item.bar = progress;
    army_items.push_back(item);
}

bool FightWindow::do_round()
{
    while (action_iterator != actions.end())
    {
	FightItem &f = *action_iterator;
	
	++action_iterator;

	for (army_items_type::iterator i = army_items.begin(),
		 end = army_items.end(); i != end; ++i)
	    if (i->army->getId() == f.id)
	    {
		i->hp -= f.damage;
		if (i->hp < 0)
		    i->hp = 0;
		i->bar->set_fraction(double(i->hp)
				     / i->army->getStat(Army::HP));
		
		break;
	    }

	if (f.turn > round)
	{
	    ++round;
	    rounds_label->set_text(String::ucompose("%1", round));

	    return Timing::CONTINUE;
	}
    }

    window->hide();
    main_loop->quit();
    
    return Timing::STOP;
}
