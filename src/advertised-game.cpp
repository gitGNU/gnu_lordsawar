//  Copyright (C) 2011 Ben Asselstine
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

//#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "advertised-game.h"
#include "xmlhelper.h"
#include "profile.h"


std::string AdvertisedGame::d_tag_name = "advertisedgame";

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

AdvertisedGame::AdvertisedGame(GameScenario *scen, Profile *p)
	:RecentlyPlayedNetworkedGame(scen, p)
{
  d_creation_date.assign_current_time();
  d_profile = new Profile(*p);
}
	
	
AdvertisedGame::AdvertisedGame(const RecentlyPlayedNetworkedGame &orig, Profile *p)
        :RecentlyPlayedNetworkedGame(orig)
{
  d_creation_date.assign_current_time();
  d_profile = new Profile(*p);
}

AdvertisedGame::AdvertisedGame(const AdvertisedGame &orig)
        :RecentlyPlayedNetworkedGame(orig), 
        d_creation_date(orig.d_creation_date)
{
  d_profile = new Profile(*orig.d_profile);
}

AdvertisedGame::AdvertisedGame(XML_Helper *helper)
	:RecentlyPlayedNetworkedGame(helper)
{
  std::string s;
  helper->getData(s, "created_on");
  d_creation_date.assign_from_iso8601(s);
  helper->registerTag(Profile::d_tag, 
		      sigc::mem_fun(*this, &AdvertisedGame::loadProfile));
}

AdvertisedGame::~AdvertisedGame()
{
  delete d_profile;
}

bool AdvertisedGame::doSave(XML_Helper *helper) const
{
  bool retval = true;
  retval &= dynamic_cast<const RecentlyPlayedNetworkedGame*>(this)->doSave(helper);
  std::string s = d_creation_date.as_iso8601();
  retval &= helper->saveData("created_on", s);
  return retval;
}

bool AdvertisedGame::saveEntry(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag(d_tag_name);
  retval &= saveContents(helper);
  retval &= d_profile->save(helper);
  retval &= helper->closeTag();
  return retval;
}

bool AdvertisedGame::loadProfile(std::string tag, XML_Helper *helper)
{
  if (tag == Profile::d_tag)
    {
      d_profile = new Profile(helper);
      return true;
    }
  return false;
}
