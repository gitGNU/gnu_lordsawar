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

#ifndef STACK_REPORT_DIALOG_H
#define STACK_REPORT_DIALOG_H

#include <memory>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>

#include "../vector.h"

class Stack;
class Player;

// dialog for showing the stacks, emits stack_selected when one is selected by
// the user (but keeps the dialog open until "close" is pressed)
class ArmiesReportDialog: public sigc::trackable
{
 public:
    ArmiesReportDialog(Player *player);

    void set_parent_window(Gtk::Window &parent);

    void run();

    sigc::signal<void, Stack *> stack_selected;
    
 private:
    std::auto_ptr<Gtk::Dialog> dialog;

    Player *player;
    Gtk::TreeView *armies_treeview;

    class ArmiesColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmiesColumns() 
        { add(image); add(player); add(stack); }
	
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
	Gtk::TreeModelColumn<Glib::ustring> player;
	Gtk::TreeModelColumn<Stack *> stack;
    };
    const ArmiesColumns armies_columns;
    Glib::RefPtr<Gtk::ListStore> armies_list;

    void on_selection_changed();

    void add_stack(Stack *stack);
};

#endif
