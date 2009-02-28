//  Copyright (C) 2007, 2008, 2009 Ben Asselstine
//  Copyright (C) 2008 Ole Laursen
//
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include "herotemplates.h"

#include "File.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "hero.h"
#include "heroproto.h"
#include <sstream>

HeroTemplates* HeroTemplates::d_instance = 0;

HeroTemplates* HeroTemplates::getInstance()
{
    if (!d_instance)
        d_instance = new HeroTemplates();

    return d_instance;
}

void HeroTemplates::deleteInstance()
{
    if (d_instance != 0)
        delete d_instance;

    d_instance = 0;
}


HeroTemplates::HeroTemplates()
{
  loadHeroTemplates();
}

HeroTemplates::~HeroTemplates()
{
  for (unsigned int i = 0; i < MAX_PLAYERS; ++i)
    for (std::vector<HeroProto *>::iterator j = d_herotemplates[i].begin();
           j != d_herotemplates[i].end(); ++j)
      delete *j;
}

HeroProto *HeroTemplates::getRandomHero(int player_id)
{
  int num = rand() % d_herotemplates[player_id].size();
  return d_herotemplates[player_id][num];
}

int HeroTemplates::loadHeroTemplates()
{
  const Armysetlist* al = Armysetlist::getInstance();
  const ArmyProto* herotype;

  // list all the army types that are heroes.
  std::vector<const ArmyProto*> heroes;
  Player *p = Playerlist::getInstance()->getNeutral();
  for (unsigned int j = 0; j < al->getSize(p->getArmyset()); j++)
    {
      const ArmyProto *a = al->getArmy (p->getArmyset(), j);
      if (a->isHero())
	heroes.push_back(a);
    }
  std::ifstream file(File::getMiscFile("heronames").c_str());
  if (file.good()) 
    {
      std::string buffer, name;
      int side, gender;

      while (std::getline(file, buffer)) 
	{
	  std::istringstream line(buffer);
	  if (!(line >> side >> gender >> name))
	    return -2;

	  if (side < 0 || side > (int) MAX_PLAYERS)
	    return -4;

	  herotype = heroes[rand() % heroes.size()];
	  HeroProto *newhero = new HeroProto (*herotype);
	  if (gender)
	    newhero->setGender(Hero::MALE);
	  else
	    newhero->setGender(Hero::FEMALE);

	  newhero->setName (name);
	  d_herotemplates[side].push_back (newhero);
	}
    }
  else
    return -1;
  file.close();
  return 0;
}

