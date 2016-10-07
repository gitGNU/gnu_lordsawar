//  Copyright (C) 2008, 2009, 2014 Ben Asselstine
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

#include "main-preferences-dialog.h"

#include "Configuration.h"
#include "snd.h"


MainPreferencesDialog::MainPreferencesDialog(Gtk::Window &parent)
 : LwDialog(parent, "main-preferences-dialog.ui")
{
  xml->get_widget("show_turn_popup_checkbutton", show_turn_popup_checkbutton);
  xml->get_widget("commentator_checkbutton", commentator_checkbutton);
  xml->get_widget("ui_combobox", ui_combobox);
  ui_combobox->signal_changed().connect
    (sigc::mem_fun(this, &MainPreferencesDialog::on_ui_form_factor_changed));

  xml->get_widget("play_music_checkbutton", play_music_checkbutton);
  xml->get_widget("music_volume_scale", music_volume_scale);
  xml->get_widget("music_volume_hbox", music_volume_hbox);
  show_turn_popup_checkbutton->signal_toggled().connect
    (sigc::mem_fun(this, &MainPreferencesDialog::on_show_turn_popup_toggled));
  commentator_checkbutton->signal_toggled().connect
    (sigc::mem_fun(this, &MainPreferencesDialog::on_show_commentator_toggled));
  play_music_checkbutton->signal_toggled().connect
    (sigc::mem_fun(this, &MainPreferencesDialog::on_play_music_toggled));
  music_volume_scale->signal_value_changed().connect
    (sigc::mem_fun(this, &MainPreferencesDialog::on_music_volume_changed));

  show_turn_popup_checkbutton->set_active(Configuration::s_showNextPlayer);
  commentator_checkbutton->set_active(Configuration::s_displayCommentator);
  play_music_checkbutton->set_active(Configuration::s_musicenable);
  music_volume_hbox->set_sensitive(Configuration::s_musicenable);
  music_volume_scale->set_value(Configuration::s_musicvolume * 100.0 / 128);
  ui_combobox->set_active(Configuration::s_ui_form_factor);
}

void MainPreferencesDialog::hide()
{
  dialog->hide();
}

void MainPreferencesDialog::run()
{
  dialog->show();
  dialog->run();

  Configuration::saveConfigurationFile(Configuration::configuration_file_path);
  dialog->hide();
}

void MainPreferencesDialog::on_show_turn_popup_toggled()
{
  Configuration::s_showNextPlayer = show_turn_popup_checkbutton->get_active();
}

void MainPreferencesDialog::on_play_music_toggled()
{
  Configuration::s_musicenable = play_music_checkbutton->get_active();

  if (play_music_checkbutton->get_active())
    Snd::getInstance()->play("intro", -1, false);
  else
    Snd::getInstance()->halt(false);
  music_volume_hbox->set_sensitive(Configuration::s_musicenable);
}

void MainPreferencesDialog::on_music_volume_changed()
{
  int volume = int(music_volume_scale->get_value() / 100 * 128);

  Configuration::s_musicvolume = volume;
  Snd::getInstance()->updateVolume();
}

void MainPreferencesDialog::on_show_commentator_toggled()
{
  Configuration::s_displayCommentator = commentator_checkbutton->get_active();
}

void MainPreferencesDialog::on_ui_form_factor_changed()
{
  Configuration::s_ui_form_factor = 
    Configuration::UiFormFactor (ui_combobox->get_active_row_number());
}
