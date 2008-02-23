//  Copyright (C) 2007, Ole Laursen
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

#include <assert.h>
#include <sigc++/functors/mem_fun.h>
#include <libglademm/xml.h>

#include "players-dialog.h"

#include "glade-helpers.h"
#include "../defs.h"
#include "../File.h"
#include "../player.h"
#include "../playerlist.h"
#include "../armysetlist.h"
#include "../stacklist.h"
#include "../citylist.h"
#include "../player.h"
#include "../real_player.h"
#include "../ai_fast.h"
#include "../ai_smart.h"
#include "../ai_dummy.h"


#define HUMAN_PLAYER_TYPE _("Human")
#define EASY_PLAYER_TYPE _("Easy")
#define HARD_PLAYER_TYPE _("Hard")
#define NO_PLAYER_TYPE _("Off")

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
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path() + "/players-dialog.glade");

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
    type_column.set_cell_data_func(
	type_renderer,
	sigc::mem_fun(*this, &PlayersDialog::cell_data_type));
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
	if (player != pl->getNeutral())
	{
	    bool found = false;
	    for (unsigned int j = 0; j < default_player_names.size(); ++j)
		if (player->getName() == default_player_names[j])
		{
		    players_to_add[j] = player;
		    found = true;
		    break;
		}

	    if (!found)
	    {
		for (unsigned int j = 0; j < players_to_add.size(); ++j)
		    if (!players_to_add[j])
			players_to_add[j] = player;
	    }
	}
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
	    //make the neutral player
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
    int min_player_id = -1;
    dialog->show_all();
    int response = dialog->run();
    Playerlist *pl = Playerlist::getInstance();

    if (response == 0)		// accepted
    {
	Uint32 default_armyset = Armysetlist::getInstance()->getArmysets()[0];
	
	// update the player list
	int c = 0;
	for (Gtk::TreeIter i = player_list->children().begin(),
		 end = player_list->children().end(); i != end; ++i, ++c)
	{
	    Player *player = (*i)[player_columns.player];
	    Glib::ustring type = (*i)[player_columns.type];
	    Player::Type type_as_enum = Player::HUMAN;
	    Glib::ustring name = (*i)[player_columns.name];
	    int gold = (*i)[player_columns.gold];

	    if (type == HUMAN_PLAYER_TYPE)
		type_as_enum = Player::HUMAN;
	    else if (type == EASY_PLAYER_TYPE)
		type_as_enum = Player::AI_FAST;
	    else if (type == HARD_PLAYER_TYPE)
		type_as_enum = Player::AI_SMART;

	    if (player && type == NO_PLAYER_TYPE) // player was removed
	    {
		pl->remove(player);

		// we need to take care of the player's things, for simplicity,
		// we just have them taken over by the neutral player
		Player* neutral = pl->getNeutral();
		Stacklist* sl = player->getStacklist();
		while (!sl->empty())
		{
		    neutral->addStack(sl->front());
		    sl->erase(sl->begin());
		}

		Citylist* cl = Citylist::getInstance();
		for (Citylist::iterator it = cl->begin(); it != cl->end(); ++it)
		    if (it->getOwner() == player)
			it->conquer(neutral);

		delete player;
	    }
	    else if (!player && type != NO_PLAYER_TYPE) // player was added
	    {
		Player *new_player = 0;
		if (type == HUMAN_PLAYER_TYPE)
		    new_player = 
			new RealPlayer(name, default_armyset,
				       Player::get_color_for_no(c), d_width,
				       d_height, type_as_enum, c);
		else if (type == EASY_PLAYER_TYPE)
		    new_player = 
			new AI_Fast(name, default_armyset,
				    Player::get_color_for_no(c), 
				    d_width, d_height, c);
		else if (type == HARD_PLAYER_TYPE)
		    new_player = 
			new AI_Smart(name, default_armyset,
				     Player::get_color_for_no(c), d_width,
				     d_height, c);
		    
		new_player->setGold(gold);

		//ideally this new player should be at position c in the 
		//playerlist, but it doesn't really matter
		Playerlist::iterator j = pl->begin();
		for (int k = 0; k < c; k++)
		  {
		    j++;
		    if (j == pl->end())
		      break;
		  }

		pl->insert(j, new_player);
	    }
	    else if (player && type != NO_PLAYER_TYPE &&
		     player->getType() != type_as_enum) // player was modified
	    {
		Player *new_player = 0;
		if (type == HUMAN_PLAYER_TYPE)
		    new_player = new RealPlayer(*player);
		else if (type == EASY_PLAYER_TYPE)
		    new_player = new AI_Fast(*player);
		else if (type == HARD_PLAYER_TYPE)
		    new_player = new AI_Smart(*player);
		    
		new_player->setName(name);
		new_player->setGold(gold);

		Playerlist::iterator pos
		    = std::find(pl->begin(), pl->end(), player);

		(*pos) = new_player;
		
		pl->remove(player);
		delete player;
	    }
	}
    }
	
    Playerlist::iterator j = pl->begin();
    for (; j != pl->end(); j++)
      {
	if (pl->getNeutral() == *j)
	  continue;
	if (min_player_id == -1)
	  min_player_id = (*j)->getId();
	else if ((*j)->getId() < (Uint32) min_player_id)
	  min_player_id = (*j)->getId();
      }
    //the first player needs to be the active player, so that when we
    //save and then load, that player goes first.
    if (min_player_id > -1)
      while (pl->getActiveplayer()->getId() != (Uint32) min_player_id)
	pl->nextPlayer();
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
