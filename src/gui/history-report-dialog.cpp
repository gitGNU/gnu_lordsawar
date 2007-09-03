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
#include "line-chart.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../File.h"
#include "../GameMap.h"
#include "../citylist.h"
#include "../playerlist.h"
#include "../history.h"
#include "../GraphicsCache.h"

HistoryReportDialog::HistoryReportDialog(Player *p, HistoryReportType type)
{
  d_player = p;
  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path()
				+ "/history-report-dialog.glade");

  Gtk::Dialog *d = 0;
  xml->get_widget("dialog", d);
  dialog.reset(d);

  generatePastCitylists();
  generatePastEventlists();

  xml->get_widget("map_image", map_image);
  historymap.reset(new HistoryMap(Citylist::getInstance()));
  historymap->map_changed.connect
    (sigc::mem_fun(this, &HistoryReportDialog::on_map_changed));

  xml->get_widget("turn_scale", turn_scale);
  dialog->set_title(_("History"));
  turn_scale->set_range(1, past_citylists.size());
  turn_scale->set_value(past_citylists.size());

  turn_scale->signal_value_changed().connect
    (sigc::bind(sigc::mem_fun(this, &HistoryReportDialog::on_turn_changed),
		turn_scale));

  xml->get_widget("history_notebook", history_notebook);
  history_notebook->set_current_page(type);
  history_notebook->signal_switch_page().connect(
	sigc::mem_fun(*this, &HistoryReportDialog::on_switch_page));

  xml->get_widget("city_label", city_label);
  xml->get_widget("gold_label", gold_label);
  xml->get_widget("winner_label", winner_label);

  events_list = Gtk::ListStore::create(events_columns);
  xml->get_widget("treeview", events_treeview);
  events_treeview->set_model(events_list);
  events_treeview->append_column("", events_columns.image);
  events_treeview->append_column("", events_columns.desc);

  Gtk::Button *close_button;
  xml->get_widget("close_button", close_button);
  close_button->signal_clicked().connect
    (sigc::mem_fun(*this, &HistoryReportDialog::on_close_button));
  closing = false;

  xml->get_widget("city_alignment", city_alignment);
  xml->get_widget("gold_alignment", gold_alignment);
  xml->get_widget("winner_alignment", winner_alignment);

  Playerlist::iterator pit = Playerlist::getInstance()->begin();
  Gdk::Color colour;
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      SDL_Color sdl = (*pit)->getColor();
      colour.set_red(sdl.r * 255); 
      colour.set_green(sdl.g * 255); 
      colour.set_blue(sdl.b * 255);
      d_colours.push_back(colour);
    }
  updateCityChart();
  updateGoldChart();
  updateWinningChart();

  fill_in_turn_info((Uint32)turn_scale->get_value());
}

void HistoryReportDialog::generatePastEventlists()
{
  bool last_turn = false;
  std::list<History*> *elist = new std::list<History*>();

  //keep a set of pointers to remember how far we are into each player's history
  std::list<History*> *hist[MAX_PLAYERS];
  Playerlist::iterator pit = Playerlist::getInstance()->begin();
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    hist[(*pit)->getId()] = (*pit)->getHistorylist();
  std::list<History*>::iterator hit[MAX_PLAYERS];
  pit = Playerlist::getInstance()->begin();
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    hit[(*pit)->getId()] = hist[(*pit)->getId()]->begin();

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
	      switch ((*hit[id])->getType())
		{
		case History::FOUND_SAGE: 
		case History::HERO_EMERGES:
		case History::HERO_QUEST_STARTED:
	       	case History::HERO_QUEST_COMPLETED:
		case History::HERO_KILLED_IN_CITY:
		case History::HERO_KILLED_IN_BATTLE:
		case History::HERO_KILLED_SEARCHING:
		case History::HERO_CITY_WON:
		  (*hit[id])->setPlayer(*pit);
		  elist->push_back(*hit[id]);
		  break;
		default:
		  break;
		}
	    }
	  if (hit[id] == hist[id]->end())
	    last_turn = true;
	}
      //and add it to the list
      past_eventlists.push_back(*elist);
      std::list<History*> *new_elist = new std::list<History*>();
      elist = new_elist;
      if (last_turn == true)
	break;

    }
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
	      else if ((*hit[id])->getType() == History::CITY_RAZED)
		{
		  Uint32 city_id;
		  city_id = dynamic_cast<History_CityRazed*>(*hit[id])->getCityId();
		  //find city with this city id in clist
		  ObjectList<City>::iterator cit = clist->begin();
		  for (; cit != clist->end(); ++cit)
		    if ((*cit).getId() == city_id)
		      {
			//change the owner to *pit
			(*cit).setBurnt(true);
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
  Uint32 turn = (Uint32)turn_scale->get_value();
  if (turn > past_citylists.size() - 1)
    historymap->updateCities(Citylist::getInstance());
  else
    historymap->updateCities(past_citylists[turn]);
  fill_in_turn_info(turn);
}

void HistoryReportDialog::fill_in_turn_info(Uint32 turn)
{
  switch (history_notebook->get_current_page())
    {
    case CITY:
      dialog->set_title(_("City History"));
      break;
    case EVENTS: 
      dialog->set_title(_("Event History"));
      break;
    case GOLD: 
      dialog->set_title(_("Gold History"));
      break;
    case WINNING:
      dialog->set_title(_("Winner History"));
      break;
    }

  //update the event list
  events_list->clear();
  if (turn <= past_eventlists.size() - 1)
    {
      std::list<History*> hist = past_eventlists[turn];
      std::list<History*>::iterator hit = hist.begin();
      for (; hit != hist.end(); hit++)
	addHistoryEvent(*hit);
    }

}

void HistoryReportDialog::on_switch_page(GtkNotebookPage *page, guint number)
{
  if (closing)
    return;
}

void HistoryReportDialog::addHistoryEvent(History *event)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Player *p = event->getPlayer();

  Glib::ustring s = "";
  Gtk::TreeIter i = events_list->append();
  (*i)[events_columns.image] = to_pixbuf(gc->getShieldPic(1, p));

  switch (event->getType())
    {
    case History::FOUND_SAGE: 
	{
	  History_FoundSage *ev;
	  ev = static_cast<History_FoundSage *>(event);
	  s = String::ucompose(_("%1 finds a sage!"), ev->getHeroName());
	  break;
	}
    case History::HERO_EMERGES:
	{
	  History_HeroEmerges *ev;
	  ev = static_cast<History_HeroEmerges *>(event);
	  s = String::ucompose(_("%1 emerges in %2!"), ev->getHeroName(),
			       ev->getCityName());
	  break;
	}
    case History::HERO_QUEST_STARTED:
	{
	  History_HeroQuestStarted *ev;
	  ev = static_cast<History_HeroQuestStarted*>(event);
	  s = String::ucompose(_("%1 begins a quest!"), ev->getHeroName());
	  break;
	}
    case History::HERO_QUEST_COMPLETED:
	{
	  History_HeroQuestCompleted *ev;
	  ev = static_cast<History_HeroQuestCompleted *>(event);
	  s = String::ucompose(_("%1 finishes a quest!"), ev->getHeroName());
	  break;
	}
    case History::HERO_KILLED_IN_CITY:
	{
	  History_HeroKilledInCity *ev;
	  ev = static_cast<History_HeroKilledInCity *>(event);
	  s = String::ucompose(_("%1 is killed in %2!"), ev->getHeroName(),
			       ev->getCityName());
	  break;
	}
    case History::HERO_KILLED_IN_BATTLE:
	{
	  History_HeroKilledInBattle *ev;
	  ev = static_cast<History_HeroKilledInBattle *>(event);
	  s = String::ucompose(_("%1 is killed in battle!"), ev->getHeroName());
	  break;
	}
    case History::HERO_KILLED_SEARCHING:
	{
	  History_HeroKilledSearching *ev;
	  ev = static_cast<History_HeroKilledSearching *>(event);
	  s = String::ucompose(_("%1 is killed while searching!"), 
			       ev->getHeroName());
	  break;
	}
    case History::HERO_CITY_WON:
	{
	  History_HeroCityWon *ev;
	  ev = static_cast<History_HeroCityWon *>(event);
	  s = String::ucompose(_("%1 conquers %2!"), ev->getHeroName(), 
			       ev->getCityName());
	  break;
	}
    default:
      s = _("unknown");
      break;
    }

  (*i)[events_columns.desc] = s;


}
void HistoryReportDialog::on_close_button()
{
  closing = true;
  //FIXME: find out why the page_switch events with crap data,
  //and then remove this function, and the closing variable too.
}

void HistoryReportDialog::updateCityChart()
{
  std::list<std::list<Uint32> >lines;
  // go through the past city list
  Playerlist::iterator pit = Playerlist::getInstance()->begin();
  Gdk::Color colour;
  pit = Playerlist::getInstance()->begin();
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      //go through the past city lists, searching for cities owned by this
      //player
  
      std::list<Uint32> line;
      for (unsigned int i = 0; i < past_citylists.size(); i++)
	{
	  Uint32 total_cities = 0;
	  ObjectList<City>::iterator it = past_citylists[i]->begin();
	  for (; it != past_citylists[i]->end(); it++)
	    {
	      if ((*it).getPlayer() == *pit)
		total_cities++;
	    }
	  line.push_back(total_cities);
	}

      line.push_back(Citylist::getInstance()->countCities(*pit));
      lines.push_back(line);

    }
  city_chart = new LineChart(lines, d_colours, Citylist::getInstance()->size());
  city_alignment->add(*manage(city_chart));
}

void HistoryReportDialog::updateGoldChart()
{
  std::list<std::list<Uint32> >lines;
  //go through the history list looking for gold events, per player
  Playerlist::iterator pit = Playerlist::getInstance()->begin();
  Gdk::Color colour;
  pit = Playerlist::getInstance()->begin();
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      std::list<History*> *hist = (*pit)->getHistorylist();
      std::list<History*>::iterator hit = hist->begin();
      std::list<Uint32> line;
      for (; hit != hist->end(); hit++)
	{
	  if ((*hit)->getType() == History::GOLD_TOTAL)
	    {
	      History_GoldTotal *event = static_cast<History_GoldTotal*>(*hit);
	      line.push_back (event->getGold());
	    }
	}
      line.push_back ((Uint32)(*pit)->getGold());
      if (line.size() > 0)
	lines.push_back(line);
    }

  gold_chart = new LineChart(lines, d_colours, 0);
  gold_alignment->add(*manage(gold_chart));
}

void HistoryReportDialog::updateWinningChart()
{
  std::list<std::list<Uint32> >lines;
  //go through the history list looking for score events, per player
  Playerlist::iterator pit = Playerlist::getInstance()->begin();
  Gdk::Color colour;
  pit = Playerlist::getInstance()->begin();
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      std::list<History*> *hist = (*pit)->getHistorylist();
      std::list<History*>::iterator hit = hist->begin();
      std::list<Uint32> line;
      for (; hit != hist->end(); hit++)
	{
	  if ((*hit)->getType() == History::SCORE)
	    {
	      History_Score *event = static_cast<History_Score*>(*hit);
	      line.push_back (event->getScore());
	    }
	}
      line.push_back ((Uint32)(*pit)->getScore());
      if (line.size() > 0)
	lines.push_back(line);
    }
  rank_chart = new LineChart(lines, d_colours, 100);
  winner_alignment->add(*manage(rank_chart));
}
