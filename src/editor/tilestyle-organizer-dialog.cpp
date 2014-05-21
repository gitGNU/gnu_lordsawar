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

#include "tilestyle-organizer-dialog.h"

#include "glade-helpers.h"
#include "ucompose.hpp"
#include "File.h"
#include "defs.h"
#include "Tile.h"
#include "GraphicsCache.h"


TileStyleOrganizerDialog::TileStyleOrganizerDialog(Tile *tile)
{
    d_tile = tile;
    std::vector<Gtk::TargetEntry> targets;
    targets.push_back(Gtk::TargetEntry("LordsawarTilestyleType", Gtk::TARGET_SAME_APP));
    
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/tilestyle-organizer-dialog.ui");

    xml->get_widget("dialog", dialog);
    dialog->set_icon_from_file(File::getMiscFile("various/castle_icon.png"));
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
    category_iconview->enable_model_drag_source(targets, Gdk::MODIFIER_MASK, Gdk::ACTION_MOVE);
    category_iconview->signal_drag_data_get().connect
      (sigc::mem_fun(*this, &TileStyleOrganizerDialog::on_category_drag_data_get));
    category_iconview->signal_drag_data_received().connect
      (sigc::mem_fun
       (*this, &TileStyleOrganizerDialog::on_category_drop_drag_data_received));
    category_iconview->signal_item_activated().connect
      (sigc::mem_fun(*this, &TileStyleOrganizerDialog::on_category_tilestyle_activated));
    unsorted_list = Gtk::ListStore::create (tilestyle_columns);
    unsorted_iconview->set_model(unsorted_list); 
    unsorted_iconview->set_pixbuf_column(tilestyle_columns.image);
    unsorted_iconview->set_text_column(tilestyle_columns.name);
    unsorted_iconview->enable_model_drag_dest(targets, Gdk::ACTION_MOVE);
    unsorted_iconview->enable_model_drag_source(targets, Gdk::MODIFIER_MASK, Gdk::ACTION_MOVE);
    unsorted_iconview->signal_drag_data_get().connect
      (sigc::mem_fun(*this, &TileStyleOrganizerDialog::on_unsorted_drag_data_get));
    unsorted_iconview->signal_drag_data_received().connect
      (sigc::mem_fun
       (*this, &TileStyleOrganizerDialog::on_unsorted_drop_drag_data_received));
    unsorted_iconview->signal_item_activated().connect
      (sigc::mem_fun(*this, &TileStyleOrganizerDialog::on_unsorted_tilestyle_activated));

    fill_category(TileStyle::UNKNOWN);
    categories_iconview->select_path(Gtk::TreeModel::Path("0"));
}
      
void TileStyleOrganizerDialog::on_category_drag_data_get(const Glib::RefPtr<Gdk::DragContext> &drag_context,
                                             Gtk::SelectionData &data,
                                             guint info, guint time)
{
  drag_context->get_source_window()->show();

  std::string s;
  TileStyle *style = get_selected_category_tilestyle();
  if (!style)
    return;
  s = String::ucompose("0x%1", TileStyle::idToString(style->getId()));
  data.set(data.get_target(), 8, (const guchar*)s.c_str(), strlen(s.c_str()));
}

void TileStyleOrganizerDialog::on_unsorted_drag_data_get(const Glib::RefPtr<Gdk::DragContext> &drag_context,
                                             Gtk::SelectionData &data,
                                             guint info, guint time)
{
  drag_context->get_source_window()->show();

  std::string s;
  TileStyle *style = get_selected_unsorted_tilestyle();
  if (!style)
    return;
  s = String::ucompose("0x%1", TileStyle::idToString(style->getId()));
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

TileStyle * TileStyleOrganizerDialog::get_selected_category_tilestyle ()
{
  typedef std::vector<Gtk::TreeModel::Path> type_list_paths;
  type_list_paths selected = category_iconview->get_selected_items();
  if (!selected.empty())
    {
      const Gtk::TreeModel::Path &path = *selected.begin();
      Gtk::TreeModel::iterator iter = category_list->get_iter(path);
      Gtk::TreeModel::Row row = *iter;
      return row[tilestyle_columns.style];
    }
  return NULL;
}

TileStyle * TileStyleOrganizerDialog::get_selected_unsorted_tilestyle ()
{
  typedef std::vector<Gtk::TreeModel::Path> type_list_paths;
  type_list_paths selected = unsorted_iconview->get_selected_items();
  if (!selected.empty())
    {
      const Gtk::TreeModel::Path &path = *selected.begin();
      Gtk::TreeModel::iterator iter = unsorted_list->get_iter(path);
      Gtk::TreeModel::Row row = *iter;
      return row[tilestyle_columns.style];
    }
  return NULL;
}

void TileStyleOrganizerDialog::fill_in_categories()
{
    categories_list->clear();
    for (guint32 i = TileStyle::LONE; i < TileStyle::UNKNOWN; i++)
      add_category (i);
}

void TileStyleOrganizerDialog::add_category(guint32 type)
{
  GraphicsCache *gc = GraphicsCache::getInstance();
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
  std::string items = String::ucompose(_("(%1 items)"), count);
  std::string markup;
  Glib::RefPtr<Gtk::ListStore> list;
  if (type == TileStyle::UNKNOWN)
    {
      label = unsorted_label;
      std::string unsorted = _("Unsorted TileStyles");
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

TileStyleOrganizerDialog::~TileStyleOrganizerDialog()
{
  delete dialog;
}
void TileStyleOrganizerDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
}

int TileStyleOrganizerDialog::run()
{
    dialog->show_all();
    int response = dialog->run();
    return response;
}

void TileStyleOrganizerDialog::hide()
{
  dialog->hide();
}
    
void TileStyleOrganizerDialog::on_categories_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, const Gtk::SelectionData& selection_data, guint c, guint time)
{
  const int length = selection_data.get_length();
  if (length >= 0 && selection_data.get_format() == 8)
    {
      char *end = NULL;
      unsigned long int id = 0;
      id = strtoul (selection_data.get_data_as_string().c_str(), &end, 0);
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

  context->drag_finish (false, false, time);
}

void TileStyleOrganizerDialog::on_category_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int a, int b, const Gtk::SelectionData& selection_data, guint c, guint time)
{
  const int length = selection_data.get_length();
  if (length >= 0 && selection_data.get_format() == 8)
    {
      char *end = NULL;
      unsigned long int id = 0;
      id = strtoul (selection_data.get_data_as_string().c_str(), &end, 0);
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
  context->drag_finish (false, false, time);
}

void TileStyleOrganizerDialog::on_unsorted_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext> &context, int a, int b, const Gtk::SelectionData& selection_data, guint c, guint time)
{
  const int length = selection_data.get_length();
  if (length >= 0 && selection_data.get_format() == 8)
    {
      char *end = NULL;
      unsigned long int id = 0;
      id = strtoul (selection_data.get_data_as_string().c_str(), &end, 0);
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
