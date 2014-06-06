//  Copyright (C) 2010, 2012, 2014 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>
#include <sstream>

#include "tilestyle-organizer-dialog.h"

#include "ucompose.hpp"
#include "File.h"
#include "defs.h"
#include "Tile.h"
#include "ImageCache.h"
#include "timing.h"

TileStyleOrganizerDialog::TileStyleOrganizerDialog(Gtk::Window &parent, Tile *tile)
 : LwEditorDialog(parent, "tilestyle-organizer-dialog.ui")
{
    d_tile = tile;
    std::vector<Gtk::TargetEntry> targets;
    targets.push_back(Gtk::TargetEntry("LordsawarTilestyleType", Gtk::TARGET_SAME_APP));
    
    xml->get_widget("categories_iconview", categories_iconview);
    xml->get_widget("category_iconview", category_iconview);
    xml->get_widget("unsorted_iconview", unsorted_iconview);
    xml->get_widget("category_label", category_label);
    xml->get_widget("unsorted_label", unsorted_label);

    categories_list = Gtk::ListStore::create (categories_columns);
    categories_iconview->set_model(categories_list); 
    categories_iconview->set_pixbuf_column(categories_columns.image);
    categories_iconview->signal_selection_changed().connect
      (sigc::mem_fun(*this, &TileStyleOrganizerDialog::on_category_selected));
    categories_iconview->enable_model_drag_dest(targets, Gdk::ACTION_MOVE);
    categories_iconview->signal_drag_data_received().connect
      (sigc::mem_fun
       (*this, &TileStyleOrganizerDialog::on_categories_drop_drag_data_received));
    fill_in_categories();
    category_list = Gtk::ListStore::create (tilestyle_columns);
    category_iconview->set_model(category_list); 
    category_iconview->set_pixbuf_column(tilestyle_columns.image);
    category_iconview->set_text_column(tilestyle_columns.name);
    category_iconview->enable_model_drag_dest(targets, Gdk::ACTION_MOVE);
    category_iconview->enable_model_drag_source(targets, Gdk::BUTTON1_MASK, Gdk::ACTION_MOVE);
    category_iconview->signal_drag_data_get().connect
      (sigc::mem_fun(*this, &TileStyleOrganizerDialog::on_category_drag_data_get));
    category_iconview->signal_drag_data_received().connect
      (sigc::mem_fun
       (*this, &TileStyleOrganizerDialog::on_category_drop_drag_data_received));
    category_iconview->signal_item_activated().connect
      (sigc::mem_fun(*this, &TileStyleOrganizerDialog::on_category_tilestyle_activated));

    category_iconview->signal_selection_changed().connect
      (sigc::bind(sigc::mem_fun(*this, &TileStyleOrganizerDialog::on_selection_made), category_iconview));
    category_iconview->signal_drag_begin().connect
      (sigc::bind(sigc::mem_fun(*this, &TileStyleOrganizerDialog::on_drag_begin), category_iconview));
    unsorted_list = Gtk::ListStore::create (tilestyle_columns);
    unsorted_iconview->set_model(unsorted_list); 
    unsorted_iconview->set_pixbuf_column(tilestyle_columns.image);
    unsorted_iconview->set_text_column(tilestyle_columns.name);
    unsorted_iconview->enable_model_drag_dest(targets, Gdk::ACTION_MOVE);
    unsorted_iconview->drag_dest_set(targets, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_MOVE);
    unsorted_iconview->enable_model_drag_source(targets, Gdk::BUTTON1_MASK, Gdk::ACTION_MOVE);
    unsorted_iconview->signal_drag_data_get().connect
      (sigc::mem_fun(*this, &TileStyleOrganizerDialog::on_unsorted_drag_data_get));
    unsorted_iconview->signal_drag_data_received().connect
      (sigc::mem_fun
       (*this, &TileStyleOrganizerDialog::on_unsorted_drop_drag_data_received));
    unsorted_iconview->signal_item_activated().connect
      (sigc::mem_fun(*this, &TileStyleOrganizerDialog::on_unsorted_tilestyle_activated));
    unsorted_iconview->signal_drag_begin().connect
      (sigc::bind(sigc::mem_fun(*this, &TileStyleOrganizerDialog::on_drag_begin), unsorted_iconview));
    unsorted_iconview->signal_selection_changed().connect
      (sigc::bind(sigc::mem_fun(*this, &TileStyleOrganizerDialog::on_selection_made), unsorted_iconview));

    if (d_tile->front())
      if (d_tile->front()->front())
        if (d_tile->front()->front()->getImage())
          {
          unsorted_iconview->property_item_width() =
            d_tile->front()->front()->getImage()->get_width();
          category_iconview->property_item_width() =
            d_tile->front()->front()->getImage()->get_width();
          }

    fill_category(TileStyle::UNKNOWN);
    categories_iconview->select_path(Gtk::TreeModel::Path("0"));
    inhibit_select = false;
}
      
void TileStyleOrganizerDialog::on_category_drag_data_get(const Glib::RefPtr<Gdk::DragContext> &drag_context, Gtk::SelectionData &data, guint info, guint time)
{
  drag_context->get_source_window()->show();
  Glib::ustring s;
  std::list<TileStyle*> st = get_selected_category_tilestyles();
  if (st.empty() == true)
    return;
  for (std::list<TileStyle*>::iterator i = st.begin(); i != st.end(); i++)
    s += String::ucompose("0x%1 ", TileStyle::idToString((*i)->getId()));
  data.set(data.get_target(), 8, (const guchar*)s.c_str(), strlen(s.c_str()));
}

void TileStyleOrganizerDialog::on_unsorted_drag_data_get(const Glib::RefPtr<Gdk::DragContext> &drag_context, Gtk::SelectionData &data, guint info, guint time)
{
  drag_context->get_source_window()->show();
  Glib::ustring s;
  std::list<TileStyle*> st = get_selected_unsorted_tilestyles();
  if (st.empty() == true)
    return;
  for (std::list<TileStyle*>::iterator i = st.begin(); i != st.end(); i++)
    s += String::ucompose("0x%1 ", TileStyle::idToString((*i)->getId()));
  data.set(data.get_target(), 8, (const guchar*)s.c_str(), strlen(s.c_str()));
}

int TileStyleOrganizerDialog::get_selected_category()
{
  typedef std::vector<Gtk::TreeModel::Path> type_list_paths;
  type_list_paths selected = categories_iconview->get_selected_items();
  if (!selected.empty())
    {
      const Gtk::TreeModel::Path &path = *selected.begin();
      Gtk::TreeModel::iterator iter = categories_list->get_iter(path);
      Gtk::TreeModel::Row row = *iter;
      return row[categories_columns.type];
    }
  return -1;
}

std::list<TileStyle*> TileStyleOrganizerDialog::get_selected_category_tilestyles ()
{
  std::list<TileStyle*> styles;
  typedef std::vector<Gtk::TreeModel::Path> paths;
  paths selected = category_iconview->get_selected_items();
  if (!selected.empty())
    {
      for (paths::iterator i = selected.begin(); i != selected.end(); i++)
        {
          Gtk::TreeModel::iterator iter = category_list->get_iter(*i);
          Gtk::TreeModel::Row row = *iter;
          styles.push_back(row[tilestyle_columns.style]);
        }
    }
  return styles;
}

std::list<TileStyle*> TileStyleOrganizerDialog::get_selected_unsorted_tilestyles ()
{
  std::list<TileStyle*> styles;
  typedef std::vector<Gtk::TreeModel::Path> paths;
  paths selected = unsorted_iconview->get_selected_items();
  if (!selected.empty())
    {
      for (paths::iterator i = selected.begin(); i != selected.end(); i++)
        {
          Gtk::TreeModel::iterator iter = unsorted_list->get_iter(*i);
          Gtk::TreeModel::Row row = *iter;
          styles.push_back(row[tilestyle_columns.style]);
        }
    }
  return styles;
}

void TileStyleOrganizerDialog::fill_in_categories()
{
    categories_list->clear();
    for (guint32 i = TileStyle::LONE; i < TileStyle::UNKNOWN; i++)
      add_category (i);
}

void TileStyleOrganizerDialog::add_category(guint32 type)
{
  ImageCache *gc = ImageCache::getInstance();
  Gtk::TreeModel::Row row = *(categories_list->append());
  row[categories_columns.image] = 
    gc->getDefaultTileStylePic(type, 80)->to_pixbuf();
  row[categories_columns.name] = TileStyle::getTypeName(TileStyle::Type(type));
  row[categories_columns.type] = type;
}

void TileStyleOrganizerDialog::empty_category()
{
  category_label->set_text("");
}

void TileStyleOrganizerDialog::add_tilestyle(Glib::RefPtr<Gtk::ListStore> list, TileStyle *tilestyle)
{
  Gtk::TreeModel::Row row = *(list->append());
  row[tilestyle_columns.image] = tilestyle->getImage()->to_pixbuf();
  row[tilestyle_columns.name] = "0x" + 
    TileStyle::idToString(tilestyle->getId());
  row[tilestyle_columns.style] = tilestyle;
}

void TileStyleOrganizerDialog::fill_category(guint32 type)
{
  Gtk::Label *label;
  guint32 count = d_tile->countTileStyles(TileStyle::Type(type));
  Glib::ustring items = String::ucompose(_("(%1 items)"), count);
  Glib::ustring markup;
  Glib::RefPtr<Gtk::ListStore> list;
  if (type == TileStyle::UNKNOWN)
    {
      label = unsorted_label;
      Glib::ustring unsorted = _("Unsorted TileStyles");
      markup = "<b>" + unsorted + "</b> " + items;
      list = unsorted_list;
    }
  else
    {
      label = category_label;
      markup = "<b>" + TileStyle::getTypeName(TileStyle::Type(type)) + 
                        " TileStyles</b> " + items;
      list = category_list;
    }
  label->set_markup(markup);
  list->clear();
  std::list<TileStyle*> styles = d_tile->getTileStyles(TileStyle::Type(type));
  for (std::list<TileStyle*>::iterator i = styles.begin(); i != styles.end();
       i++)
    add_tilestyle(list, *i);
}

void TileStyleOrganizerDialog::on_category_selected()
{
  int type = get_selected_category();
  if (type != -1)
    fill_category(TileStyle::Type(type));
  else
    empty_category();
}

void TileStyleOrganizerDialog::on_categories_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, const Gtk::SelectionData& selection_data, guint c, guint time)
{
  const int length = selection_data.get_length();
  if (length >= 0 && selection_data.get_format() == 8)
    {
      std::string idstr;
      std::istringstream ids(selection_data.get_data_as_string());
      while (1)
        {
          idstr = "";
          ids >> idstr;
          if (idstr.empty() == true)
            break;
          char *end = NULL;
          unsigned long int id = 0;
          id = strtoul (idstr.c_str(), &end, 0);
          //which category?
          int nx = 0, ny = 0;
          categories_iconview->convert_widget_to_bin_window_coords(x, y, nx, ny);
          const Gtk::TreeModel::Path &path = 
            categories_iconview->get_path_at_pos(nx, ny);
          Gtk::TreeModel::iterator iter = categories_list->get_iter(path);
          Gtk::TreeModel::Row row = *iter;
          TileStyle *style = d_tile->getTileStyle(id);
          if (style)
            {
              guint32 type = row[categories_columns.type];
              style->setType(TileStyle::Type(type));
              if (get_selected_category() != -1)
                fill_category(get_selected_category());
              fill_category(TileStyle::UNKNOWN);
            }
        }
    }

  context->drag_finish (false, false, time);
}

void TileStyleOrganizerDialog::on_category_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int a, int b, const Gtk::SelectionData& selection_data, guint c, guint time)
{
  const int length = selection_data.get_length();
  if (length >= 0 && selection_data.get_format() == 8)
    {
      std::string idstr;
      std::istringstream ids(selection_data.get_data_as_string());
      while (1)
        {
          idstr = "";
          ids >> idstr;
          if (idstr.empty() == true)
            break;
          char *end = NULL;
          unsigned long int id = 0;
          id = strtoul (idstr.c_str(), &end, 0);
          TileStyle *style = d_tile->getTileStyle(id);
          if (style)
            {
              int type = get_selected_category();
              if (type != -1)
                {
                  style->setType(TileStyle::Type(type));
                  fill_category(type);
                  fill_category(TileStyle::UNKNOWN);
                }
            }
        }
    }
  context->drag_finish (false, false, time);
}

void TileStyleOrganizerDialog::on_unsorted_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int a, int b, const Gtk::SelectionData& selection_data, guint c, guint time)
{
  const int length = selection_data.get_length();
  if (length >= 0 && selection_data.get_format() == 8)
    {
      std::string idstr;
      std::istringstream ids(selection_data.get_data_as_string());
      while (1)
        {
          idstr = "";
          ids >> idstr;
          if (idstr.empty() == true)
            break;
          char *end = NULL;
          unsigned long int id = 0;
          id = strtoul (idstr.c_str(), &end, 0);
          TileStyle *style = d_tile->getTileStyle(id);
          if (style)
            {
              style->setType(TileStyle::UNKNOWN);
              fill_category(TileStyle::UNKNOWN);
              int type = get_selected_category();
              if (type != -1)
                fill_category(TileStyle::Type(type));
            }
        }
    }
  context->drag_finish (false, false, time);
}
    
void TileStyleOrganizerDialog::on_category_tilestyle_activated(const Gtk::TreeModel::Path &path)
{
  Gtk::TreeModel::iterator iter = category_list->get_iter(path);
  Gtk::TreeModel::Row row = *iter;
  TileStyle *style = row[tilestyle_columns.style];
  tilestyle_selected.emit(style->getId());
}

void TileStyleOrganizerDialog::on_unsorted_tilestyle_activated(const Gtk::TreeModel::Path &path)
{
  Gtk::TreeModel::iterator iter = unsorted_list->get_iter(path);
  Gtk::TreeModel::Row row = *iter;
  TileStyle *style = row[tilestyle_columns.style];
  tilestyle_selected.emit(style->getId());
}

/**
 * This is how we're getting multiple drag to work.
 * 1. we remember the last multiple selection. (last_multiple_selection)
 * 2. we remember the time we made it. (time_of_last_selection)
 * 3. when we begin a drag, the icon we are dragging gets selected, thereby
 * nullifying the multiple selection.  it's a good thing we already have it
 * remembered!  so we check to see if that happened really recently, and if it
 * did, we select what we had selected before the drag started.
 * 4. we take special care to forget the multiple selections later on.
 * 5. we deselect the other iconview when we make a selection, and then take
 * special care not to let a zero selection mess up our state.
 */
void TileStyleOrganizerDialog::on_drag_begin(const Glib::RefPtr<Gdk::DragContext> &context, Gtk::IconView *iconview)
{
  Glib::TimeVal now;
  now.assign_current_time();
  now.subtract(time_of_last_selection);
  double secs = now.as_double();
  if (secs < 0.5)
    {
      inhibit_select = true;
      for (unsigned int i = 0; i < last_multiple_selection.size(); i++)
        iconview->select_path(last_multiple_selection[i]);
      inhibit_select = false;
    }
  last_multiple_selection.clear();
}

void TileStyleOrganizerDialog::on_selection_made(Gtk::IconView *iconview)
{
  if (inhibit_select)
    return;
  if (iconview->get_selected_items().size() == 0)
    return;
  if (iconview->get_selected_items().size() > 0)
    time_of_last_selection.assign_current_time();
  if (iconview->get_selected_items().size() > 1)
    {
      selection_timeout_handler.disconnect();
      last_multiple_selection = iconview->get_selected_items();
    }
  else
    {
      selection_timeout_handler.disconnect();
      selection_timeout_handler = Timing::instance().register_timer (sigc::mem_fun(*this, &TileStyleOrganizerDialog::expire_selection), 1000);
    }
  if (iconview == unsorted_iconview)
    category_iconview->unselect_all();
  else
    unsorted_iconview->unselect_all();
}

bool TileStyleOrganizerDialog::expire_selection()
{
  last_multiple_selection.clear();
  return Timing::STOP;
}
