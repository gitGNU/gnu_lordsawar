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

#include <libglademm/xml.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <gtkmm/togglebutton.h>
#include <sigc++/functors/mem_fun.h>

#include "destination-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../GameMap.h"
#include "../city.h"
#include "../armysetlist.h"
#include "../GraphicsCache.h"
#include "../vectoredunitlist.h"

DestinationDialog::DestinationDialog(City *c)
{
    city = c;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/destination-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    xml->get_widget("map_image", map_image);
    xml->get_widget("see_all_togglebutton", see_all_toggle);
    xml->get_widget("vector_togglebutton", vector_toggle);
    xml->get_widget("change_togglebutton", change_toggle);
    xml->get_widget("current_label", current_label);
    xml->get_widget("current_image", current_image);
    xml->get_widget("turns_label", turns_label);
    xml->get_widget("one_turn_away_image", one_turn_away_image);
    xml->get_widget("two_turns_away_image", two_turns_away_image);
    xml->get_widget("next_turn_1_image", next_turn_1_image);
    xml->get_widget("next_turn_2_image", next_turn_2_image);
    xml->get_widget("next_turn_3_image", next_turn_3_image);
    xml->get_widget("next_turn_4_image", next_turn_4_image);
    xml->get_widget("turn_after_1_image", turn_after_1_image);
    xml->get_widget("turn_after_2_image", turn_after_2_image);
    xml->get_widget("turn_after_3_image", turn_after_3_image);
    xml->get_widget("turn_after_4_image", turn_after_4_image);

    see_all_toggle->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DestinationDialog::on_see_all_toggled),
                               see_all_toggle));
    vector_toggle->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DestinationDialog::on_vector_toggled),
                               vector_toggle));
    change_toggle->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DestinationDialog::on_change_toggled),
                               change_toggle));

    vectormap.reset(new VectorMap(c, VectorMap::SHOW_ORIGIN_CITY_VECTORING));
    vectormap->map_changed.connect(
	sigc::mem_fun(this, &DestinationDialog::on_map_changed));

    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);
    map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK);
    map_eventbox->signal_button_press_event().connect(
	sigc::mem_fun(*this, &DestinationDialog::on_map_mouse_button_event));
  fill_in_vectoring_info();
}

void DestinationDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void DestinationDialog::run()
{
    vectormap->resize();
    vectormap->draw();
    dialog->show();
    dialog->run();
}

void DestinationDialog::on_map_changed(SDL_Surface *map)
{
  if (vectormap->getClickAction() == VectorMap::CLICK_SELECTS &&
      vector_toggle->get_active() == true)
    {
      vector_toggle->set_active(false);
    }
  else
    {
      city = vectormap->getCity();
      fill_in_vectoring_info();
    }
  map_image->property_pixbuf() = to_pixbuf(map);
}

bool DestinationDialog::on_map_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS)
	return true;	// useless event
    
    vectormap->mouse_button_event(to_input_event(e));
    
    return true;
}

void DestinationDialog::on_see_all_toggled(Gtk::ToggleButton *toggle)
{
  bool see_all;
  see_all = toggle->get_active();
  if (see_all)
    vectormap->setShowVectoring(VectorMap::SHOW_ALL_VECTORING);
  else
    vectormap->setShowVectoring(VectorMap::SHOW_ORIGIN_CITY_VECTORING);
  vectormap->draw();
}

void DestinationDialog::on_vector_toggled(Gtk::ToggleButton *toggle)
{
// the idea here is that we click on the toggle,
// and then after we click on the map, it gets untoggled
// we act when it's untoggled.
  if (toggle->get_active() == false)
    {
      vectormap->setClickAction(VectorMap::CLICK_SELECTS);
      vectormap->draw();
      fill_in_vectoring_info();
    }
  else
    {
      vectormap->setClickAction(VectorMap::CLICK_VECTORS);
      vectormap->draw();
    }
}

void DestinationDialog::on_change_toggled(Gtk::ToggleButton *toggle)
{
// the idea here is that we click on the toggle,
// and then after we click on the map, it gets untoggled
// we act when it's untoggled.
  if (toggle->get_active() == false)
    {
      vectormap->draw();
      fill_in_vectoring_info();
    }
}

void DestinationDialog::fill_in_vectoring_info()
{
  std::list<VectoredUnit*> vectored;
  std::list<VectoredUnit*>::const_iterator it;
  VectoredUnitlist *vul = VectoredUnitlist::getInstance();
  dialog->set_title(city->getName());

  Player *player = city->getPlayer();
  unsigned int as = player->getArmyset();
  Glib::RefPtr<Gdk::Pixbuf> pic;
  GraphicsCache *gc = GraphicsCache::getInstance();
  int slot = city->getProductionIndex();
  SDL_Surface *s
    = GraphicsCache::getInstance()->getArmyPic(as, 0, player, 1, NULL);
  Glib::RefPtr<Gdk::Pixbuf> empty_pic
    = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, s->w, s->h);
  empty_pic->fill(0x00000000);

  vector_toggle->set_sensitive(slot != -1 ? true : false);
  change_toggle->set_sensitive(false);

  one_turn_away_image->set(empty_pic);
  two_turns_away_image->set(empty_pic);
  next_turn_1_image->set(empty_pic);
  next_turn_2_image->set(empty_pic);
  next_turn_3_image->set(empty_pic);
  next_turn_4_image->set(empty_pic);
  turn_after_1_image->set(empty_pic);
  turn_after_2_image->set(empty_pic);
  turn_after_3_image->set(empty_pic);
  turn_after_4_image->set(empty_pic);

  Glib::ustring s1;
  Glib::ustring s4 = _("Current:");

  if (slot == -1)
    {
      pic = empty_pic;
      turns_label->set_markup("");
    }
  else
    {
      const Army* a = city->getArmy(slot);
      pic = to_pixbuf(gc->getArmyPic(as, a->getType(), player, 1, NULL));
      s1 = String::ucompose(_("%1t"), city->getDuration());
      turns_label->set_markup("<i>" + s1 + "</i>");
    }
    
  current_image->set(pic);
  current_label->set_markup("<i>" + s4 + "</i>");

  //show the units that have been vectored from this city
  vul->getVectoredUnitsComingFrom(city->getPos(), vectored);
  for (it = vectored.begin(); it != vectored.end(); it++)
    {
      int armytype = (*it)->getArmy()->getType();
      if ((*it)->getDuration() == 2)
        {
          pic = to_pixbuf(gc->getArmyPic(as, armytype, player, 1, NULL));
          one_turn_away_image->set(pic);
        }
      else if ((*it)->getDuration() == 1)
        {
          pic = to_pixbuf(gc->getArmyPic(as, armytype, player, 1, NULL));
          two_turns_away_image->set(pic);
        }
    }

  //show the units that are arriving into this city
  vectored.clear();
  vul->getVectoredUnitsGoingTo(city->getPos(), vectored);
  int count = 0;
  Gtk::Image *image = next_turn_1_image;
  for (it = vectored.begin(); it != vectored.end(); it++)
    {
      if ((*it)->getDuration() != 1)
        continue;

      switch (count)
        {
          case 0: image = next_turn_1_image; break;
          case 1: image = next_turn_2_image; break;
          case 2: image = next_turn_3_image; break;
          case 3: image = next_turn_4_image; break;
        }
      pic = to_pixbuf(gc->getArmyPic(as, (*it)->getArmy()->getType(), player, 1, NULL));
      image->set(pic);
      count++;
    }
  count = 0;
  for (it = vectored.begin(); it != vectored.end(); it++)
    {
      if ((*it)->getDuration() != 2)
        continue;
      switch (count)
        {
          case 0: image = turn_after_1_image; break;
          case 1: image = turn_after_2_image; break;
          case 2: image = turn_after_3_image; break;
          case 3: image = turn_after_4_image; break;
        }
      pic = to_pixbuf(gc->getArmyPic(as, (*it)->getArmy()->getType(), player, 1, NULL));
      image->set(pic);
      count++;
    }
}
