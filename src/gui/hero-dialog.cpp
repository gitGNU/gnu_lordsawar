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

#include <iomanip>

#include <libglademm/xml.h>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>

#include "hero-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../hero.h"
#include "../Item.h"
#include "../Itemlist.h"
#include "../GameMap.h"

HeroDialog::HeroDialog(Hero *h, Vector<int> p)
{
    hero = h;
    pos = p;
    
    Glib::RefPtr<Gnome::Glade::Xml> xml
	= Gnome::Glade::Xml::create(get_glade_path()
				    + "/hero-dialog.glade");

    Gtk::Dialog *d = 0;
    xml->get_widget("dialog", d);
    dialog.reset(d);
    d->set_title(hero->getName());

    Gtk::Label *hero_label;
    xml->get_widget("hero_label", hero_label);
    hero_label->set_markup("<b>" + hero->getName() + "</b>");

    Gtk::Image *hero_image;
    xml->get_widget("hero_image", hero_image);
    hero_image->property_pixbuf() = to_pixbuf(hero->getPixmap());

    Gtk::Label *info_label;
    xml->get_widget("info_label", info_label);
    Glib::ustring s;
    // FIXME: put in real numbers
    s += String::ucompose(_("Battle: %1"), 0);
    s += "\n";
    s += String::ucompose(_("Command: %1"), 0);
    s += "\n";
    s += String::ucompose(_("Level: %1"), hero->getLevel());
    s += "\n";
    s += String::ucompose(_("Experience: %1"),
			  std::setprecision(2), hero->getXP());
    info_label->set_text(s);

    xml->get_widget("drop_button", drop_button);
    xml->get_widget("pickup_button", pickup_button);

    drop_button->signal_clicked()
	.connect(sigc::mem_fun(this, &HeroDialog::on_drop_clicked));
    pickup_button->signal_clicked()
	.connect(sigc::mem_fun(this, &HeroDialog::on_pickup_clicked));
    
    item_list = Gtk::ListStore::create(item_columns);
    xml->get_widget("treeview", item_treeview);
    item_treeview->set_model(item_list);
    item_treeview->append_column("", item_columns.image);
    item_treeview->append_column(_("Name"), item_columns.name);
    item_treeview->append_column(_("Capabilities"), item_columns.capabilities);
    item_treeview->append_column(_("Status"), item_columns.status);

    item_treeview->get_selection()->signal_changed()
	.connect(sigc::mem_fun(this, &HeroDialog::on_selection_changed));

    on_selection_changed();

    Item *item = (*Itemlist::getInstance())[0];
    hero->addToBackpack(item, 0);
    item = (*Itemlist::getInstance())[1];
    hero->addToBackpack(item, 0);
    item = (*Itemlist::getInstance())[5];
    hero->addToBackpack(item, 0);
    // FIXME: populate the item list
    std::list<Item*> backpack = hero->getBackpack();
    for (std::list<Item*>::iterator i = backpack.begin(), end = backpack.end();
	i != end; ++i)
	add_item(*i, true);

    std::list<Item*> ground
	= GameMap::getInstance()->getTile(pos)->getItems();
    for (std::list<Item*>::iterator i = ground.begin(), end = ground.end();
	i != end; ++i)
	add_item(*i, false);
}

void HeroDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void HeroDialog::run()
{
    dialog->show();
    dialog->run();
}

void HeroDialog::on_selection_changed()
{
    Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
    if (i)
    {
	bool droppable = (*i)[item_columns.status] == _("In backpack");
	drop_button->set_sensitive(droppable);
	pickup_button->set_sensitive(!droppable);
    }
    else
    {
	drop_button->set_sensitive(false);
	pickup_button->set_sensitive(false);
    }
}

void HeroDialog::on_drop_clicked()
{
    Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
    if (i)
    {
	Item *item = (*i)[item_columns.item];
	GameMap::getInstance()->getTile(pos)->addItem(item);
	hero->removeFromBackpack(item);
	(*i)[item_columns.status] = _("On the ground");
	on_selection_changed();
    }
}

void HeroDialog::on_pickup_clicked()
{
    Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
    if (i)
    {
	Item *item = (*i)[item_columns.item];
	GameMap::getInstance()->getTile(pos)->removeItem(item);
	hero->addToBackpack(item, 0);
	(*i)[item_columns.status] = _("In backpack");
	on_selection_changed();
    }
}

void HeroDialog::add_item(Item *item, bool in_backpack)
{
    Gtk::TreeIter i = item_list->append();
    (*i)[item_columns.image] = to_pixbuf(item->getPic());
    (*i)[item_columns.name] = item->getName();

    // the capabilities column
    std::vector<Glib::ustring> s;
    if (item->getBonus(Army::STRENGTH))
	s.push_back(String::ucompose(_("Attack: +%1"), item->getValue(Army::STRENGTH)));
    if (item->getBonus(Army::RANGED))
	s.push_back(String::ucompose(_("Ranged: +%1"), item->getValue(Army::RANGED)));
    if (item->getBonus(Army::DEFENSE))
	s.push_back(String::ucompose(_("Defense: +%1"), item->getValue(Army::DEFENSE)));
    if (item->getBonus(Army::VITALITY))
	s.push_back(String::ucompose(_("Vitality: +%1"), item->getValue(Army::VITALITY)));
    if (item->getBonus(Army::HP))
	s.push_back(String::ucompose(_("Hitpoints: +%1"), item->getValue(Army::HP)));
    if (item->getBonus(Army::MOVES))
	s.push_back(String::ucompose(_("Moves: +%1"), item->getValue(Army::MOVES)));
    if (item->getBonus(Army::MOVE_BONUS))
	s.push_back(String::ucompose(_("Move bonus: +%1"), item->getValue(Army::MOVE_BONUS)));
    if (item->getBonus(Army::ARMY_BONUS))
	s.push_back(String::ucompose(_("Army bonus: +%1"), item->getValue(Army::ARMY_BONUS)));
    if (item->getBonus(Army::SIGHT))
	s.push_back(String::ucompose(_("Sight: +%1"), item->getValue(Army::SIGHT)));
    if (item->getBonus(Army::SHOTS))
	s.push_back(String::ucompose(_("Shots: +%1"), item->getValue(Army::SHOTS)));

    Glib::ustring str;
    bool first = true;
    for (std::vector<Glib::ustring>::iterator i = s.begin(), end = s.end();
	 i != end; ++i)
    {
	if (first)
	    first = false;
	else
	    str += "\n";
	str += *i;
    }
    (*i)[item_columns.capabilities] = str;
    
    if (in_backpack)
	(*i)[item_columns.status] = _("In backpack");
    else
	(*i)[item_columns.status] = _("On the ground");

    (*i)[item_columns.item] = item;
}

