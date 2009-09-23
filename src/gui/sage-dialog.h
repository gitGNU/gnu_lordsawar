//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#ifndef SAGE_DIALOG_H
#define SAGE_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "ruinmap.h"
#include "player.h"
#include "hero.h"


#include "decorated.h"

// dialog for visiting a sage
class SageDialog: public Decorated
{
 public:
    SageDialog(Player *player, Hero *hero, Ruin *r);

    void set_parent_window(Gtk::Window &parent);

    void hide();
    Reward *run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    std::auto_ptr<RuinMap> ruinmap;

    Gtk::TreeView *rewards_treeview;

    class RewardsColumns: public Gtk::TreeModelColumnRecord {
    public:
	RewardsColumns() 
        { add(name); add(reward);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Reward *> reward;
    };
    const RewardsColumns rewards_columns;
    Glib::RefPtr<Gtk::ListStore> rewards_list;

    Gtk::Image *map_image;
    Gtk::Button *continue_button;
    
    Hero *hero;
    Ruin *ruin;

    std::list<Reward*> common_rewards;
    void on_map_changed(Glib::RefPtr<Gdk::Pixmap> map);
    void addReward(Reward *reward);
    Reward *grabSelectedReward();
    void on_reward_selected();
};

#endif
