//  Copyright (C) 2007, 2008 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2012, 2014, 2015 Ben Asselstine
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
#include <numeric>
#include <vector>
#include <gtkmm.h>

#include "fight-window.h"
#include "builder-cache.h"

#include "timing.h"
#include "File.h"
#include "player.h"
#include "army.h"
#include "ImageCache.h"
#include "Configuration.h"
#include "snd.h"
#include "GameMap.h"
#include "tileset.h"
#include "tilesetlist.h"
#include "Tile.h"
#include "SmallTile.h"

FightWindow::FightWindow(Gtk::Window &parent, Fight &fight)
{
  Glib::RefPtr<Gtk::Builder> xml = BuilderCache::get("fight-window.ui");

  xml->get_widget("window", window);
  window->set_transient_for(parent);

  window->signal_key_release_event().connect_notify
    (sigc::hide(sigc::mem_fun(*this, &FightWindow::on_key_release_event)));

  Gtk::Box *attacker_close_vbox;
  Gtk::Box *defender_close_vbox;
  xml->get_widget("attacker_close_vbox", attacker_close_vbox);
  xml->get_widget("defender_close_vbox", defender_close_vbox);

  // extract attackers and defenders
  armies_type attackers, defenders;

  Fight::orderArmies (fight.getAttackers(), attackers);
  Fight::orderArmies (fight.getDefenders(), defenders);

  // add the armies
  std::vector<Gtk::Box *> close_hboxes;
  int close;
  std::map<guint32, guint32> initial_hps = fight.getInitialHPs();

  // ... attackers
  close = 0;
  for (armies_type::iterator i = attackers.begin(); i != attackers.end(); ++i)
    add_army(*i, initial_hps[(*i)->getId()], close_hboxes, attacker_close_vbox, 
             close++);

  close_hboxes.clear();

  // ... defenders
  close = 0;
  for (armies_type::iterator i = defenders.begin(); i != defenders.end(); ++i)
    add_army(*i, initial_hps[(*i)->getId()], close_hboxes, defender_close_vbox, 
             close++);

  // fill in shield pictures
  ImageCache *gc = ImageCache::getInstance();

  Gtk::Image *defender_shield_image;
  Player *p = defenders.front()->getOwner();
  xml->get_widget("defender_shield_image", defender_shield_image);
  defender_shield_image->property_pixbuf()=gc->getShieldPic(2, p)->to_pixbuf();

  Gtk::Image *attacker_shield_image;
  p = attackers.front()->getOwner();
  xml->get_widget("attacker_shield_image", attacker_shield_image);
  attacker_shield_image->property_pixbuf()=gc->getShieldPic(2, p)->to_pixbuf();

  actions = fight.getCourseOfEvents();
  d_quick = false;

  fast_round_speed = Configuration::s_displayFightRoundDelayFast; //ms
  normal_round_speed = Configuration::s_displayFightRoundDelaySlow; //ms
  Snd::getInstance()->disableBackground();
  Snd::getInstance()->play("battle", -1, true);
}

FightWindow::~FightWindow()
{
  Snd::getInstance()->halt(true);
  Snd::getInstance()->enableBackground();
  delete window;
}

void FightWindow::hide()
{
  window->hide();
}

void FightWindow::run(bool *quick)
{
  round = 0;
  action_iterator = actions.begin();

  Timing::instance().register_timer(sigc::mem_fun(this, &FightWindow::do_round), 
                                    *quick == true ? fast_round_speed : 
                                    normal_round_speed);

  window->show_all();
  main_loop = Glib::MainLoop::create();
  main_loop->run();
  if (quick && *quick == false)
    *quick = d_quick;
}

void FightWindow::add_army(Army *army, int initial_hp,
                           std::vector<Gtk::Box *> &hboxes, Gtk::Box *vbox, 
                           int current_no)
{
  Gtk::Box *army_box;
  Gtk::Image *army_image;
  Gtk::DrawingArea *water_drawingarea;
  Gtk::EventBox *eventbox;

  Glib::RefPtr<Gtk::Builder> xml = BuilderCache::get("fighter.ui");

  xml->get_widget("army_box", army_box);
  xml->get_widget("eventbox", eventbox);
  xml->get_widget("army_image", army_image);
  xml->get_widget("water_drawingarea", water_drawingarea);

  // image
  ImageCache *gc = ImageCache::getInstance();
  Glib::RefPtr<Gdk::Pixbuf> pic = gc->getArmyPic(army)->to_pixbuf();
  army_image->property_pixbuf() = pic;

  water_drawingarea->property_width_request() = pic->get_width();
  water_drawingarea->property_height_request() = 3;
  if (army->getStat(Army::SHIP, false))
    {
      SmallTile *water = 
        Tilesetlist::getInstance()->getSmallTile(GameMap::getTileset()->getBaseName(), Tile::WATER);
      Gdk::RGBA watercolor = water->getColor();
      water_drawingarea->override_background_color(watercolor, Gtk::STATE_FLAG_NORMAL);
    }

  // then add it to the right hbox
  int current_row = (current_no / max_cols);
  if (current_row >= int(hboxes.size()))
    {
      // add an hbox
      Gtk::Box *hbox = manage(new Gtk::Box (Gtk::ORIENTATION_HORIZONTAL));
      hbox->set_spacing(6);
      hboxes.push_back(hbox);

      Gtk::Alignment *a = manage(new Gtk::Alignment(Gtk::ALIGN_START));
      a->add(*hbox);
      vbox->pack_start(*a, Gtk::PACK_SHRINK);
    }

  Gtk::Box *hbox = hboxes[current_row];
  army_box->get_parent()->remove(*army_box);
  Gtk::Box *box = new Gtk::Box(Gtk::ORIENTATION_VERTICAL );
  box->pack_start(*army_box, Gtk::PACK_SHRINK);
  army_box->reparent(*box);
  hbox->pack_start(*Gtk::manage(box), Gtk::PACK_SHRINK);

  // finally add an entry for later use
  ArmyItem item;
  item.army = army;
  item.hp = initial_hp;
  item.box = eventbox;
  item.image = army_image;
  item.exploding = false;
  army_items.push_back(item);
}

bool FightWindow::do_round()
{
  ImageCache *gc = ImageCache::getInstance();
  Glib::RefPtr<Gdk::Pixbuf> expl = gc->getExplosionPic()->to_pixbuf();

  // first we clear out any explosions
  for (army_items_type::iterator i = army_items.begin(), end = army_items.end(); 
       i != end; ++i)
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
            if (fraction == 0.0)
              {
                i->box->hide();
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

void FightWindow::on_key_release_event()
{
  Timing::instance().register_timer
    (sigc::mem_fun(this, &FightWindow::do_round), fast_round_speed);
  d_quick = true;
}
