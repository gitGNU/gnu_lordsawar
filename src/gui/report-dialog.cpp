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

#include "report-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "bar-chart.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../File.h"
#include "../GameMap.h"
#include "../playerlist.h"
#include "../citylist.h"
#include "../stacklist.h"
#include "../action.h"
#include "../GraphicsCache.h"
#include "../armysetlist.h"

ReportDialog::ReportDialog(Player *player, ReportType type)
{
  d_player = player;
  Glib::RefPtr<Gnome::Glade::Xml> xml
    = Gnome::Glade::Xml::create(get_glade_path() + "/report-dialog.glade");

  Gtk::Dialog *d = 0;
  xml->get_widget("dialog", d);
  dialog.reset(d);

  xml->get_widget("map_image", map_image);
  citymap.reset(new CityMap());
  citymap->map_changed.connect
    (sigc::mem_fun(this, &ReportDialog::on_city_map_changed));
  armymap.reset(new ArmyMap());
  armymap->map_changed.connect
    (sigc::mem_fun(this, &ReportDialog::on_army_map_changed));
  City *c = Citylist::getInstance()->getFirstCity(d_player);
  vectormap.reset(new VectorMap(c, VectorMap::SHOW_ALL_VECTORING));
  vectormap->map_changed.connect
    (sigc::mem_fun(this, &ReportDialog::on_vector_map_changed));

  xml->get_widget("army_label", army_label);
  xml->get_widget("city_label", city_label);
  xml->get_widget("gold_label", gold_label);
  xml->get_widget("production_label", production_label);
  xml->get_widget("winning_label", winning_label);


  xml->get_widget("report_notebook", report_notebook);
  report_notebook->set_current_page(type);
  report_notebook->signal_switch_page().connect(
	sigc::mem_fun(*this, &ReportDialog::on_switch_page));

  armies_list = Gtk::ListStore::create(armies_columns);
  xml->get_widget("treeview", armies_treeview);
  armies_treeview->set_model(armies_list);
  armies_treeview->append_column("", armies_columns.image);
  armies_treeview->append_column("", armies_columns.desc);

  //loop through the action list looking for production actions
  Uint32 total = 0;
  std::list<Action*> *actions = player->getActionlist();
  std::list<Action*>::const_iterator it;
  for (it = actions->begin(); it != actions->end(); it++)
   if ((*it)->getType() == Action::PRODUCE_UNIT ||
      (*it)->getType() == Action::PRODUCE_VECTORED_UNIT)
     {
       addProduction(*it);
       total++;
     }
  Glib::ustring s;
  s = String::ucompose(ngettext("You produced %1 army this turn!",
				"You produced %1 armies this turn!",
		  		total), total);
  production_label->set_text(s);

  Gtk::Button *close_button;
  xml->get_widget("close_button", close_button);
  close_button->signal_clicked().connect
    (sigc::mem_fun(*this, &ReportDialog::on_close_button));
  xml->get_widget("army_alignment", army_alignment);
  xml->get_widget("city_alignment", city_alignment);
  xml->get_widget("gold_alignment", gold_alignment);
  xml->get_widget("winning_alignment", winning_alignment);
  updateArmyChart();
  updateGoldChart();
  updateCityChart();
  updateWinningChart();
  fill_in_info();
  closing = false;
}

void ReportDialog::on_close_button()
{
  closing = true;
  //FIXME: find out why the page_switch events with crap data,
  //and then remove this function, and the closing variable too.
}

void ReportDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
  //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void ReportDialog::run()
{
  citymap->resize();
  citymap->draw();
  vectormap->resize();
  vectormap->draw();
  armymap->resize();
  armymap->draw();

  dialog->show_all();
  dialog->run();
}

void ReportDialog::on_army_map_changed(SDL_Surface *map)
{
  if (report_notebook->get_current_page() == ARMY)
    map_image->property_pixbuf() = to_pixbuf(map);
}

void ReportDialog::on_city_map_changed(SDL_Surface *map)
{
  if (report_notebook->get_current_page() == CITY ||
      report_notebook->get_current_page() == GOLD ||
      report_notebook->get_current_page() == WINNING)
    map_image->property_pixbuf() = to_pixbuf(map);
}

void ReportDialog::on_vector_map_changed(SDL_Surface *map)
{
  if (report_notebook->get_current_page() == PRODUCTION)
    map_image->property_pixbuf() = to_pixbuf(map);
}

void ReportDialog::on_switch_page(GtkNotebookPage *page, guint number)
{
  if (closing)
    return;
  switch (number)
    {
    case ARMY:
      map_image->property_pixbuf() = to_pixbuf(armymap->get_surface());
      break;
    case CITY: 
    case GOLD: 
    case WINNING:
      map_image->property_pixbuf() = to_pixbuf(citymap->get_surface());
      break;
    case PRODUCTION:
      map_image->property_pixbuf() = to_pixbuf(vectormap->get_surface());
      break;
    }
  fill_in_info();
}

void ReportDialog::fill_in_info()
{
  switch (report_notebook->get_current_page())
    {
    case ARMY:
      dialog->set_title(_("Army Report"));
      break;
    case CITY: 
      dialog->set_title(_("City Report"));
      break;
    case GOLD: 
      dialog->set_title(_("Gold Report"));
      break;
    case PRODUCTION:
      dialog->set_title(_("Production Report"));
      break;
    case WINNING:
      dialog->set_title(_("Winning Report"));
      break;
    }
}

void ReportDialog::updateArmyChart()
{
  std::list<Uint32> bars;
  std::list<Gdk::Color> colours;
  Gdk::Color colour;
  Glib::ustring s;
  Uint32 total;
  Playerlist::iterator pit = Playerlist::getInstance()->begin();
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      total = 0;
      total = (*pit)->getStacklist()->countArmies();
      bars.push_back(total);
      SDL_Color sdl = (*pit)->getColor();
      colour.set_red(sdl.r * 255); colour.set_green(sdl.g * 255); colour.set_blue(sdl.b * 255);
      colours.push_back(colour);
      if (*pit == d_player)
	{
	  s = String::ucompose(ngettext("You have %1 army!",
					"You have %1 armies!",
					total), total);
	  army_label->set_text(s);
	}
    }

  army_chart = new BarChart(bars, colours, 0);
  army_alignment->add(*manage(army_chart));

}
void ReportDialog::updateCityChart()
{
  std::list<Uint32> bars;
  std::list<Gdk::Color> colours;
  Gdk::Color colour;
  Glib::ustring s;
  Uint32 total;
  Playerlist::iterator pit = Playerlist::getInstance()->begin();
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      total = Citylist::getInstance()->countCities(*pit);

      bars.push_back(total);
      SDL_Color sdl = (*pit)->getColor();
      colour.set_red(sdl.r * 255); colour.set_green(sdl.g * 255); colour.set_blue(sdl.b * 255);
      colours.push_back(colour);
      if (*pit == d_player)
	{
	  s = String::ucompose(ngettext("You have %1 city!",
					"You have %1 cities!",
					total), total);
	  city_label->set_text(s);
	}

    }
  city_chart = new BarChart(bars, colours, Citylist::getInstance()->size());
  city_alignment->add(*manage(city_chart));
}

void ReportDialog::updateGoldChart()
{
  std::list<Uint32> bars;
  std::list<Gdk::Color> colours;
  Gdk::Color colour;
  Glib::ustring s;
  Uint32 total;
  bars.clear();
  Playerlist::iterator pit = Playerlist::getInstance()->begin();
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      total = (*pit)->getGold();
      bars.push_back(total);
      SDL_Color sdl = (*pit)->getColor();
      colour.set_red(sdl.r * 255); colour.set_green(sdl.g * 255); colour.set_blue(sdl.b * 255);
      colours.push_back(colour);
      if (*pit == d_player)
	{
	  s = String::ucompose(ngettext("You have %1 gold piece!",
					"You have %1 gold pieces!",
					total), total);
	  gold_label->set_text(s);
	}
    }
  gold_chart = new BarChart(bars, colours, 0);
  gold_alignment->add(*manage(gold_chart));
}

std::string ReportDialog::calculateRank(std::list<Uint32> scores, Uint32 score)
{
  char* rank_strings[MAX_PLAYERS] =
    {
      _("first"), _("second"), _("third"), _("fourth"), _("fifth"),
      _("sixth"), _("seventh"), _("eighth"),
    };
  Uint32 rank = 0;
  std::list<Uint32>::iterator it = scores.begin();
  for (; it != scores.end(); it++)
    {
      if (score < *it)
	rank++;
    }
  Glib::ustring s = String::ucompose(_("%1"), rank_strings[rank]);
  return s;
}

void ReportDialog::updateWinningChart()
{
  std::list<Uint32> bars;
  std::list<Gdk::Color> colours;
  Gdk::Color colour;
  Glib::ustring s;
  Playerlist::iterator pit = Playerlist::getInstance()->begin();
  Uint32 score;
  for (; pit != Playerlist::getInstance()->end(); ++pit)
    {
      if (*pit == Playerlist::getInstance()->getNeutral())
	continue;
      score = (*pit)->getScore();
      bars.push_back(score);
      SDL_Color sdl = (*pit)->getColor();
      colour.set_red(sdl.r * 255); colour.set_green(sdl.g * 255); colour.set_blue(sdl.b * 255);
      colours.push_back(colour);
    }
  s = String::ucompose(_("You are coming %1"), calculateRank(bars, d_player->getScore()));
  winning_label->set_text(s);
  winning_chart = new BarChart(bars, colours, 100);
  winning_alignment->add(*manage(winning_chart));
}

void ReportDialog::addProduction(const Action *action)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Player *p = d_player;

  int army_type = 0;

  Glib::ustring s = "";
  if (action->getType() == Action::PRODUCE_UNIT)
    {
      const Action_Produce *act;
      act = dynamic_cast<const Action_Produce*>(action);
      army_type = act->getArmyType();
      Citylist::iterator cit = Citylist::getInstance()->begin();
      for (; cit != Citylist::getInstance()->end(); ++cit)
	if ((*cit).getId() == act->getCityId())
	  {
	    s += (*cit).getName();
	    break;
	  }
      if (act->getVectored())
	s += "...";
    }
  else if (action->getType() == Action::PRODUCE_VECTORED_UNIT)
    {
      const Action_ProduceVectored *act;
      act = dynamic_cast<const Action_ProduceVectored*>(action);
      army_type = act->getArmyType();
      Vector<int> pos = act->getDestination();
      City *c = Citylist::getInstance()->getObjectAt(pos);
      s+="...";
      if (c)
	s += c->getName();
      else
	s += _("Standard");
    }
  const Army *a;
  a = Armysetlist::getInstance()->getArmy(p->getArmyset(), army_type);
  Gtk::TreeIter i = armies_list->append();
  (*i)[armies_columns.image] = to_pixbuf(gc->getArmyPic(p->getArmyset(),
							army_type,
							p, 1, NULL));
  (*i)[armies_columns.desc] = s;
}
