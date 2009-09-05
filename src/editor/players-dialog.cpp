//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include <assert.h>
#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "players-dialog.h"

#include "glade-helpers.h"
#include "defs.h"
#include "File.h"
#include "player.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "player.h"
//#include "real_player.h"
//#include "ai_fast.h"
//#include "ai_smart.h"
//#include "ai_dummy.h"
#include "game-parameters.h"


namespace
{
    const int default_gold = 1000;

    Glib::ustring player_type_to_string(Uint32 type)
    {
	switch (type)
	{
	case Player::HUMAN: return HUMAN_PLAYER_TYPE;
	case Player::AI_FAST: return EASY_PLAYER_TYPE;
	case Player::AI_SMART: return HARD_PLAYER_TYPE;
	default: return NO_PLAYER_TYPE;
	}
    }
}

PlayersDialog::PlayersDialog(int width, int height)
    : type_column(_("Type"), type_renderer)
{
  d_width = width;
  d_height = height;
  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/players-dialog.ui");

  Gtk::Dialog *d = 0;
  xml->get_widget("dialog", d);
  dialog.reset(d);

  // setup the player settings
  player_list = Gtk::ListStore::create(player_columns);

  xml->get_widget("player_treeview", player_treeview);
  player_treeview->set_model(player_list);

  // the type column
  player_type_list = Gtk::ListStore::create(player_type_columns);
  Gtk::TreeModel::iterator i;
  i = player_type_list->append();
  (*i)[player_type_columns.type] = HUMAN_PLAYER_TYPE;
  i = player_type_list->append();
  (*i)[player_type_columns.type] = EASY_PLAYER_TYPE;
  i = player_type_list->append();
  (*i)[player_type_columns.type] = HARD_PLAYER_TYPE;
  i = player_type_list->append();
  (*i)[player_type_columns.type] = NO_PLAYER_TYPE;

  type_renderer.property_model() = player_type_list;
  type_renderer.property_text_column() = 0;
  type_renderer.property_has_entry() = false;
  type_renderer.property_editable() = true;

  type_renderer.signal_edited()
    .connect(sigc::mem_fun(*this, &PlayersDialog::on_type_edited));
  type_column.set_cell_data_func
    ( type_renderer, sigc::mem_fun(*this, &PlayersDialog::cell_data_type));
  player_treeview->append_column(type_column);


  // remaining columns
  player_treeview->append_column_editable(_("Name"), player_columns.name);
  player_treeview->append_column_editable(_("Gold"), player_columns.gold);

  // add default players
  default_player_names.push_back("The Sirians");
  default_player_names.push_back("Elvallie");
  default_player_names.push_back("Storm Giants");
  default_player_names.push_back("The Selentines");
  default_player_names.push_back("Grey Dwarves");
  default_player_names.push_back("Horse Lords");
  default_player_names.push_back("Orcs of Kor");
  default_player_names.push_back("Lord Bane");

  Playerlist *pl = Playerlist::getInstance();

  // merge defined players with predefined
  std::vector<Player *> players_to_add(default_player_names.size(), 0);
  for (Playerlist::iterator i = pl->begin(), end = pl->end(); i != end; ++i)
    {
      Player *player = *i;
      if (player == pl->getNeutral())
	continue;
      players_to_add[player->getId()] = player;
    }

  player_name_seq::iterator current_name = default_player_names.begin();
  for (unsigned int j = 0; j < players_to_add.size(); ++j)
    if (players_to_add[j])
      {
	Player *player = players_to_add[j];
	add_player(player_type_to_string(player->getType()),
		   player->getName(), player->getGold(), player);
	++current_name;
      }
    else
      {
	add_player(NO_PLAYER_TYPE, *current_name, default_gold, 0);
	++current_name;
      }
}

PlayersDialog::~PlayersDialog()
{
}

void PlayersDialog::set_parent_window(Gtk::Window &parent)
{
  dialog->set_transient_for(parent);
}

void PlayersDialog::run()
{
  dialog->show_all();
  int response = dialog->run();
  Playerlist *pl = Playerlist::getInstance();

  if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {

      // update the player list
      int c = 0;
      for (Gtk::TreeIter i = player_list->children().begin(),
	   end = player_list->children().end(); i != end; ++i, ++c)
	{
	  Glib::ustring type = (*i)[player_columns.type];
	  Glib::ustring name = (*i)[player_columns.name];
	  int gold = (*i)[player_columns.gold];

	  GameParameters::Player player;
	  player.name = name;
	  if (type == HUMAN_PLAYER_TYPE)
	    player.type = GameParameters::Player::HUMAN;
	  else if (type == EASY_PLAYER_TYPE)
	    player.type = GameParameters::Player::EASY;
	  else if (type == HARD_PLAYER_TYPE)
	    player.type = GameParameters::Player::HARD;
	  else if (type == NO_PLAYER_TYPE)
	    player.type = GameParameters::Player::OFF;
	  player.id = c;
	  pl->syncPlayer(player);
	  Player *p = pl->getPlayer(player.id);
	  (*i)[player_columns.player] = p;
	  if (p)
	    p->setGold(gold);
	}
    }

}

void PlayersDialog::cell_data_type(Gtk::CellRenderer *renderer,
				   const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text()
    = (*i)[player_columns.type];
}

void PlayersDialog::on_type_edited(const Glib::ustring &path,
				   const Glib::ustring &new_text)
{
  (*player_list->get_iter(Gtk::TreePath(path)))[player_columns.type]
    = new_text;
}

void PlayersDialog::add_player(const Glib::ustring &type,
			       const Glib::ustring &name, int gold,
			       Player *player)
{
  Gtk::TreeIter i = player_list->append();
  (*i)[player_columns.type] = type;
  (*i)[player_columns.name] = name;
  (*i)[player_columns.gold] = gold;
  (*i)[player_columns.player] = player;

  player_treeview->get_selection()->select(i);
}
