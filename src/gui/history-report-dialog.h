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

#ifndef HISTORY_REPORT_DIALOG_H
#define HISTORY_REPORT_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm/dialog.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../ObjectList.h"
#include "../historymap.h"
#include "../player.h"
class Citylist;

struct SDL_Surface;

// dialog for showing all ruins and temples
// the stack parameter is used as a starting position for showing ruins
class HistoryReportDialog: public sigc::trackable
{
 public:
    HistoryReportDialog();

    void generatePastCitylists();
    void set_parent_window(Gtk::Window &parent);

    void run();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    std::auto_ptr<HistoryMap> historymap;

    Gtk::Scale *turn_scale;

    std::vector<ObjectList<City>* > past_citylists;
    Gtk::Image *map_image;
    
    void on_map_changed(SDL_Surface *map);
    void on_turn_changed(Gtk::Scale *scale);
};

#endif
