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

#include "quests-report-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../QuestsManager.h"
#include "../Quest.h"

QuestsReportDialog::QuestsReportDialog(Player *theplayer)
{
    player = theplayer;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/quests-report-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);

    quests_list = Gtk::ListStore::create(quests_columns);
    xml->get_widget("treeview", quests_treeview);
    quests_treeview->set_model(quests_list);
    quests_treeview->append_column(_("Hero"), quests_columns.hero);
    quests_treeview->append_column(_("Quest"), quests_columns.description);

    dynamic_cast<Gtk::CellRendererText *>(
	quests_treeview->get_column_cell_renderer(1))
	->property_wrap_width() = 300;
    
    quests_treeview->get_selection()->signal_changed()
	.connect(sigc::mem_fun(this, &QuestsReportDialog::on_selection_changed));
    // add the quests
    std::vector<Quest*> quests
	= QuestsManager::getInstance()->getPlayerQuests(player);
    for (std::vector<Quest*>::iterator i = quests.begin(), end = quests.end();
	i != end; ++i)
	add_quest(*i);
}

void QuestsReportDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void QuestsReportDialog::run()
{
    static int width = -1;
    static int height = -1;

    if (width != -1 && height != -1)
	dialog->set_default_size(width, height);
    
    dialog->show();
    dialog->run();
    
    dialog->get_size(width, height);
}

void QuestsReportDialog::on_selection_changed()
{
    Gtk::TreeIter i = quests_treeview->get_selection()->get_selected();
    if (i)
    {
	Quest *quest = (*i)[quests_columns.quest];
	quest_selected.emit(quest);
    }
}

void QuestsReportDialog::add_quest(Quest *quest)
{
    Gtk::TreeIter i = quests_list->append();
    (*i)[quests_columns.hero] = quest->getHero()->getName();
    (*i)[quests_columns.description]
	= quest->getDescription() + "\n\n" + quest->getProgress();
    (*i)[quests_columns.quest] = quest;
}
