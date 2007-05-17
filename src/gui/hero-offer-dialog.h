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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef HERO_OFFER_DIALOG_H
#define HERO_OFFER_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/image.h>

#include "../vectormap.h"
#include "../player.h"
#include "../hero.h"

struct SDL_Surface;

// dialog for accepting/rejecting a hero
class HeroOfferDialog: public sigc::trackable
{
 public:
    HeroOfferDialog(Player *player, Hero *hero, City *city, int gold);

    void set_parent_window(Gtk::Window &parent);

    bool run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    std::auto_ptr<VectorMap> vectormap;
    void on_male_clicked();
    void on_female_clicked();

    Gtk::Image *map_image;
    
    Hero *hero;
    City *city;

    void on_map_changed(SDL_Surface *map);
};

#endif
