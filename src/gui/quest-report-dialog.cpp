//  Copyright (C) 2007, 2008, 2009, 2012 Ben Asselstine
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
#include <sigc++/functors/mem_fun.h>

#include "quest-report-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "ucompose.hpp"
#include "hero.h"
#include "defs.h"
#include "GameMap.h"

QuestReportDialog::QuestReportDialog(std::vector<Quest *>q, Hero *hero)
{
  quests = q;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path()
                                     + "/quest-report-dialog.ui");

  xml->get_widget("dialog", dialog);
  decorate(dialog);
  window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));

  xml->get_widget("map_image", map_image);

  questmap = NULL;

  Gtk::EventBox *map_eventbox;
  xml->get_widget("map_eventbox", map_eventbox);

  xml->get_widget("label", label);
  xml->get_widget("hero_label", hero_label);


  heroes_list = Gtk::ListStore::create(heroes_columns);
  xml->get_widget("heroes_treeview", heroes_treeview);
  heroes_treeview->set_model(heroes_list);
  heroes_treeview->append_column(_("Hero"), heroes_columns.hero_name);
  heroes_treeview->set_headers_visible(false);

  heroes_list->clear();
  guint32 count = 0;
  heroes_treeview->get_selection()->signal_changed()
    .connect(sigc::mem_fun(this, &QuestReportDialog::on_hero_changed));
  for (std::vector<Quest*>::iterator it = quests.begin(); it != quests.end();
       it++)
    {
      add_questing_hero (*it, (*it)->getHero());
      if ((*it)->getHero() == hero || count == 0)
        {
          Gtk::TreeModel::Row row;
          row = heroes_treeview->get_model()->children()[count];
          heroes_treeview->get_selection()->select(row);
        }
      count++;
    }

  if (quests.size() == 0)
    fill_quest_info(NULL);
}

void QuestReportDialog::add_questing_hero(Quest *quest, Hero *h)
{
    Gtk::TreeIter i = heroes_list->append();
    (*i)[heroes_columns.hero_name] = h->getName();
    (*i)[heroes_columns.quest] = quest;
}

void QuestReportDialog::fill_quest_info(Quest *q)
{
  Glib::ustring s;

  if (questmap)
    delete questmap;
  questmap = new QuestMap(q);
  questmap->map_changed.connect
    (sigc::mem_fun(this, &QuestReportDialog::on_map_changed));
  if (dialog->get_realized() == true)
    {
      questmap->resize();
      questmap->draw(Playerlist::getActiveplayer());
    }

  if (q)
    {
      set_title(String::ucompose(_("Quest for %1"), q->getHero()->getName()));

      s = q->getDescription();
      s += "\n\n";
      s += q->getProgress();
      label->set_text(s);
    }
  else
    {
      set_title(_("No Quest"));
      int num = rand() % 3;
      switch (num)
        {
        case 0:
          s = _("Seek a quest in a temple!");
          break;
        case 1:
          s = _("Quest?  What Quest?");
          break;
        case 2:
          s = _("Thou hast no quests!");
          break;
        }
      label->set_text(s);
    }
}

void QuestReportDialog::on_hero_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = heroes_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow)
    {
      Gtk::TreeModel::Row row = *iterrow;
      Quest *quest = row[heroes_columns.quest];
      fill_quest_info(quest);
    }

}
QuestReportDialog::~QuestReportDialog()
{
  delete dialog;
  delete questmap;
}

void QuestReportDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void QuestReportDialog::hide()
{
  dialog->hide();
}
void QuestReportDialog::run()
{
  questmap->resize();
  questmap->draw(Playerlist::getActiveplayer());

  dialog->show_all();
  if (quests.size() <= 1)
    {
      hero_label->hide();
      heroes_treeview->hide();
    }
  dialog->run();
}

void QuestReportDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, 
                        questmap->get_width(), questmap->get_height());
    map_image->property_pixbuf() = pixbuf;
}

