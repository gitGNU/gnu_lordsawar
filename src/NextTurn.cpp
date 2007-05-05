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

#include "NextTurn.h"

#include "playerlist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "stacklist.h"
#include "armysetlist.h"
#include "hero.h"
#include "File.h"

#include "path.h"

using namespace std;
#define debug(x) {cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<endl;}
//#define debug(x)

int
NextTurn::loadHeroTemplates()
{
  FILE *fileptr = fopen (File::getMiscFile("heronames").c_str(), "r");
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int retval;
  int gender;
  int side;
  size_t bytesread;
  char *tmp;
  const Armysetlist* al = Armysetlist::getInstance();
  Uint32 heroset = al->getHeroId();
  const Army* herotype;


  if (fileptr == NULL)
    return -1;
  while ((read = getline (&line, &len, fileptr)) != -1)
    {
      retval = sscanf (line, "%d%d%n", &side, &gender, &bytesread);
      if (retval != 2)
        {
          free (line);
          return -2;
        }
      while (isspace(line[bytesread]) && line[bytesread] != '\0')
        bytesread++;
      tmp = strchr (&line[bytesread], '\n');
      if (tmp)
        tmp[0] = '\0';
      if (strlen (&line[bytesread]) == 0)
        {
          free (line);
          return -3;
        }
      if (side < 0 || side > (int) MAX_PLAYERS)
        {
          free (line);
          return -4;
        }
      herotype = al->getArmy(heroset, rand() % al->getSize (heroset));
      Hero *newhero = new Hero (*herotype, "", NULL);
      if (gender)
        newhero->setGender(Hero::MALE);
      else
        newhero->setGender(Hero::FEMALE);
      newhero->setName (&line[bytesread]);
      d_herotemplates[side].push_back (newhero);
    }
  if (line)
    free (line);
  fclose (fileptr);
  return 0;
}

NextTurn::NextTurn(bool turnmode)
    :d_turnmode(turnmode), d_stop(false)
{
  int err;
  err = loadHeroTemplates();
}

NextTurn::~NextTurn()
{
}

void NextTurn::start()
{
    //We need the playerlist a lot, so maintain a copy of it.
    Playerlist* plist = Playerlist::getInstance();

    //set first player as active if no active player exists
    if (!plist->getActiveplayer())
        plist->nextPlayer();

    while (!d_stop)
    {
        // do various start-up tasks
        startTurn();
        supdating.emit(true);
       
        // inform everyone about the next turn 
        snextTurn.emit(plist->getActiveplayer());
        
	bool break_loop = splayerStart.emit(plist->getActiveplayer());

        //Let the player do his duties...
        plist->getActiveplayer()->startTurn();

        if (break_loop)
            return;
	
        //Now do some cleanup at the end of the turn.
        finishTurn();

        //...and initiate the next one.
        plist->nextPlayer();
        
        //if it is the first player's turn now, a new round has started
        if (plist->getActiveplayer() == plist->getFirstLiving())
        {
            finishRound();
            snextRound.emit();
        }
    }
}

void NextTurn::endTurn()
{
    // Finish off the player and transfers the control to the start function
    // again.
    finishTurn();
    Playerlist::getInstance()->nextPlayer();

    if (Playerlist::getActiveplayer() == Playerlist::getInstance()->getFirstLiving())
    {
        finishRound();
        snextRound.emit();
    }

    start();
}

void NextTurn::startTurn()
{
    //this function is called before a player starts his turn. Some
    //items you could imagine to be placed here: healing/building
    //units, check for joining heroes...

    //a shortcut
    Player* p = Playerlist::getActiveplayer();

    //if turnmode is set, create/heal armies at player's turn
    if (d_turnmode)
    {
        //build armies
        Citylist::getInstance()->nextTurn(p);

        //heal stacks
        p->getStacklist()->nextTurn();
    }

    //recruit heroes
    //a hero costs a random number of gold pieces
    int gold_needed = rand() % 1000;
    
    //we set the chance of some hero recruitment to, ehm, 10 percent
    if (((rand() % 10) == 0) && (gold_needed < p->getGold())
        && (p != Playerlist::getInstance()->getNeutral()))
    {
        int num = rand() % d_herotemplates[p->getId()].size();
        Hero *templateHero = d_herotemplates[p->getId()][num];
        Hero* newhero = new Hero(*templateHero);

        bool accepted = p->recruitHero(newhero, gold_needed);
        
        if (accepted)
        {
            // FIXME: persistent (immortal) players can exist and take heroes
            // without owning any city => segfault
            p->withdrawGold(gold_needed);
            Citylist::getInstance()->getFirstCity(p)->addHero(newhero);
        }
        else
            delete newhero;
    }
}

void NextTurn::finishTurn()
{
    //Put everything that has to be done before the next player starts
    //his turn here. E.g. one could clear some caches.
}

void NextTurn::finishRound()
{
    //Put everything that has to be done when a new round starts in here.
    //E.g. increase the round number in GameScenario. (this is done with
    //the snextRound signal, but useful for an example).
    
    if (!d_turnmode)
    {
        //do this for all players at once
        Playerlist::iterator pit = Playerlist::getInstance()->begin();
        for (; pit != Playerlist::getInstance()->end(); pit++)
        {
            if ((*pit)->isDead())
                continue;

            //produce armies
            Citylist::getInstance()->nextTurn(*pit);

            //heal armies
            (*pit)->getStacklist()->nextTurn();
        }
    }

    // heal the stacks in the ruins
    Ruinlist* rl = Ruinlist::getInstance();
    for (Ruinlist::iterator it = rl->begin(); it != rl->end(); it++)
    {
        Stack* keeper = (*it).getOccupant();
        if (keeper)
            keeper->nextTurn();
    }
}
