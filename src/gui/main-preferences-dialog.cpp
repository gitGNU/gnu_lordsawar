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

#include "glade-helpers.h"
#include "image-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "Configuration.h"
#include "sound.h"


MainPreferencesDialog::MainPreferencesDialog()
{
    Glib::RefPtr<Gtk::Builder> xml
	= Gtk::Builder::create_from_file(get_glade_path()
				    + "/main-preferences-dialog.ui");

    xml->get_widget("dialog", dialog);
    xml->get_widget("show_turn_popup_checkbutton", show_turn_popup_checkbutton);
    xml->get_widget("commentator_checkbutton", commentator_checkbutton);
    xml->get_widget("ui_combobox", ui_combobox);
    ui_combobox->signal_changed().connect(
	sigc::mem_fun(this, &MainPreferencesDialog::on_ui_form_factor_changed));

    xml->get_widget("play_music_checkbutton", play_music_checkbutton);
    xml->get_widget("music_volume_scale", music_volume_scale);
    xml->get_widget("music_volume_hbox", music_volume_hbox);
    show_turn_popup_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &MainPreferencesDialog::on_show_turn_popup_toggled));
    commentator_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &MainPreferencesDialog::on_show_commentator_toggled));
    play_music_checkbutton->signal_toggled().connect(
	sigc::mem_fun(this, &MainPreferencesDialog::on_play_music_toggled));
    music_volume_scale->signal_value_changed().connect(
	sigc::mem_fun(this, &MainPreferencesDialog::on_music_volume_changed));

    show_turn_popup_checkbutton->set_active(Configuration::s_showNextPlayer);
    commentator_checkbutton->set_active(Configuration::s_displayCommentator);
    play_music_checkbutton->set_active(Configuration::s_musicenable);
    music_volume_hbox->set_sensitive(Configuration::s_musicenable);
    music_volume_scale->set_value(Configuration::s_musicvolume * 100.0 / 128);
    ui_combobox->set_active(Configuration::s_ui_form_factor);
    
}

MainPreferencesDialog::~MainPreferencesDialog()
{
  delete dialog;
}
void MainPreferencesDialog::set_parent_window(Gtk::Window &parent)
{
    dialog->set_transient_for(parent);
    //dialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
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
    bool play_music = play_music_checkbutton->get_active();

    Configuration::s_musicenable = play_music;

    if (play_music)
    {
        Sound::getInstance()->enableBackground();
    }
    else
    {
        Sound::getInstance()->haltMusic();
        Sound::getInstance()->disableBackground();
    }
    music_volume_hbox->set_sensitive(Configuration::s_musicenable);
}

void MainPreferencesDialog::on_music_volume_changed()
{
    int volume = int(music_volume_scale->get_value() / 100 * 128);
    
#ifdef FL_SOUND
    Mix_VolumeMusic(volume);
#endif

    Configuration::s_musicvolume = volume;
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
