//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2012, 2014 Ben Asselstine
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

#include <iomanip>
#include <assert.h>

#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>

#include <gtkmm.h>
#include "main-window.h"
#include "builder-cache.h"

#include "input-helpers.h"

#include "ucompose.hpp"
#include "tileset.h"
#include "tilesetlist.h"
#include "GameMap.h"
#include "defs.h"
#include "File.h"
#include "ImageCache.h"
#include "smallmap.h"
#include "GameScenario.h"
#include "CreateScenarioRandomize.h"
#include "armysetlist.h"
#include "Itemlist.h"
#include "playerlist.h"
#include "shieldsetlist.h"
#include "citysetlist.h"
#include "ai_dummy.h"

#include "stack.h"
#include "citylist.h"
#include "city.h"
#include "templelist.h"
#include "temple.h"
#include "ruinlist.h"
#include "ruin.h"
#include "signpostlist.h"
#include "signpost.h"
#include "roadlist.h"
#include "road.h"
#include "bridgelist.h"
#include "bridge.h"
#include "portlist.h"
#include "port.h"
#include "MapGenerator.h"
#include "counter.h"

#include "editorbigmap.h"

#include "signpost-editor-dialog.h"
#include "temple-editor-dialog.h"
#include "ruin-editor-dialog.h"
#include "stack-editor-dialog.h"
#include "players-dialog.h"
#include "city-editor-dialog.h"
#include "map-info-dialog.h"
#include "new-map-dialog.h"
#include "switch-sets-dialog.h"
#include "itemlist-dialog.h"
#include "rewardlist-dialog.h"
#include "timed-message-dialog.h"
#include "backpack-editor-dialog.h"
#include "MapBackpack.h"
#include "shieldset-window.h"
#include "cityset-window.h"
#include "armyset-window.h"
#include "tileset-window.h"
#include "editor-quit-dialog.h"
#include "smallmap-editor-dialog.h"
#include "RenamableLocation.h"

    
MainWindow::MainWindow(Glib::ustring load_filename)
{
  d_load_filename = load_filename;
  bigmap = NULL;
  smallmap = NULL;
  game_scenario = NULL;
  d_create_scenario_names = NULL;
  needs_saving = false;
  Glib::RefPtr<Gtk::Builder> xml = 
    BuilderCache::get("editor/main-window.ui");

    xml->get_widget("window", window);
    window->set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));

    window->signal_delete_event().connect(
	sigc::mem_fun(*this, &MainWindow::on_delete_event));

    // the map image
    xml->get_widget("bigmap_image", bigmap_image);
    //bigmap_drawingarea->set_double_buffered(false);
    //bigmap_drawingarea->set_app_paintable(true);
    bigmap_image->signal_draw().connect
      (sigc::mem_fun(*this, &MainWindow::on_bigmap_exposed));
    bigmap_image->signal_size_allocate().connect
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
    xml->get_widget("smallmap_image", smallmap_image);
    smallmap_image->signal_draw().connect
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

    xml->get_widget("terrain_tile_style_viewport", terrain_tile_style_viewport);

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
    setup_pointer_radiobutton(xml, "move", "button_move",
			      EditorBigMap::MOVE, 1);
    setup_pointer_radiobutton(xml, "draw_port", "button_port",
			      EditorBigMap::PORT, 1);
    setup_pointer_radiobutton(xml, "draw_bridge", "button_bridge",
			      EditorBigMap::BRIDGE, 1);
    setup_pointer_radiobutton(xml, "draw_bag", "button_bag",
			      EditorBigMap::BAG, 1);

    xml->get_widget("players_hbox", players_hbox);
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
    xml->get_widget("import_map_from_sav_menuitem", import_map_from_sav_menuitem);
    import_map_from_sav_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_import_map_activated));
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
    xml->get_widget("edit_shieldset_menuitem", edit_shieldset_menuitem);
    edit_shieldset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_edit_shieldset_activated));
    xml->get_widget("edit_armyset_menuitem", edit_armyset_menuitem);
    edit_armyset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_edit_armyset_activated));
    xml->get_widget("edit_cityset_menuitem", edit_cityset_menuitem);
    edit_cityset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_edit_cityset_activated));
    xml->get_widget("edit_tileset_menuitem", edit_tileset_menuitem);
    edit_tileset_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_edit_tileset_activated));
    xml->get_widget("edit_smallmap_menuitem", edit_smallmap_menuitem);
    edit_smallmap_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_edit_smallmap_activated));
    
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
    xml->get_widget("switch_sets_menuitem", switch_sets_menuitem);
    switch_sets_menuitem->signal_activate().connect
      (sigc::mem_fun(this, &MainWindow::on_switch_sets_activated));
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
  terrain_tile_style_grid = new Gtk::Grid();
  terrain_tile_style_viewport->add(*terrain_tile_style_grid);
}

MainWindow::~MainWindow()
{
  delete bigmap;
  delete smallmap;
  delete game_scenario;
  delete d_create_scenario_names;
  delete window;
}

void MainWindow::setup_pointer_radiobutton(Glib::RefPtr<Gtk::Builder> xml,
					   Glib::ustring prefix,
					   Glib::ustring image_file,
					   EditorBigMap::Pointer pointer,
					   int size)
{
    PointerItem item;
    xml->get_widget(prefix + "_radiobutton2", item.button);
    if (prefix == "pointer")
	pointer_radiobutton = item.button;
    item.button->signal_toggled().connect(
	sigc::mem_fun(this, &MainWindow::on_pointer_radiobutton_toggled));
    item.pointer = pointer;
    item.size = size;
    pointer_items.push_back(item);

    Gtk::Image *image = new Gtk::Image(File::getEditorFile(image_file));
    item.button->set_icon_widget(*image);
    item.button->show_all();
    item.button->queue_draw();
}

void MainWindow::setup_terrain_radiobuttons()
{
    // get rid of old ones
  std::vector<Gtk::Widget*> kids = terrain_type_table->get_children();
  for (guint i = 0; i < kids.size(); i++)
    terrain_type_table->remove(*kids[i]);
  
  terrain_items.clear();

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
        item.button->set_tooltip_text(tile->getName());
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
	PixMask *pix = (*(*(*tile).begin())->begin())->getImage()->copy();
	PixMask::scale(pix, 20, 20);
	item.button->add(*manage(new Gtk::Image(pix->to_pixbuf())));
	delete pix;

	item.terrain = tile->getType();
	terrain_items.push_back(item);
    }

    terrain_type_table->show_all();
}

void MainWindow::show()
{
  bigmap_image->show_all();
  window->show();
  on_bigmap_surface_changed(bigmap_image->get_allocation());
}

void MainWindow::on_bigmap_surface_changed(Gtk::Allocation box)
{
  if (!bigmap)
    return;
  static Gtk::Allocation last_box = Gtk::Allocation(0,0,1,1);

  if (box.get_width() != last_box.get_width() || box.get_height() != last_box.get_height())
    {
      bigmap->screen_size_changed(bigmap_image->get_allocation());
      redraw();
    }
  last_box = box;
}

bool MainWindow::on_bigmap_exposed(const Cairo::RefPtr<Cairo::Context>& cr)
{
  return true;
  Cairo::RefPtr<Cairo::Surface> surface = bigmap->get_surface();
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(surface, 0, 0, bigmap_image->get_allocated_width(), bigmap_image->get_allocated_height());
  bigmap_image->property_pixbuf() = pixbuf;
  return true;
}

bool MainWindow::on_smallmap_exposed(const Cairo::RefPtr<Cairo::Context>& cr)
{
  return true;
  Cairo::RefPtr<Cairo::Surface> surface = smallmap->get_surface();
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(surface, 0, 0, 
                        smallmap->get_width(), smallmap->get_height());
  smallmap_image->property_pixbuf() = pixbuf;
  return true;
}

void MainWindow::init()
{
  show_initial_map();
  Playerlist::getInstance()->setActiveplayer(Playerlist::getInstance()->getNeutral());
  fill_players();
}

void MainWindow::hide()
{
    window->hide();
}

bool MainWindow::on_delete_event(GdkEventAny *e)
{
  if (window->property_sensitive() == false)
    return true;
  return !quit();
}

void MainWindow::show_initial_map()
{
  if (d_load_filename.empty() == false)
    {
      clear_map_state();

      bool broken;
      if (game_scenario)
	delete game_scenario;
      current_save_filename = d_load_filename;
      game_scenario = new GameScenario(current_save_filename, broken);
      Playerlist::getInstance()->syncNeutral();
      if (d_create_scenario_names)
	delete d_create_scenario_names;
      d_create_scenario_names = new CreateScenarioRandomize();
      if (broken == false)
	{
	  init_map_state();
	  bigmap->screen_size_changed(bigmap_image->get_allocation()); 
          setup_terrain_radiobuttons();
          remove_tile_style_buttons();
          setup_tile_style_buttons(Tile::GRASS);
	}
      else
	{
	  d_load_filename = "";
	  show_initial_map();
	}
    }
  else
    {
      set_filled_map(112, 156, Tile::WATER, "default", "default", "default",
		     "default");
      setup_terrain_radiobuttons();
      remove_tile_style_buttons();
      setup_tile_style_buttons(Tile::GRASS);
    }
}

void MainWindow::set_filled_map(int width, int height, int fill_style, Glib::ustring tileset, Glib::ustring shieldset, Glib::ustring cityset, Glib::ustring armyset)
{
    clear_map_state();
    d_width = width;
    d_height = height;

    GameMap::deleteInstance();
    GameMap::setWidth(width);
    GameMap::setHeight(height);
    GameMap::getInstance(tileset, shieldset, cityset);
    Itemlist::createStandardInstance();

    if (game_scenario)
      delete game_scenario;
    // sets up the lists
    game_scenario = new GameScenario(_("Untitled"), _("No description"), true);
    if (d_create_scenario_names)
      delete d_create_scenario_names;
    d_create_scenario_names = new CreateScenarioRandomize();
    //zip past the player IDs (+1 for neutral)
    for (unsigned int i = 0; i < MAX_PLAYERS + 1; i++)
      fl_counter->getNextId();


    // ...however we need to do some of the setup by hand. We need to create a
    // neutral player to give cities a player upon creation...
    guint32 armyset_id = Armysetlist::getInstance()->get(armyset)->getId();
    Shieldsetlist *ssl = Shieldsetlist::getInstance();
    Shieldset *ss = ssl->get(shieldset);
    Glib::ustring name = d_create_scenario_names->getPlayerName(Shield::NEUTRAL);
    Player* neutral = new AI_Dummy(name, armyset_id, 
				   ssl->getColor(ss->getId(), MAX_PLAYERS), 
				   width, height, MAX_PLAYERS);
    neutral->setType(Player::AI_DUMMY);
    Playerlist::getInstance()->add(neutral);
    Playerlist::getInstance()->setNeutral(neutral);
    Playerlist::getInstance()->nextPlayer();

    // fill the map with tile type
    Tileset *tset = GameMap::getTileset();
    for (unsigned int i = 0; i < tset->size(); ++i)
    {
	if ((*tset)[i]->getType() == fill_style)
	{
	    GameMap::getInstance()->fill(i);
	    break;
	}
    }

    init_map_state();
    GameMap::getInstance()->calculateBlockedAvenues();
}

void MainWindow::set_random_map(int width, int height,
				int grass, int water, int swamp, int forest,
				int hills, int mountains,
				int cities, int ruins, int temples,
				int signposts, Glib::ustring tileset,
				Glib::ustring shieldset, Glib::ustring cityset,
				Glib::ustring armyset)
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
    guint32 armyset_id = Armysetlist::getInstance()->get(armyset)->getId();
    Citysetlist::getInstance();
    Shieldsetlist *ssl = Shieldsetlist::getInstance();
    Shieldset *ss = ssl->get(shieldset);
    Glib::ustring name = d_create_scenario_names->getPlayerName(Shield::NEUTRAL);
    Player* neutral = new AI_Dummy(name, armyset_id, 
				   ssl->getColor(ss->getId(), MAX_PLAYERS), 
				   width, height, MAX_PLAYERS);
    neutral->setType(Player::AI_DUMMY);
    Playerlist::getInstance()->add(neutral);
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
    
    gen.setCityset(GameMap::getCityset());
    gen.makeMap(width, height, false);
    GameMap::deleteInstance();
    GameMap::setWidth(width);
    GameMap::setHeight(height);
    GameMap::getInstance(tileset, shieldset, cityset);
    GameMap::getInstance()->fill(&gen);

    Itemlist::createStandardInstance();
    // sets up the lists
    if (game_scenario)
      delete game_scenario;
    game_scenario = new GameScenario(_("Untitled"), _("No description"), true);
    if (d_create_scenario_names)
      delete d_create_scenario_names;
    d_create_scenario_names = new CreateScenarioRandomize();
    
    Cityset *cs = Citysetlist::getInstance()->get(cityset);
    // now fill the lists
    const Maptile::Building* build = gen.getBuildings(width, height);
    for (int j = 0; j < height; j++)
	for (int i = 0; i < width; i++)
	    switch(build[j * width + i])
	    {
	    case Maptile::CITY:
		Citylist::getInstance()->add
		  (new City(Vector<int>(i,j), cs->getCityTileWidth()));
		(*Citylist::getInstance()->rbegin())->setOwner(
		    Playerlist::getInstance()->getNeutral());
		break;
	    case Maptile::TEMPLE:
		Templelist::getInstance()->add
		  (new Temple(Vector<int>(i,j), cs->getTempleTileWidth()));
		break;
	    case Maptile::RUIN:
		Ruinlist::getInstance()->add
		  (new Ruin(Vector<int>(i,j), cs->getRuinTileWidth()));
		break;
	    case Maptile::SIGNPOST:
		Signpostlist::getInstance()->add(new Signpost(Vector<int>(i,j)));
		break;
	    case Maptile::ROAD:
		Roadlist::getInstance()->add(new Road(Vector<int>(i,j)));
		break;
	    case Maptile::BRIDGE:
		Bridgelist::getInstance()->add(new Bridge(Vector<int>(i,j)));
		break;
	    case Maptile::PORT:
		Portlist::getInstance()->add(new Port(Vector<int>(i,j)));
		break;
	    case Maptile::NONE:
		break;
	    }

    init_map_state();
}

void MainWindow::clear_map_state()
{
  if (bigmap)
    {
      delete bigmap;
      bigmap = NULL;
    }
  if (smallmap)
    {
      delete smallmap;
      smallmap = NULL;
    }
  if (game_scenario)
    {
      delete game_scenario;
      game_scenario = NULL;
    }
  if (d_create_scenario_names)
    {
      delete d_create_scenario_names;
      d_create_scenario_names = NULL;
    }
    ImageCache::deleteInstance();
}

void MainWindow::init_map_state()
{
    pointer_items[1].button->set_active();
    init_maps();
    on_pointer_radiobutton_toggled();
    on_terrain_radiobutton_toggled();
}

bool MainWindow::on_bigmap_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
	return true;	// useless event

    if (bigmap)
    {
	button_event = e;	// save it for later use
	bigmap->mouse_button_event(to_input_event(e));
	if (smallmap)
	  smallmap->draw(Playerlist::getActiveplayer());
	needs_saving = true;
        update_window_title();
    }
    
    return true;
}

bool MainWindow::on_bigmap_mouse_motion_event(GdkEventMotion *e)
{
  static guint prev = 0;
  if (bigmap)
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
    return true;
}

bool MainWindow::on_bigmap_leave_event(GdkEventCrossing *e)
{
    if (bigmap)
      bigmap->mouse_leave_event();
    return true;
}

bool MainWindow::on_smallmap_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS && e->type != GDK_BUTTON_RELEASE)
	return true;	// useless event
    
    if (smallmap)
	smallmap->mouse_button_event(to_input_event(e));
    
    return true;
}

bool MainWindow::on_smallmap_mouse_motion_event(GdkEventMotion *e)
{
  static guint prev = 0;
  if (smallmap)
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

    NewMapDialog d(*window);
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
        needs_saving = true;
        update_window_title();
    }
}

void MainWindow::on_load_map_activated()
{
    Gtk::FileChooserDialog chooser(*window, _("Choose Map to Load"));
    Glib::RefPtr<Gtk::FileFilter> map_filter = Gtk::FileFilter::create();
    map_filter->set_name(_("LordsAWar Maps (*.map)"));
    map_filter->add_pattern("*.map");
    chooser.add_filter(map_filter);
    chooser.set_current_folder(File::getUserMapDir());

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
	if (game_scenario)
	  delete game_scenario;
	game_scenario = new GameScenario(current_save_filename, broken);
        Playerlist::getInstance()->syncNeutral();
	if (d_create_scenario_names)
	  delete d_create_scenario_names;
	d_create_scenario_names = new CreateScenarioRandomize();

	if (broken)
          {
            TimedMessageDialog dialog
              (*window,String::ucompose(_("Could not load map %1."),
                                        current_save_filename), 0);
            dialog.run_and_hide();
            current_save_filename = "";
            return;
          }

	init_map_state();
	bigmap->screen_size_changed(bigmap_image->get_allocation()); 
        needs_saving = false;
        update_window_title();
        fill_players();
    }
}

void MainWindow::on_save_map_activated()
{
    if (current_save_filename.empty())
	on_save_map_as_activated();
    else
    {
	bool success = game_scenario->saveGame(current_save_filename, MAP_EXT);
	if (!success)
          {
            TimedMessageDialog dialog(*window, _("Map was not saved!"), 0);
            dialog.run_and_hide();
            on_validate_activated();
          }
        else
          {
            needs_saving = false;
            update_window_title();
          }
    }
}

void MainWindow::on_export_as_bitmap_activated()
{
    Gtk::FileChooserDialog chooser(*window, _("Choose a Name"),
				   Gtk::FILE_CHOOSER_ACTION_SAVE);
    Glib::RefPtr<Gtk::FileFilter> png_filter = Gtk::FileFilter::create();
    png_filter->set_name(_("PNG files (*.png)"));
    png_filter->add_pattern("*.png");
    chooser.add_filter(png_filter);
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
          {
            TimedMessageDialog dialog(*window, _("Map was not exported!"), 0);
            dialog.run_and_hide();
          }
    }
}

void MainWindow::on_export_as_bitmap_no_game_objects_activated()
{
    Gtk::FileChooserDialog chooser(*window, _("Choose a Name"),
				   Gtk::FILE_CHOOSER_ACTION_SAVE);
    Glib::RefPtr<Gtk::FileFilter> png_filter = Gtk::FileFilter::create();
    png_filter->set_name(_("PNG files (*.png)"));
    png_filter->add_pattern("*.png");
    chooser.add_filter(png_filter);
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
          {
            TimedMessageDialog dialog(*window, _("Map was not exported!"), 0);
            dialog.run_and_hide();
          }
    }
}

void MainWindow::on_save_map_as_activated()
{
    Gtk::FileChooserDialog chooser(*window, _("Choose a Name"),
				   Gtk::FILE_CHOOSER_ACTION_SAVE);
    Glib::RefPtr<Gtk::FileFilter> map_filter = Gtk::FileFilter::create();
    map_filter->set_name(_("LordsAWar Maps (*.map)"));
    map_filter->add_pattern("*" + MAP_EXT);
    chooser.add_filter(map_filter);
    chooser.set_current_folder(File::getUserMapDir());

    chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    chooser.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_ACCEPT);
    chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
    
    chooser.show_all();
    int res = chooser.run();
    
    if (res == Gtk::RESPONSE_ACCEPT)
    {
        Glib::ustring old_save_filename = current_save_filename;
	current_save_filename = chooser.get_filename();
	chooser.hide();

	bool success = game_scenario->saveGame(current_save_filename, MAP_EXT);
	if (!success)
          {
            TimedMessageDialog dialog(*window, _("Map was not saved!"), 0);
            dialog.run_and_hide();
            on_validate_activated();
            current_save_filename = old_save_filename;
          }
        else
          {
            needs_saving = false;
            update_window_title();
          }
    }
}

bool MainWindow::quit()
{
  if (needs_saving)
    {
      EditorQuitDialog d(*window);
      int response = d.run_and_hide();
      
      if (response == Gtk::RESPONSE_CANCEL) //we don't want to quit
	return false;

      else if (response == Gtk::RESPONSE_ACCEPT) // save and quit
	on_save_map_activated();
      //else if (Response == Gtk::CLOSE) // don't save just quit
      window->hide();
    }
  else
    window->hide();
  return true;
}

void MainWindow::on_quit_activated()
{
  quit();
}

void MainWindow::on_edit_players_activated()
{
    PlayersDialog d(*window, d_create_scenario_names, d_width, d_height);
    Player *active = Playerlist::getActiveplayer();
    int response = d.run();
    if (response == Gtk::RESPONSE_ACCEPT)
      {
	if (Playerlist::getInstance()->getPlayer(active->getId()))
	  Playerlist::getInstance()->setActiveplayer(active);
	needs_saving = true;
        update_window_title();
	fill_players();
      }
}

void MainWindow::on_edit_map_info_activated()
{
    MapInfoDialog d(*window, game_scenario);
    int response = d.run();
    if (response == Gtk::RESPONSE_ACCEPT)
      {
        needs_saving = true;
        update_window_title();
      }
}

void MainWindow::on_edit_shieldset_activated()
{
  Gtk::Main *kit = Gtk::Main::instance();;
  ShieldSetWindow *shieldset_window = new ShieldSetWindow
    (window, GameMap::getShieldset()->getConfigurationFile());
  shieldset_window->get_window().property_transient_for() = window;
  shieldset_window->shieldset_saved.connect
    (sigc::mem_fun(this, &MainWindow::on_shieldset_saved));
  kit->run(shieldset_window->get_window());
  delete shieldset_window;
}

void MainWindow::on_shieldset_saved(guint32 id)
{
  //did we save the active shieldset?
  Shieldset *shieldset = GameMap::getShieldset();
  if (id == shieldset->getId())
    {
      ImageCache::getInstance()->reset();
      GameMap::getInstance()->reloadShieldset();
      fill_players();
      bigmap->screen_size_changed(bigmap_image->get_allocation()); 
      redraw();
      needs_saving = true;
      update_window_title();
    }
}

void MainWindow::on_edit_armyset_activated()
{
  Gtk::Main *kit = Gtk::Main::instance();;
  guint32 army_set_id = Playerlist::getActiveplayer()->getArmyset();
  Armyset *armyset = Armysetlist::getInstance()->get(army_set_id);
  Glib::ustring file = armyset->getConfigurationFile();
 
  ArmySetWindow* armyset_window = new ArmySetWindow (window, file);
  armyset_window->get_window().property_transient_for() = window;
  armyset_window->armyset_saved.connect
    (sigc::mem_fun(this, &MainWindow::on_armyset_saved));
  kit->run(armyset_window->get_window());
  delete armyset_window;
}

void MainWindow::on_armyset_saved(guint32 id)
{
  //did we save any of the active armysets?
  if (Playerlist::getInstance()->hasArmyset(id) == true)
    {
      ImageCache::getInstance()->reset();
      //we're doing reload before, because we need the maps to be updated.
      //but then the armyset* gets changed and the switch has no effect.
      Armysetlist::getInstance()->reload(id);
      GameMap::getInstance()->switchArmysets(Armysetlist::getInstance()->get(id));
      bigmap->screen_size_changed(bigmap_image->get_allocation()); 
      redraw();
      needs_saving = true;
      update_window_title();
    }
}

void MainWindow::on_edit_cityset_activated()
{
  Gtk::Main *kit = Gtk::Main::instance();;
  Cityset *cityset = GameMap::getCityset();
  Glib::ustring file = cityset->getConfigurationFile();
  CitySetWindow* cityset_window = new CitySetWindow (window, file);
  cityset_window->get_window().property_transient_for() = window;
  cityset_window->cityset_saved.connect
    (sigc::mem_fun(this, &MainWindow::on_cityset_saved));
  kit->run(cityset_window->get_window());
  delete cityset_window;
}

void MainWindow::on_cityset_saved(guint32 id)
{
  //did we save the active cityset?
  if (id == GameMap::getInstance()->getCitysetId())
    {
      ImageCache::getInstance()->reset();
      GameMap::getInstance()->reloadCityset();
      bigmap->screen_size_changed(bigmap_image->get_allocation()); 
      redraw();
      needs_saving = true;
      update_window_title();
    }
}

void MainWindow::on_edit_smallmap_activated()
{
  SmallmapEditorDialog d(*window);
  bool changed = d.run();
  d.hide();
  Rectangle r = Rectangle(0, 0, GameMap::getWidth(), GameMap::getHeight());
  smallmap->redraw_tiles(r);
  smallmap->resize();
  redraw();
  if (changed)
    needs_saving = true;
  update_window_title();
}

void MainWindow::on_edit_tileset_activated()
{
  Gtk::Main *kit = Gtk::Main::instance();;
  Tileset *tileset = GameMap::getTileset();
  Glib::ustring file = tileset->getConfigurationFile();
  TileSetWindow* tileset_window = new TileSetWindow (window, file);
  tileset_window->get_window().property_transient_for() = window;
  tileset_window->tileset_saved.connect
    (sigc::mem_fun(this, &MainWindow::on_tileset_saved));
  kit->run(tileset_window->get_window());
  delete tileset_window;
}

void MainWindow::on_tileset_saved(guint32 id)
{
  //did we save the active tileset?
  if (id == GameMap::getInstance()->getTilesetId())
    {
      ImageCache::getInstance()->reset();
      Tilesetlist::getInstance()->reload(id);
      GameMap::getInstance()->switchTileset(Tilesetlist::getInstance()->get(id));
      smallmap->resize();
      bigmap->screen_size_changed(bigmap_image->get_allocation()); 
      setup_terrain_radiobuttons();
      on_terrain_radiobutton_toggled();
      redraw();
      needs_saving = true;
      update_window_title();
    }
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
  bigmap->draw(Playerlist::getViewingplayer());
}

void MainWindow::on_grid_toggled()
{
  bigmap->toggle_grid();
}

void MainWindow::remove_tile_style_buttons()
{
  Glib::ListHandle<Gtk::Widget*> children = 
    terrain_tile_style_grid->get_children();
  if (!children.empty()) 
    {
      Glib::ListHandle<Gtk::Widget*>::iterator child = children.begin();
      for (; child != children.end(); child++)
	terrain_tile_style_grid->remove(**child);
    }
  tile_style_items.clear();
}

void MainWindow::setup_tile_style_buttons(Tile::Type terrain)
{
  Gtk::RadioButtonGroup group;
  //iterate through tilestyles of a certain TERRAIN tile
  Tileset *tileset = GameMap::getTileset();
  guint32 index = tileset->getIndex(terrain);
  Tile *tile = (*tileset)[index];
  if (tile == NULL)
    return;
	  
  TileStyleItem auto_item;
  auto_item.button = manage(new Gtk::RadioButton(group));


  auto_item.button->set_label(_("Auto"));
  auto_item.button->property_draw_indicator() = false;

  auto_item.button->signal_toggled().connect
    (sigc::mem_fun(this, &MainWindow::on_tile_style_radiobutton_toggled));
  terrain_tile_style_grid->attach(*manage(auto_item.button), 0, 0, 1, 1);

  auto_item.tile_style_id = -1;
  tile_style_items.push_back(auto_item);

  int r = 0, c = 1, max_rows = 4;
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

          terrain_tile_style_grid->attach(*manage(item.button), c, r, 1, 1);
          item.button->signal_toggled().connect
            (sigc::mem_fun(this, 
                           &MainWindow::on_tile_style_radiobutton_toggled));

          PixMask *pix = tilestyle->getImage()->copy();
          PixMask::scale(pix, 40, 40);
          item.button->add(*manage(new Gtk::Image(pix->to_pixbuf())));
          item.tile_style_id = tilestyle->getId();

          tile_style_items.push_back(item);
          c++;
          if (c >= max_rows)
            {
              c = 0;
              r++;
            }
        }
    }
  terrain_tile_style_viewport->show_all();

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
	{
          Tileset *tileset = GameMap::getTileset();
	  Tile *tile = (*tileset)[tileset->getIndex(get_terrain())];
	  if (tile->consistsOnlyOfLoneAndOtherStyles())
	    pointer_items[1].button->set_active();
	  else
            {
              //if 1x1 and tilestyle is Auto, then things won't look right.
              //e.g. a 1x1 forest will simply get smoothed away to grass.
              if (pointer_items[1].button->get_active() &&
                  tile_style_items[0].button->get_active())
                pointer_items[2].button->set_active();
            }
	  break;
	}
    case Tile::MOUNTAIN:
	{
          Tileset *tileset = GameMap::getTileset();
	  Tile *tile = (*tileset)[tileset->getIndex(get_terrain())];
	  if (tile->consistsOnlyOfLoneAndOtherStyles())
	    pointer_items[1].button->set_active();
	  else
            {
              //same as the rest, but 3x3 instead of 2x2.
              if (pointer_items[1].button->get_active() &&
                  tile_style_items[0].button->get_active())
                pointer_items[3].button->set_active();
            }
        }
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
    
    int i = get_pointer_index();
    pointer = pointer_items[i].pointer;
    size = pointer_items[i].size;
    
    if (bigmap)
	bigmap->set_pointer(pointer, size, get_terrain(),
			    get_tile_style_id());
    players_hbox->set_sensitive (pointer == EditorBigMap::STACK || 
				 pointer == EditorBigMap::CITY);
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

void MainWindow::on_smallmap_water_changed()
{
  //this is so that the radial water can be redrawn again.
  //otherwise the land doesn't get erased on the smallmap
  smallmap->resize();
}

void MainWindow::on_bigmap_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, bigmap_image->get_allocated_width(), bigmap_image->get_allocated_height());
  bigmap_image->property_pixbuf() = pixbuf;
}

void MainWindow::on_smallmap_changed(Cairo::RefPtr<Cairo::Surface> map, Gdk::Rectangle r)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, 
                        smallmap->get_width(), smallmap->get_height());
  smallmap_image->property_pixbuf() = pixbuf;
}

void MainWindow::init_maps()
{
    // init the smallmap
    if (smallmap)
      delete smallmap;
    smallmap =new SmallMap;
    smallmap->resize();
    smallmap->map_changed.connect(
	sigc::mem_fun(this, &MainWindow::on_smallmap_changed));

    // init the bigmap
    if (bigmap)
      delete bigmap;
    bigmap = new EditorBigMap;
    bigmap->mouse_on_tile.connect(
	sigc::mem_fun(this, &MainWindow::on_mouse_on_tile));
    bigmap->objects_selected.connect(
	sigc::mem_fun(this, &MainWindow::on_objects_selected));
    bigmap->map_changed.connect(
	sigc::mem_fun(this, &MainWindow::on_bigmap_changed));
    bigmap->map_water_changed.connect
      (sigc::mem_fun(this, &MainWindow::on_smallmap_water_changed));
    bigmap->bag_selected.connect
      (sigc::mem_fun(this, &MainWindow::on_bag_selected));
                                       

    // grid is on by default
    bigmap->toggle_grid();
    
    // connect the two maps
    bigmap->view_changed.connect(
	sigc::mem_fun(smallmap, &SmallMap::set_view));
    bigmap->map_tiles_changed.connect(
	sigc::mem_fun(smallmap, &SmallMap::redraw_tiles));
    smallmap->view_changed.connect(
	sigc::mem_fun(bigmap, &EditorBigMap::set_view));

    //trigger the bigmap to resize the view box in the smallmap
    bigmap->screen_size_changed(bigmap_image->get_allocation()); 
}

void MainWindow::on_mouse_on_tile(Vector<int> tile)
{
    Glib::ustring str;
    if (tile.x >= 0 && tile.y >= 0)
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
	    else if (dynamic_cast<MapBackpack*>(*i))
		s = _("Bag");
	    
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
	StackEditorDialog d(*window, o);
	int response = d.run();
	if (response == Gtk::RESPONSE_ACCEPT)
          {
            needs_saving = true;
            update_window_title();
          }

	// we might have changed something visible
	redraw();
    }
    else if (City *o = dynamic_cast<City *>(object))
    {
	CityEditorDialog d(*window, o, d_create_scenario_names);
	int response = d.run();
	if (response == Gtk::RESPONSE_ACCEPT)
          {
            needs_saving = true;
            update_window_title();
          }

	// we might have changed something visible
	redraw();
    }
    else if (Ruin *o = dynamic_cast<Ruin *>(object))
    {
	RuinEditorDialog d(*window, o, d_create_scenario_names);
	int response = d.run();
	if (response == Gtk::RESPONSE_ACCEPT)
          {
            needs_saving = true;
            update_window_title();
          }
	redraw();
    }
    else if (Signpost *o = dynamic_cast<Signpost *>(object))
    {
	SignpostEditorDialog d(*window, o, d_create_scenario_names);
	int response = d.run();
	if (response == Gtk::RESPONSE_ACCEPT)
          {
            needs_saving = true;
            update_window_title();
          }
    }
    else if (Temple *o = dynamic_cast<Temple *>(object))
    {
	TempleEditorDialog d(*window, o, d_create_scenario_names);
	int response = d.run();
	if (response == Gtk::RESPONSE_ACCEPT)
          {
            needs_saving = true;
            update_window_title();
          }

	// we might have changed something visible
	redraw();
    }
    else if (MapBackpack *b = dynamic_cast<MapBackpack*>(object))
      {
	BackpackEditorDialog d(*window, b);
	int response = d.run();
	if (response == Gtk::RESPONSE_ACCEPT)
          {
            needs_saving = true;
            update_window_title();
          }
      }
}

void MainWindow::on_smooth_map_activated()
{
  GameMap::getInstance()->applyTileStyles(0, 0, GameMap::getHeight(), 
					  GameMap::getWidth(), true);
  redraw();
}

void MainWindow::on_smooth_screen_activated()
{
  bigmap->smooth_view();
}

void MainWindow::on_edit_items_activated()
{
  ItemlistDialog d(*window);
  int response = d.run_and_hide();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

void MainWindow::on_edit_rewards_activated()
{
  RewardlistDialog d(*window);
  int response = d.run_and_hide();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
    }
}

void MainWindow::randomize_city(City *c)
{
  Glib::ustring name = d_create_scenario_names->popRandomCityName();
  if (name != "")
    c->setName(name);
  c->setRandomArmytypes(true, 1);
}

void MainWindow::on_random_all_cities_activated()
{
  Citylist *cl = Citylist::getInstance();
  for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    randomize_city(*it);
  needs_saving = true;
  update_window_title();
}

void MainWindow::on_random_unnamed_cities_activated()
{
  Citylist *cl = Citylist::getInstance();
  for (Citylist::iterator it = cl->begin(); it != cl->end(); it++)
    {
      if ((*it)->isUnnamed() == true)
	randomize_city(*it);
    }
  needs_saving = true;
  update_window_title();
}

void MainWindow::randomize_ruin(Ruin *r)
{
  Glib::ustring name = d_create_scenario_names->popRandomRuinName();
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
  needs_saving = true;
  update_window_title();
}

void MainWindow::on_random_unnamed_ruins_activated()
{
  Ruinlist *rl = Ruinlist::getInstance();
  for (Ruinlist::iterator it = rl->begin(); it != rl->end(); it++)
    {
      if ((*it)->isUnnamed() == true)
	randomize_ruin(*it);
    }
  needs_saving = true;
  update_window_title();
}

void MainWindow::on_random_all_temples_activated()
{
  Templelist *tl = Templelist::getInstance();
  for (Templelist::iterator it = tl->begin(); it != tl->end(); it++)
    {
      Glib::ustring name = d_create_scenario_names->popRandomTempleName();
      if (name != "")
	{
	  Location *l = *it;
	  RenamableLocation *renamable_temple = 
	    static_cast<RenamableLocation*>(l);
	  renamable_temple->setName(name);
	}
    }
  needs_saving = true;
  update_window_title();
}

void MainWindow::on_random_unnamed_temples_activated()
{
  Templelist *tl = Templelist::getInstance();
  for (Templelist::iterator it = tl->begin(); it != tl->end(); it++)
    {
      if ((*it)->isUnnamed() == true)
	{
	  Glib::ustring name = d_create_scenario_names->popRandomTempleName();
	  if (name != "")
	    {
	      Location *l = *it;
	      RenamableLocation *renamable_temple = 
		static_cast<RenamableLocation*>(l);
	      renamable_temple->setName(name);
	    }
	}
    }
  needs_saving = true;
  update_window_title();
}

void MainWindow::randomize_signpost(Signpost *signpost)
{
  Glib::ustring name = "";
  if (d_create_scenario_names->getNumSignposts() > 0 &&
      (rand() % d_create_scenario_names->getNumSignposts()) == 0)
    name = d_create_scenario_names->popRandomSignpost();
  else

    name = d_create_scenario_names->getDynamicSignpost(signpost);
  if (name != "")
    signpost->setName(name);
}

void MainWindow::on_random_all_signs_activated()
{
  Signpostlist *sl = Signpostlist::getInstance();
  for (Signpostlist::iterator it = sl->begin(); it != sl->end(); it++)
    randomize_signpost(*it);
  needs_saving = true;
  update_window_title();
}

void MainWindow::on_random_unnamed_signs_activated()
{
  Signpostlist *sl = Signpostlist::getInstance();
  for (Signpostlist::iterator it = sl->begin(); it != sl->end(); it++)
    {
      if ((*it)->getName() == DEFAULT_SIGNPOST)
	randomize_signpost(*it);
    }
  needs_saving = true;
  update_window_title();
}

void MainWindow::on_help_about_activated()
{
  Gtk::AboutDialog* dialog;

  Glib::RefPtr<Gtk::Builder> xml
    = Gtk::Builder::create_from_file(File::getMiscFile("glade/about-dialog.ui"));

  xml->get_widget("dialog", dialog);
  dialog->set_transient_for(*window);
  dialog->set_icon_from_file(File::getMiscFile("various/tileset_icon.png"));

  dialog->set_version(PACKAGE_VERSION);
  dialog->set_logo(ImageCache::loadMiscImage("castle_icon.png")->to_pixbuf());
  dialog->show_all();
  dialog->run();
  delete dialog;

  return;
}

void MainWindow::on_validate_activated()
{
  Glib::ustring s;
  std::list<Glib::ustring> errors;
  std::list<Glib::ustring> warnings;
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
  TimedMessageDialog dialog(*window, s, 0);
  dialog.run_and_hide();
}
      
void MainWindow::clear_save_file_of_scenario_specific_data()
{
  Playerlist *plist = Playerlist::getInstance();
  for (Playerlist::iterator i = plist->begin(); i != plist->end(); i++)
    {
      (*i)->clearActionlist();
      (*i)->clearHistorylist();
      (*i)->clearStacklist();
      (*i)->clearFogMap();
      (*i)->setGold(1000);
      (*i)->revive();
    }
}

void MainWindow::on_import_map_activated()
{
    Gtk::FileChooserDialog chooser(*window, _("Choose Game to Load Map from"));
    Glib::RefPtr<Gtk::FileFilter> sav_filter = Gtk::FileFilter::create();
    sav_filter->set_name(_("LordsAWar Saved Games (*.sav)"));
    sav_filter->add_pattern("*" + SAVE_EXT);
    chooser.add_filter(sav_filter);
    chooser.set_current_folder(File::getSavePath());

    chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    chooser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
    chooser.set_default_response(Gtk::RESPONSE_ACCEPT);
	
    chooser.show_all();
    int res = chooser.run();
    
    if (res == Gtk::RESPONSE_ACCEPT)
    {
        Glib::ustring filename = chooser.get_filename();
	chooser.hide();

	clear_map_state();

	bool broken;
	if (game_scenario)
	  delete game_scenario;
	game_scenario = new GameScenario(filename, broken);

	if (broken)
	{
          TimedMessageDialog dialog
            (*window, String::ucompose(_("Could not load game %1."),
                                       filename), 0);
          dialog.run_and_hide();
	    current_save_filename = "";
	    return;
	}

	if (d_create_scenario_names)
	  delete d_create_scenario_names;
	d_create_scenario_names = new CreateScenarioRandomize();

	//now lets get rid of stuff.
	clear_save_file_of_scenario_specific_data();

	init_map_state();
	bigmap->screen_size_changed(bigmap_image->get_allocation()); 
        fill_players();
    }
}
      
void MainWindow::redraw()
{
  bigmap->draw(Playerlist::getViewingplayer());
  smallmap->draw(Playerlist::getActiveplayer());
}

void MainWindow::on_switch_sets_activated()
{
  SwitchSetsDialog d(*window);
  int response = d.run();
  if (response == Gtk::RESPONSE_ACCEPT)
    {
      needs_saving = true;
      update_window_title();
      ImageCache::getInstance()->reset();
      bigmap->screen_size_changed(bigmap_image->get_allocation()); 
      smallmap->resize();
      if (d.get_tileset_changed())
        {
          setup_terrain_radiobuttons();
          on_terrain_radiobutton_toggled();
        }
      redraw();
      fill_players();
    }
}

void MainWindow::on_player_toggled(PlayerItem item)
{
  Player *p = Playerlist::getInstance()->getPlayer(item.player_id);
  if (p)
    Playerlist::getInstance()->setActiveplayer(p);
  fill_players();
}

void MainWindow::fill_players()
{
  for (std::list<PlayerItem>::iterator it = player_buttons.begin();
       it != player_buttons.end(); it++)
    players_hbox->remove(dynamic_cast<Gtk::Widget&>(*(*it).button));
  player_buttons.clear();
  Playerlist *pl = Playerlist::getInstance();
  bool sensitive = players_hbox->get_sensitive();
  if (!sensitive)
    players_hbox->set_sensitive(true);
  for (Playerlist::iterator it = pl->begin(); it != pl->end(); it++)
    {
      Gtk::ToggleButton *toggle = new Gtk::ToggleButton();
      toggle->foreach(sigc::mem_fun(toggle, &Gtk::Container::remove));

      Gtk::Image *image = new Gtk::Image();
      image->property_pixbuf() = 
	ImageCache::getInstance()->getShieldPic(1, *it)->to_pixbuf();
      toggle->add(*manage(image));
      toggle->show_all();
      if (*it == pl->getActiveplayer())
	toggle->set_active(true);

      struct PlayerItem item;
      item.button = toggle;
      item.player_id = (*it)->getId();
      player_buttons.push_back(item);
      toggle->set_tooltip_text((*it)->getName());
      players_hbox->pack_start(*manage(toggle), Gtk::PACK_SHRINK);
      toggle->signal_toggled().connect
	(sigc::bind(sigc::mem_fun(this, &MainWindow::on_player_toggled), item));
    }
  players_hbox->show_all();
  if (!sensitive)
    players_hbox->set_sensitive(false);
}

void MainWindow::update_window_title()
{
  Glib::ustring title = "";
  if (current_save_filename != "")
    {
      if (needs_saving == true)
        title +="*";
      title += File::get_basename(current_save_filename, true);
      title += " - ";
    }
  title += _("LordsAWar! Scenario Editor");
  window->set_title(title);
}
    
int MainWindow::get_pointer_index()
{
  int c = 0;
  for (std::vector<PointerItem>::iterator i = pointer_items.begin(),
       end = pointer_items.end(); i != end; ++i, c++)
    {
      if (i->button->get_active())
        return c;
    }
  return 0;
}

void MainWindow::on_bag_selected(Vector<int> tile)
{
  MapBackpack *bag = 
    GameMap::getInstance()->getTile(tile)->getBackpack();
  BackpackEditorDialog d(*window, dynamic_cast<Backpack*>(bag));
  d.run();
}
