//  Copyright (C) 2008, 2009, 2011, 2014 Ben Asselstine
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

#ifndef GUI_ITEMLIST_DIALOG_H
#define GUI_ITEMLIST_DIALOG_H

#include <gtkmm.h>

#include "Itemlist.h"
#include "lw-editor-dialog.h"

//! Scenario editor.  Edits the global list of Item objects in the scenario.
class ItemlistDialog: public LwEditorDialog
{
 public:
    ItemlistDialog(Gtk::Window &parent);
    ~ItemlistDialog() {};

 private:
    Glib::ustring current_save_filename;
    Itemlist *d_itemlist; //current itemlist
    ItemProto *d_item; //current item
    Gtk::Entry *name_entry;
    Gtk::TreeView *items_treeview;
    Gtk::Button *add_item_button;
    Gtk::Button *remove_item_button;
    Gtk::VBox *item_vbox;
    Gtk::CheckButton *add1str_checkbutton;
    Gtk::CheckButton *add2str_checkbutton;
    Gtk::CheckButton *add3str_checkbutton;
    Gtk::CheckButton *add1stack_checkbutton;
    Gtk::CheckButton *add2stack_checkbutton;
    Gtk::CheckButton *add3stack_checkbutton;
    Gtk::CheckButton *flystack_checkbutton;
    Gtk::CheckButton *doublemovestack_checkbutton;
    Gtk::CheckButton *add2goldpercity_checkbutton;
    Gtk::CheckButton *add3goldpercity_checkbutton;
    Gtk::CheckButton *add4goldpercity_checkbutton;
    Gtk::CheckButton *add5goldpercity_checkbutton;
    Gtk::CheckButton *steals_gold_checkbutton;
    Gtk::CheckButton *sinks_ships_checkbutton;
    Gtk::CheckButton *pickup_bags_checkbutton;
    Gtk::CheckButton *add_mp_checkbutton;
    Gtk::CheckButton *banish_worms_checkbutton;
    Gtk::CheckButton *burn_bridge_checkbutton;
    Gtk::CheckButton *capture_keeper_checkbutton;
    Gtk::CheckButton *summon_monster_checkbutton;
    Gtk::CheckButton *disease_city_checkbutton;
    Gtk::CheckButton *raise_defenders_checkbutton;
    Gtk::CheckButton *persuade_neutral_city_checkbutton;
    Gtk::CheckButton *teleport_to_city_checkbutton;
    Gtk::SpinButton *uses_spinbutton;
    Gtk::Button *kill_army_type_button;
    Gtk::SpinButton *steal_percent_spinbutton;
    Gtk::Button *summon_army_type_button;
    Gtk::ComboBox *building_type_to_summon_on_combobox;
    Gtk::SpinButton *disease_armies_percent_spinbutton;
    Gtk::SpinButton *add_mp_spinbutton;
    Gtk::Button *defender_army_type_button;
    Gtk::SpinButton *num_defenders_spinbutton;


    class ItemsColumns: public Gtk::TreeModelColumnRecord {
    public:
	ItemsColumns() 
        { add(name); add(item);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<ItemProto *> item;
    };
    const ItemsColumns items_columns;
    Glib::RefPtr<Gtk::ListStore> items_list;

    bool on_delete_event(GdkEventAny *e);

    void addItemProto(ItemProto *itemproto);
    void update_item_panel();
    void update_itemlist_buttons();

    void fill_item_info(ItemProto *item);

    //callbacks
    void on_name_changed();
    void on_add_item_clicked();
    void on_remove_item_clicked();
    void on_item_selected();


    void on_checkbutton_toggled(Gtk::CheckButton *checkbutton, 
				ItemProto::Bonus bonus);
    void on_add1str_toggled();
    void on_add2str_toggled();
    void on_add3str_toggled();
    void on_add1stack_toggled();
    void on_add2stack_toggled();
    void on_add3stack_toggled();
    void on_flystack_toggled();
    void on_doublemovestack_toggled();
    void on_add2goldpercity_toggled();
    void on_add3goldpercity_toggled();
    void on_add4goldpercity_toggled();
    void on_add5goldpercity_toggled();
    void on_steals_gold_toggled();
    void on_sinks_ships_toggled();
    void on_banish_worms_toggled();
    void on_burn_bridge_toggled();
    void on_uses_changed();
    void on_kill_army_type_clicked();
    void update_kill_army_type_name();
    void on_capture_keeper_toggled();
    void on_pickup_bags_toggled();
    void on_add_mp_toggled();
    void on_summon_monster_toggled();
    void on_summon_army_type_clicked();
    void update_summon_army_type_name();
    void on_disease_city_toggled();
    void on_steal_percent_changed();
    void on_steal_percent_text_changed(const Glib::ustring &s, int *p);
    void on_disease_armies_percent_changed();
    void on_disease_armies_percent_text_changed(const Glib::ustring &s, int *p);
    void on_add_mp_changed();
    void on_add_mp_text_changed(const Glib::ustring &s, int *p);
    void on_raise_defenders_toggled();
    void on_defender_type_clicked();
    void on_num_defenders_changed();
    void on_num_defenders_text_changed(const Glib::ustring &s, int *p);
    void update_raise_defender_army_type_name();
    void on_persuade_neutral_city_toggled();
    void on_teleport_to_city_toggled();
};

#endif
