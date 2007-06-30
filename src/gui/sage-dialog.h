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

#ifndef SAGE_DIALOG_H
#define SAGE_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/image.h>

#include "../ruinmap.h"
#include "../player.h"
#include "../hero.h"

struct SDL_Surface;

// dialog for visiting a sage
class SageDialog: public sigc::trackable
{
 public:
    SageDialog(Player *player, Hero *hero, Ruin *r);

    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    std::auto_ptr<RuinMap> ruinmap;

    Gtk::Image *map_image;
    
    Hero *hero;
    Ruin *ruin;

    void on_map_changed(SDL_Surface *map);
};

#endif
