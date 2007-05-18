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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.*

#include <config.h>

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>

#include "cities-report-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../playerlist.h"
#include "../player.h"
#include "../citylist.h"
#include "../city.h"
#include "../FogMap.h"

CitiesReportDialog::CitiesReportDialog(Player *theplayer)
{
    player = theplayer;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/cities-report-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    cities_list = Gtk::ListStore::create(cities_columns);
    xml->get_widget("treeview", cities_treeview);
    cities_treeview->set_model(cities_list);
    cities_treeview->append_column(_("Name"), cities_columns.name);
    cities_treeview->append_column(_("Income"), cities_columns.income);
    cities_treeview->append_column(_("Player"), cities_columns.player);
    cities_treeview->append_column("", cities_columns.production_image);
    cities_treeview->append_column(_("Production"), cities_columns.production);
    cities_treeview->append_column(_("Progress"), cities_columns.production_progress);

    cities_treeview->get_selection()->signal_changed()
	.connect(sigc::mem_fun(this, &CitiesReportDialog::on_selection_changed));
    // add the cities, first our own
    Citylist *cl = Citylist::getInstance();
    for (Citylist::iterator i = cl->begin(), end = cl->end(); i != end; ++i)
    {
	if (i->getPlayer() == player)
	    add_city(*i);
    }

    // then those we can see, ordered by player
    for (Playerlist::iterator i = Playerlist::getInstance()->begin(),
	     end = Playerlist::getInstance()->end(); i != end; ++i)
    {
	Player *p = *i;
        if (p == player)
            continue;

	for (Citylist::iterator j = cl->begin(), jend = cl->end();
	     j != jend; ++j)
	{
	    if (j->getPlayer() != p)
		continue;

	    // FIXME: fog map is not working properly
	    //if (player->getFogMap()->getFogTile(j->getPos()) == FogMap::OPEN)
		add_city(*j);
	}
    }
}

void CitiesReportDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void CitiesReportDialog::run()
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    dialog->run();
    
    dialog->get_size(width, height);
}

void CitiesReportDialog::on_selection_changed()
{
    Gtk::TreeIter i = cities_treeview->get_selection()->get_selected();
    if (i)
    {
	City *city = (*i)[cities_columns.city];
	city_selected.emit(city);
    }
}

void CitiesReportDialog::add_city(City &city)
{
    Gtk::TreeIter i = cities_list->append();
    (*i)[cities_columns.name] = city.getName();
    (*i)[cities_columns.income] = String::ucompose("%1", city.getGold());
    (*i)[cities_columns.player] = city.getPlayer()->getName();
    if (city.getPlayer() == player)
    {
	int index = city.getProductionIndex();
        if (index != -1)
        {
	    const Army* a = city.getArmy(index);
	    // FIXME: smaller image needed
	    //(*i)[cities_columns.production_image] = to_pixbuf(a->getPixmap());
	    (*i)[cities_columns.production] = a->getName();
	    // note to translators: %1 is steps completed and %2 is total steps
	    // in the production
	    (*i)[cities_columns.production_progress] =
		String::ucompose(_("%1/%2"),
				 a->getProduction() - city.getDuration() + 1,
				 a->getProduction());
        }
        else
	    (*i)[cities_columns.production] = _("None");
    }
    (*i)[cities_columns.city] = &city;
}
