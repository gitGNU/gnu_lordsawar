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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


//This file contains the various macros used within lordsawar.

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <libintl.h>

#define LORDSAWAR_SAVEGAME_VERSION "0.3.8c"
#define LORDSAWAR_CONFIG_VERSION "0.3.7b"
#define ENABLE_NLS 1 // Very important to activate localization
#define _(string) gettext(string) // Macro for the gettext
#define __(astring) std::string(gettext(astring.c_str()))


//-----------------------------------------------------------------------------
//some standard timers. They can easier be changed here than somewhere deep
//within the code, and sometimes you have to tweak them a little bit.
const unsigned int TIMER_BIGMAP_SELECTOR=500;
const unsigned int TIMER_BIGMAP_SCROLLING=30;   //milliseconds
const unsigned int TIMER_SMALLMAP_REFRESH=50;
const unsigned int CITY_LEVELS=4; 
const unsigned int MAX_PLAYERS=8;
const unsigned int TEMPLE_TYPES=2; 
const unsigned int STONE_TYPES=9; 
const unsigned int ROAD_TYPES=12; 
const unsigned int SIGNPOSTS_RATIO=6; // #:1 dynamic vs static signposts
const unsigned int MAX_ARMIES_VECTORED_TO_ONE_CITY = 4;
const unsigned int MAX_TURNS_FOR_VECTORING = 2;

// from www.boost.org - derivation from this class makes the derived class
// noncopyable
class noncopyable
{
protected:
  noncopyable() {}
  ~noncopyable() {}
private:
  noncopyable(const noncopyable&);
  const noncopyable& operator=(const noncopyable&);
};


#endif // DEFINITIONS_H
