//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2012, 2014 Ben Asselstine
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

#include <gtkmm.h>

#include "army-info-tip.h"

#include "glade-helpers.h"
#include "image-helpers.h"

#include "ucompose.hpp"
#include "vector.h"
#include "defs.h"
#include "army.h"
#include "armyprodbase.h"
#include "armyproto.h"
#include "GraphicsCache.h"
#include "playerlist.h"
#include "city.h"
#include "File.h"
#include "shield.h"

ArmyInfoTip::ArmyInfoTip(Gtk::Widget *target, const Army *army)
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/army-info-window.ui");

    xml->get_widget("window", window);
    Gtk::Image *army_image;
    xml->get_widget("army_image", army_image);
    Player *p;
    int armyset;
    p = army->getOwner();
    armyset = army->getArmyset();
    GraphicsCache *gc = GraphicsCache::getInstance();
    army_image->property_pixbuf() = 
      gc->getCircledArmyPic(armyset, army->getTypeId(), p, 
                            army->getMedalBonuses(), false, Shield::NEUTRAL, 
                            true)->to_pixbuf();

    // fill in terrain image
    Gtk::Image *terrain_image;
    xml->get_widget("terrain_image", terrain_image);
    terrain_image->property_pixbuf() = gc->getMoveBonusPic(army->getMoveBonus(), false)->to_pixbuf();
    //terrain_image->hide();

    // fill in info
    Gtk::Label *info_label;
    xml->get_widget("info_label", info_label);
    Glib::ustring s;
    s += army->getName();
    s += "\n";
    // note to translators: %1 is ranged strength
    s += String::ucompose(_("Strength: %1"),
			  army->getStat(Army::STRENGTH));
    s += "\n";
	
    // note to translators: %1 is remaining moves, %2 is total moves
    s += String::ucompose(_("Moves: %1/%2"),
			  army->getMoves(), army->getStat(Army::MOVES));
    s += "\n";
    s += String::ucompose(_("Upkeep: %1"), army->getUpkeep());
    info_label->set_text(s);

    // move into correct position
    window->get_child()->show();
    Vector<int> pos(0, 0);
    target->get_window()->get_origin(pos.x, pos.y);
    if (target->get_has_window() == false)
      {
	Gtk::Allocation a = target->get_allocation();
	pos.x += a.get_x();
	pos.y += a.get_y();
      }
    Vector<int> size(0, 0);
    window->get_size(size.x, size.y);
    window->set_gravity(Gdk::GRAVITY_SOUTH);
    pos.y -= size.y + 2;

    window->move(pos.x, pos.y);
    window->show();
}

ArmyInfoTip::ArmyInfoTip(Gtk::Widget *target, const ArmyProdBase *army, 
			 City *city)
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/army-info-window.ui");

    xml->get_widget("window", window);
    Gtk::Image *army_image;
    xml->get_widget("army_image", army_image);
    Player *p = city->getOwner();
    int armyset;
    armyset = army->getArmyset();
    GraphicsCache *gc = GraphicsCache::getInstance();
    army_image->property_pixbuf() = 
      gc->getCircledArmyPic(armyset, army->getTypeId(), p, NULL, false,
                            Shield::NEUTRAL, true)->to_pixbuf();

    // fill in terrain image
    Gtk::Image *terrain_image;
    xml->get_widget("terrain_image", terrain_image);
    terrain_image->property_pixbuf() = gc->getMoveBonusPic(army->getMoveBonus(), false)->to_pixbuf();
    //terrain_image->hide();

    // fill in info
    Gtk::Label *info_label;
    xml->get_widget("info_label", info_label);
    Glib::ustring s;
    s += army->getName();
    s += "\n";
    // note to translators: %1 is melee strength
    s += String::ucompose(_("Strength: %1"),
			  army->getStrength());
    s += "\n";
    // note to translators: %1 is total moves
    s += String::ucompose(_("Moves: %1"), army->getMaxMoves());
    s += "\n";
    s += String::ucompose(_("Time: %1"), army->getProduction());
    s += "\n";
    s += String::ucompose(_("Cost: %1"), army->getProductionCost());
    info_label->set_text(s);

    // move into correct position
    window->get_child()->show();
    Vector<int> pos(0, 0);
    target->get_window()->get_origin(pos.x, pos.y);
    if (target->get_has_window() == false)
      {
	Gtk::Allocation a = target->get_allocation();
	pos.x += a.get_x();
	pos.y += a.get_y();
      }
    Vector<int> size(0, 0);
    window->get_size(size.x, size.y);
    window->set_gravity(Gdk::GRAVITY_SOUTH);
    pos.y -= size.y + 2;

    window->move(pos.x, pos.y);
    window->show();
}

ArmyInfoTip::ArmyInfoTip(Gtk::Widget *target, const ArmyProto *army)
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/army-info-window.ui");

    xml->get_widget("window", window);
    Gtk::Image *army_image;
    xml->get_widget("army_image", army_image);
    Player *p = Playerlist::getInstance()->getActiveplayer();
    int armyset;
    armyset = army->getArmyset();
    GraphicsCache *gc = GraphicsCache::getInstance();
    army_image->property_pixbuf() = 
      gc->getCircledArmyPic(armyset, army->getId(), p, NULL, false,
                            Shield::NEUTRAL, true)->to_pixbuf();

    // fill in terrain image
    Gtk::Image *terrain_image;
    xml->get_widget("terrain_image", terrain_image);
    terrain_image->property_pixbuf() = gc->getMoveBonusPic(army->getMoveBonus(), false)->to_pixbuf();
    //terrain_image->hide();

    // fill in info
    Gtk::Label *info_label;
    xml->get_widget("info_label", info_label);
    Glib::ustring s;
    s += army->getName();
    s += "\n";
    // note to translators: %1 is melee strength, %2 is ranged strength
    s += String::ucompose(_("Strength: %1"),
			  army->getStrength());
    s += "\n";
    // note to translators: %1 is remaining moves, %2 is total moves
    s += String::ucompose(_("Movement: %1"), army->getMaxMoves());
    s += "\n";
    s += String::ucompose(_("Time: %1"), army->getProduction());
    s += "\n";
    s += String::ucompose(_("Cost: %1"), army->getUpkeep());
    info_label->set_text(s);

    // move into correct position
    window->get_child()->show();
    Vector<int> pos(0, 0);
    target->get_window()->get_origin(pos.x, pos.y);
    if (target->get_has_window() == false)
      {
	Gtk::Allocation a = target->get_allocation();
	pos.x += a.get_x();
	pos.y += a.get_y();
      }
    Vector<int> size(0, 0);
    window->get_size(size.x, size.y);
    window->set_gravity(Gdk::GRAVITY_SOUTH);
    pos.y -= size.y + 2;

    window->move(pos.x, pos.y);
    window->show();
}

ArmyInfoTip::~ArmyInfoTip()
{
  delete window;
}
