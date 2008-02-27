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
#include <gtkmm/label.h>

#include "history-report-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "line-chart.h"
#include "report-dialog.h"
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
      if (*pit == d_player)
	d_colours.push_front(colour);
      else
	d_colours.push_back(colour);
    }

  generatePastCityCounts();
  city_chart = new LineChart(past_citycounts, d_colours, 
			     Citylist::getInstance()->size());
  city_alignment->add(*manage(city_chart));

  generatePastGoldCounts();
  gold_chart = new LineChart(past_goldcounts, d_colours, 0);
  gold_alignment->add(*manage(gold_chart));

  generatePastWinningCounts();
  rank_chart = new LineChart(past_rankcounts, d_colours, 100);
  winner_alignment->add(*manage(rank_chart));

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
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      hist[(*pit)->getId()] = (*pit)->getHistorylist();
    }
  std::list<History*>::iterator hit[MAX_PLAYERS];
  pit = Playerlist::getInstance()->begin();
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      hit[(*pit)->getId()] = hist[(*pit)->getId()]->begin();
    }

  unsigned int count = 0;
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
	  if (hit[id] == hist[id]->end())
	    continue;
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
		case History::HERO_FINDS_ALLIES:
		case History::PLAYER_VANQUISHED:
		case History::DIPLOMATIC_TREACHERY:
		case History::DIPLOMATIC_WAR:
		case History::DIPLOMATIC_PEACE:
		  (*hit[id])->setOwner(*pit);
		  elist->push_back(*hit[id]);
		  break;
		case History::START_TURN:
		case History::GOLD_TOTAL:
		case History::CITY_WON:
		case History::CITY_RAZED:
		case History::SCORE:
		  break;
		}
	    }
	  if (hit[id] == hist[id]->end())
	    {
	      count++;
	      if (count == Playerlist::getInstance()->size() - 2)
		last_turn = true;
	    }
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
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      hist[(*pit)->getId()] = (*pit)->getHistorylist();
    }
  std::list<History*>::iterator hit[MAX_PLAYERS];
  pit = Playerlist::getInstance()->begin();
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      hit[(*pit)->getId()] = hist[(*pit)->getId()]->begin();
    }

  //start off with an initial city list where all cities are neutral owned
  LocationList<City> *clist = new LocationList<City>();
  Citylist *cl = Citylist::getInstance();
  for (Citylist::iterator it = cl->begin(); it != cl->end(); ++it)
    clist->push_back(*it);
  for (LocationList<City>::iterator it = clist->begin(); it != clist->end(); ++it)
    {
      (*it).setOwner(Playerlist::getInstance()->getNeutral());
      (*it).setBurnt(false);
    }

  unsigned int count = 0;
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
	  if (hit[id] == hist[id]->end())
	    continue;
	  for (; hit[id] != hist[id]->end(); hit[id]++)
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
		  LocationList<City>::iterator cit = clist->begin();
		  for (; cit != clist->end(); ++cit)
		    if ((*cit).getId() == city_id)
		      {
			(*cit).setOwner(*pit);
			break;
		      }
		}
	      else if ((*hit[id])->getType() == History::CITY_RAZED)
		{
		  Uint32 city_id;
		  city_id = dynamic_cast<History_CityRazed*>(*hit[id])->getCityId();
		  //find city with this city id in clist
		  LocationList<City>::iterator cit = clist->begin();
		  for (; cit != clist->end(); ++cit)
		    if ((*cit).getId() == city_id)
		      {
			//change the owner to neutral 
			(*cit).setOwner(Playerlist::getInstance()->getNeutral());
			(*cit).setBurnt(true);
			break;
		      }
		}
	    }
	  if (hit[id] == hist[id]->end())
	    {
	      count++;
	      if (count == Playerlist::getInstance()->size() - 2)
		last_turn = true;
	    }
	}
      //and add it to the list
      past_citylists.push_back(clist);
      LocationList<City> *new_clist = new LocationList<City>(*clist);
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
  historymap->resize();
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
  city_chart->set_x_indicator(turn);
  gold_chart->set_x_indicator(turn);
  rank_chart->set_x_indicator(turn);
  fill_in_turn_info(turn);
}

void HistoryReportDialog::fill_in_turn_info(Uint32 turn)
{
  Glib::ustring s;
  Uint32 count;
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

  //update the gold chart
  //on turn # you had # gold pieces
  std::list<Uint32> goldlist = *past_goldcounts.begin();
  std::list<Uint32>::iterator it = goldlist.begin();
  count=1;
  for (; it != goldlist.end(); it++, count++)
    {
      if (count == turn)
	{
	  count = *it;
	  break;
	}
    }
  s = String::ucompose(_("On turn %1 you %2 "), turn, 
		       turn == past_citylists.size() ? "have" : "had");
  s += String::ucompose(ngettext("%1 gold piece!",
				 "%1 gold pieces!",
				 count), count);
  gold_label->set_text(s);

  //update the city chart
  std::list<Uint32> citylist = *past_citycounts.begin();
  it = citylist.begin();
  count=1;
  for (; it != citylist.end(); it++, count++)
    {
      if (count == turn)
	{
	  count = *it;
	  break;
	}
    }
  s = String::ucompose(_("On turn %1 you %2 "), turn,
		       turn == past_citylists.size() ? "have" : "had");
  s += String::ucompose(ngettext("%1 city!",
				 "%1 cities!",
				 count), count);
  city_label->set_text(s);

  //on turn # you were coming #
  std::list<Uint32> scores;
  std::list<std::list<Uint32> >::iterator rit = past_rankcounts.begin();
  for (; rit != past_rankcounts.end(); rit++)
    {
      it = (*rit).begin();
      count=1;
      for (; it != (*rit).end(); it++, count++)
	{
	  if (count == turn)
	    {
	      count = *it;
	      scores.push_back(*it);
	      break;
	    }
	}
    }
  s = String::ucompose(_("On turn %1 you %2 coming %3"), turn,
		       turn == past_citylists.size() ? "are" : "were",
		       ReportDialog::calculateRank(scores, *scores.begin()));
  winner_label->set_text(s);
}

void HistoryReportDialog::on_switch_page(GtkNotebookPage *page, guint number)
{
  if (closing)
    return;
}

void HistoryReportDialog::addHistoryEvent(History *event)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Player *p = event->getOwner();

  Glib::ustring s = "";
  Gtk::TreeIter i = events_list->append();

  switch (event->getType())
    {
    case History::FOUND_SAGE: 
	{
	  History_FoundSage *ev;
	  ev = static_cast<History_FoundSage *>(event);
	  s = String::ucompose(_("%1 finds a sage!"), ev->getHeroName());
	  (*i)[events_columns.image] = to_pixbuf(gc->getShieldPic(1, p));
	  break;
	}
    case History::HERO_EMERGES:
	{
	  History_HeroEmerges *ev;
	  ev = static_cast<History_HeroEmerges *>(event);
	  s = String::ucompose(_("%1 emerges in %2!"), ev->getHeroName(),
			       ev->getCityName());
	  (*i)[events_columns.image] = to_pixbuf(gc->getShieldPic(1, p));
	  break;
	}
    case History::HERO_QUEST_STARTED:
	{
	  History_HeroQuestStarted *ev;
	  ev = static_cast<History_HeroQuestStarted*>(event);
	  s = String::ucompose(_("%1 begins a quest!"), ev->getHeroName());
	  (*i)[events_columns.image] = to_pixbuf(gc->getShieldPic(1, p));
	  break;
	}
    case History::HERO_QUEST_COMPLETED:
	{
	  History_HeroQuestCompleted *ev;
	  ev = static_cast<History_HeroQuestCompleted *>(event);
	  s = String::ucompose(_("%1 finishes a quest!"), ev->getHeroName());
	  (*i)[events_columns.image] = to_pixbuf(gc->getShieldPic(1, p));
	  break;
	}
    case History::HERO_KILLED_IN_CITY:
	{
	  History_HeroKilledInCity *ev;
	  ev = static_cast<History_HeroKilledInCity *>(event);
	  s = String::ucompose(_("%1 is killed in %2!"), ev->getHeroName(),
			       ev->getCityName());
	  (*i)[events_columns.image] = to_pixbuf(gc->getShieldPic(1, p));
	  break;
	}
    case History::HERO_KILLED_IN_BATTLE:
	{
	  History_HeroKilledInBattle *ev;
	  ev = static_cast<History_HeroKilledInBattle *>(event);
	  s = String::ucompose(_("%1 is killed in battle!"), ev->getHeroName());
	  (*i)[events_columns.image] = to_pixbuf(gc->getShieldPic(1, p));
	  break;
	}
    case History::HERO_KILLED_SEARCHING:
	{
	  History_HeroKilledSearching *ev;
	  ev = static_cast<History_HeroKilledSearching *>(event);
	  s = String::ucompose(_("%1 is killed while searching!"), 
			       ev->getHeroName());
	  (*i)[events_columns.image] = to_pixbuf(gc->getShieldPic(1, p));
	  break;
	}
    case History::HERO_CITY_WON:
	{
	  History_HeroCityWon *ev;
	  ev = static_cast<History_HeroCityWon *>(event);
	  s = String::ucompose(_("%1 conquers %2!"), ev->getHeroName(), 
			       ev->getCityName());
	  (*i)[events_columns.image] = to_pixbuf(gc->getShieldPic(1, p));
	  break;
	}
    case History::PLAYER_VANQUISHED:
	{
	  History_PlayerVanquished *ev;
	  ev = static_cast<History_PlayerVanquished*>(event);
	  s = String::ucompose(_("%1 is vanquished!"), p->getName());
	  (*i)[events_columns.image] = to_pixbuf(gc->getShieldPic(1, p));
	  break;
	}
    case History::DIPLOMATIC_PEACE:
	{
	  History_DiplomacyPeace *ev;
	  ev = static_cast<History_DiplomacyPeace*>(event);
	  Playerlist *pl = Playerlist::getInstance();
	  Player *opponent = pl->getPlayer(ev->getOpponentId());
	  s = String::ucompose(_("%1 at peace with %2!"), p->getName(), 
			       opponent->getName());
	  break;
	}
    case History::DIPLOMATIC_WAR:
	{
	  History_DiplomacyWar *ev;
	  ev = static_cast<History_DiplomacyWar*>(event);
	  Playerlist *pl = Playerlist::getInstance();
	  Player *opponent = pl->getPlayer(ev->getOpponentId());
	  s = String::ucompose(_("%1 at war with %2!"), p->getName(), 
			       opponent->getName());
	  break;
	}
    case History::DIPLOMATIC_TREACHERY:
	{
	  History_DiplomacyTreachery *ev;
	  ev = static_cast<History_DiplomacyTreachery*>(event);
	  Playerlist *pl = Playerlist::getInstance();
	  Player *opponent = pl->getPlayer(ev->getOpponentId());
	  s = String::ucompose(_("Treachery by %1 on %2!"), p->getName(),
			       opponent->getName());
	  (*i)[events_columns.image] = to_pixbuf(gc->getShieldPic(1, p));
	  break;
	}
    case History::HERO_FINDS_ALLIES:
	{
	  History_HeroFindsAllies *ev;
	  ev = static_cast<History_HeroFindsAllies*>(event);
	  s = String::ucompose(_("%1 finds allies!"), ev->getHeroName());
	  (*i)[events_columns.image] = to_pixbuf(gc->getShieldPic(1, p));
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

void HistoryReportDialog::generatePastWinningCounts()
{
  //go through the history list looking for score events, per player
  Playerlist::iterator pit = Playerlist::getInstance()->begin();
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
      if (*pit == d_player)
	past_rankcounts.push_front(line);
      else
	past_rankcounts.push_back(line);
    }
}

void HistoryReportDialog::generatePastCityCounts()
{
  // go through the past city list
  Playerlist::iterator pit = Playerlist::getInstance()->begin();
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
	  LocationList<City>::iterator it = past_citylists[i]->begin();
	  for (; it != past_citylists[i]->end(); it++)
	    {
	      if ((*it).getOwner() == *pit)
		total_cities++;
	    }
	  line.push_back(total_cities);
	}

      line.push_back(Citylist::getInstance()->countCities(*pit));
      if (*pit == d_player)
	past_citycounts.push_front(line);
      else
	past_citycounts.push_back(line);

    }
}

void HistoryReportDialog::generatePastGoldCounts()
{
  //go through the history list looking for gold events, per player
  Playerlist::iterator pit = Playerlist::getInstance()->begin();
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
      if (*pit == d_player)
	past_goldcounts.push_front(line);
      else
	past_goldcounts.push_back(line);
    }
}

