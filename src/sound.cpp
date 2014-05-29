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
#include "sound.h"
#include "File.h"
#include "Configuration.h"
#include "defs.h"
#include "xmlhelper.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)


Sound* Sound::s_instance = 0;

#ifdef FL_SOUND
// used for fading in a music piece after another faded out
Mix_Music* _my_toplay = 0;

// used for removing a music piece (we only need to have 1 removed at a time
Mix_Music* _my_toremove = 0;

// how often the music piece is played
int loopcount=1;

void _startNext()
{
    if (_my_toplay != 0)
    {
        Mix_FadeInMusic(_my_toplay, loopcount, 1500);
        _my_toplay = 0;
    }

    if (_my_toremove != 0)
    {
        Mix_FreeMusic(_my_toremove);
        _my_toremove = 0;
    }

    Sound::getInstance()->nextPiece();
}
#endif

Sound* Sound::getInstance()
{
    if (s_instance == 0)
        s_instance = new Sound();

    return s_instance;
}

void Sound::deleteInstance()
{
    if (s_instance == 0)
        return;

    delete s_instance;
    s_instance = 0;
}

Sound::Sound()
    :d_broken(false), d_background(false), d_block(false)
{
    debug("Sound constructor")
#ifdef FL_SOUND
    d_music = 0;

    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
    {
        std::cerr << "Couldn't initialize SDL-mixer\n";
        std::cerr <<Mix_GetError() <<std::endl;
        d_broken = true;
        return;
    }
    
    XML_Helper helper(File::getMusicFile("music.xml"), std::ios::in, false);
    helper.registerTag("piece", sigc::mem_fun(this, &Sound::loadMusic));

    if (!helper.parse())
    {
        std::cerr<< _("Error loading music descriptions; disabling music.") << std::endl;
        d_broken = true;
        Mix_CloseAudio();
	return;
    }

    Mix_HookMusicFinished(_startNext);
#endif
    debug("Music list contains " <<d_musicMap.size <<" entries.")
    debug("background list has " <<d_bgMap.size <<" entries.")
}

Sound::~Sound()
{
    debug("Sound destructor")
#ifdef FL_SOUND
    haltMusic(false);
    disableBackground(false);
    
    // remove all music pieces
    std::map<Glib::ustring, MusicItem*>::iterator it;
    for (it = d_musicMap.begin(); it != d_musicMap.end(); it++)
        delete (*it).second;
            
    if (d_music)
      {
        Mix_FreeMusic(d_music);
	d_music = NULL;
      }
    
    //if (!d_broken)
        //Mix_CloseAudio();
#endif
}

bool Sound::setMusic(bool enable, int volume)
{
    if (volume < 0 || volume > 128)
        return false;

    Configuration::s_musicenable = enable;
    Configuration::s_musicvolume = volume;
#ifdef FL_SOUND
    Mix_VolumeMusic(volume);
#endif

    return true;
}

bool Sound::isMusicEnabled()
{
    return Configuration::s_musicenable;
}

int Sound::getMusicVolume()
{
    return Configuration::s_musicvolume;
}

bool Sound::playMusic(Glib::ustring piece, int nloops, bool fade)
{
    debug("playing Music")
#ifdef FL_SOUND
    if (d_broken || !Configuration::s_musicenable)
        return true;

    // first, load the music piece
    if (d_musicMap[piece] == 0)
        return false;

    if (Mix_PlayingMusic() || Mix_FadingMusic())
        haltMusic(false);

    Mix_Music* music = Mix_LoadMUS(File::getMusicFile(d_musicMap[piece]->file).c_str());
    if (!music)
    {
        //try to load an alternative
        Glib::ustring alias = d_musicMap[piece]->alias;
        music = Mix_LoadMUS(File::getMusicFile(d_musicMap[alias]->file).c_str());
        if (!music)
            return false;
    }

    // now start to block playing music (well, a mutex/synchronization 
    // would propably be a good idea here)
    d_block = true;

    // then end other music pieces and/or start ours.
    if (Mix_PlayingMusic() || Mix_FadingMusic())
    {
        _my_toremove = d_music;
        _my_toplay = music;
        d_music = music;
        loopcount = nloops;

        if (fade)
            Mix_FadeOutMusic(1500);
        else
            Mix_HaltMusic();
        return true;
    }

    // no other music playing => start ours.
    Mix_FadeInMusic(music, nloops, 1500);
#endif
    return true;
}

bool Sound::haltMusic(bool fade)
{
    debug("stopping music")
#ifdef FL_SOUND
    d_block = false;

    if (Mix_PlayingMusic() || Mix_FadingMusic())
    {
        _my_toremove = d_music;
        d_music = 0;

        if (fade)
            Mix_FadeOutMusic(1500);
        else
            Mix_HaltMusic();
        return true;
    }

    // no music is playing => call nextPiece manually
    nextPiece();
#endif
    return true;
}

void Sound::enableBackground()
{
    debug("enabling background music")
    d_background = true;
    nextPiece();
}

void Sound::disableBackground(bool fade)
{
    debug("disabling background music")
    d_background = false;

    if (d_block)
        return;

#ifdef FL_SOUND
    if (Mix_PlayingMusic() || Mix_FadingMusic())
    {
        _my_toremove = d_music;
        d_music = 0;

        if (fade)
            Mix_FadeOutMusic(1500);
        else
            Mix_HaltMusic();
    }
#endif
}

void Sound::nextPiece()
{
    debug("Sound::nextPiece")
    if (d_block || !d_background || !isMusicEnabled())
        return;

#ifdef FL_SOUND
    if (Mix_PlayingMusic() || Mix_FadingMusic())
        return;

    // remove the old music
    if (d_music)
      {
        Mix_FreeMusic(d_music);
	d_music = NULL;
      }

    // select a random music piece from the list of background pieces
    while (!d_music && !d_bgMap.empty())
    {
        int select = rand() % d_bgMap.size();
        d_music = Mix_LoadMUS(File::getMusicFile(d_musicMap[d_bgMap[select]]->file).c_str());
        if (!d_music)
        {
            // file does not exist. Remove this entry from the list
            d_bgMap[select] = d_bgMap[0];
            d_bgMap.erase(d_bgMap.begin());
        }
    }

    if (!d_music)
        // well, no background music without sound files...
        return;

    Mix_FadeInMusic(d_music, 1, 1500);
#endif
}

bool Sound::loadMusic(Glib::ustring tag, XML_Helper* helper)
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
