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

#include "ruin-report-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "GameMap.h"
#include "File.h"
#include "sound.h"
#include "ruin.h"
#include "ruinlist.h"
#include "templelist.h"

RuinReportDialog::RuinReportDialog(Vector<int> pos)
{
  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path()
				  + "/ruin-report-dialog.glade");

  Gtk::Dialog *d = 0;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  decorate(dialog.get());
  window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));

  xml->get_widget("map_image", map_image);

  NamedLocation *l = NULL;
  Ruin *ruin = Ruinlist::getInstance()->getNearestRuin(pos);
  Temple *temple = Templelist::getInstance()->getNearestObject(pos);
  if (temple && !ruin)
    l = temple;
  else if (ruin && !temple)
    l = ruin;
  else if (!temple && !ruin)
    return;
  else if (ruin->getPos() == pos)
    l = ruin;
  else if (temple->getPos() == pos)
    l = temple;
  else
    l = ruin;

  ruinmap.reset(new RuinMap(l));
  ruinmap->map_changed.connect(
    sigc::mem_fun(this, &RuinReportDialog::on_map_changed));

  Gtk::EventBox *map_eventbox;
  xml->get_widget("map_eventbox", map_eventbox);

  map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK);
  map_eventbox->signal_button_press_event().connect(
    sigc::mem_fun(*this, &RuinReportDialog::on_map_mouse_button_event));
  set_title(_("Ruins and Temples"));

  xml->get_widget("name_label", name_label);
  xml->get_widget("type_label", type_label);
  xml->get_widget("explored_label", explored_label);
  xml->get_widget("description_label", description_label);

  fill_in_ruin_info();
}

void RuinReportDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
  //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void RuinReportDialog::hide()
{
  dialog->hide();
}

void RuinReportDialog::run()
{
  ruinmap->resize();
  ruinmap->draw(Playerlist::getActiveplayer());

  Sound::getInstance()->playMusic("hero", 1);
  dialog->show_all();
  dialog->run();
  Sound::getInstance()->haltMusic();
}

void RuinReportDialog::on_map_changed(SDL_Surface *map)
{
  map_image->property_pixbuf() = to_pixbuf(map);
  fill_in_ruin_info();
}

bool RuinReportDialog::on_map_mouse_button_event(GdkEventButton *e)
{
  if (e->type != GDK_BUTTON_PRESS)
    return true;	// useless event
    
  ruinmap->mouse_button_event(to_input_event(e));
    
  return true;
}

void RuinReportDialog::fill_in_ruin_info()
{
  NamedLocation *l = ruinmap->getNamedLocation();
  name_label->set_text(l->getName());
  description_label->set_text(l->getDescription());
  Ruin *ruin = Ruinlist::getInstance()->getObjectAt(l->getPos());
  Temple *temple = Templelist::getInstance()->getObjectAt(l->getPos());
  if (ruin)
    {
      if (ruin->getType() == Ruin::RUIN)
        type_label->set_text(_("Ruin"));
      else if (ruin->getType() == Ruin::STRONGHOLD)
        type_label->set_text(_("Stronghold"));

      if (ruin->isSearched())
        explored_label->set_text(_("Yes"));
      else
	{
	  std::string hint = "  ";
	  explored_label->set_text(_("No"));
	  //add the difficulty hint.
	  if (ruin->getOccupant() != NULL)
	    {
	      Stack *s = ruin->getOccupant();
	      switch ((*s->front()).getStat(Army::STRENGTH))
		{
		case 9: 
		  hint += _("It is especially well-guarded."); break;
		case 8: 
		  hint += _("Rumour speaks of a formidable force within."); 
		  break;
		case 7: 
		  hint += _("Even heroes are wary of this site."); break;
		case 6: 
		  hint += _("Bones litter this place."); break;
		case 5: case 4: case 3: case 2: case 1: 
		  hint += _("It is guarded."); break;
		case 0: 
		  hint += ""; break;
		default: 
		  hint += ""; break;
		}
	    }
	  else
	    hint += _("Bones litter this place.");
	  description_label->set_text(description_label->get_text() + hint);
	}
    }
  else if (temple)
    {
      type_label->set_text(_("Temple"));
      explored_label->set_text(_("No"));
    }
  else
    type_label->set_text("");

}
