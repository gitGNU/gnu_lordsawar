//  Copyright (C) 2008, 2009, 2011 Ben Asselstine
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

#include <config.h>

#include <iostream>
#include <iomanip>
#include <assert.h>
#include <libgen.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>

#include <gtkmm.h>
#include "itemlist-dialog.h"

#include "gui/input-helpers.h"
#include "gui/error-utils.h"

#include "defs.h"
#include "Configuration.h"
#include "Itemlist.h"
#include "Tile.h"
#include "playerlist.h"
#include "armysetlist.h"

#include "ucompose.hpp"

#include "glade-helpers.h"
#include "select-army-dialog.h"


ItemlistDialog::ItemlistDialog()
{
  d_itemlist = Itemlist::getInstance();
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + 
				    "/itemlist-dialog.ui");

    xml->get_widget("dialog", dialog);

    xml->get_widget("name_entry", name_entry);
    name_entry->signal_changed().connect
      (sigc::mem_fun(this, &ItemlistDialog::on_name_changed));
    xml->get_widget("items_treeview", items_treeview);
    xml->get_widget("add_item_button", add_item_button);
    add_item_button->signal_clicked().connect
      (sigc::mem_fun(this, &ItemlistDialog::on_add_item_clicked));
    xml->get_widget("remove_item_button", remove_item_button);
    remove_item_button->signal_clicked().connect
      (sigc::mem_fun(this, &ItemlistDialog::on_remove_item_clicked));
    xml->get_widget("item_vbox", item_vbox);
    xml->get_widget("kill_army_type_button", kill_army_type_button);
    kill_army_type_button->signal_clicked().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_kill_army_type_clicked));
    xml->get_widget("summon_army_type_button", summon_army_type_button);
    summon_army_type_button->signal_clicked().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_summon_army_type_clicked));
    xml->get_widget("building_type_to_summon_on_combobox", 
                    building_type_to_summon_on_combobox);
    xml->get_widget("disease_city_checkbutton", disease_city_checkbutton);
    disease_city_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_disease_city_toggled));
    xml->get_widget("disease_armies_percent_spinbutton", 
                    disease_armies_percent_spinbutton);
    disease_armies_percent_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ItemlistDialog::on_disease_armies_percent_changed));
    disease_armies_percent_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, 
                     &ItemlistDialog::on_disease_armies_percent_text_changed));

    xml->get_widget("raise_defenders_checkbutton", raise_defenders_checkbutton);
    raise_defenders_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_raise_defenders_toggled));
    xml->get_widget("defender_army_type_button", defender_army_type_button);
    defender_army_type_button->signal_clicked().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_defender_type_clicked));
    xml->get_widget("num_defenders_spinbutton", num_defenders_spinbutton);
    num_defenders_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ItemlistDialog::on_num_defenders_changed));
    num_defenders_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &ItemlistDialog::on_num_defenders_text_changed));
    xml->get_widget("persuade_neutral_city_checkbutton", 
                    persuade_neutral_city_checkbutton);
    persuade_neutral_city_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_persuade_neutral_city_toggled));

    items_list = Gtk::ListStore::create(items_columns);
    items_treeview->set_model(items_list);
    items_treeview->append_column("", items_columns.name);
    items_treeview->set_headers_visible(false);

    Itemlist::iterator iter = d_itemlist->begin();
    for (;iter != d_itemlist->end(); iter++)
      addItemProto((*iter).second);
      

    xml->get_widget("add1str_checkbutton", add1str_checkbutton);
    add1str_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_add1str_toggled));
    xml->get_widget("add2str_checkbutton", add2str_checkbutton);
    add2str_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_add2str_toggled));
    xml->get_widget("add3str_checkbutton", add3str_checkbutton);
    add3str_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_add3str_toggled));
    xml->get_widget("add1stack_checkbutton", add1stack_checkbutton);
    add1stack_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_add1stack_toggled));
    xml->get_widget("add2stack_checkbutton", add2stack_checkbutton);
    add2stack_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_add2stack_toggled));
    xml->get_widget("add3stack_checkbutton", add3stack_checkbutton);
    add3stack_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_add3stack_toggled));
    xml->get_widget("flystack_checkbutton", flystack_checkbutton);
    flystack_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_flystack_toggled));
    xml->get_widget("doublemovestack_checkbutton", doublemovestack_checkbutton);
    doublemovestack_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_doublemovestack_toggled));
    xml->get_widget("add2goldpercity_checkbutton", add2goldpercity_checkbutton);
    add2goldpercity_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_add2goldpercity_toggled));
    xml->get_widget("add3goldpercity_checkbutton", add3goldpercity_checkbutton);
    add3goldpercity_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_add3goldpercity_toggled));
    xml->get_widget("add4goldpercity_checkbutton", add4goldpercity_checkbutton);
    add4goldpercity_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_add4goldpercity_toggled));
    xml->get_widget("add5goldpercity_checkbutton", add5goldpercity_checkbutton);
    add5goldpercity_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_add5goldpercity_toggled));
    xml->get_widget("steals_gold_checkbutton", steals_gold_checkbutton);
    steals_gold_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_steals_gold_toggled));
    xml->get_widget("pickup_bags_checkbutton", pickup_bags_checkbutton);
    pickup_bags_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_pickup_bags_toggled));
    xml->get_widget("add_mp_checkbutton", add_mp_checkbutton);
    add_mp_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_add_mp_toggled));
    xml->get_widget("sinks_ships_checkbutton", sinks_ships_checkbutton);
    sinks_ships_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_sinks_ships_toggled));
    xml->get_widget("banish_worms_checkbutton", banish_worms_checkbutton);
    banish_worms_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_banish_worms_toggled));
    xml->get_widget("burn_bridge_checkbutton", burn_bridge_checkbutton);
    burn_bridge_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_burn_bridge_toggled));
    xml->get_widget("capture_keeper_checkbutton", capture_keeper_checkbutton);
    capture_keeper_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_capture_keeper_toggled));
    xml->get_widget("summon_monster_checkbutton", summon_monster_checkbutton);
    summon_monster_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_summon_monster_toggled));
    xml->get_widget("uses_spinbutton", uses_spinbutton);
    uses_spinbutton->signal_changed().connect(
	sigc::mem_fun(this, &ItemlistDialog::on_uses_changed));
    xml->get_widget("steal_percent_spinbutton", steal_percent_spinbutton);
    steal_percent_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ItemlistDialog::on_steal_percent_changed));
    steal_percent_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &ItemlistDialog::on_steal_percent_text_changed));
    xml->get_widget("add_mp_spinbutton", add_mp_spinbutton);
    add_mp_spinbutton->signal_changed().connect
      (sigc::mem_fun(this, &ItemlistDialog::on_add_mp_changed));
    add_mp_spinbutton->signal_insert_text().connect
      (sigc::mem_fun(this, &ItemlistDialog::on_add_mp_text_changed));

    items_treeview->get_selection()->signal_changed().connect
      (sigc::mem_fun(*this, &ItemlistDialog::on_item_selected));
    d_item = NULL;
    guint32 max = d_itemlist->size();
    if (max)
      {
	Gtk::TreeModel::Row row;
	row = items_treeview->get_model()->children()[0];
	if(row)
	  items_treeview->get_selection()->select(row);
      }
    update_item_panel();
    update_itemlist_buttons();
}

void
ItemlistDialog::update_itemlist_buttons()
{
  if (!items_treeview->get_selection()->get_selected())
    remove_item_button->set_sensitive(false);
  else
    remove_item_button->set_sensitive(true);
  if (d_itemlist == NULL)
    add_item_button->set_sensitive(false);
  else
    add_item_button->set_sensitive(true);
}

void
ItemlistDialog::update_item_panel()
{
  //if nothing selected in the treeview, then we don't show anything in
  //the item panel
  if (items_treeview->get_selection()->get_selected() == 0)
    {
      //clear all values
      name_entry->set_text("");
      item_vbox->set_sensitive(false);
      return;
    }
  item_vbox->set_sensitive(true);
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      // Row selected
      Gtk::TreeModel::Row row = *iterrow;

      ItemProto *a = row[items_columns.item];
      d_item = a;
      fill_item_info(a);
    }
}

ItemlistDialog::~ItemlistDialog()
{
  delete dialog;
}

void ItemlistDialog::show()
{
  dialog->show();
}

void ItemlistDialog::hide()
{
  dialog->hide();
}

void ItemlistDialog::addItemProto(ItemProto *itemproto)
{
  Gtk::TreeIter i = items_list->append();
  (*i)[items_columns.name] = itemproto->getName();
  (*i)[items_columns.item] = itemproto;
}

void ItemlistDialog::on_item_selected()
{
  update_item_panel();
  update_itemlist_buttons();
}

static int inhibit_bonus_checkbuttons;

void ItemlistDialog::fill_item_info(ItemProto *item)
{
  name_entry->set_text(item->getName());
  inhibit_bonus_checkbuttons = 1;
  add1str_checkbutton->set_active(item->getBonus(ItemProto::ADD1STR));
  add2str_checkbutton->set_active(item->getBonus(ItemProto::ADD2STR));
  add3str_checkbutton->set_active(item->getBonus(ItemProto::ADD3STR));
  add1stack_checkbutton->set_active(item->getBonus(ItemProto::ADD1STACK));
  add2stack_checkbutton->set_active(item->getBonus(ItemProto::ADD2STACK));
  add3stack_checkbutton->set_active(item->getBonus(ItemProto::ADD3STACK));
  flystack_checkbutton->set_active(item->getBonus(ItemProto::FLYSTACK));
  doublemovestack_checkbutton->set_active 
    (item->getBonus(ItemProto::DOUBLEMOVESTACK));
  add2goldpercity_checkbutton->set_active
    (item->getBonus(ItemProto::ADD2GOLDPERCITY));
  add3goldpercity_checkbutton->set_active
    (item->getBonus(ItemProto::ADD3GOLDPERCITY));
  add4goldpercity_checkbutton->set_active
    (item->getBonus(ItemProto::ADD4GOLDPERCITY));
  add5goldpercity_checkbutton->set_active
    (item->getBonus(ItemProto::ADD5GOLDPERCITY));
  steals_gold_checkbutton->set_active (item->getBonus(ItemProto::STEAL_GOLD));
  pickup_bags_checkbutton->set_active (item->getBonus(ItemProto::PICK_UP_BAGS));
  add_mp_checkbutton->set_active (item->getBonus(ItemProto::ADD_2MP_STACK));
  sinks_ships_checkbutton->set_active (item->getBonus(ItemProto::SINK_SHIPS));
  banish_worms_checkbutton->set_active (item->getBonus(ItemProto::BANISH_WORMS));
  burn_bridge_checkbutton->set_active (item->getBonus(ItemProto::BURN_BRIDGE));
  capture_keeper_checkbutton->set_active 
    (item->getBonus(ItemProto::CAPTURE_KEEPER));
  uses_spinbutton->set_value(double(item->getNumberOfUsesLeft()));
  steal_percent_spinbutton->set_value(item->getPercentGoldToSteal());
  steal_percent_spinbutton->set_sensitive
    (steals_gold_checkbutton->get_active());
  add_mp_spinbutton->set_value(item->getMovementPointsToAdd());
  add_mp_spinbutton->set_sensitive (add_mp_checkbutton->get_active());
  summon_monster_checkbutton->set_active 
    (item->getBonus(ItemProto::SUMMON_MONSTER));
  building_type_to_summon_on_combobox->set_active(item->getBuildingTypeToSummonOn());
  building_type_to_summon_on_combobox->set_sensitive(summon_monster_checkbutton->get_active());
  disease_city_checkbutton->set_active(item->getBonus(ItemProto::DISEASE_CITY));
  disease_armies_percent_spinbutton->set_sensitive(disease_city_checkbutton->get_active());
  disease_armies_percent_spinbutton->set_value(item->getPercentArmiesToKill());
  update_kill_army_type_name();
  update_summon_army_type_name();
  raise_defenders_checkbutton->set_active
    (item->getBonus(ItemProto::RAISE_DEFENDERS));
  num_defenders_spinbutton->set_sensitive
    (raise_defenders_checkbutton->get_active());
  num_defenders_spinbutton->set_value(item->getNumberOfArmiesToRaise());
  update_raise_defender_army_type_name();
  persuade_neutral_city_checkbutton->set_active 
    (item->getBonus(ItemProto::PERSUADE_NEUTRALS));

  inhibit_bonus_checkbuttons = 0;
}

void ItemlistDialog::on_name_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ItemProto *a = row[items_columns.item];
      a->setName(name_entry->get_text());
      row[items_columns.name] = name_entry->get_text();
    }
}

void ItemlistDialog::on_add_item_clicked()
{
  //add a new empty item to the itemlist
  ItemProto *a = new ItemProto("Untitled");
  d_itemlist->add(a);
  //add it to the treeview
  Gtk::TreeIter i = items_list->append();
  a->setName("Untitled");
  (*i)[items_columns.name] = a->getName();
  (*i)[items_columns.item] = a;
}

void ItemlistDialog::on_remove_item_clicked()
{
  //erase the selected row from the treeview
  //remove the item from the itemlist
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      Gtk::TreeModel::Row row = *iterrow;
      ItemProto *a = row[items_columns.item];
      items_list->erase(iterrow);
      d_itemlist->remove(a);
    }
}

void ItemlistDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

int ItemlistDialog::run()
{
    dialog->show_all();
    return dialog->run();
}

void ItemlistDialog::on_checkbutton_toggled(Gtk::CheckButton *checkbutton, 
					    ItemProto::Bonus bonus)
{
  if (inhibit_bonus_checkbuttons)
    return;
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      // Row selected
      Gtk::TreeModel::Row row = *iterrow;

      d_item = row[items_columns.item];
    }
  else
    return;
  if (checkbutton->get_active())
    d_item->addBonus(bonus);
  else
    d_item->removeBonus(bonus);
}

void ItemlistDialog::on_add1str_toggled()
{
  on_checkbutton_toggled(add1str_checkbutton, ItemProto::ADD1STR);
}

void ItemlistDialog::on_add2str_toggled()
{
  on_checkbutton_toggled(add2str_checkbutton, ItemProto::ADD2STR);
}

void ItemlistDialog::on_add3str_toggled()
{
  on_checkbutton_toggled(add3str_checkbutton, ItemProto::ADD3STR);
}

void ItemlistDialog::on_add1stack_toggled()
{
  on_checkbutton_toggled(add1stack_checkbutton, ItemProto::ADD1STACK);
}

void ItemlistDialog::on_add2stack_toggled()
{
  on_checkbutton_toggled(add2stack_checkbutton, ItemProto::ADD2STACK);
}

void ItemlistDialog::on_add3stack_toggled()
{
  on_checkbutton_toggled(add3stack_checkbutton, ItemProto::ADD3STACK);
}

void ItemlistDialog::on_flystack_toggled()
{
  on_checkbutton_toggled(flystack_checkbutton, ItemProto::FLYSTACK);
}

void ItemlistDialog::on_doublemovestack_toggled()
{
  on_checkbutton_toggled(doublemovestack_checkbutton, ItemProto::DOUBLEMOVESTACK);
}

void ItemlistDialog::on_add2goldpercity_toggled()
{
  on_checkbutton_toggled(add2goldpercity_checkbutton, ItemProto::ADD2GOLDPERCITY);
}

void ItemlistDialog::on_add3goldpercity_toggled()
{
  on_checkbutton_toggled(add2goldpercity_checkbutton, ItemProto::ADD3GOLDPERCITY);
}

void ItemlistDialog::on_add4goldpercity_toggled()
{
  on_checkbutton_toggled(add4goldpercity_checkbutton, ItemProto::ADD4GOLDPERCITY);
}

void ItemlistDialog::on_add5goldpercity_toggled()
{
  on_checkbutton_toggled(add5goldpercity_checkbutton, ItemProto::ADD5GOLDPERCITY);
}

void ItemlistDialog::on_steals_gold_toggled()
{
  on_checkbutton_toggled(steals_gold_checkbutton, ItemProto::STEAL_GOLD);
  steal_percent_spinbutton->set_sensitive
    (steals_gold_checkbutton->get_active());
}

void ItemlistDialog::on_pickup_bags_toggled()
{
  on_checkbutton_toggled(pickup_bags_checkbutton, ItemProto::PICK_UP_BAGS);
}

void ItemlistDialog::on_add_mp_toggled()
{
  on_checkbutton_toggled(add_mp_checkbutton, ItemProto::ADD_2MP_STACK);
  add_mp_spinbutton->set_sensitive (add_mp_checkbutton->get_active());
}

void ItemlistDialog::on_sinks_ships_toggled()
{
  on_checkbutton_toggled(sinks_ships_checkbutton, ItemProto::SINK_SHIPS);
}
	
void ItemlistDialog::on_banish_worms_toggled()
{
  on_checkbutton_toggled(banish_worms_checkbutton, ItemProto::BANISH_WORMS);
  kill_army_type_button->set_sensitive(banish_worms_checkbutton->get_active());
}

void ItemlistDialog::on_burn_bridge_toggled()
{
  on_checkbutton_toggled(burn_bridge_checkbutton, ItemProto::BURN_BRIDGE);
}

void ItemlistDialog::on_uses_changed()
{
  if (inhibit_bonus_checkbuttons)
    return;
  Glib::RefPtr<Gtk::TreeSelection> selection = items_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      // Row selected
      Gtk::TreeModel::Row row = *iterrow;
      d_item = row[items_columns.item];
  
      d_item->setNumberOfUsesLeft(int(uses_spinbutton->get_value()));
    }
  else
    return;
}
	
void ItemlistDialog::on_kill_army_type_clicked()
{
    Player *neutral = Playerlist::getInstance()->getNeutral();
    SelectArmyDialog d(neutral, false, true);
    d.set_parent_window(*dialog);
    d.run();

    const ArmyProto *army = d.get_selected_army();
    if (army)
      d_item->setArmyTypeToKill(army->getTypeId());
    else
      d_item->setArmyTypeToKill(0);

    update_kill_army_type_name();
}

void ItemlistDialog::update_kill_army_type_name()
{
    Player *neutral = Playerlist::getInstance()->getNeutral();
    Glib::ustring name;
    if (banish_worms_checkbutton->get_active() == true)
      {
        Armysetlist *asl = Armysetlist::getInstance();
	name = asl->getArmy(neutral->getArmyset(), 
                            d_item->getArmyTypeToKill())->getName();
      }
    else
      name = _("No army type selected");
    
    kill_army_type_button->set_label(name);
}
	
void ItemlistDialog::on_capture_keeper_toggled()
{
  on_checkbutton_toggled(capture_keeper_checkbutton, ItemProto::CAPTURE_KEEPER);
}
    
void ItemlistDialog::on_summon_monster_toggled()
{
  on_checkbutton_toggled(summon_monster_checkbutton, ItemProto::SUMMON_MONSTER);
  summon_army_type_button->set_sensitive
    (summon_monster_checkbutton->get_active());
  building_type_to_summon_on_combobox->set_sensitive
    (summon_monster_checkbutton->get_active());
}
    
void ItemlistDialog::on_summon_army_type_clicked()
{
    Player *neutral = Playerlist::getInstance()->getNeutral();
    Glib::ustring name;
    if (summon_monster_checkbutton->get_active() == true)
      {
        Armysetlist *asl = Armysetlist::getInstance();
	name = asl->getArmy(neutral->getArmyset(), 
                            d_item->getArmyTypeToSummon())->getName();
      }
    else
	name = _("No army type selected");
    
    summon_army_type_button->set_label(name);
}

void ItemlistDialog::update_summon_army_type_name()
{
    Player *neutral = Playerlist::getInstance()->getNeutral();
    Glib::ustring name;
    if (summon_monster_checkbutton->get_active() == true)
      {
        summon_army_type_button->set_sensitive(true);
        Armysetlist *asl = Armysetlist::getInstance();
	name = asl->getArmy(neutral->getArmyset(), 
                            d_item->getArmyTypeToSummon())->getName();
      }
    else
      {
	name = _("No army type selected");
        summon_army_type_button->set_sensitive(false);
      }
    
    summon_army_type_button->set_label(name);
}

void ItemlistDialog::on_disease_city_toggled()
{
  on_checkbutton_toggled(disease_city_checkbutton, ItemProto::DISEASE_CITY);
  disease_armies_percent_spinbutton->set_sensitive
    (disease_city_checkbutton->get_active());
}

void ItemlistDialog::on_persuade_neutral_city_toggled()
{
  on_checkbutton_toggled(persuade_neutral_city_checkbutton, 
                         ItemProto::PERSUADE_NEUTRALS);
}


void ItemlistDialog::on_raise_defenders_toggled()
{
  on_checkbutton_toggled(raise_defenders_checkbutton, ItemProto::RAISE_DEFENDERS);
  num_defenders_spinbutton->set_sensitive
    (raise_defenders_checkbutton->get_active());
  defender_army_type_button->set_sensitive
    (raise_defenders_checkbutton->get_active());
}

void ItemlistDialog::on_steal_percent_changed()
{
  if (inhibit_bonus_checkbuttons)
    return;
  if (d_item)
    d_item->setPercentGoldToSteal(steal_percent_spinbutton->get_value());
}

void ItemlistDialog::on_steal_percent_text_changed(const Glib::ustring &s, int *p)
{
  steal_percent_spinbutton->set_value(atoi(steal_percent_spinbutton->get_text().c_str()));
  on_steal_percent_changed();
}

void ItemlistDialog::on_disease_armies_percent_changed()
{
  if (inhibit_bonus_checkbuttons)
    return;
  if (d_item)
    d_item->setPercentArmiesToKill
      (disease_armies_percent_spinbutton->get_value());
}

void ItemlistDialog::on_disease_armies_percent_text_changed(const Glib::ustring &s, int *p)
{
  disease_armies_percent_spinbutton->set_value(atoi(disease_armies_percent_spinbutton->get_text().c_str()));
  on_disease_armies_percent_changed();
}

void ItemlistDialog::on_add_mp_changed()
{
  if (inhibit_bonus_checkbuttons)
    return;
  if (d_item)
    d_item->setMovementPointsToAdd (add_mp_spinbutton->get_value());
}

void ItemlistDialog::on_add_mp_text_changed(const Glib::ustring &s, int *p)
{
  add_mp_spinbutton->set_value(atoi(add_mp_spinbutton->get_text().c_str()));
  on_add_mp_changed();
}

void ItemlistDialog::on_num_defenders_changed()
{
  if (inhibit_bonus_checkbuttons)
    return;
  if (d_item)
    d_item->setNumberOfArmiesToRaise(num_defenders_spinbutton->get_value());
}

void ItemlistDialog::on_num_defenders_text_changed(const Glib::ustring &s, int *p)
{
  num_defenders_spinbutton->set_value(atoi(num_defenders_spinbutton->get_text().c_str()));
  on_num_defenders_changed();
}

void ItemlistDialog::on_defender_type_clicked()
{
    Player *neutral = Playerlist::getInstance()->getNeutral();
    Glib::ustring name;
    if (raise_defenders_checkbutton->get_active() == true)
      {
        Armysetlist *asl = Armysetlist::getInstance();
	name = asl->getArmy(neutral->getArmyset(), 
                            d_item->getArmyTypeToRaise())->getName();
      }
    else
	name = _("No army type selected");
    
    defender_army_type_button->set_label(name);
}

void ItemlistDialog::update_raise_defender_army_type_name()
{
    Player *neutral = Playerlist::getInstance()->getNeutral();
    Glib::ustring name;
    if (raise_defenders_checkbutton->get_active() == true)
      {
        defender_army_type_button->set_sensitive(true);
        Armysetlist *asl = Armysetlist::getInstance();
	name = asl->getArmy(neutral->getArmyset(), 
                            d_item->getArmyTypeToRaise())->getName();
      }
    else
      {
	name = _("No army type selected");
        defender_army_type_button->set_sensitive(false);
      }
    
    defender_army_type_button->set_label(name);
}
