//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
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

#include <iomanip>
#include <SDL_video.h>
#include <assert.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>

#include <gtkmm.h>
#include "main-window.h"

#include "image-helpers.h"
#include "input-helpers.h"
#include "error-utils.h"

#include "ucompose.hpp"
#include "tileset.h"
#include "tilesetlist.h"
#include "GameMap.h"
#include "defs.h"
#include "sound.h"
#include "File.h"
#include "GraphicsCache.h"
#include "GraphicsLoader.h"
#include "smallmap.h"
#include "GameScenario.h"
#include "CreateScenarioRandomize.h"
#include "armysetlist.h"
#include "Itemlist.h"
#include "playerlist.h"
#include "shieldsetlist.h"
#include "ai_dummy.h"

#include "stack.h"
#include "city.h"
#include "ruin.h"
#include "signpost.h"
#include "temple.h"
#include "citylist.h"
#include "templelist.h"
#include "ruinlist.h"
#include "signpostlist.h"
#include "roadlist.h"
#include "bridgelist.h"
#include "portlist.h"
#include "MapGenerator.h"
#include "counter.h"

#include "glade-helpers.h"
#include "editorbigmap.h"

#include "signpost-dialog.h"
#include "temple-dialog.h"
#include "ruin-dialog.h"
#include "stack-dialog.h"
#include "players-dialog.h"
#include "city-dialog.h"
#include "map-info-dialog.h"
#include "new-map-dialog.h"
#include "itemlist-dialog.h"
#include "rewardlist-dialog.h"
#include "timed-message-dialog.h"


MainWindow::MainWindow()
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path() + "/main-window.ui");

    Gtk::Window *w = 0;
    xml->get_widget("window", w);
    window.reset(w);
    w->set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));

    w->signal_delete_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_delete_event));

    // the map image
    xml->get_widget("bigmap_drawingarea", bigmap_drawingarea);
    //bigmap_drawingarea->set_double_buffered(false);
    //bigmap_drawingarea->set_app_paintable(true);
    bigmap_drawingarea->signal_expose_event().connect
      (sigc::mem_fun(*this, &MainWindow::on_bigmap_exposed));
    bigmap_drawingarea->signal_size_allocate().connect
      (sigc::mem_fun(*this, &MainWindow::on_bigmap_surface_changed));
    xml->get_widget("bigmap_eventbox", bigmap_eventbox);

    bigmap_eventbox->add_events(Gdk::BUTTON_PRESS_MASK | 
				Gdk::BUTTON_RELEASE_MASK | 
				Gdk::POINTER_MOTION_MASK |
				Gdk::KEY_PRESS_MASK);
    bigmap_eventbox->signal_button_press_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_bigmap_mouse_button_event));
    bigmap_eventbox->signal_button_release_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_bigmap_mouse_button_event));
    bigmap_eventbox->signal_motion_notify_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_bigmap_mouse_motion_event));
    bigmap_eventbox->signal_key_press_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_bigmap_key_event));
    bigmap_eventbox->signal_leave_notify_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_bigmap_leave_event));
    xml->get_widget("map_drawingarea", map_drawingarea);
    map_drawingarea->signal_expose_event().connect
      (sigc::mem_fun(*this, &MainWindow::on_smallmap_exposed));
    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);
    map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
			     Gdk::POINTER_MOTION_MASK);
    map_eventbox->signal_button_press_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_smallmap_mouse_button_event));
    map_eventbox->signal_button_release_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_smallmap_mouse_button_event));
    map_eventbox->signal_motion_notify_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_smallmap_mouse_motion_event));

    xml->get_widget("terrain_tile_style_hbox", terrain_tile_style_hbox);

    // setup pointer radiobuttons
    xml->get_widget("terrain_type_table", terrain_type_table);

    setup_pointer_radiobutton(xml, "pointer", "button_selector",
			      EditorBigMap::POINTER, 1);
    setup_pointer_radiobutton(xml, "draw_1", "button_1x1",
			      EditorBigMap::TERRAIN, 1);
    setup_pointer_radiobutton(xml, "draw_2", "button_2x2",
			      EditorBigMap::TERRAIN, 2);
    setup_pointer_radiobutton(xml, "draw_3", "button_3x3",
			      EditorBigMap::TERRAIN, 3);
    setup_pointer_radiobutton(xml, "draw_6", "button_6x6",
			      EditorBigMap::TERRAIN, 6);
    setup_pointer_radiobutton(xml, "draw_stack", "button_stack",
			      EditorBigMap::STACK, 1);
    setup_pointer_radiobutton(xml, "draw_ruin", "button_ruin",
			      EditorBigMap::RUIN, 1);
    setup_pointer_radiobutton(xml, "draw_signpost", "button_signpost",
			      EditorBigMap::SIGNPOST, 1);
    setup_pointer_radiobutton(xml, "draw_temple", "button_temple",
			      EditorBigMap::TEMPLE, 1);
    setup_pointer_radiobutton(xml, "draw_road", "button_road",
			      EditorBigMap::ROAD, 1);
    setup_pointer_radiobutton(xml, "draw_city", "button_castle",
			      EditorBigMap::CITY, 1);
    setup_pointer_radiobutton(xml, "erase", "button_erase",
			      EditorBigMap::ERASE, 1);
    setup_pointer_radiobutton(xml, "draw_port", "button_port",
			      EditorBigMap::PORT, 1);
    setup_pointer_radiobutton(xml, "draw_bridge", "button_bridge",
			      EditorBigMap::BRIDGE, 1);
    on_pointer_radiobutton_toggled();


    xml->get_widget("mouse_position_label", mouse_position_label);
    
    
    // connect callbacks for the menu
    xml->get_widget("new_map_menuitem", new_map_menuitem);
    new_map_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_new_map_activated));
    xml->get_widget("load_map_menuitem", load_map_menuitem);
    load_map_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_load_map_activated));
    xml->get_widget("save_map_menuitem", save_map_menuitem);
    save_map_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_save_map_activated));
    xml->get_widget("save_map_as_menuitem", save_map_as_menuitem);
    save_map_as_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_save_map_as_activated));
    xml->get_widget("export_as_bitmap_menuitem", export_as_bitmap_menuitem);
    export_as_bitmap_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_export_as_bitmap_activated));
    xml->get_widget("export_as_bitmap_no_game_objects_menuitem",
		    export_as_bitmap_no_game_objects_menuitem);
    export_as_bitmap_no_game_objects_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_export_as_bitmap_no_game_objects_activated));
    xml->get_widget("validate_menuitem", validate_menuitem);
    validate_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_validate_activated));
    xml->get_widget("quit_menuitem", quit_menuitem);
    quit_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_quit_activated));

    xml->get_widget("edit_players_menuitem", edit_players_menuitem);
    edit_players_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_edit_players_activated));
    xml->get_widget("edit_map_info_menuitem", edit_map_info_menuitem);
    edit_map_info_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_edit_map_info_activated));
    
    xml->get_widget("fullscreen_menuitem", fullscreen_menuitem);
    fullscreen_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_fullscreen_activated));
    xml->get_widget("toggle_tile_graphics_menuitem", toggle_tile_graphics_menuitem);
    toggle_tile_graphics_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_tile_graphics_toggled));
    xml->get_widget("toggle_grid_menuitem", toggle_grid_menuitem);
    toggle_grid_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_grid_toggled));
    xml->get_widget("smooth_map_menuitem", smooth_map_menuitem);
    smooth_map_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_smooth_map_activated));
    xml->get_widget("smooth_screen_menuitem", smooth_screen_menuitem);
    smooth_screen_menuitem->signal_activate().connect
      (sigc::mem_fun (this, &MainWindow::on_smooth_screen_activated));
    xml->get_widget("edit_items_menuitem", edit_items_menuitem);
    edit_items_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_edit_items_activated));
    xml->get_widget("edit_rewards_menuitem", edit_rewards_menuitem);
    edit_rewards_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_edit_rewards_activated));
    xml->get_widget ("random_all_cities_menuitem", random_all_cities_menuitem);
    random_all_cities_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_random_all_cities_activated));
    xml->get_widget ("random_unnamed_cities_menuitem", 
		     random_unnamed_cities_menuitem);
    random_unnamed_cities_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_random_unnamed_cities_activated));
    xml->get_widget ("random_all_ruins_menuitem", random_all_ruins_menuitem);
    random_all_ruins_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_random_all_ruins_activated));
    xml->get_widget ("random_unnamed_ruins_menuitem", 
		     random_unnamed_ruins_menuitem);
    random_unnamed_ruins_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_random_unnamed_ruins_activated));
    xml->get_widget ("random_all_temples_menuitem", 
		     random_all_temples_menuitem);
    random_all_temples_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_random_all_temples_activated));
    xml->get_widget ("random_unnamed_temples_menuitem", 
		     random_unnamed_temples_menuitem);
    random_unnamed_temples_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_random_unnamed_temples_activated));
    xml->get_widget ("random_all_signs_menuitem", 
		     random_all_signs_menuitem);
    random_all_signs_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_random_all_signs_activated));
    xml->get_widget ("random_unnamed_signs_menuitem", 
		     random_unnamed_signs_menuitem);
    random_unnamed_signs_menuitem->signal_activate().connect
       (sigc::mem_fun(this, &MainWindow::on_random_unnamed_signs_activated));
    xml->get_widget ("help_about_menuitem", help_about_menuitem);
    help_about_menuitem->signal_activate().connect
       (sigc::mem_fun(this, &MainWindow::on_help_about_activated));
}

MainWindow::~MainWindow()
{
}

void MainWindow::setup_pointer_radiobutton(Glib::RefPtr<Gtk::Builder> xml,
					   std::string prefix,
					   std::string image_file,
					   EditorBigMap::Pointer pointer,
					   int size)
{
    PointerItem item;
    xml->get_widget(prefix + "_radiobutton", item.button);
    if (prefix == "pointer")
	pointer_radiobutton = item.button;
    item.button->signal_toggled().connect(
	sigc::mem_fun(this, &MainWindow::on_pointer_radiobutton_toggled));
    item.pointer = pointer;
    item.size = size;
    pointer_items.push_back(item);

    Gtk::Image *image;
    xml->get_widget(prefix + "_image", image);
    image->property_file() = File::getEditorFile(image_file);
    item.button->property_draw_indicator() = false;
}

void MainWindow::setup_terrain_radiobuttons()
{
    // get rid of old ones
    terrain_type_table->children().erase(terrain_type_table->children().begin(),
					 terrain_type_table->children().end());

    // then add new ones from the tile set
    Tileset *tset = GameMap::getInstance()->getTileset();
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
	    sigc::mem_fun(this, &MainWindow::on_terrain_radiobutton_toggled));

	Glib::RefPtr<Gdk::Pixbuf> pic;
	pic = (*(*(*tile).begin())->begin())->getImage()->scale_simple(20, 20, Gdk::INTERP_NEAREST);
	item.button->add(*manage(new Gtk::Image(pic)));

	item.terrain = tile->getType();
	terrain_items.push_back(item);
    }

    terrain_type_table->show_all();
}

void MainWindow::show()
{
  bigmap_drawingarea->show_all();
  window->show();
  on_bigmap_surface_changed(bigmap_drawingarea->get_allocation());
}

void MainWindow::on_bigmap_surface_changed(Gtk::Allocation box)
{
  if (!bigmap.get())
    return;
  static Gtk::Allocation last_box = Gtk::Allocation(0,0,1,1);

  if (box.get_width() != last_box.get_width() || box.get_height() != last_box.get_height())
    {
      bigmap->screen_size_changed(bigmap_drawingarea->get_allocation());
      bigmap->draw();
      smallmap->draw(Playerlist::getActiveplayer());
    }
  last_box = box;
}
bool MainWindow::on_bigmap_exposed(GdkEventExpose *event)
{
  Glib::RefPtr<Gdk::Window> window = bigmap_drawingarea->get_window();
  if (window)
    {
      Glib::RefPtr<Gdk::Pixmap> surface = bigmap->get_surface();
      window->draw_drawable(bigmap_drawingarea->get_style()->get_white_gc(),
			     surface, event->area.x, event->area.y, 
			     event->area.x, event->area.y, 
			     event->area.width, event->area.height);
    }
  return true;
}
bool MainWindow::on_smallmap_exposed(GdkEventExpose *event)
{
  Glib::RefPtr<Gdk::Window> window = map_drawingarea->get_window();
  if (window)
    {
      Glib::RefPtr<Gdk::Pixmap> surface = smallmap->get_surface();
      window->draw_drawable(map_drawingarea->get_style()->get_white_gc(),
			     surface, event->area.x, event->area.y, 
			     event->area.x, event->area.y, 
			     event->area.width, event->area.height);
    }
  return true;
}

void MainWindow::init()
{
  GraphicsLoader::instantiateImages(Armysetlist::getInstance());
  GraphicsLoader::instantiateImages(Tilesetlist::getInstance());
  GraphicsLoader::instantiateImages(Shieldsetlist::getInstance());
  show_initial_map();
}

void MainWindow::hide()
{
    window->hide();
}

bool MainWindow::on_delete_event(GdkEventAny *e)
{
    hide();
    
    return true;
}

void MainWindow::show_initial_map()
{
    set_filled_map(112, 156, Tile::WATER, "default", "default", "default",
		   "default");
    setup_terrain_radiobuttons();
    remove_tile_style_buttons();
    setup_tile_style_buttons(Tile::GRASS);
}

void MainWindow::set_filled_map(int width, int height, int fill_style, std::string tileset, std::string shieldset, std::string cityset, std::string armyset)
{
    clear_map_state();
    d_width = width;
    d_height = height;

    GameMap::deleteInstance();
    GameMap::setWidth(width);
    GameMap::setHeight(height);
    GameMap::getInstance(tileset, shieldset, cityset);
    Itemlist::createStandardInstance();

    // sets up the lists
    game_scenario.reset(new GameScenario(_("Untitled"), _("No description"), true));
    d_create_scenario_names.reset(new CreateScenarioRandomize());
    //zip past the player IDs (+1 for neutral)
    for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
      fl_counter->getNextId();


    // ...however we need to do some of the setup by hand. We need to create a
    // neutral player to give cities a player upon creation...
    guint32 armyset_id = 
      Armysetlist::getInstance()->getArmyset(armyset)->getId();
    Shieldsetlist *ssl = Shieldsetlist::getInstance();
    Player* neutral = new AI_Dummy(_("Neutral"), armyset_id, 
				   ssl->getColor(shieldset, MAX_PLAYERS), 
				   width, height, MAX_PLAYERS);
    neutral->setType(Player::AI_DUMMY);
    Playerlist::getInstance()->push_back(neutral);
    Playerlist::getInstance()->setNeutral(neutral);
    Playerlist::getInstance()->nextPlayer();

    // fill the map with tile type
    Tileset* tset = GameMap::getInstance()->getTileset();
    for (unsigned int i = 0; i < tset->size(); ++i)
    {
	if ((*tset)[i]->getType() == fill_style)
	{
	    GameMap::getInstance()->fill(i);
	    break;
	}
    }

    init_map_state();
}

void MainWindow::set_random_map(int width, int height,
				int grass, int water, int swamp, int forest,
				int hills, int mountains,
				int cities, int ruins, int temples,
				int signposts, std::string tileset,
				std::string shieldset, std::string cityset,
				std::string armyset)
{
    clear_map_state();

    GameMap::deleteInstance();
    GameMap::setWidth(width);
    GameMap::setHeight(height);
    GameMap::getInstance(tileset, shieldset, cityset);

    //zip past the player IDs
    if (fl_counter)
	delete fl_counter;
    fl_counter = new FL_Counter(MAX_PLAYERS + 1);
    
    // We need to create a neutral player to give cities a player upon
    // creation...
    guint32 armyset_id = 
      Armysetlist::getInstance()->getArmyset(armyset)->getId();
    Shieldsetlist *ssl = Shieldsetlist::getInstance();
    Player* neutral = new AI_Dummy(_("Neutral"), armyset_id, 
				   ssl->getColor(shieldset, MAX_PLAYERS), 
				   width, height, MAX_PLAYERS);
    neutral->setType(Player::AI_DUMMY);
    Playerlist::getInstance()->push_back(neutral);
    Playerlist::getInstance()->setNeutral(neutral);
    Playerlist::getInstance()->nextPlayer();
    
    // create a random map
    MapGenerator gen;
        
    // first, fill the generator with data
    gen.setNoCities(cities);
    gen.setNoRuins(ruins);
    gen.setNoTemples(temples);
    gen.setNoSignposts(signposts);
    
    // if sum > 100 (percent), divide everything by a factor, the numeric error
    // is said to be grass
    int sum = grass + water + forest + swamp + hills + mountains;

    if (sum > 100)
    {
        double factor = 100 / static_cast<double>(sum);
        water = static_cast<int>(water / factor);
        forest = static_cast<int>(forest / factor);
        swamp = static_cast<int>(swamp / factor);
        hills = static_cast<int>(hills / factor);
        mountains = static_cast<int>(mountains / factor);
    }
    
    gen.setPercentages(water, forest, swamp, hills, mountains);
    
    gen.makeMap(width, height, false);
    GameMap::getInstance()->fill(&gen);

    Itemlist::createStandardInstance();
    // sets up the lists
    game_scenario.reset(new GameScenario(_("Untitled"), _("No description"), true));
    d_create_scenario_names.reset(new CreateScenarioRandomize());
    
    // now fill the lists
    const Maptile::Building* build = gen.getBuildings(width, height);
    for (int j = 0; j < height; j++)
	for (int i = 0; i < width; i++)
	    switch(build[j * width + i])
	    {
	    case Maptile::CITY:
		Citylist::getInstance()->push_back(new City(Vector<int>(i,j)));
		(*Citylist::getInstance()->rbegin())->setOwner(
		    Playerlist::getInstance()->getNeutral());
		break;
	    case Maptile::TEMPLE:
		Templelist::getInstance()->push_back(new Temple(Vector<int>(i,j)));
		break;
	    case Maptile::RUIN:
		Ruinlist::getInstance()->push_back(new Ruin(Vector<int>(i,j)));
		break;
	    case Maptile::SIGNPOST:
		Signpostlist::getInstance()->push_back(new Signpost(Vector<int>(i,j)));
		break;
	    case Maptile::ROAD:
		Roadlist::getInstance()->push_back(new Road(Vector<int>(i,j)));
		break;
	    case Maptile::BRIDGE:
		Bridgelist::getInstance()->push_back(new Bridge(Vector<int>(i,j)));
		break;
	    case Maptile::PORT:
		Portlist::getInstance()->push_back(new Port(Vector<int>(i,j)));
		break;
	    case Maptile::NONE:
		break;
	    }

    init_map_state();
}

void MainWindow::clear_map_state()
{
    bigmap.reset();
    smallmap.reset();
    game_scenario.reset();
    d_create_scenario_names.reset();
    GraphicsCache::deleteInstance();
}

void MainWindow::init_map_state()
{
    pointer_radiobutton->set_active();
    init_maps();
    //bigmap->screen_size_changed(Gtk::Allocation(0,0,640,480)); //fixme
}


bool MainWindow::on_bigmap_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
	return true;	// useless event

    if (bigmap.get())
    {
	button_event = e;	// save it for later use
	bigmap->mouse_button_event(to_input_event(e));
	if (smallmap.get())
	  smallmap->draw(Playerlist::getActiveplayer());
    }
    
    return true;
}

bool MainWindow::on_bigmap_mouse_motion_event(GdkEventMotion *e)
{
  static guint prev = 0;
  if (bigmap.get())
    {
      guint delta = e->time - prev;
      if (delta > 40 || delta < 0)
	{
	  bigmap->mouse_motion_event(to_input_event(e));
	  prev = e->time;
	}
    }
    
    return true;
}

bool MainWindow::on_bigmap_key_event(GdkEventKey *e)
{
#if 0
    if (bigmap.get()) {
	KeyPressEvent k;
	bigmap->key_press_event(k);
    }
#endif
    
    return true;
}

bool MainWindow::on_bigmap_leave_event(GdkEventCrossing *e)
{
    if (bigmap.get())
    {
	bigmap->mouse_leave_event();
    }
    
    return true;
}

bool MainWindow::on_smallmap_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
	return true;	// useless event
    
    if (smallmap.get())
	smallmap->mouse_button_event(to_input_event(e));
    
    return true;
}

bool MainWindow::on_smallmap_mouse_motion_event(GdkEventMotion *e)
{
  static guint prev = 0;
  if (smallmap.get())
    {
      guint delta = e->time - prev;
      if (delta > 100 || delta < 0)
	{
	  smallmap->mouse_motion_event(to_input_event(e));
	  prev = e->time;
	}
    }
    
    return true;
}

void MainWindow::on_new_map_activated()
{
    current_save_filename = "";

    NewMapDialog d;
    d.set_parent_window(*window.get());
    d.run();

    if (d.map_set)
    {
	if (d.map.fill_style == -1)
	    set_random_map(d.map.width, d.map.height,
			   d.map.grass, d.map.water, d.map.swamp, d.map.forest,
			   d.map.hills, d.map.mountains,
			   d.map.cities, d.map.ruins, d.map.temples, 
			   d.map.signposts, d.map.tileset, 
			   d.map.shieldset, d.map.cityset, d.map.armyset);
	else
	    set_filled_map(d.map.width, d.map.height, d.map.fill_style, 
			   d.map.tileset, d.map.shieldset, d.map.cityset,
			   d.map.armyset);
    }
}

void MainWindow::on_load_map_activated()
{
    Gtk::FileChooserDialog chooser(*window.get(), _("Choose Map to Load"));
    Gtk::FileFilter sav_filter;
    sav_filter.add_pattern("*.map");
    chooser.set_filter(sav_filter);
    chooser.set_current_folder(Configuration::s_savePath);

    chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
    chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
	
    chooser.show_all();
    int res = chooser.run();
    
    if (res == Gtk::RESPONSE_ACCEPT)
    {
	current_save_filename = chooser.get_filename();
	chooser.hide();

	clear_map_state();

	bool broken;
	game_scenario.reset(new GameScenario(current_save_filename, broken));
	d_create_scenario_names.reset(new CreateScenarioRandomize());

	if (broken)
	{
	    show_error(String::ucompose(_("Could not load map %1."),
					current_save_filename));
	    current_save_filename = "";
	    return;
	}

	init_map_state();
    }
}

void MainWindow::on_save_map_activated()
{
    if (current_save_filename.empty())
	on_save_map_as_activated();
    else
    {
	bool success = game_scenario->saveGame(current_save_filename, "map");
	if (!success)
	    show_error(_("Map was not saved!"));
    }
}

void MainWindow::on_export_as_bitmap_activated()
{
    Gtk::FileChooserDialog chooser(*window.get(), _("Choose a Name"),
				   Gtk::FILE_CHOOSER_ACTION_SAVE);
    Gtk::FileFilter sav_filter;
    sav_filter.add_pattern("*.png");
    chooser.set_filter(sav_filter);
    chooser.set_current_folder(Glib::get_home_dir());
    chooser.set_do_overwrite_confirmation();

    chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
    chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
    
    chooser.show_all();
    int res = chooser.run();
    
    if (res == Gtk::RESPONSE_ACCEPT)
    {
	current_save_filename = chooser.get_filename();
	chooser.hide();

	bool success = bigmap->saveAsBitmap(current_save_filename);
	if (!success)
	    show_error(_("Map was not exported!"));
    }
}

void MainWindow::on_export_as_bitmap_no_game_objects_activated()
{
    Gtk::FileChooserDialog chooser(*window.get(), _("Choose a Name"),
				   Gtk::FILE_CHOOSER_ACTION_SAVE);
    Gtk::FileFilter sav_filter;
    sav_filter.add_pattern("*.png");
    chooser.set_filter(sav_filter);
    chooser.set_current_folder(Glib::get_home_dir());
    chooser.set_do_overwrite_confirmation();

    chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
    chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
    
    chooser.show_all();
    int res = chooser.run();
    
    if (res == Gtk::RESPONSE_ACCEPT)
    {
	current_save_filename = chooser.get_filename();
	chooser.hide();

	bool success = bigmap->saveUnderlyingMapAsBitmap(current_save_filename);
	if (!success)
	    show_error(_("Map was not exported!"));
    }
}

void MainWindow::on_save_map_as_activated()
{
    Gtk::FileChooserDialog chooser(*window.get(), _("Choose a Name"),
				   Gtk::FILE_CHOOSER_ACTION_SAVE);
    Gtk::FileFilter sav_filter;
    sav_filter.add_pattern("*.map");
    chooser.set_filter(sav_filter);
    chooser.set_current_folder(Configuration::s_savePath);

    chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
    chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
    
    chooser.show_all();
    int res = chooser.run();
    
    if (res == Gtk::RESPONSE_ACCEPT)
    {
	current_save_filename = chooser.get_filename();
	chooser.hide();

	bool success = game_scenario->saveGame(current_save_filename, "map");
	if (!success)
	    show_error(_("Map was not saved!"));
    }
}

void MainWindow::on_quit_activated()
{
    // FIXME: ask
    bool end = true;

    if (end) {
    }
    window->hide();
}

void MainWindow::on_edit_players_activated()
{
    PlayersDialog d(d_width, d_height);
    d.set_parent_window(*window.get());
    d.run();
}

void MainWindow::on_edit_map_info_activated()
{
    MapInfoDialog d(game_scenario.get());
    d.set_parent_window(*window.get());
    d.run();
}

void MainWindow::on_fullscreen_activated()
{
    if (fullscreen_menuitem->get_active())
	window->fullscreen();
    else
	window->unfullscreen();
}
			 
void MainWindow::on_tile_graphics_toggled()
{
  bigmap->toggleViewStylesOrTypes();
  bigmap->draw();
}
void MainWindow::on_grid_toggled()
{
  bigmap->toggle_grid();
}


void MainWindow::remove_tile_style_buttons()
{
  Glib::ListHandle<Gtk::Widget*> children = 
    terrain_tile_style_hbox->get_children();
  if (!children.empty()) 
    {
      Glib::ListHandle<Gtk::Widget*>::iterator child = children.begin();
      for (; child != children.end(); child++)
	terrain_tile_style_hbox->remove(**child);
    }
  tile_style_items.clear();
}

void MainWindow::setup_tile_style_buttons(Tile::Type terrain)
{
  Gtk::RadioButton::Group group;
  //iterate through tilestyles of a certain TERRAIN tile
  Tileset *tileset = GameMap::getInstance()->getTileset();
  guint32 index = tileset->getIndex(terrain);
  Tile *tile = (*tileset)[index];
  if (tile == NULL)
    return;
	  
  TileStyleItem auto_item;
  auto_item.button = manage(new Gtk::RadioButton);
  terrain_tile_style_hbox->pack_start(*manage(auto_item.button), 
				      Gtk::PACK_SHRINK, 0, 0);
  auto_item.button->set_label(_("Auto"));
  auto_item.button->property_draw_indicator() = false;
	  
  auto_item.button->signal_toggled().connect
    (sigc::mem_fun(this, &MainWindow::on_tile_style_radiobutton_toggled));

  auto_item.tile_style_id = -1;
  group = auto_item.button->get_group();
  tile_style_items.push_back(auto_item);

  for (Tile::iterator it = tile->begin(); it != tile->end(); it++)
    {
      TileStyleSet *tilestyleset = *it;
      //loop through tile style sets
      for (unsigned int j = 0; j < tilestyleset->size(); j++)
	{
	  //now loop through the tile styles
	  TileStyle *tilestyle = (*tilestyleset)[j];

	  //let's make a button
	  TileStyleItem item;
	  item.button = manage(new Gtk::RadioButton);
	  item.button->set_group(group);
	  item.button->property_draw_indicator() = false;

	  terrain_tile_style_hbox->pack_start(*manage(item.button), 
					      Gtk::PACK_SHRINK, 0, 0);
	  item.button->signal_toggled().connect
	    (sigc::mem_fun(this, 
			   &MainWindow::on_tile_style_radiobutton_toggled));

	  Glib::RefPtr<Gdk::Pixbuf> pic = tilestyle->getImage();
	  item.button->add(*manage(new Gtk::Image(pic)));
	  item.tile_style_id = tilestyle->getId();

	  tile_style_items.push_back(item);
	}
    }

  terrain_tile_style_hbox->show_all();
}

    
void MainWindow::auto_select_appropriate_pointer()
{
  switch (get_terrain())
    {
    case Tile::GRASS:
      //do 1x1
      pointer_items[1].button->set_active();
      break;
    case Tile::WATER:
    case Tile::FOREST:
    case Tile::SWAMP:
    case Tile::HILLS:
    case Tile::VOID:
	{
	  Tileset *tileset = GameMap::getInstance()->getTileset();
	  Tile *tile = (*tileset)[tileset->getIndex(get_terrain())];
	  if (tile->consistsOnlyOfLoneAndOtherStyles())
	    pointer_items[1].button->set_active();
	  else
	    pointer_items[2].button->set_active();
	  break;
	}
    case Tile::MOUNTAIN:
	//do 6x6
      pointer_items[4].button->set_active();
      break;
    }
}

void MainWindow::on_tile_style_radiobutton_toggled()
{
  on_pointer_radiobutton_toggled();
      
  //was the first one (auto) clicked?  if so, we want 1x1
  if (get_tile_style_id() != -1)
    pointer_items[1].button->set_active();
  else
    auto_select_appropriate_pointer();
}

void MainWindow::on_terrain_radiobutton_toggled()
{
  remove_tile_style_buttons();
  setup_tile_style_buttons(get_terrain());
  on_pointer_radiobutton_toggled();
  auto_select_appropriate_pointer();
}

void MainWindow::on_pointer_radiobutton_toggled()
{
    EditorBigMap::Pointer pointer = EditorBigMap::POINTER;
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
    
    if (bigmap.get())
	bigmap->set_pointer(pointer, size, get_terrain(),
			    get_tile_style_id());
    terrain_type_table->set_sensitive(pointer == EditorBigMap::TERRAIN);
    terrain_tile_style_hbox->set_sensitive(pointer == EditorBigMap::TERRAIN);
}

Tile::Type MainWindow::get_terrain()
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

int MainWindow::get_tile_style_id()
{
  int tile_style_id = -1;
  for (std::vector<TileStyleItem>::iterator i = tile_style_items.begin(),
       end = tile_style_items.end(); i != end; ++i)
    {
	if (i->button->get_active())
	  {
	    tile_style_id = i->tile_style_id;
	    break;
	  }
    }

    return tile_style_id;
}

void MainWindow::on_bigmap_changed(Glib::RefPtr<Gdk::Pixmap> map)
{
  int width = 0;
  int height = 0;
  map->get_size(width, height);
  Glib::RefPtr<Gdk::Window> window = bigmap_drawingarea->get_window();
  if (window)
    {
      Gdk::Rectangle r = Gdk::Rectangle(0, 0, width, height);
      window->invalidate_rect(r, true);
    }
}
void MainWindow::on_smallmap_changed(Glib::RefPtr<Gdk::Pixmap> map)
{
  int width = 0;
  int height = 0;
  map->get_size(width, height);
  map_drawingarea->property_width_request() = width;
  map_drawingarea->property_height_request() = height;
      
  Glib::RefPtr<Gdk::Window> window = map_drawingarea->get_window();
  if (window)
    {
      Gdk::Rectangle r = Gdk::Rectangle(0, 0, width, height);
      window->invalidate_rect(r, true);
    }
  //map_image->property_pixmap() = map;
  //map.clear();
  //still resides at smallmap->get_surface()
}


void MainWindow::init_maps()
{
    // init the bigmap
    bigmap.reset(new EditorBigMap);
    bigmap->mouse_on_tile.connect(
	sigc::mem_fun(this, &MainWindow::on_mouse_on_tile));
    bigmap->objects_selected.connect(
	sigc::mem_fun(this, &MainWindow::on_objects_selected));
    bigmap->map_changed.connect(
	sigc::mem_fun(this, &MainWindow::on_bigmap_changed));
    // grid is on by default
    bigmap->toggle_grid();
    
    // init the smallmap
    smallmap.reset(new SmallMap);
    smallmap->map_changed.connect(
	sigc::mem_fun(this, &MainWindow::on_smallmap_changed));

    // connect the two maps
    bigmap->view_changed.connect(
	sigc::mem_fun(smallmap.get(), &SmallMap::set_view));
    bigmap->map_tiles_changed.connect(
	sigc::mem_fun(smallmap.get(), &SmallMap::redraw_tiles));
    smallmap->view_changed.connect(
	sigc::mem_fun(bigmap.get(), &EditorBigMap::set_view));

    smallmap->resize(GameMap::get_dim() * 2);
}

void MainWindow::on_mouse_on_tile(Vector<int> tile)
{
    Glib::ustring str;
    if (tile.x > 0 && tile.y > 0)
	// note to translators: this is a coordinate pair (x, y)
	str = "<i>" + String::ucompose(_("(%1, %2)"), tile.x, tile.y) + "</i>";
    
    mouse_position_label->set_markup(str);
}

void MainWindow::on_objects_selected(std::vector<UniquelyIdentified *> objects)
{
    assert(!objects.empty());

    if (objects.size() == 1)
    {
	popup_dialog_for_object(objects.front());
    }
    else
    {
	// show a popup
	Gtk::Menu *menu = manage(new Gtk::Menu);
	for (std::vector<UniquelyIdentified *>::iterator i = objects.begin(), end = objects.end();
	     i != end; ++i)
	{
	    Glib::ustring s;
	    if (dynamic_cast<Stack *>(*i))
		s = _("Stack");
	    else if (dynamic_cast<City *>(*i))
		s = _("City");
	    else if (dynamic_cast<Ruin *>(*i))
		s = _("Ruin");
	    else if (dynamic_cast<Signpost *>(*i))
		s = _("Signpost");
	    else if (dynamic_cast<Temple *>(*i))
		s = _("Temple");
	    
	    Gtk::MenuItem *item = manage(new Gtk::MenuItem(s));
	    item->signal_activate().connect(
		sigc::bind(sigc::mem_fun(this, &MainWindow::popup_dialog_for_object), *i));
	    item->show();
	    menu->add(*item);
	}
	menu->popup(button_event->button, button_event->time);
    }
}

void MainWindow::popup_dialog_for_object(UniquelyIdentified *object)
{
    if (Stack *o = dynamic_cast<Stack *>(object))
    {
	StackDialog d(o);
	d.set_parent_window(*window.get());
	d.run();

	// we might have changed something visible
	bigmap->draw();
	smallmap->draw(Playerlist::getActiveplayer());
    }
    else if (City *o = dynamic_cast<City *>(object))
    {
	CityDialog d(o, d_create_scenario_names.get());
	d.set_parent_window(*window.get());
	d.run();

	// we might have changed something visible
	bigmap->draw();
	smallmap->draw(Playerlist::getActiveplayer());
    }
    else if (Ruin *o = dynamic_cast<Ruin *>(object))
    {
	RuinDialog d(o, d_create_scenario_names.get());
	d.set_parent_window(*window.get());
	d.run();
    }
    else if (Signpost *o = dynamic_cast<Signpost *>(object))
    {
	SignpostDialog d(o, d_create_scenario_names.get());
	d.set_parent_window(*window.get());
	d.run();
    }
    else if (Temple *o = dynamic_cast<Temple *>(object))
    {
	TempleDialog d(o, d_create_scenario_names.get());
	d.set_parent_window(*window.get());
	d.run();

	// we might have changed something visible
	bigmap->draw();
    }
}

void MainWindow::on_smooth_map_activated()
{
  GameMap::getInstance()->applyTileStyles(0, 0, GameMap::getHeight(), 
					  GameMap::getWidth(), true);
  bigmap->draw();
}

void MainWindow::on_smooth_screen_activated()
{
  bigmap->smooth_view();
}

void MainWindow::on_edit_items_activated()
{
  ItemlistDialog d;
  d.set_parent_window(*window.get());
  d.run();
}

void MainWindow::on_edit_rewards_activated()
{
  RewardlistDialog d;
  d.set_parent_window(*window.get());
  d.run();
}

void MainWindow::randomize_city(City *c)
{
  std::string name = d_create_scenario_names->popRandomCityName();
  if (name != "")
    c->setName(name);
  c->setRandomArmytypes(true, 1);
}

void MainWindow::on_random_all_cities_activated()
{
  Citylist *cl = Citylist::getInstance();
  for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    randomize_city(*it);
}

void MainWindow::on_random_unnamed_cities_activated()
{
  Citylist *cl = Citylist::getInstance();
  for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    {
      if ((*it)->isUnnamed() == true)
	randomize_city(*it);
    }
}

void MainWindow::randomize_ruin(Ruin *r)
{
  std::string name = d_create_scenario_names->popRandomRuinName();
  if (name != "")
    {
      Location *l = r;
      RenamableLocation *renamable_ruin = static_cast<RenamableLocation*>(l);
      renamable_ruin->setName(name);
    }
}

void MainWindow::on_random_all_ruins_activated()
{
  Ruinlist *rl = Ruinlist::getInstance();
  for (Ruinlist::iterator it = rl->begin(); it != rl->end(); it++)
    randomize_ruin(*it);
}

void MainWindow::on_random_unnamed_ruins_activated()
{
  Ruinlist *rl = Ruinlist::getInstance();
  for (Ruinlist::iterator it = rl->begin(); it != rl->end(); it++)
    {
      if ((*it)->isUnnamed() == true)
	randomize_ruin(*it);
    }
}

void MainWindow::on_random_all_temples_activated()
{
  Templelist *tl = Templelist::getInstance();
  for (Templelist::iterator it = tl->begin(); it != tl->end(); it++)
    {
      std::string name = d_create_scenario_names->popRandomTempleName();
      if (name != "")
	{
	  Location *l = *it;
	  RenamableLocation *renamable_temple = 
	    static_cast<RenamableLocation*>(l);
	  renamable_temple->setName(name);
	}
    }
}

void MainWindow::on_random_unnamed_temples_activated()
{
  Templelist *tl = Templelist::getInstance();
  for (Templelist::iterator it = tl->begin(); it != tl->end(); it++)
    {
      if ((*it)->isUnnamed() == true)
	{
	  std::string name = d_create_scenario_names->popRandomTempleName();
	  if (name != "")
	    {
	      Location *l = *it;
	      RenamableLocation *renamable_temple = 
		static_cast<RenamableLocation*>(l);
	      renamable_temple->setName(name);
	    }
	}
    }
}

void MainWindow::randomize_signpost(Signpost *signpost)
{
  std::string name = "";
  if ((rand() % d_create_scenario_names->getNumSignposts())  == 0)
    name = d_create_scenario_names->popRandomSignpost();
  else
    name = d_create_scenario_names->getDynamicSignpost(signpost);
  if (name == "")
    name = d_create_scenario_names->popRandomSignpost();
  signpost->setName(name);
}

void MainWindow::on_random_all_signs_activated()
{
  Signpostlist *sl = Signpostlist::getInstance();
  for (Signpostlist::iterator it = sl->begin(); it != sl->end(); it++)
    randomize_signpost(*it);
}

void MainWindow::on_random_unnamed_signs_activated()
{
  Signpostlist *sl = Signpostlist::getInstance();
  for (Signpostlist::iterator it = sl->begin(); it != sl->end(); it++)
    {
      if ((*it)->getName() == DEFAULT_SIGNPOST)
	randomize_signpost(*it);
    }
}

void MainWindow::on_help_about_activated()
{
  std::auto_ptr<Gtk::AboutDialog> dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(get_glade_path() + "/../about-dialog.ui");

  Gtk::AboutDialog *d;
  xml->get_widget("dialog", d);
  dialog.reset(d);
  dialog->set_transient_for(*window.get());
  d->set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));

  dialog->set_version(PACKAGE_VERSION);
  dialog->set_logo(GraphicsLoader::getMiscPicture("castle_icon.png"));
  dialog->show_all();
  dialog->run();

  return;
}

void MainWindow::on_validate_activated()
{
  Glib::ustring s;
  std::list<std::string> errors;
  std::list<std::string> warnings;
  game_scenario->validate(errors, warnings);
  if (errors.size())
    s = errors.front();
  else
    {
      if (warnings.size())
	s = warnings.front();
      else
	s = _("No errors.");
    }
  if (errors.size() > 1)
    s += String::ucompose(ngettext("\nThere is %1 more error", "\nThere are %1 more errors", errors.size() - 1), errors.size() - 1);
  if (warnings.size())
    s += String::ucompose(ngettext("\nThere is %1 warning", "\nThere are %1 warnings", warnings.size()), warnings.size());
  TimedMessageDialog dialog(*window.get(), s, 0);
  dialog.show_all();
  dialog.run();
  dialog.hide();
}
