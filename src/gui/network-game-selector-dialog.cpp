//  Copyright (C) 2008, 2009 Ben Asselstine
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

#include "network-game-selector-dialog.h"

#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include "glade-helpers.h"
#include "input-helpers.h"
#include "defs.h"
#include "File.h"
#include "profile.h"

NetworkGameSelectorDialog::NetworkGameSelectorDialog(Profile *p)
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file
	(get_glade_path() + "/pick-network-game-to-join-dialog.ui");

    xml->get_widget("dialog", dialog);
    decorate(dialog);
    dialog->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));

    window_closed.connect(sigc::mem_fun(dialog, &Gtk::Dialog::hide));

    xml->get_widget("hostname_entry", hostname_entry);
    xml->get_widget("port_spinbutton", port_spinbutton);
    hostname_entry->set_activates_default(true);
    hostname_entry->signal_changed().connect
	(sigc::mem_fun(this, &NetworkGameSelectorDialog::on_hostname_changed));
    xml->get_widget("clear_button", clear_button);
    clear_button->signal_clicked().connect
      (sigc::mem_fun(*this, &NetworkGameSelectorDialog::on_clear_clicked));
    xml->get_widget("connect_button", connect_button);
    connect_button->set_sensitive(false);
    recently_joined_games_list = 
      Gtk::ListStore::create(recently_joined_games_columns);
    xml->get_widget("recent_treeview", recent_treeview);
    recent_treeview->set_model(recently_joined_games_list);
    recent_treeview->append_column("Name", recently_joined_games_columns.name);
    recent_treeview->append_column("Turn", recently_joined_games_columns.turn);
    recent_treeview->append_column("Players", recently_joined_games_columns.number_of_players);
    recent_treeview->append_column("Cities", recently_joined_games_columns.number_of_cities);
    recent_treeview->append_column("Host", recently_joined_games_columns.host);
    recent_treeview->append_column("Port", recently_joined_games_columns.port);
    recent_treeview->set_headers_visible(true);
    recent_treeview->get_selection()->signal_changed().connect
          (sigc::mem_fun(*this, &NetworkGameSelectorDialog::on_recent_game_selected));
    
    RecentlyPlayedGameList *rpgl = RecentlyPlayedGameList::getInstance();
    rpgl->pruneGames();
    for (RecentlyPlayedGameList::iterator it = rpgl->begin(); it != rpgl->end();
	 it++)
      {
	if ((*it)->getPlayMode() == GameScenario::NETWORKED)
	  {
	    RecentlyPlayedNetworkedGame *game;
	    game = dynamic_cast<RecentlyPlayedNetworkedGame*>(*it);
            if (game->getProfileId() == p->getId())
              addRecentlyJoinedGame(game);
	  }
      }
    port_spinbutton->set_value(LORDSAWAR_PORT);

    if (recently_joined_games_list->children().size() == 0)
      clear_button->set_sensitive(false);
    else
      {
        Gtk::TreeModel::Row row;
        row = recent_treeview->get_model()->children()[0];
        if (row)
          recent_treeview->get_selection()->select(row);
      }
}
	    
NetworkGameSelectorDialog::~NetworkGameSelectorDialog()
{
  delete dialog;
}

void NetworkGameSelectorDialog::addRecentlyJoinedGame(RecentlyPlayedNetworkedGame*recent)
{
    Gtk::TreeIter i = recently_joined_games_list->append();
    (*i)[recently_joined_games_columns.name] = recent->getName();
    (*i)[recently_joined_games_columns.turn] = recent->getRound();
    (*i)[recently_joined_games_columns.number_of_players] = recent->getNumberOfPlayers();
    (*i)[recently_joined_games_columns.number_of_cities] = recent->getNumberOfCities();
    (*i)[recently_joined_games_columns.host] = recent->getHost();
    (*i)[recently_joined_games_columns.port] = recent->getPort();
}


void NetworkGameSelectorDialog::on_hostname_changed()
{
  //validate the ip/hostname
  if (hostname_entry->get_text().length() > 0)
    {
      //connect_button->grab_focus();
      connect_button->set_sensitive(true);
      connect_button->property_can_focus() = true;
      connect_button->property_can_default() = true;
      connect_button->property_has_default() = true;
      hostname_entry->property_activates_default() = true;
      connect_button->property_receives_default() = true;
    }
  else
    connect_button->set_sensitive(false);
}

void NetworkGameSelectorDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
  //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void NetworkGameSelectorDialog::hide()
{
  dialog->hide();
}

bool NetworkGameSelectorDialog::run()
{
  int response = dialog->run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      hide();
      game_selected.emit(hostname_entry->get_text(), LORDSAWAR_PORT);
      return true;
    }
  else
    return false;
}
          
void NetworkGameSelectorDialog::on_recent_game_selected()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = recent_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();
  Gtk::TreeModel::Row row = *iterrow;
  hostname_entry->set_text(row[recently_joined_games_columns.host]);
  port_spinbutton->set_value(row[recently_joined_games_columns.port]);
}
      
void NetworkGameSelectorDialog::on_clear_clicked()
{
  RecentlyPlayedGameList *rpgl = RecentlyPlayedGameList::getInstance();
  rpgl->removeAllNetworkedGames();
  rpgl->save();
  recently_joined_games_list->clear();
  recently_joined_games_list.reset();
}
