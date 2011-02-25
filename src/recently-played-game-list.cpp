// Copyright (C) 2008, 2011 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "recently-played-game-list.h"
#include "recently-played-game.h"
#include <limits.h>
#include <fstream>
#include <iostream>
#include "xmlhelper.h"
#include "Configuration.h"
#include <sigc++/functors/mem_fun.h>
#include "defs.h"
#include "profile.h"
#include "profilelist.h"

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

std::string RecentlyPlayedGameList::d_tag = "recentlyplayedgamelist";

RecentlyPlayedGameList* RecentlyPlayedGameList::s_instance = 0;

RecentlyPlayedGameList* RecentlyPlayedGameList::getInstance()
{
  if (s_instance == 0)
    s_instance = new RecentlyPlayedGameList();

  return s_instance;
}

bool RecentlyPlayedGameList::saveToFile(std::string filename) const
{
  bool retval = true;
  XML_Helper helper(filename, std::ios::out, false);
  retval &= save(&helper);
  helper.close();
  return retval;
}

bool RecentlyPlayedGameList::loadFromFile(std::string filename)
{
  std::ifstream in(filename.c_str());
  if (in)
    {
      XML_Helper helper(filename.c_str(), std::ios::in, false);
      helper.registerTag(RecentlyPlayedGame::d_tag, sigc::mem_fun(this, &RecentlyPlayedGameList::load_tag));
      bool retval = helper.parse();
      if (retval == false)
	unlink(filename.c_str());
      return retval;
    }
  return true;
}

RecentlyPlayedGameList* RecentlyPlayedGameList::getInstance(XML_Helper* helper)
{
  if (s_instance)
    deleteInstance();

  s_instance = new RecentlyPlayedGameList(helper);
  return s_instance;
}

void RecentlyPlayedGameList::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = 0;
}

RecentlyPlayedGameList::RecentlyPlayedGameList()
{
}

RecentlyPlayedGameList::RecentlyPlayedGameList(XML_Helper* helper)
{
  helper->registerTag(RecentlyPlayedGame::d_tag, sigc::mem_fun(this, &RecentlyPlayedGameList::load_tag));
}

RecentlyPlayedGameList::~RecentlyPlayedGameList()
{
  for (RecentlyPlayedGameList::iterator it = begin(); it != end(); it++)
    delete *it;
}

bool RecentlyPlayedGameList::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->begin(LORDSAWAR_RECENTLY_PLAYED_VERSION);
  retval &= helper->openTag(RecentlyPlayedGameList::d_tag);

  for (const_iterator it = begin(); it != end(); it++)
    (*it)->save(helper);

  retval &= helper->closeTag();

  return retval;
}

bool RecentlyPlayedGameList::load_tag(std::string tag, XML_Helper* helper)
{
  if (helper->getVersion() != LORDSAWAR_RECENTLY_PLAYED_VERSION)
    {
      return false;
    }
  if (tag == RecentlyPlayedGame::d_tag)
    {
      RecentlyPlayedGame *g = RecentlyPlayedGame::handle_load(helper);
      push_back(g);
      return true;
    }
  return false;
}

void RecentlyPlayedGameList::addNetworkedEntry(GameScenario *game_scenario, Profile *p, std::string host, guint32 port)
{
  if (Configuration::s_remember_recent_games == false)
    return;
  RecentlyPlayedNetworkedGame *g = NULL;
  switch (GameScenario::PlayMode(game_scenario->getPlayMode()))
    {
      case GameScenario::NETWORKED:
	g = new RecentlyPlayedNetworkedGame(game_scenario, p);
	g->fillData(host, port);
	break;
      default:
	break;
    }
  if (g)
    push_back(g);
  sort(orderByTime);
}

void RecentlyPlayedGameList::addEntry(GameScenario *game_scenario, Profile *p, 
                                      std::string filename)
{
  if (Configuration::s_remember_recent_games == false)
    return;
  switch (GameScenario::PlayMode(game_scenario->getPlayMode()))
    {
      case GameScenario::HOTSEAT:
	  {
	    RecentlyPlayedHotseatGame *g = NULL;
	    g = new RecentlyPlayedHotseatGame(game_scenario, p);
	    g->fillData(filename);
	    push_back(g);
	    break;
	  }
      case GameScenario::PLAY_BY_MAIL:
	  {
	    RecentlyPlayedPbmGame *g = NULL;
	    g = new RecentlyPlayedPbmGame(game_scenario, p);
	    g->fillData(filename);
	    push_back(g);
	    break;
	  }
      default:
	break;
    }
}

bool RecentlyPlayedGameList::orderByTime(RecentlyPlayedGame*rhs, RecentlyPlayedGame *lhs)
{
  if (rhs->getTimeOfLastPlay() > lhs->getTimeOfLastPlay())
    return true;
  else
    return false;
}

void RecentlyPlayedGameList::pruneGames()
{
  sort(orderByTime);
  pruneGamesBelongingToRemovedProfiles();
  pruneSameNamedAndSameHostGames();
  pruneOldGames(TWO_WEEKS_OLD);
  pruneTooManyGames(10);
}

void RecentlyPlayedGameList::pruneGamesBelongingToRemovedProfiles()
{
  for (RecentlyPlayedGameList::iterator it = begin(); it != end();)
    {
      Profile *p = 
        Profilelist::getInstance()->findProfileById((*it)->getProfileId());
      if (!p)
        {
          delete *it;
          it = erase (it);
        }
    }
}

void RecentlyPlayedGameList::pruneTooManyGames(int too_many)
{
  int count = 0;
  for (RecentlyPlayedGameList::iterator it = begin(); it != end();)
    {
      count++;
      if (count > too_many)
	{
	  delete *it;
	  it = erase (it);
	  continue;
	}
      it++;
    }
}

void RecentlyPlayedGameList::pruneSameNamedAndSameHostGames()
{
  for (RecentlyPlayedGameList::iterator it = begin(); it != end();)
    {
      int count = 0;
      for (RecentlyPlayedGameList::iterator rit = begin(); rit != end(); rit++)
        {
          if ((*it)->getPlayMode() == GameScenario::NETWORKED &&
              (*rit)->getPlayMode() == GameScenario::NETWORKED)
            {
              RecentlyPlayedNetworkedGame *i = 
                dynamic_cast<RecentlyPlayedNetworkedGame*>((*it));
              RecentlyPlayedNetworkedGame *j = 
                dynamic_cast<RecentlyPlayedNetworkedGame*>((*rit));
              if (j->getHost() == i->getHost() &&
                  j->getPort() == i->getPort() && count > 0)
                {
                  count++;
                  break;
                }
              else if (j->getHost() == i->getHost() &&
                       j->getPort() == i->getPort() && count == 0)
                count++;
            }
        }
      if (count > 1)
        {
          delete *it;
          it = erase (it);
          continue;
        }
      it++;
    }
}

void RecentlyPlayedGameList::pruneOldGames(int stale)
{
  time_t now = time(NULL);
  for (RecentlyPlayedGameList::iterator it = begin(); it != end();)
    {
      if ((*it)->getTimeOfLastPlay() + stale < now)
	{
	  delete *it;
	  it = erase (it);
	  continue;
	}
      it++;
    }
}

bool RecentlyPlayedGameList::removeEntry(std::string id)
{
  bool found = false;
  for (RecentlyPlayedGameList::iterator it = begin(); it != end();)
    {
      if ((*it)->getId() == id)
	{
	  delete *it;
	  it = erase (it);
	  found = true;
	  continue;
	}
      it++;
    }
  return found;
}

void RecentlyPlayedGameList::updateEntry(GameScenario *game_scenario)
{
  for (RecentlyPlayedGameList::iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == game_scenario->getId())
	{
	  (*it)->setTimeOfLastPlay(time(NULL));
	  (*it)->setRound(game_scenario->getRound());
	}
    }
}
	
void RecentlyPlayedGameList::removeAllNetworkedGames()
{
  for (RecentlyPlayedGameList::iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getPlayMode() == GameScenario::NETWORKED)
	{
	  erase (it);
	  delete *it;
	  it = begin();
	  continue;
	}
    }
}

bool RecentlyPlayedGameList::load()
{
  return loadFromFile(File::getSavePath() + "/" + RECENTLY_PLAYED_LIST);
}

bool RecentlyPlayedGameList::save() const
{
  return saveToFile(File::getSavePath() + "/" + RECENTLY_PLAYED_LIST);
}
// End of file
