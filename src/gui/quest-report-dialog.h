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

#ifndef QUEST_REPORT_DIALOG_H
#define QUEST_REPORT_DIALOG_H

#include <memory>
#include <vector>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "questmap.h"
#include "Quest.h"
#include "reward.h"


#include "decorated.h"

// dialog for depicting a quest
class QuestReportDialog: public Decorated
{
 public:
    QuestReportDialog(Quest *quest);

    void set_parent_window(Gtk::Window &parent);

    void run();

    void hide();
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    std::auto_ptr<QuestMap> questmap;

    Gtk::Image *map_image;
    Gtk::Label *label;
    
    Quest *quest;

    void on_map_changed(Glib::RefPtr<Gdk::Pixmap> map);
};

#endif
