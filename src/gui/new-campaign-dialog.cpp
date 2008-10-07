//  Copyright (C) 2007, Ole Laursen
//  Copyright (C) 2007, 2008 Ben Asselstine
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

#include <config.h>

#include <list>

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>

#include "new-campaign-dialog.h"

#include "glade-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "Configuration.h"
#include "File.h"
#include "xmlhelper.h"
#include "Campaign.h"


NewCampaignDialog::NewCampaignDialog()
{
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/new-campaign-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    decorate(dialog.get());
    window_closed.connect(sigc::mem_fun(dialog.get(), &Gtk::Dialog::hide));

    xml->get_widget("name_label", name_label);
    xml->get_widget("description_label", description_label);
    xml->get_widget("load_button", load_button);

    campaigns_list = Gtk::ListStore::create(campaigns_columns);
    xml->get_widget("treeview", campaigns_treeview);
    campaigns_treeview->set_model(campaigns_list);
    campaigns_treeview->append_column("", campaigns_columns.name);

    campaigns_treeview->get_selection()->signal_changed()
	.connect(sigc::mem_fun(this, &NewCampaignDialog::on_selection_changed));
    // add the campaigns 
    std::list<std::string> lm = File::scanCampaigns();
    for (std::list<std::string>::iterator i = lm.begin(), end = lm.end();
	i != end; ++i)
	add_campaign(*i);

    load_button->set_sensitive(false);
}

void NewCampaignDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void NewCampaignDialog::hide()
{
  dialog->hide();
}

void NewCampaignDialog::run()
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    int response = dialog->run();
    if (response != 1)
	selected_filename = "";
    
    dialog->get_size(width, height);
}

std::string NewCampaignDialog::get_campaign_filename() 
{
    return selected_filename;
}

void NewCampaignDialog::add_campaign(std::string filename)
{
    Gtk::TreeIter i = campaigns_list->append();
    (*i)[campaigns_columns.filename] = filename;
    selected_filename = File::getCampaignFile(std::string((*i)[campaigns_columns.filename]));

    XML_Helper helper(selected_filename, std::ios::in, 
		      Configuration::s_zipfiles);

    helper.registerTag (Campaign::d_tag, sigc::mem_fun
			(this, &NewCampaignDialog::scan_campaign_details));

    if (!helper.parse())
      {
	std::cerr << "Error: Could not parse " << selected_filename << std::endl;
	(*i)[campaigns_columns.name] = filename;
	    
	return;
      }
    else
      {
	(*i)[campaigns_columns.name] = loaded_campaign_name;
	(*i)[campaigns_columns.comment] = loaded_campaign_comment;
      }

    helper.close();
}


void NewCampaignDialog::on_selection_changed()
{
    Gtk::TreeIter i = campaigns_treeview->get_selection()->get_selected();

    if (i)
      {
	name_label->set_text((*i)[campaigns_columns.name]);
	description_label->set_text((*i)[campaigns_columns.comment]);
	load_button->set_sensitive(true);
      }
    else
      load_button->set_sensitive(false);
}

bool NewCampaignDialog::scan_campaign_details(std::string tag, 
					      XML_Helper* helper)
{
    if (tag == "campaign")
    {
        if (helper->getVersion() != LORDSAWAR_SAVEGAME_VERSION)
        {
            std::cerr << "scenario has wrong version, we want "
		      << LORDSAWAR_SAVEGAME_VERSION <<",\n"
		      << "scenario offers " << helper->getVersion() <<".\n";
            return false;
        }

	std::string name;
        helper->getData(name, "name");
	std::string comment;
        helper->getData(comment, "comment");

	loaded_campaign_name = name;
	loaded_campaign_comment = comment;
	return true;
    }

    return false;
}

