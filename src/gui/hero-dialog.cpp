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

#include "hero-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "../ucompose.hpp"
#include "../defs.h"
#include "../hero.h"
#include "../Item.h"
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

    xml->get_widget("info_label1", info_label1);
    xml->get_widget("info_label2", info_label2);
    fill_in_info_labels();
	
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
    item_treeview->append_column(_("Attributes"), item_columns.attributes);
    item_treeview->append_column(_("Status"), item_columns.status);

    item_treeview->get_selection()->signal_changed()
	.connect(sigc::mem_fun(this, &HeroDialog::on_selection_changed));

    on_selection_changed();

#if 0
    // debug code
    #include "../Itemlist.h"
    static bool first = true;

    if (first)
    {
	Item *item = (*Itemlist::getInstance())[0];
	hero->addToBackpack(item, 0);
	item = (*Itemlist::getInstance())[1];
	hero->addToBackpack(item, 0);
	item = (*Itemlist::getInstance())[5];
	hero->addToBackpack(item, 0);
	first = false;
    }
#endif
    
    // populate the item list
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
    GameMap *gm = GameMap::getInstance();
    dialog->show();
    dialog->run();
    if (gm->getTile(pos)->getItems().size() > 0 && 
        gm->getTile(pos)->getMaptileType() == Tile::WATER)
      {
        // splash, items lost forever
        while (gm->getTile(pos)->getItems().size())
          {
            std::list<Item*>::iterator i = gm->getTile(pos)->getItems().begin();
            gm->getTile(pos)->removeItem(*i);
          }
      }
    
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
	fill_in_info_labels();
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
	fill_in_info_labels();
    }
}

void HeroDialog::add_item(Item *item, bool in_backpack)
{
    Gtk::TreeIter i = item_list->append();
    //(*i)[item_columns.image] = to_pixbuf(item->getPic());
    (*i)[item_columns.name] = item->getName();

    // the attributes column
    std::vector<Glib::ustring> s;
    if (item->getBonus(Army::STRENGTH))
	s.push_back(String::ucompose(_("Strength: +%1"), item->getValue(Army::STRENGTH)));
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
    (*i)[item_columns.attributes] = str;
    
    if (in_backpack)
	(*i)[item_columns.status] = _("In backpack");
    else
	(*i)[item_columns.status] = _("On the ground");

    (*i)[item_columns.item] = item;
}

void HeroDialog::fill_in_info_labels()
{
    Glib::ustring s;
    // fill in first column
    // FIXME: put in real numbers
    s += String::ucompose(_("Battle: %1"), 0);
    s += "\n";
    s += String::ucompose(_("Command: %1"), 0);
    s += "\n";
    s += String::ucompose(_("Level: %1"), hero->getLevel());
    s += "\n";
    s += String::ucompose(_("Experience: %1"),
			  std::setprecision(2), hero->getXP());
    info_label1->set_text(s);

    // fill in second column
    s = "";
    // note to translators: %1 is melee strength, %2 is ranged strength
    s += String::ucompose(_("Strength: %1"),
			  hero->getStat(Army::STRENGTH));
    s += "\n";
    // note to translators: %1 is remaining moves, %2 is total moves
    s += String::ucompose(_("Moves: %1/%2"),
			  hero->getMoves(), hero->getStat(Army::MOVES));
    s += "\n";
    s += String::ucompose(_("Upkeep: %1"), hero->getUpkeep());
    info_label2->set_text(s);
}
