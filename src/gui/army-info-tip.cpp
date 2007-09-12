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

#include <config.h>

#include <libglademm/xml.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>

#include "army-info-tip.h"

#include "glade-helpers.h"
#include "image-helpers.h"

#include "../ucompose.hpp"
#include "../vector.h"
#include "../defs.h"
#include "../army.h"
#include "../GraphicsCache.h"
#include "../playerlist.h"


ArmyInfoTip::ArmyInfoTip(Gtk::Widget *target, const Army *army, ArmyInfoTipType type)
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/army-info-window.glade");

    Gtk::Window *w = 0;
    xml->get_widget("window", w);
    window.reset(w);

    Gtk::Image *army_image;
    xml->get_widget("army_image", army_image);
    Player *p = Playerlist::getActiveplayer();
    GraphicsCache *gc = GraphicsCache::getInstance();
    army_image->property_pixbuf() = to_pixbuf(gc->getArmyPic(p->getArmyset(), 
                                                             army->getType(), 
                                                             p, 1, NULL));

    // fill in terrain image
    Gtk::Image *terrain_image;
    xml->get_widget("terrain_image", terrain_image);
    SDL_Surface *terrain = gc->getMoveBonusPic(army->getMoveBonus(), false);
    terrain_image->property_pixbuf() = to_pixbuf(terrain);
    //terrain_image->hide();

    // fill in info
    Gtk::Label *info_label;
    xml->get_widget("info_label", info_label);
    Glib::ustring s;
    s += army->getName();
    s += "\n";
    // note to translators: %1 is melee strength, %2 is ranged strength
    s += String::ucompose(_("Strength: %1"),
			  army->getStat(Army::STRENGTH));
    s += "\n";
    if (type == ARMY_TYPE)
      {
	// note to translators: %1 is remaining moves, %2 is total moves
	s += String::ucompose(_("Movement: %1"), army->getStat(Army::MOVES));
	s += "\n";
	s += String::ucompose(_("Time: %1"), army->getProduction());
	s += "\n";
	s += String::ucompose(_("Cost: %1"), army->getUpkeep());
      }
    else if (type == ARMY_INSTANCE)
      {
	// note to translators: %1 is remaining moves, %2 is total moves
	s += String::ucompose(_("Moves: %1/%2"),
			      army->getMoves(), army->getStat(Army::MOVES));
	s += "\n";
	s += String::ucompose(_("Upkeep: %1"), army->getUpkeep());
      }
    info_label->set_text(s);

    // move into correct position
    window->get_child()->show();
    Vector<int> pos(0, 0);
    target->get_window()->get_origin(pos.x, pos.y);
    if (target->has_no_window())
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
