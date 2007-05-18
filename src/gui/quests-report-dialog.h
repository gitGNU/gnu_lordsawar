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

#ifndef QUESTS_REPORT_DIALOG_H
#define QUESTS_REPORT_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>

#include "../vector.h"

class Quest;
class Player;

// dialog for showing the quests, emits quest_selected when one is selected by
// the user (but keeps the dialog open until "close" is pressed)
class QuestsReportDialog: public sigc::trackable
{
 public:
    QuestsReportDialog(Player *player);

    void set_parent_window(Gtk::Window &parent);

    void run();

    sigc::signal<void, Quest *> quest_selected;
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;

    Player *player;
    Gtk::TreeView *quests_treeview;

    class QuestsColumns: public Gtk::TreeModelColumnRecord {
    public:
	QuestsColumns() 
        { add(hero); add(description); add(quest); }
	
	Gtk::TreeModelColumn<Glib::ustring> hero;
	Gtk::TreeModelColumn<Glib::ustring> description;
	Gtk::TreeModelColumn<Quest *> quest;
    };
    const QuestsColumns quests_columns;
    Glib::RefPtr<Gtk::ListStore> quests_list;

    void on_selection_changed();

    void add_quest(Quest *quest);
};

#endif
