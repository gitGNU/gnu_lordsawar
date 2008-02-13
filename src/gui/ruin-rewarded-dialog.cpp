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

#include <config.h>

#include <libglademm/xml.h>
#include <gtkmm/eventbox.h>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm/label.h>

#include "ruin-rewarded-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../GameMap.h"
#include "../File.h"
#include "../sound.h"
#include "../reward.h"
#include "../ruin.h"


RuinRewardedDialog::RuinRewardedDialog(Reward_Ruin *reward)
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/ruin-rewarded-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("map_image", map_image);

    ruinmap.reset(new RuinMap(reward->getRuin()));
    ruinmap->map_changed.connect(
	sigc::mem_fun(this, &RuinRewardedDialog::on_map_changed));

    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);

    xml->get_widget("label", label);
    dialog->set_title(_("A Sage!"));

    d_reward = reward;
}

void RuinRewardedDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void RuinRewardedDialog::run()
{
  ruinmap->resize();
  ruinmap->draw();

  Glib::ustring s;
  s += String::ucompose(_("The sages show thee the site of %1\n"),
			d_reward->getRuin()->getName());
  Reward *reward = d_reward->getRuin()->getReward();
  if (reward->getType() == Reward::ALLIES)
    s += _("where powerful allies can be found!");
  else if (reward->getType() == Reward::ITEM)
    {
      Item *item = static_cast<Reward_Item*>(reward)->getItem();
      s += String::ucompose(_("where the %1 can be found!"), item->getName());
    }
  else if (reward->getType() == Reward::MAP)
    s += _("where a map can be found!");
  else if (reward->getType() == Reward::RUIN)
    s += _("where crapola can be found!");
  else if (reward->getType() == Reward::GOLD)
    s += _("where gold can be found!");
  else //this one shouldn't happen
    s += _("where something important can be found!");

  label->set_text(s);

  dialog->show_all();
  dialog->run();
}

void RuinRewardedDialog::on_map_changed(SDL_Surface *map)
{
  map_image->property_pixbuf() = to_pixbuf(map);
}
