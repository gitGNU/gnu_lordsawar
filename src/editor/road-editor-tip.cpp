//  Copyright (C) 2015 Ben Asselstine
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

#include "road-editor-tip.h"
#include "builder-cache.h"

#include "ucompose.hpp"
#include "vector.h"
#include "defs.h"
#include "File.h"
#include "road.h"
#include "PixMask.h"
#include "ImageCache.h"

RoadEditorTip::RoadEditorTip(Gtk::Widget *target, MapTipPosition mpos, Road *r)
{
  road = r;
  Glib::RefPtr<Gtk::Builder> xml = 
    BuilderCache::editor_get("road-editor-tip.ui");

  xml->get_widget("window", window);
  xml->get_widget("button_box", button_box);

  fill_road_buttons();
  buttons[int(r->getType())]->set_active(true);
  connect_signals();

  // move into correct position
  window->get_child()->show_all();
  Vector<int> p(0, 0);
  target->get_window()->get_origin(p.x, p.y);
  if (target->get_has_window() == false)
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

void RoadEditorTip::fill_road_buttons()
{
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    {
      buttons[i] = manage(new Gtk::RadioButton);
      buttons[i]->set_group(group);
      buttons[i]->property_active() = false;
      buttons[i]->property_draw_indicator() = false;
      PixMask *pix = ImageCache::getInstance()->getRoadPic(Road::Type(i));
      buttons[i]->add(*manage(new Gtk::Image(pix->to_pixbuf())));
      button_box->pack_start(*buttons[i], Gtk::PACK_SHRINK);
    }
}

void RoadEditorTip::connect_signals()
{
  for (unsigned int i = 0; i < ROAD_TYPES; i++)
    buttons[i]->signal_toggled().connect(sigc::bind(sigc::mem_fun(this, &RoadEditorTip::on_road_selected), i));
}

RoadEditorTip::~RoadEditorTip()
{
  delete window;
}
    
void RoadEditorTip::on_road_selected(int type)
{
  if (buttons[type]->get_active() == true)
    road_picked.emit(road->getPos(), type);
}
