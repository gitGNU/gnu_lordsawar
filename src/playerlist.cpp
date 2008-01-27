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

#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "playerlist.h"

#include "citylist.h"
#include "defs.h"
#include "xmlhelper.h"
#include "history.h"
#include "citylist.h"
#include "stacklist.h"

using namespace std;

//#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<endl<<flush;}
#define debug(x)

Playerlist* Playerlist::s_instance = 0;
Player* Playerlist::d_activeplayer = 0;
bool Playerlist::s_finish = false;

Playerlist* Playerlist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Playerlist();

    return s_instance;
}

Playerlist* Playerlist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Playerlist(helper);

    return s_instance;
}

void Playerlist::deleteInstance()
{
    if (s_instance)
    {
        delete s_instance;
        s_instance = 0;
    }
}

Playerlist::Playerlist()
    :d_neutral(0)
{
    s_finish = false;
    d_activeplayer = 0;
}

Playerlist::Playerlist(XML_Helper* helper)
    :d_neutral(0)
{
    d_activeplayer=0;
    //load data. This consists currently of two values: size (for checking
    //the size; we don't use it yet) and active(which player is active)
    //we do it by calling load with playerlist as string
    load("playerlist", helper);
    s_finish = false;

    helper->registerTag("player", sigc::mem_fun(this, &Playerlist::load));
}

Playerlist::~Playerlist()
{
    iterator it = begin();
    while (!empty())
        it = flErase(it);
}

bool Playerlist::checkPlayers()
{
    bool last = false;
    bool dead = false;
    debug("checkPlayers()");
    iterator it = begin ();
	  
    while (it != end ())
    {
        debug("checkPlayers() iter");
        //ignore the neutral player as well as dead and immortal ones
        if ((*it) == d_neutral || (*it)->isDead() || (*it)->isImmortal())
        {
            debug("checkPlayers() dead?");
            it++;
            continue;
        }

        if (!Citylist::getInstance()->countCities((*it)))
        {
            debug("checkPlayers() city?");
            iterator nextit = it;
            nextit++;

            (*it)->kill();
	    if (getNoOfPlayers() == 1)
	      last = true;
            splayerDead.emit(*it);
            dead = true;
	    if (last)
	      break;

            it = nextit;    // do this at the end to catch abuse of invalid it
        } else {
            debug("checkPlayers() inc");
            ++it;
        }
    }
  return dead;
}

void Playerlist::nextPlayer()
{
    debug("nextPlayer()");

    iterator it;

    if (!d_activeplayer)
    {
        it = begin();
    }
    else
    {
        for (it = begin(); it != end(); ++it)
        {
            if ((*it) == d_activeplayer)
            {
                it++;
                break;
            }
        }
    }

    // in case we have got a dead player, continue iterating. This breaks
    // if we have ONLY dead players which we assume never happens.
    while ((it == end()) || ((*it)->isDead()))
    {
        if (it == end())
        {
            it = begin();
            continue;
        }
        it++;
    }

    d_activeplayer = (*it);
    debug("got player: " <<d_activeplayer->getName())
}

Player* Playerlist::getPlayer(string name) const
{
    debug("getPlayer()");
    for (const_iterator it = begin(); it != end(); ++it)
    {
        if ((*it)->getName() == name) return (*it);
    }
    return 0;
}

Player* Playerlist::getPlayer(Uint32 id) const
{
    debug("getPlayer()");
    for (const_iterator it = begin(); it != end(); it++)
    {
        if (id == (*it)->getId())
            return (*it);
    }

    return 0;
}

int Playerlist::getNoOfPlayers() const
{
    int number = 0;

    for (const_iterator it = begin(); it != end(); it++)
    {
        if (((*it) != d_neutral) && !(*it)->isDead())
            number++;
    }

    return number;
}

Player* Playerlist::getFirstLiving() const
{
    for (const_iterator it = begin(); ; it++)
        if (!(*it)->isDead())
            return (*it);
}

bool Playerlist::save(XML_Helper* helper) const
{
    //to prevent segfaults
    if (!d_activeplayer)
        d_activeplayer = (*begin());

    bool retval = true;

    retval &= helper->openTag("playerlist");
    retval &= helper->saveData("active", d_activeplayer->getId());
    retval &= helper->saveData("neutral", d_neutral->getId());

    for (const_iterator it = begin(); it != end(); it++)
        retval &= (*it)->save(helper);

    retval &= helper->closeTag();

    return retval;
}

bool Playerlist::load(string tag, XML_Helper* helper)
{
    static Uint32 active = 0;
    static Uint32 neutral = 0;

    if (tag == "playerlist") //only called in the constructor
    {
        helper->getData(active, "active");
        helper->getData(neutral, "neutral");
        return true;
    }

    if (tag != "player")
        return false;

    //else tag == "player"
    Player* p = Player::loadPlayer(helper);
    if(p == 0)
        return false;

    //insert player...
    push_back(p);

    //set neutral and active
    if (p->getId() == neutral)
        d_neutral = p;
    if (p->getId() == active)
        d_activeplayer = p;

    return true;
}

Playerlist::iterator Playerlist::flErase(Playerlist::iterator it)
{
    if ((*it) == d_neutral)
        d_neutral = 0;

    delete (*it);
    return erase (it);
}

struct rankable_t
{
  Uint32 score;
  Player *player;
};

bool compareDiplomaticScores (const struct rankable_t lhs,
			      const struct rankable_t rhs)
{
  /* make ties prefer normal player order */
  if (lhs.score == rhs.score) 
    return lhs.player->getId() > rhs.player->getId();
  else
    return lhs.score < rhs.score;
}
void Playerlist::calculateDiplomaticRankings()
{
  unsigned int i = 0;
  char* titles[MAX_PLAYERS] = 
    {
      "Statesman",
      "Diplomat",
      "Pragmatist",
      "Politician",
      "Deciever",
      "Scoundrel",
      "Turncoat",
      "Running Dog",
    };

  //determine the rank for each player
  //add up the scores for all living players, and sort
  std::list<struct rankable_t> rankables;
  for (iterator pit = begin (); pit != end (); pit++)
    {
      if ((*pit) == d_neutral)
	continue;
      if ((*pit)->isDead () == true)
	continue;
      struct rankable_t rankable;
      rankable.score = 0;
      for (iterator it = begin (); it != end (); it++)
	{
	  if ((*it) == d_neutral)
	    continue;
	  if ((*it)->isDead () == true)
	    continue;
	  if (*pit == *it)
	    continue;
	  rankable.score += (*pit)->getDiplomaticScore(*it);
	}
      rankable.player = *pit;
      rankables.push_back(rankable);
    }
  rankables.sort (compareDiplomaticScores);
  std::reverse (rankables.begin (), rankables.end ());

  i = 1;
  for (std::list<struct rankable_t>::iterator rit = rankables.begin (); 
       rit != rankables.end (); rit++)
    {
      (*rit).player->setDiplomaticRank(i);
      i++;
    }

  // given the rankings, what are the titles?
  // the titles are depleted from the middle as players die.
  // 7 means deplete first, 0 means deplete last.
  unsigned int deplete[MAX_PLAYERS] = { 0, 2, 4, 6, 7, 5, 3, 1 };

  unsigned int numAlive = countPlayersAlive ();
  // okay, we take the first numAlive titles

  std::vector<std::string> available_titles;
  for (i = 0; i < numAlive; i++)
    {
      for (unsigned int j = 0; j < MAX_PLAYERS; j++)
	{
	  if (deplete[j] == i)
	    available_titles.push_back (std::string(titles[deplete[j]]));
	}
    }

  for (const_iterator it = begin (); it != end (); it++)
    {
      if ((*it) == d_neutral)
	continue;
      if ((*it)->isDead () == true)
	continue;
      Uint32 rank = (*it)->getDiplomaticRank();
      // recall that the first rank is 1, and not 0.
      (*it)->setDiplomaticTitle (available_titles[rank - 1]);
    }

  return;
}

void Playerlist::calculateWinners()
{
    Uint32 score;
    Uint32 total_gold = 0;
    Uint32 total_armies = 0;
    Uint32 total_cities = 0;
    for (const_iterator it = begin(); it != end(); it++)
      {
	if ((*it) == d_neutral)
	  continue;
	if ((*it)->isDead() == true)
	  continue;
	total_gold += (*it)->getGold();
	total_armies += (*it)->getStacklist()->countArmies();
      }
    total_cities = Citylist::getInstance()->size();

    for (const_iterator it = begin(); it != end(); it++)
      {
	if ((*it) == d_neutral)
	  continue;
	if ((*it)->isDead() == true)
	  continue;

	Citylist *clist = Citylist::getInstance();
	float city_component = (float)
	  ((float) clist->countCities(*it)/ (float)total_cities) * 70.0;
	float gold_component = (float)
	  ((float) (*it)->getGold() / (float)total_gold) * 10.0;
	float army_component = (float)
	  ((float) (*it)->getStacklist()->countArmies() / 
	   (float)total_armies) * 20.0;
	score = (Uint32) (city_component + gold_component + army_component);
	History_Score *item = new History_Score();
	item->fillData(score);
	(*it)->getHistorylist()->push_back(item);

	History_GoldTotal* gold = new History_GoldTotal();
	gold->fillData((*it)->getGold());
	(*it)->getHistorylist()->push_back(gold);
      }

    return;
}

Uint32 Playerlist::countHumanPlayersAlive()
{
  Uint32 retval = 0;
  for (const_iterator it = begin(); it != end(); it++)
    if ((*it)->isDead() == false && (*it)->getType() == Player::HUMAN)
      retval++;
  return retval;
}

Uint32 Playerlist::countPlayersAlive ()
{
  Uint32 numAlive = 0; 

  for (const_iterator it = begin (); it != end (); it++)
    {
      if ((*it) == d_neutral)
	continue;
      if ((*it)->isDead () == true)
	continue;
      numAlive++;
    }
  return numAlive;
}

void Playerlist::negotiateDiplomacy()
{
  // hold diplomatic talks, and determine diplomatic outcomes
  for (iterator pit = begin(); pit != end(); pit++)
    {
      if ((*pit)->isDead())
	continue;

      if ((*pit) == getNeutral())
	continue;
  
      for (iterator it = begin(); it != end(); it++)
	{
      
	  if ((*it)->isDead())
	    continue;

	  if ((*it) == getNeutral())
	    continue;

	  if ((*it) == (*pit))
	    break;
  
	  Player::DiplomaticState old_state = (*pit)->getDiplomaticState(*it);
	  Player::DiplomaticState new_state = (*pit)->negotiateDiplomacy(*it);
	  (*pit)->declareDiplomacy (new_state, (*it));
	  (*pit)->proposeDiplomacy (Player::NO_PROPOSAL, (*it));
	  (*it)->declareDiplomacy (new_state, (*pit));
	  (*it)->proposeDiplomacy (Player::NO_PROPOSAL, (*pit));
	  if (old_state != new_state)
	    {
	      Player *me = *pit;
	      Player *them = *it;
	      if (new_state == Player::AT_PEACE)
		{
		  //their view of me goes up
		  them->improveDiplomaticRelationship(me, 1);
		  //their allies think better of me
		  me->improveAlliesRelationship (them, 1, Player::AT_PEACE);
		  //their enemies think less of me
		  them->deteriorateAlliesRelationship (me, 1, Player::AT_WAR);

		  History_DiplomacyPeace *item = new History_DiplomacyPeace();
		  item->fillData(*it);
		  (*pit)->getHistorylist()->push_back(item);
		}
	      else if (new_state == Player::AT_WAR)
		{
		  //their view of me goes down
		  them->deteriorateDiplomaticRelationship(me, 1);
		  //their allies view of me goes down
		  them->deteriorateAlliesRelationship(me, 1, Player::AT_PEACE);
		  //their enemies view of me goes up
		  me->improveAlliesRelationship(them, 1, Player::AT_WAR);

		  History_DiplomacyWar *item = new History_DiplomacyWar();
		  item->fillData(them);
		  me->getHistorylist()->push_back(item);
		}
	    }
	}
    }
}
