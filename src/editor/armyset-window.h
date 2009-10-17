//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#ifndef GUI_ARMYLIST_WINDOW_H
#define GUI_ARMYLIST_WINDOW_H

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
    ArmySetWindow(std::string load_filename = "");
    ~ArmySetWindow();

    void show();
    void hide();

    Gtk::Window &get_window() { return *window; }

 private:
    Gtk::Window* window;
    std::string current_save_filename;
    Armyset *d_armyset; //current armyset
    ArmyProto *d_army; //current army
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
    Gtk::TreeView *armies_treeview;
    Gtk::TextView *description_textview;
    Gtk::FileChooserButton *white_image_filechooserbutton;
    Gtk::FileChooserButton *green_image_filechooserbutton;
    Gtk::FileChooserButton *yellow_image_filechooserbutton;
    Gtk::FileChooserButton *light_blue_image_filechooserbutton;
    Gtk::FileChooserButton *red_image_filechooserbutton;
    Gtk::FileChooserButton *dark_blue_image_filechooserbutton;
    Gtk::FileChooserButton *orange_image_filechooserbutton;
    Gtk::FileChooserButton *black_image_filechooserbutton;
    Gtk::FileChooserButton *neutral_image_filechooserbutton;
    Gtk::SpinButton *production_spinbutton;
    Gtk::SpinButton *cost_spinbutton;
    Gtk::SpinButton *upkeep_spinbutton;
    Gtk::SpinButton *strength_spinbutton;
    Gtk::SpinButton *moves_spinbutton;
    Gtk::SpinButton *exp_spinbutton;
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
    Gtk::CheckButton *add1strinhills_checkbutton;
    Gtk::CheckButton *add1strincity_checkbutton;
    Gtk::CheckButton *add2strincity_checkbutton;
    Gtk::CheckButton *add1stackinhills_checkbutton;
    Gtk::CheckButton *suballcitybonus_checkbutton;
    Gtk::CheckButton *sub1enemystack_checkbutton;
    Gtk::CheckButton *add1stack_checkbutton;
    Gtk::CheckButton *add2stack_checkbutton;
    Gtk::CheckButton *suballnonherobonus_checkbutton;
    Gtk::CheckButton *suballherobonus_checkbutton;
    Gtk::Button *add_army_button;
    Gtk::Button *remove_army_button;
    Gtk::VBox *army_vbox;
    Gtk::MenuItem *new_armyset_menuitem;
    Gtk::MenuItem *load_armyset_menuitem;
    Gtk::MenuItem *save_armyset_menuitem;
    Gtk::MenuItem *validate_armyset_menuitem;
    Gtk::MenuItem *edit_armyset_info_menuitem;
    Gtk::MenuItem *edit_ship_picture_menuitem;
    Gtk::MenuItem *edit_standard_picture_menuitem;
    Gtk::MenuItem *quit_menuitem;
    Gtk::MenuItem *help_about_menuitem;

    class ArmiesColumns: public Gtk::TreeModelColumnRecord {
    public:
	ArmiesColumns() 
        { add(name); add(army);}
	
	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<ArmyProto *> army;
    };
    const ArmiesColumns armies_columns;
    Glib::RefPtr<Gtk::ListStore> armies_list;

    bool on_delete_event(GdkEventAny *e);

    void addArmyType(guint32 army_type);
    void update_army_panel();
    void update_armyset_buttons();
    void update_armyset_menuitems();

    void on_new_armyset_activated();
    void on_load_armyset_activated();
    void on_save_armyset_activated();
    void on_validate_armyset_activated();
    void on_quit_activated();
    void on_edit_armyset_info_activated();
    void on_edit_standard_picture_activated();
    void on_edit_ship_picture_activated();
    void on_help_about_activated();
    void on_army_selected();
    void fill_army_image(Gtk::FileChooserButton *button, Gtk::Image *image, Shield::Colour c, ArmyProto *army);
    void fill_army_info(ArmyProto *army);

    bool load(std::string tag, XML_Helper *helper);

    //callbacks
    void on_name_changed();
    void on_description_changed();
    void on_image_changed(Gtk::FileChooserButton *button, Gtk::Image *image, Shield::Colour c);
    void on_white_image_changed();
    void on_green_image_changed();
    void on_yellow_image_changed();
    void on_light_blue_image_changed();
    void on_red_image_changed();
    void on_dark_blue_image_changed();
    void on_orange_image_changed();
    void on_black_image_changed();
    void on_neutral_image_changed();
    void on_production_changed();
    void on_cost_changed();
    void on_upkeep_changed();
    void on_strength_changed();
    void on_moves_changed();
    void on_exp_changed();
    void on_sight_changed();
    void on_gender_none_toggled();
    void on_gender_male_toggled();
    void on_gender_female_toggled();
    void on_awardable_toggled();
    void on_defends_ruins_toggled();

    void on_move_forests_toggled();
    void on_move_marshes_toggled();
    void on_move_hills_toggled();
    void on_move_mountains_toggled();
    void on_can_fly_toggled();
    void on_add1strinopen_toggled();
    void on_add2strinopen_toggled();
    void on_add1strinforest_toggled();
    void on_add1strinhills_toggled();
    void on_add1strincity_toggled();
    void on_add2strincity_toggled();
    void on_add1stackinhills_toggled();
    void on_suballcitybonus_toggled();
    void on_sub1enemystack_toggled();
    void on_add1stack_toggled();
    void on_add2stack_toggled();
    void on_suballnonherobonus_toggled();
    void on_suballherobonus_toggled();
    void on_add_army_clicked();
    void on_remove_army_clicked();

    void update_filechooserbutton(Gtk::FileChooserButton *b, ArmyProto *a, Shield::Colour c);

    void on_white_all_checked();

    void load_armyset(std::string filename);
};

#endif
