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
#include <sigc++/functors/mem_fun.h>
#include <gtkmm/label.h>

#include "history-report-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../File.h"
#include "../GameMap.h"
#include "../citylist.h"
#include "../playerlist.h"
#include "../history.h"

HistoryReportDialog::HistoryReportDialog()
{
  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path()
				+ "/history-report-dialog.glade");

  Gtk::Dialog *d = 0;
  xml->get_widget("dialog", d);
  dialog.reset(d);

  generatePastCitylists();
  xml->get_widget("map_image", map_image);
  historymap.reset(new HistoryMap(past_citylists[past_citylists.size()-1]));
  historymap->map_changed.connect
    (sigc::mem_fun(this, &HistoryReportDialog::on_map_changed));

  xml->get_widget("turn_scale", turn_scale);
  dialog->set_title(_("History"));
  turn_scale->set_range(1, past_citylists.size());
  turn_scale->set_value(past_citylists.size());

  turn_scale->signal_value_changed().connect
    (sigc::bind(sigc::mem_fun(this, &HistoryReportDialog::on_turn_changed),
		turn_scale));
}

void HistoryReportDialog::generatePastCitylists()
{
  bool last_turn = false;

  //keep a set of pointers to remember how far we are into each player's history
  std::list<History*> *hist[MAX_PLAYERS];
  Playerlist::iterator pit = Playerlist::getInstance()->begin();
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    hist[(*pit)->getId()] = (*pit)->getHistorylist();
  std::list<History*>::iterator hit[MAX_PLAYERS];
  pit = Playerlist::getInstance()->begin();
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    hit[(*pit)->getId()] = hist[(*pit)->getId()]->begin();

  //start off with an initial city list where all cities are neutral owned
  ObjectList<City> *clist = new ObjectList<City>();
  Citylist *cl = Citylist::getInstance();
  for (Citylist::iterator it = cl->begin(); it != cl->end(); ++it)
    clist->push_back(*it);
  for (ObjectList<City>::iterator it = clist->begin(); it != clist->end(); ++it)
    (*it).setPlayer(Playerlist::getInstance()->getNeutral());

  while (1)
    {
      //now we see what cities we took this turn
      pit = Playerlist::getInstance()->begin();
      for (; pit != Playerlist::getInstance()->end(); ++pit)
	{
	  if (*pit == Playerlist::getInstance()->getNeutral())
	    continue;
	  //dump everything up to the next turn
	  Uint32 id = (*pit)->getId();
	  for (; hit[id] != hist[id]->end(); ++hit[id])
	    {
	      if ((*hit[id])->getType() == History::START_TURN)
		{
		  hit[id]++;
		  break;
		}
	      else if ((*hit[id])->getType() == History::CITY_WON)
		{
		  Uint32 city_id;
		  city_id = dynamic_cast<History_CityWon*>(*hit[id])->getCityId();
		  //find city with this city id in clist
		  ObjectList<City>::iterator cit = clist->begin();
		  for (; cit != clist->end(); ++cit)
		    if ((*cit).getId() == city_id)
		      {
			//change the owner to *pit
			(*cit).setPlayer(*pit);
			break;
		      }
		}
	    }
	  if (hit[id] == hist[id]->end())
	    last_turn = true;
	}
      //and add it to the list
      past_citylists.push_back(clist);
      ObjectList<City> *new_clist = new ObjectList<City>(*clist);
      clist = new_clist;
      if (last_turn == true)
	break;

    }
}

void HistoryReportDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
  //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void HistoryReportDialog::run()
{
  historymap->resize(GameMap::get_dim() * 2);
  historymap->draw();

  dialog->show_all();
  dialog->run();
}

void HistoryReportDialog::on_map_changed(SDL_Surface *map)
{
  map_image->property_pixbuf() = to_pixbuf(map);
}

void HistoryReportDialog::on_turn_changed(Gtk::Scale *scale)
{
  //tell the historymap to show another set of cities
  historymap->updateCities(past_citylists[(int)scale->get_value()-1]);
}
