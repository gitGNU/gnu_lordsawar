//  Copyright (C) 2009 Ben Asselstine
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

#include "city-info-tip.h"

#include "glade-helpers.h"
#include "image-helpers.h"

#include "ucompose.hpp"
#include "vector.h"
#include "defs.h"
#include "GraphicsCache.h"
#include "playerlist.h"
#include "city.h"
#include "decorated.h"
#include "File.h"


CityInfoTip::CityInfoTip(Gtk::Widget *target, MapTipPosition mpos, const City *city)
{
    GraphicsCache *gc = GraphicsCache::getInstance();
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/city-info-window.ui");

    xml->get_widget("window", window);
    Decorated decorator;
    decorator.decorate(window,File::getMiscFile("various/background.png"), 200);

    Gtk::Image *left_shield_image;
    xml->get_widget("left_shield_image", left_shield_image);
    Gtk::Image *right_shield_image;
    xml->get_widget("right_shield_image", right_shield_image);
    Gtk::Image *income_image;
    xml->get_widget("income_image", income_image);
    Gtk::Image *defense_image;
    xml->get_widget("defense_image", defense_image);
    Gtk::Label *name_label;
    xml->get_widget("name_label", name_label);
    Gtk::Label *income_label;
    xml->get_widget("income_label", income_label);
    Gtk::Label *defense_label;
    xml->get_widget("defense_label", defense_label);
    Gtk::Label *capital_label;
    xml->get_widget("capital_label", capital_label);
    Gtk::Image *capital_image;
    xml->get_widget("capital_image", capital_image);
    Gtk::Label *razed_label;
    xml->get_widget("razed_label", razed_label);

    if (city->isBurnt() == true)
      razed_label->set_markup("<b>" + std::string(_("Razed!")) + "</b>");
      
    name_label->set_markup("<b>" + city->getName() + "</b>");
    left_shield_image->property_pixbuf() = 
      gc->getShieldPic(1, city->getOwner())->to_pixbuf();
    right_shield_image->property_pixbuf() = 
      gc->getShieldPic(1, city->getOwner())->to_pixbuf();
    income_image->property_file() = 
      File::getMiscFile("various/smallincome.png");
    defense_image->property_file() = 
      File::getMiscFile("various/smalldefense.png");
    income_label->set_markup(String::ucompose("<b>%1</b>",
					      city->getGold()));
    defense_label->set_markup(String::ucompose("<b>%1</b>",
					       city->getDefenseLevel()));
			  
    if (city->isCapital())
      {
	Glib::ustring s = "Capital of ";
	s += city->getCapitalOwner()->getName();
	capital_label->set_text (s);
	capital_image->property_pixbuf() = 
	  gc->getShieldPic(1, city->getCapitalOwner())->to_pixbuf();
      }
    else
      {
	Glib::RefPtr<Gdk::Pixbuf> empty_pic
	  = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, 1, 1);
	empty_pic->fill(0x00000000);
	capital_image->property_pixbuf() = empty_pic;
      }

    // move into correct position
    window->get_child()->show();
    Vector<int> p(0, 0);
    target->get_window()->get_origin(p.x, p.y);
    if (target->has_no_window())
    {
	Gtk::Allocation a = target->get_allocation();
	p.x += a.get_x();
	p.y += a.get_y();
    }
    Vector<int> size(0, 0);
    window->get_size(size.x, size.y);
    switch (mpos.justification)
    {
    case MapTipPosition::LEFT:
	window->set_gravity(Gdk::GRAVITY_NORTH_WEST);
	break;
    case MapTipPosition::RIGHT:
	window->set_gravity(Gdk::GRAVITY_NORTH_EAST);
	p.x -= size.x;
	break;
    case MapTipPosition::TOP:
	window->set_gravity(Gdk::GRAVITY_NORTH_WEST);
	break;
    case MapTipPosition::BOTTOM:
	window->set_gravity(Gdk::GRAVITY_SOUTH_WEST);
	p.y -= size.y;
	break;
    }

    p += mpos.pos;
	
    window->move(p.x, p.y);
    window->show();
}

CityInfoTip::~CityInfoTip()
{
  delete window;
}
