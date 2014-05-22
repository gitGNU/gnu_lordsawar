//  Copyright (C) 2010, 2014 Ben Asselstine
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

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "smallmap-editor-dialog.h"

#include "glade-helpers.h"
#include "image-helpers.h"
#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "GameMap.h"
#include "GraphicsCache.h"
#include "playerlist.h"
#include "tilesetlist.h"

SmallmapEditorDialog::SmallmapEditorDialog()
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/smallmap-editor-dialog.ui");

    xml->get_widget("dialog", dialog);

    xml->get_widget("smallmap_image", smallmap_image);
    smallmap_image->signal_event().connect
      (sigc::mem_fun(*this, &SmallmapEditorDialog::on_smallmap_exposed));


    smallmap = new EditableSmallMap();
    smallmap->map_changed.connect(
	sigc::mem_fun(this, &SmallmapEditorDialog::on_map_changed));
    smallmap->road_start_placed.connect
      (sigc::mem_fun(this, &SmallmapEditorDialog::on_road_start_placed));
    smallmap->road_finish_placed.connect
      (sigc::mem_fun(this, &SmallmapEditorDialog::on_road_finish_placed));
    smallmap->road_can_be_created.connect
      (sigc::mem_fun(this, &SmallmapEditorDialog::on_road_can_be_created));
    smallmap->map_edited.connect
      (sigc::mem_fun(this, &SmallmapEditorDialog::on_map_edited));

    xml->get_widget("map_eventbox", map_eventbox);
    map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::POINTER_MOTION_MASK);
    map_eventbox->signal_button_press_event().connect(
	sigc::mem_fun(*this, &SmallmapEditorDialog::on_map_mouse_button_event));
    map_eventbox->signal_motion_notify_event().connect(
	sigc::mem_fun(*this, &SmallmapEditorDialog::on_map_mouse_motion_event));
    xml->get_widget("modes_hbox", modes_hbox);
    xml->get_widget("terrain_type_table", terrain_type_table);
    xml->get_widget("building_types_hbox", building_types_hbox);
    xml->get_widget("road_start_radiobutton", road_start_radiobutton);
    road_start_radiobutton->signal_toggled().connect(
	sigc::mem_fun(*this, &SmallmapEditorDialog::on_road_start_toggled));
    xml->get_widget("road_start_entry", road_start_entry);
    xml->get_widget("road_finish_entry", road_finish_entry);
    xml->get_widget("road_finish_radiobutton", road_finish_radiobutton);
    road_finish_radiobutton->signal_toggled().connect(
	sigc::mem_fun(*this, &SmallmapEditorDialog::on_road_finish_toggled));
    xml->get_widget("create_road_button", create_road_button);
    create_road_button->signal_clicked().connect(
	sigc::mem_fun(*this, &SmallmapEditorDialog::on_create_road_clicked));
    xml->get_widget("clear_points_button", clear_points_button);
    clear_points_button->signal_clicked().connect(
	sigc::mem_fun(*this, &SmallmapEditorDialog::on_clear_points_clicked));

    setup_pointer_radiobuttons(xml);
    setup_terrain_radiobuttons();
    pointer_radiobutton->set_active(true);
    d_needs_saving = false;
}

SmallmapEditorDialog::~SmallmapEditorDialog()
{
  delete dialog;
  delete smallmap;
}

void SmallmapEditorDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
}

void SmallmapEditorDialog::hide()
{
  dialog->hide();
}

bool SmallmapEditorDialog::run()
{
    smallmap->resize();
    smallmap->draw(Playerlist::getActiveplayer());
    dialog->show();
    on_pointer_radiobutton_toggled();
    on_terrain_radiobutton_toggled();
    dialog->run();
    return d_needs_saving;
}

void SmallmapEditorDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map,
                                          Gdk::Rectangle r)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, 
                        smallmap->get_width(), smallmap->get_height());
  smallmap_image->property_pixbuf() = pixbuf;
}

bool SmallmapEditorDialog::on_map_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS)
	return true;	// useless event
    
    smallmap->mouse_button_event(to_input_event(e));
    
    return true;
}

bool SmallmapEditorDialog::on_map_mouse_motion_event(GdkEventMotion *e)
{
    smallmap->mouse_motion_event(to_input_event(e));
    return true;
}

    
void SmallmapEditorDialog::on_create_road_clicked()
{
  if (smallmap->create_road())
    on_clear_points_clicked();
}
    
void SmallmapEditorDialog::on_clear_points_clicked()
{
  road_start_entry->set_text("");
  road_finish_entry->set_text("");
  smallmap->clear_road();
  pointer_radiobutton->set_active();
}
    
void SmallmapEditorDialog::on_road_start_toggled()
{
  smallmap->set_pointer(EditableSmallMap::PICK_NEW_ROAD_START, 1, 
                        get_terrain());
  terrain_type_table->set_sensitive(false);
  update_cursor();
}

void SmallmapEditorDialog::on_road_finish_toggled()
{
  smallmap->set_pointer(EditableSmallMap::PICK_NEW_ROAD_FINISH, 1, 
                        get_terrain());
  terrain_type_table->set_sensitive(false);
  update_cursor();
}

void SmallmapEditorDialog::setup_terrain_radiobuttons()
{
    // get rid of old ones
  std::vector<Gtk::Widget*> kids = terrain_type_table->get_children();
  for (guint i = 0; i < kids.size(); i++)
    terrain_type_table->remove(*kids[i]);

    // then add new ones from the tile set
    Tileset *tset = GameMap::getTileset();
    Gtk::RadioButton::Group group;
    bool group_set = false;
    const int no_columns = 6;
    for (unsigned int i = 0; i < tset->size(); ++i)
    {
	Tile *tile = (*tset)[i];
	TerrainItem item;
	item.button = manage(new Gtk::RadioButton);
	if (group_set)
	    item.button->set_group(group);
	else
	{
	    group = item.button->get_group();
	    group_set = true;
	}
	item.button->property_draw_indicator() = false;

	int row = i / no_columns, column = i % no_columns;
	
	terrain_type_table->attach(*item.button, column, column + 1,
				   row, row + 1, Gtk::SHRINK);
	item.button->signal_toggled().connect(
	    sigc::mem_fun(this, &SmallmapEditorDialog::on_terrain_radiobutton_toggled));

	Glib::RefPtr<Gdk::Pixbuf> pic;
	PixMask *pix = (*(*(*tile).begin())->begin())->getImage()->copy();
	PixMask::scale(pix, 40, 40);
	item.button->add(*manage(new Gtk::Image(pix->to_pixbuf())));
	delete pix;

	item.terrain = tile->getType();
	terrain_items.push_back(item);
    }

    terrain_type_table->show_all();
}

void SmallmapEditorDialog::on_terrain_radiobutton_toggled()
{
  on_pointer_radiobutton_toggled();
}

void SmallmapEditorDialog::setup_pointer_radiobutton(Glib::RefPtr<Gtk::Builder> xml,
                                                     std::string prefix,
                                                     std::string image_file,
                                                     EditableSmallMap::Pointer pointer,
                                                     int size)
{
    PointerItem item;
    xml->get_widget(prefix + "_radiobutton", item.button);
    if (prefix == "pointer")
	pointer_radiobutton = item.button;
    item.button->signal_toggled().connect(
	sigc::mem_fun(this, &SmallmapEditorDialog::on_pointer_radiobutton_toggled));
    item.pointer = pointer;
    item.size = size;
    pointer_items.push_back(item);

    Gtk::Image *image;
    xml->get_widget(prefix + "_image", image);
    image->property_file() = File::getEditorFile(image_file);
    item.button->property_draw_indicator() = false;
}

    
void SmallmapEditorDialog::setup_pointer_radiobuttons(Glib::RefPtr<Gtk::Builder> xml)
{
    setup_pointer_radiobutton(xml, "pointer", "button_selector",
			      EditableSmallMap::POINTER, 1);
    setup_pointer_radiobutton(xml, "draw_1", "button_1x1",
			      EditableSmallMap::TERRAIN, 1);
    setup_pointer_radiobutton(xml, "draw_2", "button_2x2",
			      EditableSmallMap::TERRAIN, 2);
    setup_pointer_radiobutton(xml, "draw_3", "button_3x3",
			      EditableSmallMap::TERRAIN, 3);
    setup_pointer_radiobutton(xml, "draw_6", "button_6x6",
			      EditableSmallMap::TERRAIN, 6);
    setup_pointer_radiobutton(xml, "draw_12", "button_12x12",
			      EditableSmallMap::TERRAIN, 12);
    setup_pointer_radiobutton(xml, "draw_ruin", "button_ruin",
			      EditableSmallMap::RUIN, 1);
    setup_pointer_radiobutton(xml, "draw_temple", "button_temple",
			      EditableSmallMap::TEMPLE, 1);
    setup_pointer_radiobutton(xml, "draw_city", "button_castle",
			      EditableSmallMap::CITY, 1);
    setup_pointer_radiobutton(xml, "erase", "button_erase",
			      EditableSmallMap::ERASE, 1);
}

void SmallmapEditorDialog::on_pointer_radiobutton_toggled()
{
    EditableSmallMap::Pointer pointer = EditableSmallMap::POINTER;
    int size = 1;
    
    for (std::vector<PointerItem>::iterator i = pointer_items.begin(),
	     end = pointer_items.end(); i != end; ++i)
    {
	if (i->button->get_active())
	{
	    pointer = i->pointer;
	    size = i->size;
	    break;
	}
    }
    
    if (smallmap)
	smallmap->set_pointer(pointer, size, get_terrain());

    terrain_type_table->set_sensitive(pointer == EditableSmallMap::TERRAIN);
      
    update_cursor();
}

void SmallmapEditorDialog::update_cursor()
{
    Vector<int> hotspot = Vector<int>(-1,-1);
    Glib::RefPtr<Gdk::Pixbuf> cursor =  smallmap->get_cursor(hotspot);
    map_eventbox->get_window()->set_cursor 
      (Gdk::Cursor::create
       (Gdk::Display::get_default(),  cursor, hotspot.x, hotspot.y));
}

Tile::Type SmallmapEditorDialog::get_terrain()
{
    Tile::Type terrain = Tile::GRASS;
    for (std::vector<TerrainItem>::iterator i = terrain_items.begin(),
	     end = terrain_items.end(); i != end; ++i)
    {
	if (i->button->get_active())
	{
	    terrain = i->terrain;
	    break;
	}
    }

    return terrain;
}

bool SmallmapEditorDialog::on_smallmap_exposed(GdkEvent *event)
{
  Glib::RefPtr<Gdk::Window> window = smallmap_image->get_window();
  if (window)
    {
      Cairo::RefPtr<Cairo::Surface> surface = smallmap->get_surface();
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
        Gdk::Pixbuf::create(surface, 0, 0, 
                            smallmap->get_width(), smallmap->get_height());
      smallmap_image->property_pixbuf() = pixbuf;
    }
  return true;
}

void SmallmapEditorDialog::on_road_start_placed(Vector<int> pos)
{
  Glib::ustring s = String::ucompose("%1,%2", pos.x, pos.y);
  road_start_entry->set_text(s);
  pointer_radiobutton->set_active();
  GameMap::getInstance()->calculateBlockedAvenues();
}

void SmallmapEditorDialog::on_road_finish_placed(Vector<int> pos)
{
  Glib::ustring s = String::ucompose("%1,%2", pos.x, pos.y);
  road_finish_entry->set_text(s);
  pointer_radiobutton->set_active();
}

void SmallmapEditorDialog::on_road_can_be_created(bool create_road)
{
  create_road_button->set_sensitive(create_road);
}
      
void SmallmapEditorDialog::on_map_edited()
{
  d_needs_saving = true;
  if (get_terrain() == Tile::WATER)
    smallmap->resize();
  smallmap->check_road();
}
