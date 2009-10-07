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

#ifndef RUIN_REPORT_DIALOG_H
#define RUIN_REPORT_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "ruinmap.h"
#include "player.h"


#include "decorated.h"
// dialog for showing all ruins and temples
// the stack parameter is used as a starting position for showing ruins
class RuinReportDialog: public Decorated
{
 public:
    RuinReportDialog(Vector<int> pos);
    ~RuinReportDialog();

    void set_parent_window(Gtk::Window &parent);

    void hide();
    void run();
    
 private:
    Gtk::Dialog* dialog;
    RuinMap* ruinmap;

    Gtk::Image *map_image;
    
    void on_map_changed(Glib::RefPtr<Gdk::Pixmap> map);
    bool on_map_mouse_button_event(GdkEventButton *e);
    void fill_in_ruin_info();
    Gtk::Label *name_label;
    Gtk::Label *type_label;
    Gtk::Label *explored_label;
    Gtk::Label *description_label;
};

#endif
