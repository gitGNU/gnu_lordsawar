//  Copyright (C) 2008, 2009 Ben Asselstine
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

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "diplomacy-dialog.h"
#include "diplomacy-report-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"
#include "GraphicsCache.h"
#include "playerlist.h"
#include "player.h"

DiplomacyDialog::DiplomacyDialog(Player *player)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
  Playerlist *pl = Playerlist::getInstance();
  d_player = player;
  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/diplomacy-dialog.ui");

  Gtk::Dialog *d = 0;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  decorate(dialog.get());
  window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));

  xml->get_widget("proposals_table", d_proposals_table);
  xml->get_widget("offers_table", d_offers_table);
  xml->get_widget("player_label", d_player_label);
  xml->get_widget("player_shield_image", d_player_shield_image);
  xml->get_widget("report_button", d_report_button);
    
  d_report_button->signal_clicked().connect
    (sigc::mem_fun(*this, &DiplomacyDialog::on_report_clicked));

  // put the shields across the top of the proposals table, minus our own
  Uint32 i = 0;
  Uint32 j = 0;
  for (unsigned int k = 0; k < MAX_PLAYERS; k++)
    {
      Player *p = pl->getPlayer(k);
      if (p == NULL)
	continue;
      if (pl->getNeutral() == p)
	continue;
      if (p == d_player)
	continue;
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = to_pixbuf(gc->getShieldPic(2, p));
      Gtk::Image *im = manage(new Gtk::Image(pixbuf));
      //im->set_padding(11, 0);
      d_proposals_table->attach(*im, i + 0, i + 1, 0, 1, 
				Gtk::SHRINK, Gtk::SHRINK);
      i++;
    }
  d_proposals_table->set_col_spacings (16);
    
  d_player_shield_image->property_pixbuf() = 
    to_pixbuf(gc->getShieldPic(2, d_player));

  d_player_label->set_text(d_player->getName());

  //fill in diplomatic state
  i = 0;
  j = 0;
  for (unsigned int k = 0; k < MAX_PLAYERS; k++)
    {
      Player *p = pl->getPlayer(k);
      if (p == NULL)
	continue;
      if (pl->getNeutral() == p)
	continue;
      if (p == d_player)
	continue;
      if (p->isDead())
	{
	  i++;
	  continue;
	}
      j = 0;
      Player::DiplomaticState state = d_player->getDiplomaticState (p);
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = to_pixbuf(gc->getDiplomacyPic(1, 
								       state));
      Gtk::Image *im = manage(new Gtk::Image(pixbuf));
      d_proposals_table->attach(*im, i + 0, i + 1, j + 1, j + 2, 
				Gtk::SHRINK, Gtk::SHRINK);
      Player::DiplomaticProposal proposal = p->getDiplomaticProposal (d_player);
      if (proposal != Player::NO_PROPOSAL)
	{
	  j = 1;
	  Glib::RefPtr<Gdk::Pixbuf> pixbuf2;
	  switch (proposal)
	    {
	    case Player::PROPOSE_PEACE:
	      pixbuf2 = to_pixbuf(gc->getDiplomacyPic(1, Player::AT_PEACE));
	      break;
	    case Player::PROPOSE_WAR_IN_FIELD:
	      pixbuf2 = 
		to_pixbuf(gc->getDiplomacyPic(1, Player::AT_WAR_IN_FIELD));
	      break;
	    case Player::PROPOSE_WAR:
	      pixbuf2 = to_pixbuf(gc->getDiplomacyPic(1, Player::AT_WAR));
	      break;
	    default:
	      continue;
	    }

	  Gtk::Image *im2 = manage(new Gtk::Image(pixbuf2));
	  d_proposals_table->attach(*im2, i + 0, i + 1, j + 1, j + 2, 
				    Gtk::SHRINK, Gtk::SHRINK);
	}
      i++;
    }


  // fill in the togglebuttons
  i = 0;
  j = 0;
  for (unsigned int k = 0; k < MAX_PLAYERS; k++)
    {
      Player *p = pl->getPlayer(k);
      if (p == NULL)
	continue;
      if (pl->getNeutral() == p)
	continue;
      if (p == d_player)
	continue;

      //show the peace radio buttons
      j = 0;
      Gtk::RadioButton *radio1= manage(new Gtk::RadioButton);
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
	to_pixbuf(gc->getDiplomacyPic(1, Player::AT_PEACE));
      radio1->add(*manage(new Gtk::Image(pixbuf)));
      radio1->set_mode(false);
      Gtk::RadioButtonGroup group = radio1->get_group();
      if (p->isDead())
	radio1->set_sensitive(false);
      else
	radio1->set_active (d_player->getDiplomaticProposal(p) == 
			    Player::PROPOSE_PEACE);
      radio1->signal_toggled().connect(
	    sigc::bind(sigc::mem_fun
		       (this, &DiplomacyDialog::on_proposal_toggled),
		       radio1, p, Player::PROPOSE_PEACE));
      d_offers_table->attach(*radio1, i, i + 1, j + 0, j + 1,
			     Gtk::SHRINK, Gtk::SHRINK);

      j = 1;
      Gtk::RadioButton *radio2= manage(new Gtk::RadioButton);
      Glib::RefPtr<Gdk::Pixbuf> pixbuf2 = 
	to_pixbuf(gc->getDiplomacyPic(1, Player::AT_WAR_IN_FIELD));
      radio2->add(*manage(new Gtk::Image(pixbuf2)));
      radio2->set_mode(false);
      radio2->set_group(group);
      if (p->isDead())
	radio2->set_sensitive(false);
      else
	radio2->set_active (d_player->getDiplomaticProposal(p) == 
			    Player::PROPOSE_WAR_IN_FIELD);
      radio2->signal_toggled().connect(
	    sigc::bind(sigc::mem_fun(this, 
				     &DiplomacyDialog::on_proposal_toggled),
		       radio2, p, Player::PROPOSE_WAR_IN_FIELD));
      d_offers_table->attach(*radio2, i, i + 1, j + 0, j + 1,
			     Gtk::SHRINK, Gtk::SHRINK);

      j = 2;
      Gtk::RadioButton *radio3= manage(new Gtk::RadioButton);
      Glib::RefPtr<Gdk::Pixbuf> pixbuf3 = 
	to_pixbuf(gc->getDiplomacyPic(1, Player::AT_WAR));
      radio3->add(*manage(new Gtk::Image(pixbuf3)));
      radio3->set_mode(false);
      radio3->set_group(group);
      if (p->isDead())
	radio3->set_sensitive(false);
      else
	radio3->set_active (d_player->getDiplomaticProposal(p) == 
			    Player::PROPOSE_WAR);
      radio3->signal_toggled().connect(
	    sigc::bind(sigc::mem_fun(this, 
				     &DiplomacyDialog::on_proposal_toggled),
		       radio3, p, Player::PROPOSE_WAR));
      d_offers_table->attach(*radio3, i, i + 1, j + 0, j + 1,
			     Gtk::SHRINK, Gtk::SHRINK);
      i++;
    }
}

void DiplomacyDialog::on_proposal_toggled (Gtk::ToggleButton *toggle, 
					   Player *player, 
					   Player::DiplomaticProposal proposal)
{
  if (toggle->get_active() == true)
    d_player->proposeDiplomacy (proposal, player);
}

void DiplomacyDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
  //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);

}

void DiplomacyDialog::on_report_clicked()
{
  DiplomacyReportDialog d(d_player);
  d.set_parent_window(*dialog.get());
  d.run();
  d.hide();
}

void DiplomacyDialog::hide()
{
  dialog->hide();
}
void DiplomacyDialog::run()
{
  dialog->show_all();
  dialog->run();
}

