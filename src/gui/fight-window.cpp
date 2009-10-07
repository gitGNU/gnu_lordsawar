//  Copyright (C) 2007, 2008 Ole Laursen
//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include <config.h>

#include <assert.h>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <vector>
#include <gtkmm.h>

#include "fight-window.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "ucompose.hpp"
#include "timing.h"
#include "File.h"
#include "defs.h"
#include "player.h"
#include "playerlist.h"
#include "stack.h"
#include "army.h"
#include "GraphicsCache.h"
#include "Configuration.h"

FightWindow::FightWindow(Fight &fight)
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/fight-window.ui");

    xml->get_widget("window", window);
    
    window->signal_key_release_event().connect_notify(sigc::mem_fun(*this, &FightWindow::on_key_release_event));

    Gtk::VBox *attacker_close_vbox;
    Gtk::VBox *defender_close_vbox;
    xml->get_widget("attacker_close_vbox", attacker_close_vbox);
    xml->get_widget("defender_close_vbox", defender_close_vbox);

    // extract attackers and defenders
    armies_type attackers, defenders;

    Fight::orderArmies (fight.getAttackers(), attackers);
    Fight::orderArmies (fight.getDefenders(), defenders);

    int rows = compute_max_rows(attackers, defenders);
    
    // add the armies
    std::vector<Gtk::HBox *> close_hboxes;
    int close;
    std::map<guint32, guint32> initial_hps = fight.getInitialHPs();

    // ... attackers
    close = 0;
    for (armies_type::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
      add_army(*i, initial_hps[(*i)->getId()],
               close_hboxes, attacker_close_vbox, close++, rows,
	       Gtk::ALIGN_LEFT);
    }

    close_hboxes.clear();
    
    // ... defenders
    close = 0;
    for (armies_type::iterator i = defenders.begin(); i != defenders.end(); ++i)
    {
	add_army(*i, initial_hps[(*i)->getId()],
                 close_hboxes, defender_close_vbox, close++, rows,
		 Gtk::ALIGN_LEFT);
    }
    
    // fill in shield pictures
    GraphicsCache *gc = GraphicsCache::getInstance();
    Player* p;

    Gtk::Image *defender_shield_image;
    p = defenders.front()->getOwner();
    xml->get_widget("defender_shield_image", defender_shield_image);
    defender_shield_image->property_pixbuf()=gc->getShieldPic(2, p)->to_pixbuf();

    Gtk::Image *attacker_shield_image;
    p = attackers.front()->getOwner();
    xml->get_widget("attacker_shield_image", attacker_shield_image);
    attacker_shield_image->property_pixbuf()=gc->getShieldPic(2, p)->to_pixbuf();
  
    actions = fight.getCourseOfEvents();
    d_quick = false;

    fast_round_speed = 
      Configuration::s_displayFightRoundDelayFast; //milliseconds
    normal_round_speed = 
      Configuration::s_displayFightRoundDelaySlow; //milliseconds
}

FightWindow::~FightWindow()
{
  delete window;
}

void FightWindow::set_parent_window(Gtk::Window &parent)
{
    window->set_transient_for(parent);
    window->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void FightWindow::hide()
{
  window->hide();
}

void FightWindow::run(bool *quick)
{

    round = 0;
    action_iterator = actions.begin();
    
    Timing::instance().register_timer(
	sigc::mem_fun(this, &FightWindow::do_round), 
	*quick == true ? fast_round_speed : normal_round_speed);
    
    window->show_all();
    main_loop = Glib::MainLoop::create();
    main_loop->run();
    if (quick && *quick == false)
      *quick = d_quick;
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

void FightWindow::add_army(Army *army, int initial_hp,
			   std::vector<Gtk::HBox *> &hboxes,
			   Gtk::VBox *vbox,
			   int current_no, int max_rows,
			   Gtk::AlignmentEnum alignment)
{
    GraphicsCache *gc = GraphicsCache::getInstance();
    // construct the army box
    Gtk::VBox *army_box = manage(new Gtk::VBox);
	
    // image
    Glib::RefPtr<Gdk::Pixbuf> pic = gc->getArmyPic(army)->to_pixbuf();
    Gtk::Image *image = new Gtk::Image();
    image->property_pixbuf() = pic;
    army_box->add(*manage(image));
    
    // hit points graph
    Gtk::ProgressBar *progress = manage(new Gtk::ProgressBar);
    progress->set_fraction(double(initial_hp) / army->getStat(Army::HP));
    progress->property_width_request() = pic->get_width();
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
    item.hp = initial_hp;
    item.bar = progress;
    item.image = image;
    item.exploding = false;
    army_items.push_back(item);
}

bool FightWindow::do_round()
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Glib::RefPtr<Gdk::Pixbuf> expl = gc->getExplosionPic()->to_pixbuf();

  // first we clear out any explosions
  for (army_items_type::iterator i = army_items.begin(),
         end = army_items.end(); i != end; ++i)
  {
    if (!i->exploding)
      continue;
    
    Glib::RefPtr<Gdk::Pixbuf> empty_pic
      = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, expl->get_width(), expl->get_height());
    empty_pic->fill(0x00000000);
    i->image->property_pixbuf() = empty_pic;
    i->exploding = false;
    return Timing::CONTINUE;
  }

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
        double fraction = double(i->hp) / i->army->getStat(Army::HP);
        i->bar->set_fraction(fraction);
        if (fraction == 0.0)
        {
          i->bar->hide();
          i->image->property_pixbuf() = expl;
          i->exploding = true;
        }
		
        break;
      }

    if (f.turn > round)
    {
      ++round;

      return Timing::CONTINUE;
    }
  }

  window->hide();
  main_loop->quit();
    
  return Timing::STOP;
}
void FightWindow::on_key_release_event(GdkEventKey* event)
{
    Timing::instance().register_timer(
	sigc::mem_fun(this, &FightWindow::do_round), fast_round_speed);
    d_quick = true;
}
