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
#include <sigc++/functors/mem_fun.h>

#include "quest-assigned-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "GameMap.h"

QuestAssignedDialog::QuestAssignedDialog(Hero *h, Quest *q)
{
  hero = h;
  quest = q;
    
  Glib::RefPtr<Gnome::Glade::Xml> xml
      = Gnome::Glade::Xml::create(get_glade_path()
				  + "/quest-assigned-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    decorate(dialog.get());
    window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));

    xml->get_widget("map_image", map_image);

    questmap.reset(new QuestMap(quest));
    questmap->map_changed.connect(
	sigc::mem_fun(this, &QuestAssignedDialog::on_map_changed));

    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);

    set_title(String::ucompose(_("Quest for %1"), hero->getName()));

    xml->get_widget("label", label);
    Glib::ustring s;
    if (quest)
	s = quest->getDescription();
    else
	s = _("This hero already has a quest.");
    label->set_text(s);
    
}

void QuestAssignedDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void QuestAssignedDialog::hide()
{
  dialog->hide();
}

void QuestAssignedDialog::run()
{
    questmap->resize();
    questmap->draw(Playerlist::getActiveplayer());

    dialog->show_all();
    dialog->run();
}

void QuestAssignedDialog::on_map_changed(SDL_Surface *map)
{
    map_image->property_pixbuf() = to_pixbuf(map);
}

