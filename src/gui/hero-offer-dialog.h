//  Copyright (C) 2007 Ole Laursen
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

#ifndef HERO_OFFER_DIALOG_H
#define HERO_OFFER_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "heromap.h"
#include "player.h"
#include "hero.h"
#include "heroproto.h"

#include "decorated.h"

//! dialog for accepting/rejecting a hero
class HeroOfferDialog: public Decorated
{
 public:
    HeroOfferDialog(Player *player, HeroProto *hero, City *city, int gold);
    ~HeroOfferDialog();

    void set_parent_window(Gtk::Window &parent);

    bool run();
    void hide();
    
 private:
    Gtk::Dialog* dialog;
    //! The smallmap that shows where the Hero is emerging.
    HeroMap* heromap;

    Gtk::Image *map_image;
    Gtk::Image *hero_image;
    Gtk::RadioButton *male_radiobutton;
    Gtk::Entry *name_entry;
    Gtk::Button *accept_button;
    
    HeroProto *hero;
    City *city;

    void on_male_toggled();
    void on_map_changed(Cairo::RefPtr<Cairo::Surface> map);
    void update_buttons();
    void on_name_changed();

};

#endif
