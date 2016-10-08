//  Copyright (C) 2007, 2008, 2009, 2010, 2014, 2015 Ben Asselstine
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

#pragma once
#ifndef GUI_ARMYSET_WINDOW_H
#define GUI_ARMYSET_WINDOW_H

#include <memory>
#include <vector>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <gtkmm.h>

#include "armyproto.h"
#include "armyset.h"
#include "shield.h"

//! Armyset Editor.  Edit an Armyset.
class ArmySetWindow: public sigc::trackable
{
 public:
    ArmySetWindow(Glib::ustring load_filename = "");
    ~ArmySetWindow() {delete window;};
    
    void show() {window->show();};
    void hide() {window->hide();};

    Gtk::Window &get_window() { return *window; }

    sigc::signal<void, guint32> armyset_saved;

 private:
    Gtk::Window* window;
    Glib::ustring current_save_filename;
    Armyset *d_armyset; //current armyset
    ArmyProto *d_army; //current army
    bool needs_saving;
    bool inhibit_needs_saving;
    bool inhibit_updates;
    Gtk::Image *white_image;
    Gtk::Image *green_image;
    Gtk::Image *yellow_image;
    Gtk::Image *light_blue_image;
    Gtk::Image *red_image;
    Gtk::Image *dark_blue_image;
    Gtk::Image *orange_image;
    Gtk::Image *black_image;
    Gtk::Image *neutral_image;
    Gtk::Entry *name_entry;
    Gtk::ScrolledWindow *armies_scrolledwindow;
    Gtk::TreeView *armies_treeview;
    Gtk::TextView *description_textview;
    Gtk::Button *white_image_button;
    Gtk::Button *green_image_button;
    Gtk::Button *yellow_image_button;
    Gtk::Button *light_blue_image_button;
    Gtk::Button *red_image_button;
    Gtk::Button *dark_blue_image_button;
    Gtk::Button *orange_image_button;
    Gtk::Button *black_image_button;
    Gtk::Button *neutral_image_button;
    Gtk::SpinButton *production_spinbutton;
    Gtk::SpinButton *cost_spinbutton;
    Gtk::SpinButton *new_cost_spinbutton;
    Gtk::SpinButton *upkeep_spinbutton;
    Gtk::SpinButton *strength_spinbutton;
    Gtk::SpinButton *moves_spinbutton;
    Gtk::SpinButton *exp_spinbutton;
    Gtk::SpinButton *id_spinbutton;
    Gtk::RadioButton *gender_none_radiobutton;
    Gtk::RadioButton *gender_male_radiobutton;
    Gtk::RadioButton *gender_female_radiobutton;
    Gtk::CheckButton *awardable_checkbutton;
    Gtk::CheckButton *defends_ruins_checkbutton;
    Gtk::SpinButton *sight_spinbutton;
    Gtk::CheckButton *move_forests_checkbutton;
    Gtk::CheckButton *move_marshes_checkbutton;
    Gtk::CheckButton *move_hills_checkbutton;
    Gtk::CheckButton *move_mountains_checkbutton;
    Gtk::CheckButton *can_fly_checkbutton;
    Gtk::CheckButton *add1strinopen_checkbutton;
    Gtk::CheckButton *add2strinopen_checkbutton;
    Gtk::CheckButton *add1strinforest_checkbutton;
    Gtk::CheckButton *add2strinforest_checkbutton;
    Gtk::CheckButton *add1strinhills_checkbutton;
    Gtk::CheckButton *add2strinhills_checkbutton;
    Gtk::CheckButton *add1strincity_checkbutton;
    Gtk::CheckButton *add2strincity_checkbutton;
    Gtk::CheckButton *add1stackinhills_checkbutton;
    Gtk::CheckButton *suballcitybonus_checkbutton;
    Gtk::CheckButton *sub1enemystack_checkbutton;
    Gtk::CheckButton *sub2enemystack_checkbutton;
    Gtk::CheckButton *add1stack_checkbutton;
    Gtk::CheckButton *add2stack_checkbutton;
    Gtk::CheckButton *suballnonherobonus_checkbutton;
    Gtk::CheckButton *suballherobonus_checkbutton;
    Gtk::Button *add_army_button;
    Gtk::Button *remove_army_button;
    Gtk::Box *army_vbox;
    Gtk::MenuItem *new_armyset_menuitem;
    Gtk::MenuItem *load_armyset_menuitem;
    Gtk::MenuItem *save_armyset_menuitem;
    Gtk::MenuItem *save_as_menuitem;
    Gtk::MenuItem *validate_armyset_menuitem;
    Gtk::MenuItem *edit_armyset_info_menuitem;
    Gtk::MenuItem *edit_ship_picture_menuitem;
    Gtk::MenuItem *edit_standard_picture_menuitem;
    Gtk::MenuItem *edit_bag_picture_menuitem;
    Gtk::MenuItem *quit_menuitem;
    Gtk::MenuItem *help_about_menuitem;
    Gtk::Button *make_same_button;

    class ArmiesColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmiesColumns() 
        { add(name); add(army);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<ArmyProto *> army;
    };
    const ArmiesColumns armies_columns;
    Glib::RefPtr<Gtk::ListStore> armies_list;
    bool inhibit_scrolldown;

    bool on_delete_event();

    void addArmyType(guint32 army_type);
    void update_army_panel();
    void update_armyset_buttons();
    void update_armyset_menuitems();

    void on_new_armyset_activated();
    void on_load_armyset_activated();
    void on_save_armyset_activated();
    void on_save_as_activated();
    void on_validate_armyset_activated();
    void on_quit_activated();
    bool on_window_closed(GdkEventAny*);
    bool quit();
    void on_edit_armyset_info_activated();
    void on_edit_standard_picture_activated();
    void on_edit_bag_picture_activated();
    void on_edit_ship_picture_activated();
    void on_help_about_activated();
    void on_army_selected();
    void fill_army_image(Gtk::Button *button, Gtk::Image *image, Shield::Colour c, ArmyProto *army);
    void fill_army_info(ArmyProto *army);

    //callbacks
    void on_name_changed();
    void on_description_changed();
    void on_image_changed(Gtk::Button *button, Gtk::Image *image, Shield::Colour c);
    void on_production_changed();
    void on_production_text_changed();
    void on_cost_changed();
    void on_cost_text_changed();
    void on_new_cost_changed();
    void on_new_cost_text_changed();
    void on_upkeep_changed();
    void on_upkeep_text_changed();
    void on_strength_changed();
    void on_strength_text_changed();
    void on_moves_changed();
    void on_moves_text_changed();
    void on_exp_changed();
    void on_exp_text_changed();
    void on_sight_changed();
    void on_sight_text_changed();
    void on_id_changed();
    void on_id_text_changed();
    void on_gender_none_toggled();
    void on_gender_male_toggled();
    void on_gender_female_toggled();
    void on_awardable_toggled();
    void on_defends_ruins_toggled();
    void on_movebonus_toggled(Gtk::CheckButton *button, guint32 val);
    void on_armybonus_toggled(Gtk::CheckButton *button, guint32 val);
    void on_add_army_clicked();
    void on_remove_army_clicked();

    void on_white_all_checked();

    bool load_armyset(Glib::ustring filename);
    bool save_current_armyset();
    void update_window_title();
    void on_make_same_clicked();

    void show_add_file_error(Armyset *a, Gtk::Window &d, Glib::ustring file);
    void refresh_armies();
};

#endif
