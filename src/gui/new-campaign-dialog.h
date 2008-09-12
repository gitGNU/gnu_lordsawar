//  Copyright (C) 2008 Ben Asselstine
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

#ifndef NEW_CAMPAIGN_DIALOG_H
#define NEW_CAMPAIGN_DIALOG_H

#include <memory>
#include <string>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>

class XML_Helper;

#include "decorated.h"
// dialog for starting a new campaign
class NewCampaignDialog: public Decorated
{
 public:
    NewCampaignDialog();

    void set_parent_window(Gtk::Window &parent);

    void run();
    void hide();

    std::string get_campaign_filename();

 private:
    std::auto_ptr<Gtk::Dialog> dialog;
    Gtk::Button *load_button;
    Gtk::Label *name_label;
    Gtk::Label *description_label;
    Gtk::TreeView *campaigns_treeview;

    class CampaignsColumns: public Gtk::TreeModelColumnRecord {
    public:
	CampaignsColumns() 
        { add(name); add(comment); add(filename); }
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Glib::ustring> comment;
	Gtk::TreeModelColumn<std::string> filename;
    };
    const CampaignsColumns campaigns_columns;
    Glib::RefPtr<Gtk::ListStore> campaigns_list;
    
    std::string selected_filename;
    
    std::string loaded_campaign_name;
    std::string loaded_campaign_comment;

    void on_selection_changed();
    bool scan_campaign_details(std::string tag, XML_Helper* helper);
    void add_campaign(std::string filename);
};

#endif
