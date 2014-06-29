// Copyright (C) 2006 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2014 Ben Asselstine
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

#include <glibmm.h>
#include <iostream>
#include <sigc++/functors/mem_fun.h>
#include <gstreamermm/playbin2.h>
#include <gstreamermm/fakesink.h>
#include <gstreamermm/bus.h>
#include <gstreamermm/message.h>
#include <gstreamermm/elementfactory.h>
#include "snd.h"
#include "File.h"
#include "Configuration.h"
#include "defs.h"
#include "xmlhelper.h"
#include "timing.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)


Snd* Snd::s_instance = 0;

Snd* Snd::getInstance()
{
    if (s_instance == 0)
        s_instance = new Snd();

    return s_instance;
}

void Snd::deleteInstance()
{
    if (s_instance == 0)
        return;

    delete s_instance;
    s_instance = 0;
}

Snd::Snd()
    :d_nloops(1), d_broken(false), d_background(false)
{
    debug("Snd constructor")

    XML_Helper helper(File::getMusicFile("music.xml"), std::ios::in);
    helper.registerTag("piece", sigc::mem_fun(this, &Snd::loadMusic));

    if (!helper.parseXML())
    {
        std::cerr<< _("Error loading music descriptions; disabling music.") << std::endl;
        d_broken = true;
	return;
    }

    back = Gst::PlayBin2::create();
    effect = Gst::PlayBin2::create();
    effect->get_bus()->add_watch(sigc::bind(sigc::mem_fun(*this, &Snd::on_bus_message), effect));
    back->get_bus()->add_watch(sigc::bind(sigc::mem_fun(*this, &Snd::on_bus_message), back));
    debug("Music list contains " <<d_musicMap.size <<" entries.")
    debug("background list has " <<d_bgMap.size <<" entries.")
}

Snd::~Snd()
{
    debug("Snd destructor")
    halt(false);
    disableBackground(false);
    
    // remove all music pieces
    std::map<Glib::ustring, MusicItem*>::iterator it;
    for (it = d_musicMap.begin(); it != d_musicMap.end(); it++)
        delete (*it).second;
}

bool Snd::setMusic(bool enable, int volume)
{
    if (volume < 0 || volume > 128)
        return false;

    Configuration::s_musicenable = enable;
    Configuration::s_musicvolume = volume;
    effect->property_volume() = (double)Configuration::s_musicvolume/128.0;
    back->property_volume() = (double)Configuration::s_musicvolume/128.0;

    return true;
}

bool Snd::isMusicEnabled()
{
    return Configuration::s_musicenable;
}

int Snd::getMusicVolume()
{
    return Configuration::s_musicvolume;
}

bool Snd::play(Glib::ustring piece, int nloops, bool fade)
{
  debug("playing Music")
    if (d_broken || !Configuration::s_musicenable)
      return true;

  // first, load the music piece
  if (d_musicMap[piece] == 0)
    return false;

  d_nloops = nloops;
  effect->set_state(Gst::STATE_NULL);
  effect->property_uri() = 
    Glib::filename_to_uri(File::getMusicFile(d_musicMap[piece]->file));
  effect->property_video_sink() = Gst::FakeSink::create();
  effect->property_audio_sink() = Gst::ElementFactory::create_element("autoaudiosink", "output");
  if (fade)
    {
      effect->property_volume() = 0.0;
      Timing::instance().register_timer
        (sigc::bind(sigc::mem_fun(this, &Snd::on_effect_fade), 0.01), 100);
    }
  else
    effect->property_volume() = (double)Configuration::s_musicvolume/128.0;
  effect->set_state(Gst::STATE_PLAYING);

  return true;
}

bool Snd::on_bus_message(const Glib::RefPtr<Gst::Bus> &bus, const Glib::RefPtr<Gst::Message> & msg, Glib::RefPtr<Gst::Element> playbin)
{
  switch (msg->get_message_type())
    {
    case Gst::MESSAGE_EOS:
      if (playbin == effect)
        {
          if (d_nloops > 0)
            d_nloops--;
          if (d_nloops == 0)
            return TRUE;
          playbin->seek(Gst::FORMAT_TIME, Gst::SEEK_FLAG_FLUSH, 0);
        }
      else if (playbin == back)
        {
          nextPiece();
        }
      break;
    default:
      break;
    }
  return true;
}
  
bool Snd::halt(bool fade)
{
  debug("stopping music")

  if (fade == false)
    effect->set_state(Gst::STATE_NULL);
  else
    Timing::instance().register_timer
      (sigc::bind(sigc::mem_fun(this, &Snd::on_effect_fade), -0.02), 100);
  return true;
}

bool Snd::on_effect_fade(double step)
{
  double volume = effect->property_volume();
  double max = (double)Configuration::s_musicvolume/128.0;
  if (step < 0)
    {
      if (volume > std::abs(step))
        effect->property_volume() = (volume + step);
      else
        effect->property_volume() = 0.0;
    }
  else if (step > 0)
    {
      if (volume < max-step)
        effect->property_volume() = (volume + step);
      else
        effect->property_volume() = max;
    }
  if (step < 0 && effect->property_volume() <= 0.0)
    {
      effect->property_volume() = 0.0;
      return Timing::STOP;
    }
  if (step > 0 && effect->property_volume() >= max)
    {
      effect->property_volume() = max;
      return Timing::STOP;
    }
  return Timing::CONTINUE;
}

void Snd::enableBackground()
{
    debug("enabling background music")
    d_background = true;
    nextPiece();
}

void Snd::disableBackground(bool fade)
{
    debug("disabling background music")
    d_background = false;

  back->set_state(Gst::STATE_NULL);
}

void Snd::nextPiece()
{
    debug("Snd::nextPiece")
    if (!d_background || !isMusicEnabled())
        return;

    // select a random music piece from the list of background pieces
    while (!d_bgMap.empty())
      {
        int i = rand() % d_bgMap.size();
        if (!File::exists(File::getMusicFile(d_musicMap[d_bgMap[i]]->file)))
            continue;
        back->set_state(Gst::STATE_NULL);
        back->property_uri() = 
          Glib::filename_to_uri(File::getMusicFile(d_musicMap[d_bgMap[i]]->file));
        back->property_video_sink() = Gst::FakeSink::create();
        back->property_audio_sink() = Gst::ElementFactory::create_element("autoaudiosink", "output");
        back->property_volume() = (double)Configuration::s_musicvolume/128.0;
        back->set_state(Gst::STATE_PLAYING);
        break;
      }
}

bool Snd::loadMusic(Glib::ustring tag, XML_Helper* helper)
{
    if (tag != "piece")
    {
        std::cerr <<"Loading music: Wrong tag name\n";
        return false;
    }

    Glib::ustring tagname;
    MusicItem* item = new MusicItem();
    
    bool retval = true;
    retval &= helper->getData(tagname, "name");
    retval &= helper->getData(item->file, "filename");
    retval &= helper->getData(item->background, "background");
    retval &= helper->getData(item->alias, "alias");
    
    if (retval)
    {
        d_musicMap[tagname] = item;
        if (item->background)
            d_bgMap.push_back(tagname);
    }
    
    return retval;
}
        
void Snd::updateVolume()
{
  effect->property_volume() = (double)Configuration::s_musicvolume/128.0;
  back->property_volume() = (double)Configuration::s_musicvolume/128.0;
}
