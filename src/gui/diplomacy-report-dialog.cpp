//  Copyright (C) 2008, 2009, 2014 Ben Asselstine
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

#include "diplomacy-report-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "GraphicsCache.h"
#include "playerlist.h"
#include "player.h"

DiplomacyReportDialog::DiplomacyReportDialog(Gtk::Window &parent, Player *player)
 : LwDialog(parent, "diplomacy-report-dialog.ui")
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Playerlist *pl = Playerlist::getInstance();
  d_player = player;
  xml->get_widget("diplomacy_table", d_table);

  int order[MAX_PLAYERS];

  /* find the diplomatic order of the players */
  for (guint32 i = 0; i < MAX_PLAYERS; i++)
    {
      order[i] = -1;
      for (Playerlist::iterator it = pl->begin(); it != pl->end(); ++it)
	{
	  if (pl->getNeutral() == *it)
	    continue;
	  if ((*it)->isDead() == true)
	    continue;
	  if (i != (*it)->getDiplomaticRank() - 1)
	    continue;
	  order[i] = (int) (*it)->getId();
	}
    }

  /* show the players in order of their diplomatic ranking. */
  for (guint32 i = 0; i < MAX_PLAYERS; i++)
    {
      if (order[i] == -1)
	continue;
      Player *p = pl->getPlayer(order[i]);

      Glib::RefPtr<Gdk::Pixbuf> pix = gc->getShieldPic(2, p)->to_pixbuf();
      Gtk::Image *im = manage(new Gtk::Image());
      im->property_pixbuf() = pix;
      d_table->attach(*im, 1, 2, i + 1, i + 2, Gtk::SHRINK, Gtk::SHRINK);
      Gtk::Image *im2 = manage(new Gtk::Image());
      im2->property_pixbuf() = pix;
      d_table->attach(*im2, i + 2, i + 3, 0, 0 + 1, Gtk::SHRINK, Gtk::SHRINK);
      Gtk::Label *label = manage(new Gtk::Label(p->getDiplomaticTitle()));
      d_table->attach(*label, 0, 1, i + 1, i + 2, Gtk::SHRINK, Gtk::SHRINK);
  
      for (guint32 j = 0; j < MAX_PLAYERS; j++)
	{
	  if (order[j] == -1)
	    continue;
	  Player::DiplomaticState state;
	  state = p->getDiplomaticState(pl->getPlayer(order[j]));
	  Glib::RefPtr<Gdk::Pixbuf> pix2 = gc->getDiplomacyPic(0, state)->to_pixbuf();
	  Gtk::Image *im3 = manage(new Gtk::Image());
	  im3->property_pixbuf() = pix2;
	  d_table->attach(*im3, i + 2, i + 3, j + 1, j + 2, 
			  Gtk::SHRINK, Gtk::SHRINK);
	}
    }
}

DiplomacyReportDialog::~DiplomacyReportDialog()
{
}
